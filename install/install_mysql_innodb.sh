#!/bin/bash
#install_mysql_innodb.sh
#author:hectorgang,harlylei
#date:2013-07-09

#export net=eth1



PATH=/data/home/boss/bin:/usr/local/bin:/usr/bin:/usr/X11R6/bin:/bin:/usr/games:/opt/gnome/bin:/usr/lib/mit/bin:/usr/lib/mit/sbin:/usr/sbin/.
export PATH

source ./comm.sh


argtips="$0 mode[webank/default] username port innodb_buffer_size data_disk log_disk [mysql_variables like character-set-server=utf8&collation-server=utf8_general_ci&lower_case_table_names=1&skip-name-resolve=]"

if [ $# -lt 6 ];then
   #echo "$0 username port innodb_buffer_size mysql_file master_or_slave data_disk log_disk if_mysql_exist if_force_install tokudb_cache_size"
#echo "$0 mode[webank/default] username port innodb_buffer_size data_disk log_disk "
   echo $argtips
   exit 1
fi

#sed -e 's/#tokudb_checkpointing_period=60/#tokudb_checkpointing_period=60\nwait_timeout=172800\ninteractive_timeout=172800/' template.cnf 

mode=$1
user=$2
mysql_port=$3
change_innodb_buffer_pool_size_before_use=$4
data_disk=$5
log_disk=$6

#remove trailing / otherwise mysqld can't find relay log paths from the
#relaylog.index file.
data_disk=`echo $data_disk | sed 's|/*$||g'`
log_disk=`echo $log_disk | sed 's|/*$||g'`


#Calculate pool instances size , only for Percna-MySQL
buffer_pool_MB=`echo $change_innodb_buffer_pool_size_before_use | grep -oP '[0-9]*'`
#buffer_pool_GB=`expr $buffer_pool_MB / 1024`
buffer_pool_GB=`echo "scale=1; $buffer_pool_MB/1024" | bc`
#change_innodb_buffer_pool_instances_before_use=`expr ${buffer_pool_GB} \* 2`
change_innodb_buffer_pool_instances_before_use=`echo "${buffer_pool_GB}*2" | bc`
change_innodb_buffer_pool_instances_before_use=`echo ${change_innodb_buffer_pool_instances_before_use} | awk -F. '{print $1}'`
if [ ${change_innodb_buffer_pool_instances_before_use} -lt 2 ];then
   change_innodb_buffer_pool_instances_before_use=2
fi
if [ ${change_innodb_buffer_pool_instances_before_use} -gt  64 ];then
   change_innodb_buffer_pool_instances_before_use=64
fi


variables=""
if [ $# -ge 7 ];then
    variables=$7
fi


if_force_install=0 #force is 0,can not cover

base_dir=`pwd | awk -F"/install" '{print $1}'`
conf_dir="${base_dir}/etc"

if [ ! -d ${conf_dir} ];then
    mkdir ${conf_dir} 
fi
chown $user:users ${conf_dir} -R
base_file="${base_dir}/install/template.cnf"
cnf_file="${conf_dir}/my_${mysql_port}.cnf"
add_init_file="${conf_dir}/add_my_${mysql_port}.ini"

if [ $mode != "webank" -a $mode != "default" ];then
    echo "$mode is not valid"
    echo $argtips
    exit 1
fi


#echo $net,`env net`
if [ $net"e" == "e" ];then
    net=eth1
fi
echo "net:"$net
echo "modifyconf":${modifyconf}

installXtrabackup
installPerconaToolkit

cp ${base_dir}/bin/mysqlbinlog /usr/bin -f
cp ${base_dir}/bin/mysqldump /usr/bin -f

mysql_extra_port=$(($mysql_port+10000))
echo "mysql_extra_port: " $mysql_extra_port

change_key_buffer_before_use=256M
log="/tmp/install_mysql_${mysql_port}.log"
touch $log
chown $user:users $log

port_cnt=`netstat -tnpl | awk '{print $4}' | awk -F ':' '{print $2}'  | grep -w "${mysql_port}" | wc -l`
if [ ${modifyconf}"e" != "1""e" ];then
    if [ ${port_cnt} -eq 0 ];then
        echo "[`date +'%Y-%m-%d %H:%M:%S'`] info | port '${mysql_port}' does not be used" | tee -a ${log}
    else
        echo "[`date +'%Y-%m-%d %H:%M:%S'`] error | port '${mysql_port}' has been used, exit" | tee -a ${log}
        exit 1
    fi
fi

#update conf,read last cnf 
if [ ${modifyconf}"e" == "1""e" ];then
    change_ip_before_use=`cat "${cnf_file}" | grep -E -- "^bind_address    =" | awk -F"=" '{print $2}'|sed -e 's/ //g' | head -n1`
    echo "read last ip:${change_ip_before_use}"
    #server_id
    change_server_id_before_use=`cat "${cnf_file}" | grep -E -- "^server-id=" | awk -F"=" '{print $2}'|sed -e 's/ //g' | head -n1`
    echo "read last server_id:${change_server_id_before_use}"
fi

if [ $change_ip_before_use"e" == "e" ]; then
    change_ip_before_use=`/sbin/ifconfig $net | grep "inet addr:" | cut -d":" -f2 | cut -d" " -f1`
    if [ $change_ip_before_use"e" == "e" ]; then
        change_ip_before_use=`/sbin/ifconfig $net | grep "inet " | awk '{print $2}' `
        if [ $change_ip_before_use"e" == "e" ]; then
             echo "net:$net,can't find local ip"
            exit 1
        fi  
    fi
fi


if [ "m_${change_server_id_before_use}" == "m_" ];then
    #change_server_id_before_use=$(($RANDOM % 100000))
    serverid_tmpfile="/tmp/server_id_${change_ip_before_use}_${mysql_port}"
    change_server_id_before_use=`cat ${serverid_tmpfile} 2>/dev/null`

    #if [ "m_${change_server_id_before_use}" == "m_" ];then
         subip=`echo $change_ip_before_use | awk -F"." '{print or(lshift($3,24),lshift($4,16))}'`
         change_server_id_before_use=$((${subip}|(($RANDOM % 256)<<8)|(${mysql_port}&0xff)))
         #echo ${serverid_tmpfile}
         echo $change_server_id_before_use >${serverid_tmpfile}
    #fi      
fi

echo "server_id:" $change_server_id_before_use

data_dir="${data_disk}/${mysql_port}/dbdata_raw/data" 
innodb_dir="${data_disk}/${mysql_port}/dbdata_raw/dbdata" 
tmp_dir="${data_disk}/${mysql_port}/dbdata_raw/tmpdir"
if [ ${modifyconf}"e" != "1""e" ];then
    if [ -d ${data_dir}/mysql ];then 
        if [ "m_${if_force_install}" == "m_1" ];then
            echo "[`date +'%Y-%m-%d %H:%M:%S'`] warn | dir '${data_dir}/mysql' has existed, but continue" |tee -a ${log}
            tmp_dir=/tmp/mysql_bak_`date +"%Y_%m_%d_%H_%M_%S"` 
            mkdir ${tmp_dir}
            mv -R ${data_dir}/mysql /tmp/${tmp_dir}
            test -d ${data_dir}/performance_schema && mv ${data_dir}/performance_schema ${tmp_dir}/
        else
            echo "[`date +'%Y-%m-%d %H:%M:%S'`] error | dir '${data_dir}/mysql' has existed, and exit" |tee -a ${log}
            exit 1
        fi
    fi
fi

aiomaxnr=1048576
grep "fs.aio-max-nr = ${aiomaxnr}" /etc/sysctl.conf
if [ $? -eq 0 ];then
    echo "[`date +'%Y-%m-%d %H:%M:%S'`] info get fs.aio-max-nr = ${aiomaxnr}" |tee -a ${log}
else
    echo "fs.aio-max-nr = ${aiomaxnr}" >>/etc/sysctl.conf
    /sbin/sysctl -p /etc/sysctl.conf
    echo "[`date +'%Y-%m-%d %H:%M:%S'`] info set fs.aio-max-nr = ${aiomaxnr}" |tee -a ${log}
fi

modifynofile /etc/security/limits.conf 400000 
modifynofile /etc/security/limits.d/80-nofile.conf 400000 

prod_dir="${data_disk}/${mysql_port}/prod"
log_dir="${log_disk}/${mysql_port}/dblogs"
log_arch="${data_disk}/${mysql_port}/logs/innodb"
log_bin="${log_dir}/bin"
log_relay="${log_dir}/relay"
log_tmp="${log_dir}/tmp"

mkdir -p ${log_arch} ${log_bin} ${log_relay} ${log_tmp}  ${data_dir}  ${prod_dir} ${innodb_dir} ${tmp_dir}

echo -e "${log_arch}\n${log_bin}\n${log_relay}\n${log_tmp}\n${data_dir}\n${prod_dir}" > ${conf_dir}/data_dir.txt
#base_file="${base_dir}/install/template.cnf"
#cnf_file="${conf_dir}/my_${mysql_port}.cnf"

echo "change_tokudb_cache_size_before_use:" ${change_tokudb_cache_size_before_use}


cat ${base_file}| sed "s|rootdatadir_arg|${data_disk}|g" | sed "s|rootlogdir_arg|${log_disk}|g" |sed "s|prod_dir|${prod_dir}|g" | sed "s|change_extra_port_before_use|${mysql_extra_port}|g" | sed "s|change_user_before_use|${user}|g" | sed "s|base_dir|${base_dir}|g" | sed "s|change_tokudb_cache_size_before_use|${change_tokudb_cache_size_before_use}|g"  |sed "s|data_dir|${data_dir}|g"|sed "s|log_bin_arg|${log_bin}|g"|sed "s|log_relay|${log_relay}|g"|sed "s|log_dir|${log_dir}|g"|sed "s|tmp_dir|${tmp_dir}|g"|sed "s|data_dir|${data_dir}|g"|sed "s|innodb_dir|${innodb_dir}|g"|sed "s|log_arch$|${log_arch}|g" |sed "s|log_tmp|${log_tmp}|g" > ${cnf_file}

cat "${cnf_file}"| sed -e "s/change_port_before_use/${mysql_port}/g"|sed -e "s/change_ip_before_use/$change_ip_before_use/g"|sed -e "s/change_server_id_before_use/$change_server_id_before_use/g"|sed -e "s/change_innodb_buffer_pool_size_before_use/$change_innodb_buffer_pool_size_before_use/g"| sed -e "s/change_innodb_buffer_pool_instances_before_use/$change_innodb_buffer_pool_instances_before_use/g" |sed -e "s/change_key_buffer_before_use/$change_key_buffer_before_use/g" > ${cnf_file}.tmp; mv ${cnf_file}.tmp  ${cnf_file} 

if [ $mode == "webank" ];then
    sed -e 's/userstat=1/userstat=0/g' -e 's/transaction_isolation=REPEATABLE-READ/transaction_isolation=READ-COMMITTED/g' -e 's/wait_timeout=28800/wait_timeout=172800/g' -e 's/interactive_timeout=28800/interactive_timeout=172800/g' -e 's/^ *sql_mode=.*/sql_mode=STRICT_TRANS_TABLE/g' -i ${cnf_file}
fi


chown -R $user:users ${data_disk}/${mysql_port}  ${log_disk}/${mysql_port}  
datestr=`date -d today +"%Y-%m-%d"`
timestr=`date -d today +"%H:%M:%S"`
echo "#${datestr} ${timestr} $*" >>${cnf_file}
chown $user:users   ${cnf_file}
if [ $? -eq 0 ];then
    echo "[`date +'%Y-%m-%d %H:%M:%S'`] info | chown -R $user:users ${log_arch} ${log_bin} ${log_relay} ${log_tmp}  ${data_dir}  ${prod_dir}" |tee -a ${log}
else
    echo "[`date +'%Y-%m-%d %H:%M:%S'`] fail | chown -R $user:users ${log_arch} ${log_bin} ${log_relay} ${log_tmp}  ${data_dir}  ${prod_dir}" |tee -a ${log}
fi

mysql_dir=${base_dir}
echo "mysql_dir:${mysql_dir}"


if [ ${variables}"e" != "e" ];then
    ./modifycnf --port=${mysql_port}  --type=overwrite  --variables="${variables}" --outputfile="frominstall_my_${mysql_port}.ini"
    if [ $? -ne 0 ];then
        echo "[`date +'%Y-%m-%d %H:%M:%S'`] error | modifycnf error, and exit" |tee -a ${log}
        exit 1
    fi
    ./modifycnf --port=${mysql_port}  --type=overwrite  --variables="${variables}"
    if [ $? -ne 0 ];then
        echo "[`date +'%Y-%m-%d %H:%M:%S'`] error | modifycnf error, and exit" |tee -a ${log}
        exit 1
    fi
else
    ./modifycnf --port=${mysql_port}  --type=overwrite   --outputfile="frominstall_my_${mysql_port}.ini"
    if [ $? -ne 0 ];then
        echo "[`date +'%Y-%m-%d %H:%M:%S'`] error | modifycnf error, and exit" |tee -a ${log}
        exit 1
    fi
    ./modifycnf --port=${mysql_port} --type=overwrite 
    if [ $? -ne 0 ];then
        echo "[`date +'%Y-%m-%d %H:%M:%S'`] error | modifycnf error, and exit" |tee -a ${log}
        exit 1
    fi
fi

chown $user:users   ../etc/frominstall_my_${mysql_port}.ini
chown $user:users   ${add_init_file}

if [ ${modifyconf}"e" != "1""e" ];then
    cnf_file_tmp=${cnf_file}.tmp
    sed -e 's/log-bin  = /#log-bin  = /' ${cnf_file} > ${cnf_file_tmp}
    sed -e 's/log-error = /#log-error = /' -i ${cnf_file_tmp} 
    su $user -c "cd ${mysql_dir};./bin/mysqld --defaults-file=${cnf_file_tmp} --initialize 2>&1 | tee -a ${log}"

    if [ $? -ne 0 ];then
        echo "[`date +'%Y-%m-%d %H:%M:%S'`] error | fail to init hosts tables, and exit" |tee -a ${log}
        exit 1
    fi
    init_mysql_pwd=`grep "A temporary password is generated" ${log} | tail -n1 | awk -F"root@localhost: " '{print $2}'`
    #echo "init_mysql_pwd:tail -n1 ${log}"
    if [ "m_"$init_mysql_pwd == "m_" ];then
        echo "[`date +'%Y-%m-%d %H:%M:%S'`] error | fail to get mysql init password, and exit" |tee -a ${log}
        exit 1
    fi

    # disable binlogging so no need to do semisync wait during bootstrapping.
    su $user -c "./bootmysql.sh $base_dir ${cnf_file_tmp} ${user}"

    install_succ=0
    for i in `seq 0 180`
    do
       #echo "----- ${mysql_dir}/bin/mysql"
       ${mysql_dir}/bin/mysql --connect-expired-password -uroot -p"$init_mysql_pwd" -S ${prod_dir}/mysql.sock -e "ALTER USER 'root'@'localhost' IDENTIFIED BY 'root';"

       if (( $? == 0 ));then
            #config_db ${m_s} "/usr/local/master_slave.cnf"
            #for replication
            ${mysql_dir}/bin/mysql -uroot -p'root' -S${prod_dir}/mysql.sock -e " ;flush privileges;"
            ${mysql_dir}/bin/mysql -uroot -p'root' -S${prod_dir}/mysql.sock -e "create user  'tdsqlsys_repl'@'%' identified  WITH mysql_native_password BY  '*EE92CB4BBCA8B11B210BE3544631158A39ACB0F7';grant replication slave,replication client on *.* to 'tdsqlsys_repl'@'%'; flush privileges;"

            #for kp
            ${mysql_dir}/bin/mysql -uroot -p'root' -S${prod_dir}/mysql.sock -e "create user  'tdsqlsys_kp_new'@'%' identified  WITH mysql_native_password BY  '*CA47A9E4FFBE9054CF11B70BF495C03BBAD6DEDD';grant all on *.* to 'tdsqlsys_kp_new'@'%' with grant option;revoke File,Shutdown,super on *.* from 'tdsqlsys_kp_new'@'%' ;flush privileges;"
            #for agent
            ${mysql_dir}/bin/mysql -uroot -p'root' -S${prod_dir}/mysql.sock -e "create user  'tdsqlsys_agent'@'localhost' identified  WITH mysql_native_password BY  '*52070E9E4F996BFF87753629DA3B26D4D83AAFD1' ; grant all on *.* to 'tdsqlsys_agent'@'localhost' with grant option;flush privileges;"
            #for security,remove test
            ${mysql_dir}/bin/mysql -uroot -p'root' -S${prod_dir}/mysql.sock -e "set sql_log_bin=0;delete from mysql.db where Db='test\_%' and Host='%' ;delete from mysql.db where Db='test' and Host='%';flush privileges;"
            #local load so
            #${mysql_dir}/bin/mysql -uroot -p'root' -S${prod_dir}/mysql.sock -e "set sql_log_bin=0; INSTALL PLUGIN iothreadreport SONAME 'iothreadreport.so'; create database test;create database SysDB;use SysDB;CREATE TABLE StatusTable  ( ip varchar(20), port int, ts timestamp, primary key(ip,port));"
            ${mysql_dir}/bin/mysql -uroot -p'root' -S${prod_dir}/mysql.sock -e "set sql_log_bin=0;  create database test;create database SysDB;use SysDB;CREATE TABLE StatusTable  ( ip varchar(20), port int, ts timestamp, primary key(ip,port));"
            #delete root,must put the last line 
            ${mysql_dir}/bin/mysql -uroot -p'root' -S${prod_dir}/mysql.sock -e "set sql_log_bin=0;delete from mysql.user where user='root';flush privileges;"

            install_succ=1
        
            break
       fi
       sleep 1
    done

    sed -e 's/#skip_name_resolve=on/skip_name_resolve=on/' -i  ${cnf_file}
    su $user -c "./restartmysql.sh ${mysql_port}"
    rm ${cnf_file_tmp}

    #exit status
    if (( ${install_succ} == 1 ));then
        echo "[`date +'%Y-%m-%d %H:%M:%S'`] installing finished:${mysql_port}!" |tee -a ${log}
        exit 0
    else
        echo "[`date +'%Y-%m-%d %H:%M:%S'`] installing fail:${mysql_port}!" |tee -a ${log}
        exit 1
    fi
    echo "[`date +'%Y-%m-%d %H:%M:%S'`] installing finished!" |tee -a ${log}
else
    echo "[`date +'%Y-%m-%d %H:%M:%S'`] modifyconf finished!" |tee -a ${log}
fi

sed -e 's/#skip_name_resolve=on/skip_name_resolve=on/' -i  ${cnf_file}

