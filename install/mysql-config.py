#!/bin/python

import os
import sys
import re
import time
import random
import fcntl
import struct
import socket
import subprocess

def get_ip_address(ifname):
    s = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
    return socket.inet_ntoa(fcntl.ioctl(
        s.fileno(),
        0x8915,  # SIOCGIFADDR
        struct.pack('256s', ifname[:15])
    )[20:24])

def multiple_replace(string, rep_dict):
    pattern = re.compile("|".join([re.escape(k) for k in rep_dict.keys()]), re.M)
    return pattern.sub(lambda x: rep_dict[x.group(0)], string)

def get_root_init_pass():
    install_inf = open('install.inf', 'r')
    lines = install_inf.readlines()
    for line in lines:
        if 'A temporary password is generated for root@localhost' in line:
            ret = line.split('root@localhost: ')[1][:-1]
            return ret

class MysqlConfig:

    def __init__(self, config_template_file, server_port, server_data_prefix, server_log_prefix, user, install_path):

        data_path = server_data_prefix + "/" + server_port
        prod_dir = data_path + "/prod"
        data_dir = data_path + "/dbdata_raw/data"
        innodb_dir = data_path + "/dbdata_raw/dbdata"
        os.makedirs(prod_dir)
        os.makedirs(data_dir)
        os.makedirs(innodb_dir)
        
        log_path = server_log_prefix + "/" + server_port
        log_dir = log_path + "/dblogs"
        tmp_dir = log_path + "/dblogs/tmp"
        log_relay = log_path + "/dblogs/relay"
        log_bin_arg = log_path + "/dblogs/bin"
        log_arch = log_path + "/dblogs/arch"
        os.makedirs(log_dir)
        os.makedirs(tmp_dir)
        os.makedirs(log_relay)
        os.makedirs(log_bin_arg)
        os.makedirs(log_arch)

        replace_items = {
                "change_port_before_use": server_port,
                "change_ip_before_use": get_ip_address("eth1"),
                "change_extra_port_before_use": str(int(server_port)+10000),
                "base_dir": install_path,
                "change_server_id_before_use": str(random.randint(1,1000)),
                "my_change_port_before_use": server_port,
                "change_user_before_use": user,
                "prod_dir": prod_dir,
                "data_dir": data_dir,
                "innodb_dir": innodb_dir,
                "log_dir": log_dir,
                "tmp_dir": tmp_dir,
                "log_relay": log_relay,
                "log_bin_arg": log_bin_arg,
                "log_arch": log_arch,
                "log-error =": "#log-error =",
                "rootdatadir_arg": server_data_prefix,
                "rootlogdir_arg": server_log_prefix
                }

        config_template = open(config_template_file, 'r').read()
        conf = multiple_replace(config_template, replace_items)

        etc_path = install_path + "/etc"
        if not os.path.exists(etc_path):
            os.mkdir(etc_path)
        cnf_file_path = etc_path+"/my_"+server_port+".cnf"
        cnf_file = open(cnf_file_path, 'w')
        cnf_file.write(conf)
        cnf_file.close()

        os.system(" ".join(["chown", user+":users", etc_path, "-R"]))
        os.system(" ".join(["chown", user+":users", log_path, "-R"]))
        os.system(" ".join(["chown", user+":users", data_path, "-R"]))

        os.system(" ".join(["su", user, "-c",  "\"../bin/mysqld", "--defaults-file="+cnf_file_path, "--user="+user, "--initialize 2> install.inf\""]))
        root_init_password = get_root_init_pass()
        assert(root_init_password != None)
        
        os.system("sed -i -e 's/#log-error = /log-error = /' " + cnf_file_path)
        #subprocess.Popen(["nohup", "./bin/mysqld_safe", '--defaults-file='+cnf_file_path], cwd=install_path)
        os.system(" ".join(["su", user, "-c", "\"./bootmysql.sh", install_path, cnf_file_path, user+"\""]))

        change_pwd_sql = "ALTER USER 'root'@'localhost' IDENTIFIED BY 'root_pwd';"
        init_sql = "grant all on *.* to kp@'%' identified by 'kp123456' with grant option;flush privileges;" \
                + "grant replication slave,replication client on *.* to 'repl'@'%' identified by password '*1E8485D9FEEA3615B2F5B98CCB4EC4D40015A22E';flush privileges;" \
                + "grant all on *.* to 'agent'@'localhost' identified by password '*E70D9E391EC5371454808E6C49CE43E1AD512626' with grant option;flush privileges;" \
                + "grant select on SysDB.* to 'readsysdb'@'%' identified by password '*681F64401EA005451EE0490D8B56EFF1F6F7CAE0';flush privileges;" \
                + "grant Select,Insert,Update,Delete,Create,Drop,Process,References,Index,Alter,SHOW DATABASES,CREATE TEMPORARY TABLES,LOCK TABLES,Execute,CREATE VIEW,SHOW VIEW,CREATE ROUTINE,ALTER ROUTINE,Event,Trigger,REPLICATION CLIENT,reload on *.* to  'gw'@'%' identified by password '*91A99D0D8B9CCC48F3B8987F2318CFE537A4381D' ;flush privileges;"\
                + "set global rpl_semi_sync_master_timeout=1000;set sql_log_bin=0;create database SysDB;use SysDB;CREATE TABLE StatusTable(ip varchar(20), port int, ts timestamp, primary key(ip,port));set global rpl_semi_sync_master_timeout=10000;" \
                + "select now();select version();"
        os.chdir(install_path)
        sys_cmd = " ".join(['./bin/mysql', '--connect-expired-password', '-S' + data_path + '/prod/mysql.sock', '-uroot', '-p'+"'"+root_init_password+"'", '-e', '"' + change_pwd_sql + '"'])
        print sys_cmd
        for _ in xrange(30):
            if os.system(sys_cmd) == 0:
                os.system(" ".join(["./bin/mysql", "--connect-expired-password", '-S' + data_path + '/prod/mysql.sock', '-uroot -proot_pwd', '-e', '"' + init_sql + '"\n']))
                break
            os.system('sleep 1\n')


if __name__ == "__main__":
    try:
        args = dict([arg.split('=') for arg in sys.argv[1:]])
        server_port = args["server_port"]
        server_data_prefix = args["server_data_prefix"]
        server_log_prefix = args["server_log_prefix"]
        user = args["user"]
        config_template_file = "./template.cnf"
        install_path = os.getcwd()[:-8]
        MysqlConfig(config_template_file, server_port, server_data_prefix, server_log_prefix, user, install_path)
    except KeyError, e:
        print 'sudo python mysql-config.py server_port=? server_data_prefix=? server_log_prefix=? user=?'
