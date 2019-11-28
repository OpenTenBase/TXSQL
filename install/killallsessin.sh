#!/bin/sh

if [ $# -lt 1 ];then
   echo "usage: $0 port "
   echo "arg num: $#"
   exit
fi



port=$1
base_dir=`pwd | awk -F"/install" '{print $1}'`
etcfile=${base_dir}/etc/my_${port}.cnf
sockfile=` grep "socket      =" ${etcfile}  | tail -n1| awk -F"=" '{print $2}'`

#echo ${sockfile}

./tool/jmysql ${port} "select * from information_schema.processlist where USER not in('system user','unauthenticated user')  and user not like 'tdsqlsys_%'" "-N" | awk '{print "kill " $1 ";"}' | grep -v unixsocket | ./tool/jmysql ${port}

