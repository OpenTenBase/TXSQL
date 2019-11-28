if [ $# -lt 1 ]
then
    echo "usage: ./post-install.sh <installation-dir-full-path>"
    exit 0;
fi

is_ci="0"

#cur=`pwd`

if [ $# -ge 2 ];then
    is_ci=$2
fi

echo $is_ci

mkdir -p $1

env_info="x86_64"
is_arm=`uname -a | grep aarch64  | head -n1 | wc -l `
#is_arm=0
echo "is_arm:$is_arm"
if [ $is_arm != "0" ];then
	env_info="arm"
fi

#cp ./percona_strip.sh $1
cp -r ./install $1
cp -rf mysql_install/mysql-server-8.0.18/* $1

mkdir $1/xtrabackup -p
mkdir -p $1/share_lib $1/.run_so $1/etc
#the xtrabackup dir must be at ../

if [ ! -d "$1/install/tool" ]; then
	mkdir -p $1/install/tool
fi

cp -r ./env_depend/${env_info}/xtrabackup/* $1/xtrabackup
cp ./env_depend/${env_info}/lib/libjemalloc.so.3.6.0  $1/share_lib
cp ./env_depend/${env_info}/mysqldeptool/*  $1/install
cp ./env_depend/${env_info}/mysqldeptool/tool/*  $1/install/tool
cp ./env_depend/${env_info}/mysqldeptool/gtid/*  $1/install

#cd install dir
cd $1

cd ./bin
#make sure the built binaries have exe modes.
ls |while read f ; do chmod 0755 $f ; done
cd ../install
ls |while read f ; do chmod -R 0755 $f ; done
#cp libjemalloc.so.3.6.0 ../.run_so

# these files must also exist in install/ dir, comm.sh need them.
#cp keyring_udf.so ../lib/mysql/plugin
cd ../xtrabackup
ls |while read f ; do chmod 0755 $f ; done

cd ..
mkdir log -p

if [ $is_ci != "0" ];then
    cp -r bin ../bin-symbol
    cp -r lib ../lib-symbol
    cp -r xtrabackup ../xtrabackup-symbol

    strip -gs bin/*
    strip -gs lib/*
    strip -gs install/*
    strip -gs xtrabackup/*

    rm -rf mysql-test

fi


rm include -rf
rm sql-bench -rf
rm lib/lib* -f
rm install/runtime.err -f

cur=`pwd`
cd bin;rm -rf aria_* innochecksum myisam* tokuft* wsrep_sst* msql2mysql  mysqlaccess   mysql_convert_table_format mysqld_multi mysql_find_rows mysql_fix_extensions mysqlhotcopy mysql_setpermission  mysql_waitpid mysql_zap  replace
cd $cur

#rm lib/plugin/ha_xtradb.so 
rm lib/plugin/feedback.so  -f
rm lib/plugin/ha_spider.so -f
rm lib/plugin/handlersocket.so -f
rm lib/plugin/ha_mroonga.so -f
