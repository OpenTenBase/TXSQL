#!/bin/sh

if [ $# -ne 1 ];then
        echo "usage: $0 port"
        exit -1
fi

port=$1

#pid=`ps -ef | grep "bin/mysqld " | grep 4002| tail -n1 | awk '{print "gdb -p "$2 " "}'  `

pid=`ps -ef | grep "bin/mysqld " | grep $port | tail -n1 | awk '{print ""$2 " "}'  `
gdb -p $pid -x gdbfile.txt
