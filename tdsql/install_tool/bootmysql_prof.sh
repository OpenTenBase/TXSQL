#!/bin/bash
if [ $# -ne 3 ];then
        echo "usage: $0 basedir etcfile mysqluser"
        exit -1
fi

ulimit -n 300000

base_dir=$1
etcfile=$2
mysqluser=$3

log_dir=`grep "#log dir is=" ${etcfile} | head -n1 | awk -F"=" '{print $2}'`
#log_dir=""

if [ "m_${log_dir}" == "m_" ];then
	log_dir="${HOME}"
fi

runso_dir="${base_dir}/.run_so"
if [ ! -d  ${runso_dir} ] ; then
    mkdir ${runso_dir}
fi

env_info="x86_64"
is_arm=`uname -a | grep aarch64  | head -n1 | wc -l `
page_size=`getconf PAGE_SIZE`

libjemalloc=libjemalloc.so.5.2.1.prof

if [ ! -e ${runso_dir}/${libjemalloc} ] ; then
    if [ e"$is_arm" != e"0" -a ! -f ${base_dir}/share_lib/${libjemalloc} ] ; then
        cp ${base_dir}/share_lib/${libjemalloc}_${page_size}  ${base_dir}/share_lib/${libjemalloc} -rf
    fi

        if [ -f  ${base_dir}/share_lib/${libjemalloc} ] ; then
                cp ${base_dir}/share_lib/${libjemalloc}  ${runso_dir}
        else # old DB instance.
                cp ${base_dir}/install/${libjemalloc} ${runso_dir}
                cp ${base_dir}/install/${libjemalloc} ${base_dir}/share_lib
        fi

fi


export LD_PRELOAD="${runso_dir}/${libjemalloc}"
export MALLOC_CONF="prof:true,lg_prof_interval:30" #1G打一次
cd ${base_dir}; nohup ./bin/mysqld_safe --defaults-file=${etcfile} --user=${mysqluser} >>${log_dir}/nohup.out &

