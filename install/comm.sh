#!/bin/sh

function installXtrabackup
{
    cwd=`pwd`
    #if [ ! -f /usr/lib/libz.so.1 ]; then
		#cp libz.so.1.2.8 /usr/lib/
		#cd /usr/lib
		#ln -s libz.so.1.2.8 libz.so.1
		#cd $cwd
    #fi

    #if [ ! -f /usr/lib64/libev.so.4 ]; then
        #cp libev.so.4.0.0 /usr/lib64
		#cd /usr/lib64
		#ln -s libev.so.4.0.0 libev.so.4
		#cd $cwd
    #fi

#    if [ ! -f /usr/bin/innobackupex ];then
#        rpm --nodeps -ivh percona-xtrabackup-24-2.4.1-1.el6.x86_64.rpm && echo "Success" && return
#		echo "Failed installing percona-xtrabackup"
#		return
#    else
#        /usr/bin/innobackupex --version 1>&2 2>tmp-version-number-file.txt
#		verstr=`cat tmp-version-number-file.txt`
#        if [[ $verstr != *"innobackupex version 2.4.1"* ]]
#		then
#			if [[ $verstr == *"InnoDB Backup Utility v1.5.1-xtrabackup;"* ]]
#			then
#				rpm -e percona-xtrabackup-2.2.3 && echo  "Old version percona-xtrabackup-2.2.3 detected and removed before installing percona-xtrabackup-2.4.1" >&2
#				rm tmp-version-number-file.txt
#        		rpm --nodeps -ivh percona-xtrabackup-24-2.4.1-1.el6.x86_64.rpm && echo "Success" && return
#			fi
#            echo "Error: Uninstall old version percona-xtrabackup first."
#        else
            echo "Success"
#        fi
#
#		rm tmp-version-number-file.txt
#    fi
}

function installPerconaToolkit
{
    # install percona-toolkit-2.2.16 if not exist or not of this version.
    verstr=""
    if [ -f /usr/local/bin/pt-query-digest ] ; then 
        verstr=`/usr/local/bin/pt-query-digest --version | while read a b c ; do echo $b; done`
    fi

    if [ $verstr"emp" != "2.2.16emp" ] ; then
         dir=`pwd`
	 rm -rf /usr/local/percona-toolkit-2.2.16
         tar zxvf percona-toolkit-2.2.16.tar.gz -C /usr/local/ && cd /usr/local/percona-toolkit-2.2.16 && perl Makefile.PL INSTALL_BASE=/usr/local && make && make test && make install
         cd $dir
    fi

    echo "Success"
}

#arg filename,num
#for example /etc/security/limits.d/80-nofile.conf,400000
function modifynofile
{
    filename=${1}
    nofile=${2}
    if [ $nofile -lt 100000 ];then
        nofile=100000
    fi
        
    if [  -f $filename ];then
        #echo "filename:$filename,nofile:$nofile"
        #grep -- "-    nofile     ${nofile}" $filename
        grep -- "-    nofile     " $filename | tail -n1 | grep -- "-    nofile     ${nofile}" 
        if [ $? -eq 0 ];then
            echo "[`date +'%Y-%m-%d %H:%M:%S'`] info is ok, filename:$filename,nofile:$nofile" 
        else
           echo "*          -    nofile     ${nofile}" >>$filename
           echo "[`date +'%Y-%m-%d %H:%M:%S'`] info set filename:$filename,nofile:$nofile" 
        fi
    fi
}

