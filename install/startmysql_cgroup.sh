#!/bin/sh

if [ $# -ne 1 ];then
    echo "usage: $0 port"
    exit -1
fi

port=$1

#cgroup check
if [ -d /data/tdsqlcmd ];then
  sudo /data/tdsqlcmd/base_cmd "./tdsql_cgroup_build --instanceport=${port} --mode=init"
  sudo /data/tdsqlcmd/base_cmd "./tdsql_addto_cgroup --cgrouptype='cpu,blkio' --port=$port --pid=$$ "
else
  sudo su -c "./tdsql_cgroup_build --instanceport=${port} --mode=init"
  sudo su -c "./tdsql_addto_cgroup --cgrouptype='cpu,blkio' --port=$port --pid=$$ "
fi

if [ $? -eq 0 ];then
    ./startmysql.sh $*
    exit $? 
else
    echo "./tdsql_addto_cgroup fail,can't startreport.sh"
    exit -1
fi
