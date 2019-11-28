#!/bin/sh

if [ $# -ne 1 ];then
   echo "usage: $0 port"
   exit
fi

port=$1
base_dir=`pwd | awk -F"/install" '{print $1}'`
etcfile=${base_dir}/etc/my_${port}.cnf
sockfile=` grep "socket *=" ${etcfile}  | tail -n1| awk -F"=" '{print $2}'`


./tool/stopmysql ${port}


programe="bin/mysqld"
cmd="ps -ef | grep ${programe} | grep -v vim | grep -v grep | grep -v defunct | grep -- 'my_${port}.cnf' | awk '{print \$2}' | head -n 1"
echo $cmd

stopfail=1
for i in `seq 0 30`
#for i in `seq 0 15`
do
	pid=$(eval ${cmd})
	#echo $pid
	if [ $pid"e" != "e" ];then
       	 	echo "mysqld is running,count:${i}."
	else
		echo "mysqld is stoped"
		stopfail=0
		break
	fi

   	sleep 1
done

#force stop
if [ $stopfail -ne 0 ];then
   ps -ef | grep my_${port}.cnf | grep ${programe} | grep -- "--defaults-file="| grep -v grep | awk '{print $2}'  | xargs kill -9
fi

exit $stopfail
