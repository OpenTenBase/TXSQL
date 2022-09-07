#!/bin/bash
<< 'COMMENT'
  Usage:
  1. cmd prepare
  2. cmd run [ro|wo|rw|i|up|upnon] 128
COMMENT

########==========================Variables===============================

sock=/tmp/30002_txsql.sock
profile_data="../profile-data"
perf_data_file="../perf.data"
table_count=16
table_size=10000
sb_database="sbtest"

threads_count=$3
warmup_time=10
report_interval=5  # s
# stage1
# If changed, you should modified it in compile_optimize_build.sh too!
time=300  
# stage2
# If changed, you should modified it in compile_optimize_build.sh too!
stage2_time=120 

sb_log_dir="./logs_sb"
log_prefix="test"


readonly lua_dir="/usr/share/sysbench/" #lua_dir="/usr/local/share/sysbench/"
threads=(1 8 32 128 512 1024)

########==========================Funtion=================================

sysbench_prepare(){
  mysql -S $sock -e "drop database if exists $sb_database"
  mysql -S $sock -e "create database $sb_database"

  sysbench $lua_dir/oltp_insert.lua \
  --mysql-socket=${sock} --mysql-user=root \
  --mysql-db=${sb_database}  --tables=${table_count} --table-size=${table_size}  \
  --rand-type=uniform --threads=16  prepare  2>&1
}

run(){
  threads_count=${2}
  file_log="${sb_log_dir}/${log_prefix}_sysbench.${threads_count}.${sb_database}.${1}.log.`date "+%Y%m%d_%H%M%S"`"
  echo "log: $file_log"

  script=""
  case $1 in
  ro)
    script="oltp_read_only.lua"
    ;;
  wo)
    script="oltp_write_only.lua"
    ;;
  rw)
    script="oltp_read_write.lua"
    ;;
  up)
    script="oltp_update_index.lua"
    ;;
  upnon)
      script="oltp_update_non_index.lua"
      ;;
  i)
      script="oltp_insert.lua"
      ;;
  *)
    script=$1
    ;;
  esac

  echo "running ${script} ..." >> ${file_log}
  echo "connecting to server ${master_ip}:${master_port} with user ${username}, using db ${sb_database}" >> ${file_log}
  echo "table count:${table_count}, table size:${table_size}, threads count:${threads_count}, time:${time}, warmup:${warmup_time}" >> ${file_log}

  sysbench ${lua_dir}/${script} \
  --mysql-socket=${sock} --mysql-user=root \
  --mysql-db=${sb_database}  --tables=${table_count} --table-size=${table_size}  \
  --rand-type=uniform --threads=${threads_count} \
  --time=${time} --report-interval=${report_interval} \
  run >> ${file_log} 
}

stage1_run() {
  if [ ! -d ${profile_data} ];then
    echo "${profile_data} not exist, bye."
    exit 2
  fi

  # Make sure mysqld has permission to create *.gcda files in $profile_data.
  # Clean *.gcda files in $profile_data.
  if [ "${profile_data}x" != "x" ];then
    chown -R mysql: ${profile_data}
    rm ${profile_data}/*.gcda 2>/dev/null
  fi

  for thread in 1 32 256
  do
      for case in rw ro i wo up upnon
    do
      run $case $thread
    done
  done
}

stage2_run() {
  # run sysbench test
  time=$stage2_time

  run rw 8 &
  run ro 8 &
  run i 8 &
  run wo 8 &
  run up 8 &
  run upnon 8 &
  sleep 3
  
  # generate perf.data
  if [ -f ${perf_data_file} ];then
    rm ${perf_data_file}
  fi

  pidfile=`cat 1.cnf |grep pid|awk '{print $NF}'`
  pid=`cat ${pidfile}`  
  echo "pid of mysqld: $pid, running perf record ..."
  perf record -o ${perf_data_file} -e cycles:u -j any,u -a -p ${pid} -- sleep $((${stage2_time}-10)) &

  # generate perf.fdata 
  if [ ! -d /data/tmp ];then
    mkdir /data/tmp
  fi
  echo "[info] export TMPDIR=/data/tmp"
  echo "[info] perf2bolt $mysqld_path -p perf.data -o perf.fdata"
}

##================================main======================

if [ ! -d $sb_log_dir ];then
  mkdir $sb_log_dir
  echo "creating sb_log_dir: $sb_log_dir"
fi

script=""
case $1 in
  prepare)
    sysbench_prepare
    exit $?
    ;;
  stage1)
      stage1_run
    exit $?
    ;;
  stage2)
      stage2_run
    exit $?
    ;;
  run)
    continue
    ;;
  *)
    echo "invalid input, cmd [prepare|run|cleanup]"
    exit 2
esac

if [ "${threads_count}x" != "x" ];then
  echo "run with specified threads_count."
  run ${2} ${threads_count}
else
  echo "not specified threads_count, running user defined script."

  for case in rw ro wo up upnon i
  do
    for thread in ${threads[@]}
    do
      run $case $thread
      sleep 5
    done
  done
fi


