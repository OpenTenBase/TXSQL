#!/bin/bash
#install_mysql_innodb.sh
#author:hectorgang
#date:2013-07-09
#exe example: ./install_mysql_innodb.sh test 3307 4G  mysql-5.5.32-linux2.6-x86_64.tar.gz master /data /data/log 1 1 2G

#./install_mysql_innodb.sh tdengine 4001 2G . master /data /data/log 1  1 1G

PATH=/data/home/boss/bin:/usr/local/bin:/usr/bin:/usr/X11R6/bin:/bin:/usr/games:/opt/gnome/bin:/usr/lib/mit/bin:/usr/lib/mit/sbin:/usr/sbin/.
export PATH

if [ $# -ne 1 ] && [ $# -ne 2 ];then
   echo "$0  port isRepidRemove[optional]"
   exit
fi


mysql_port=$1

base_dir=`pwd | awk -F"/install" '{print $1}'`
mysql_dir=${base_dir}
etcfile=${base_dir}/etc/my_${mysql_port}.cnf

if [ ! -f  $etcfile ];then
        echo "have not find etc file:$etcfile,can't not uninstall\n"
        exit -1
fi

user=`grep "user=" ${etcfile} | tail -n1 | awk -F"=" '{print $2}' `
data_disk=`grep "rootdatadir=" ${etcfile} | tail -n1 | awk -F"=" '{print $2}' `
log_disk=`grep "rootlogdir=" ${etcfile} | tail -n1 | awk -F"=" '{print $2}' `

echo $user $data_disk $log_disk

#exit 0

dbdata_raw_dir=${data_disk}/${mysql_port}/dbdata_raw
data_dir="${data_disk}/${mysql_port}/dbdata_raw/data" 
prod_dir="${data_disk}/${mysql_port}/prod"
log_dir="${log_disk}/${mysql_port}/dblogs"
log_arch=${data_disk}/${mysql_port}/logs
log_bin="${log_dir}/bin"
log_relay="${log_dir}/relay"
log_tmp="${log_dir}/tmp"




#stop password
#./tool/stopmysql ${mysql_port}

ps -ef | grep my_${mysql_port}.cnf | grep "bin/mysqld" | grep -- "--defaults-file="| grep -v grep | awk '{print $2}'  | xargs kill -9


if [ $# > 1 ];then
   repidRemove=$2
fi



if [ $repidRemove"e" == "e" ];then

   echo "./srmnew -c10 ${dbdata_raw_dir}"
   su $user -c "./srmnew -c10 ${dbdata_raw_dir}"

   echo "./srmnew -c4 -rf ${log_dir}"
   su $user -c "./srmnew -c4 ${log_dir}"

   echo "./srmnew -c4 ${log_arch}"
   su $user -c "./srmnew -c4 ${log_arch}"

   echo "./srmnew -c10 ${data_disk}/${mysql_port}"
   ./srmnew -c10 ${data_disk}/${mysql_port}

   echo "./srmnew -c4 ${log_disk}/${mysql_port}"
   ./srmnew -c4 ${log_disk}/${mysql_port}

else

   echo "rm -rf ${dbdata_raw_dir}"
   su $user -c "rm -rf ${dbdata_raw_dir}"

   echo "rm -rf ${log_dir}"
   su $user -c "rm -rf ${log_dir}"

   echo "rm -rf ${log_arch}"
   su $user -c "rm -rf  ${log_arch}"

   echo "rm -rf ${data_disk}/${mysql_port}"
   rm -rf ${data_disk}/${mysql_port}

   echo "rm -rf ${log_disk}/${mysql_port}"
   rm -rf ${log_disk}/${mysql_port}

fi
