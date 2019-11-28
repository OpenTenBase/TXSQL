#!/bin/sh

if [ $# -lt 1 ];then
    echo "usage: $0 port"
    exit
fi

port=$1
base_dir=`pwd | awk -F"/install" '{print $1}'`
etcfile=${base_dir}/etc/my_${port}.cnf
sockfile=` grep "socket      =" ${etcfile}  | tail -n1| awk -F"=" '{print $2}'`

date=`date -d today +"%Y%m%d"`
time=`date -d today +"%H%M%S"`

dir="../log/dump_log/log_${date}_${time}"
mkdir -p ${dir} 

file="${dir}/${port}"

#show engine innodb status\G;
./tool/jmysql $port   "show engine innodb status\G;" "--connect-timeout=5 -f" >> $file"_innodb_status_${date}.log"

#show threadpool status\G;
./tool/jmysql $port   "show threadpool status\G;" "--connect-timeout=5 -f" >> $file"_threadpool_${date}.log"

#select * FROM information_schema.metadata_lock_info\G;
./tool/jmysql $port   "select * FROM information_schema.metadata_lock_info;" "--connect-timeout=5 -f" >> $file"_metalock_${date}.log"

#select * from information_schema.INNODB_TRX\G;
./tool/jmysql $port   "select * from information_schema.INNODB_TRX;" "--connect-timeout=5 -f " >> $file"_innodb_trx_${date}.log"

#select * from information_schema.INNODB_LOCK_WAITS\G;
./tool/jmysql $port    "select * from information_schema.INNODB_LOCK_WAITS;" "--connect-timeout=5 -f">> $file"_innodb_lock_waits_${date}.log"

#select * from information_schema.INNODB_LOCKS\G;
./tool/jmysql $port    "select * from information_schema.INNODB_LOCKS;" "--connect-timeout=5 -f" >> $file"_innodb_locks_${date}.log"

#select * from information_schema. PROCESSLIST\G;
./tool/jmysql $port    "select * from information_schema.PROCESSLIST order by TIME desc;" "--connect-timeout=5 -f" >> $file"_innodb_processlist_${date}.log"

#harly lock analysis
./tool/autounlock $port >> $file"_lock_analysis_${date}.log"

#dstat 
echo "=========================dstat========================" >> $file"_sysdump_${date}.log"
dstat -t  -a --proc-count -i -l -m -p --aio --disk-util 1 3 >> $file"_sysdump_${date}.log"

#meminfo
echo "=========================meminfo========================" >> $file"_sysdump_${date}.log"
cat /proc/meminfo >> $file"_sysdump_${date}.log"

#vmstat
echo "=========================vmstat========================" >> $file"_sysdump_${date}.log"
cat /proc/vmstat >> $file"_sysdump_${date}.log"

#cpu
echo "=========================top========================" >> $file"_sysdump_${date}.log"
top -b -n 1 >> $file"_sysdump_${date}.log"

#mem
echo "=========================free========================" >> $file"_sysdump_${date}.log"
free -m >> $file"_sysdump_${date}.log"

#io
echo "=========================iostat========================" >> $file"_sysdump_${date}.log"
iostat -t 1 -x 3 >> $file"_sysdump_${date}.log"
