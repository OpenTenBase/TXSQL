#!/bin/sh

if [ $# -ne 1 ];then
	echo "usage: $0 port"
	exit -1
fi

port=$1


./stopmysql.sh $port
./startmysql_cgroup.sh $port
