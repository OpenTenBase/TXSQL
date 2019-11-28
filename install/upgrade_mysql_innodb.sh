#!/bin/bash
# update_conf.sh
# author: daviezhao
# date:2016-03-15

PATH=/data/home/boss/bin:/usr/local/bin:/usr/bin:/usr/X11R6/bin:/bin:/usr/games:/opt/gnome/bin:/usr/lib/mit/bin:/usr/lib/mit/sbin:/usr/sbin/.
export PATH

argtips="$0 port-number [new-data-disk [new-log-disk]]"

if [ $# -lt 1 ] || [ $# -gt 3 ]
then
    echo $argtips;
    exit 1;
fi

mysql_port=$1

if [ $# -ge 2 ]
then
    data_disk=$2

    if [$# -eq 3 ]
    then
        log_disk=$3
    fi
fi

base_dir=`pwd | awk -F"/install" '{print $1}'`
conf_dir="${base_dir}/etc"

cnf_file="${conf_dir}/my_${mysql_port}.cnf"
add_init_file="${conf_dir}/add_my_${mysql_port}.ini"
from_install_file="${conf_dir}/frominstall_my_${mysql_port}.ini"


if [ ! -d ${conf_dir} ] || [ ! -f ${cnf_file} ] || [ ! -f ${add_init_file} ]; then
    echo "No config dir/file or .ini file to work on."
    exit 1
fi
# install or upgrade percona xtrabackup and percona tookit.
./install_percona_xtrabackup.sh

user=`grep "^user *=" ${cnf_file} | awk -F"=" '{print $2}' `

# use old config file var values.
if [ $data_disk"e" == "e" ]; 
then
    data_disk=`grep "^#rootdatadir *=" ${cnf_file} | awk -F"=" '{print $2}' `

    if [ $log_disk"e" == "e" ];
    then
        log_disk=`grep "^#rootlogdir *=" ${cnf_file} | awk -F"=" '{print $2}'`
    fi
fi

innodb_bufpool_size=`grep "^innodb_buffer_pool_size *=" ${cnf_file} | awk -F"=" '{print $2}' `

#this changed in new template.cn since r030d002, so we must keep the old one for an old instance.
innodb_log_group_dir_setting=`grep "^ *innodb_log_group_home_dir *=.*" ${cnf_file}` 

echo $innodb_log_group_dir_setting;

port=`grep "^port *=" ${cnf_file} | awk -F"=" '{print $2}' | head -n1 `
export modifyconf="1" # let install_mysql_innodb.sh modify conf only.

# call this script to use new template file to generate the .cnf config file
# again, but use old one's argument values with which it was generated, except
# that you can specify a different data-disk and log-disk if you want.
# The .ini file isn't modified at all, and no need to append its arguments here.
# The 'mode' argument isn't relevant, it's
# only used to generate a new config file, not for modify an old one, so simply
# specify 'default', and you can't modify the mode of a config file.
echo "configs: $user,$port,$innodb_bufpool_size,$data_disk,$log_disk"

#we don't want to modify the *.ini files so back up and restore them.
cp ${add_init_file} ${add_init_file}.bakkk
cp ${from_install_file}  ${from_install_file}.bakkk
./install_mysql_innodb.sh default $user $port $innodb_bufpool_size $data_disk $log_disk
mv -f ${from_install_file}.bakkk  ${from_install_file}
mv -f ${add_init_file}.bakkk ${add_init_file}

# modify paths of existing config file since base-dir changed.
sed -e "s|^ *plugin-dir *=.*|plugin-dir=${base_dir}/lib/mysql/plugin\n|g" -e "s|^ *language *=.*|lc-messages-dir=${base_dir}/share\n|g" -e "s|^ *!include *.*add_my.*|!include ${base_dir}/etc/add_my_${mysql_port}.ini\n|g"  -e "s|^ *!include *.*frominstall_my.*|!include ${base_dir}/etc/frominstall_my_${mysql_port}.ini\n|g"  -e "s|innodb_log_group_home_dir *=.*|${innodb_log_group_dir_setting}|g"   ${cnf_file} > ${cnf_file}.tmp
mv -f ${cnf_file}.tmp ${cnf_file}

#if so file changed, copy new one to .run_so. so far only libjemalloc needs this.
if [ -e ${base_dir}/share_lib -a -e ${base_dir}/.run_so ] ; then
  md5_old_jemalloc=`md5sum  ${base_dir}/.run_so/libjemalloc* |awk -F" " '{print $1}'`
  md5_new_jemalloc=`md5sum  ${base_dir}/share_lib/libjemalloc* |awk -F" " '{print $1}'`
  if [ ${md5_old_jemalloc} != ${md5_new_jemalloc} ] ; then
	rm ${base_dir}/.run_so/libjemalloc*   # if it's still in use, the file will be deleted when not used.
    cp  ${base_dir}/share_lib/libjemalloc*  ${base_dir}/.run_so # the new one will be used when mysqld restarts.
    echo "libjemalloc so updated."
  fi
fi
