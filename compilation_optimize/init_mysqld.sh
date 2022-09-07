#!/bin/bash
mysqld="../bld-release/runtime_output_directory/mysqld"
conf=1.cnf
datadir=mysql_root/1

if [ ! -f ${conf} ];then
    echo "${conf} not exist, bye."
    exit 2
fi
chown -R mysql: ../profile-data

# create mysql_root
if [ ! -d ${datadir} ];then
    echo "${datadir} not exist, creating..."
    mkdir -p ${datadir}
    mkdir ${datadir}/data
    mkdir ${datadir}/log
    mkdir ${datadir}/tmp
    chown -R mysql: ${datadir}
    sleep 1
fi

# initialize mysql
if [ "${1}x" == "x" ];then
    # replace path of conf
    pwd=`pwd`
    sed -i "s#lz_pwd#${pwd}#g" $conf
    
    $mysqld --defaults-file=1.cnf --user=mysql --initialize-insecure
    sleep 3
fi

# start mysql
$mysqld  --defaults-file=1.cnf --user=mysql &
