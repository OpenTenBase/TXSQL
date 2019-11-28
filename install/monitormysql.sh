#!/bin/sh

if [ $# -ne 1 ];then
   echo "usage: $0 port"
   exit
fi

port=$1
base_dir=`pwd | awk -F"/install" '{print $1}'`
etcfile=${base_dir}/etc/my_${port}.cnf
sockfile=` grep "socket *=" ${etcfile}  | tail -n1| awk -F"=" '{print $2}'`

#echo ${sockfile}

./tool/monitormysql ${port} |\
awk -F"|" \
' BEGIN{ count=0; }\
 {if($2 ~ /Variable_name/ && ++count %30 == 1){\
    print "----------|---------|--- MySQL Command Status --|----- Innodb row operation ----|-- Buffer Pool Read ---------|---- sync-----";\
    print "---Time---|---QPS---|select insert update delete commit|  read inserted updated deleted|   logical    physical| datasync   logsync";\
}\
else if ($2 ~ /Queries/){queries=$3;}\
else if ($2 ~ /Com_select /){com_select=$3;}\
else if ($2 ~ /Com_insert /){com_insert=$3;}\
else if ($2 ~ /Com_update /){com_update=$3;}\
else if ($2 ~ /Com_delete /){com_delete=$3;}\
else if ($2 ~ /Com_commit /){com_commit=$3;}\
else if ($2 ~ /Innodb_rows_read/){innodb_rows_read=$3;}\
else if ($2 ~ /Innodb_rows_deleted/){innodb_rows_deleted=$3;}\
else if ($2 ~ /Innodb_rows_inserted/){innodb_rows_inserted=$3;}\
else if ($2 ~ /Innodb_rows_updated/){innodb_rows_updated=$3;}\
else if ($2 ~ /Innodb_buffer_pool_read_requests/){innodb_lor=$3;}\
else if ($2 ~ /Innodb_buffer_pool_reads/){innodb_phr=$3;}\
else if ($2 ~ /Threads_running/){thread_running=$3;}\
else if ($2 ~ /Innodb_data_fsyncs/){Innodb_data_fsyncs=$3;}\
else if ($2 ~ /Innodb_os_log_fsyncs/){Innodb_os_log_fsyncs=$3;}\
else if ($2 ~ /Uptime / && count >= 2){\
  printf(" %s |%9d",strftime("%H:%M:%S"),queries);\
  printf("|%6d %6d %6d %6d %6d",com_select,com_insert,com_update,com_delete,com_commit);\
  printf("|%6d %8d %7d %7d",innodb_rows_read,innodb_rows_inserted,innodb_rows_updated,innodb_rows_deleted);\
  printf("|%10d %11d",innodb_lor,innodb_phr);\
  printf("|%10d %11d\n",Innodb_data_fsyncs,Innodb_os_log_fsyncs);\
} }'


