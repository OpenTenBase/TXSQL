#!/bin/bash
#install_mysql_innodb.sh
#author:hectorgang,harlylei
#date:2013-07-09

#export net=eth1


PATH=/data/home/boss/bin:/usr/local/bin:/usr/bin:/usr/X11R6/bin:/bin:/usr/games:/opt/gnome/bin:/usr/lib/mit/bin:/usr/lib/mit/sbin:/usr/sbin/.
export PATH

#source ./comm.sh

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

if [ -d /data/tdsqlcmd ];then
   sudo /data/tdsqlcmd/base_cmd "./tdsql_addto_cgroup --cgrouptype="cpu,blkio" --port=$mysql_port --pid=$$ "
else
   sudo su -c "./tdsql_addto_cgroup --cgrouptype="cpu,blkio" --port=$mysql_port --pid=$$ "
fi

if [ $? -eq 0 ];then
     if [ -d /data/tdsqlcmd ];then
       sudo /data/tdsqlcmd/base_cmd "export change_ip_before_use=$change_ip_before_use;./install_mysql_innodb.sh $1 $2 $3 $4 $5 $6 \"$7\" "
     else
       sudo su -c "export change_ip_before_use=$change_ip_before_use;./install_mysql_innodb.sh $1 $2 $3 $4 $5 $6 \"$7\" "
     fi
     exit $? 
else
    echo "./tdsql_addto_cgroup fail,can't $0"
    exit -1
fi
