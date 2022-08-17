#!/usr/bin/perl

### 修改这里的参数配置作为输入参数
### 请把her.cnf 和 my.cnf copy 到当前目录
my $dest_version = "8.0.15";
my $dest_version_number = "20190530_test";
my $dest_dev_type = "ts85";

my $dest_pack_name = sprintf("mysql-txsql-%s-%s-linux-x86_64_%s.tar.gz", $dest_version, $dest_version_number, $dest_dev_type);
my $dest_dir_name = sprintf("mysql-txsql-%s-%s-linux-x86_64_%s", $dest_version, $dest_version_number, $dest_dev_type);
print "dest_dir_name=$dest_dir_name\n, dest_pack_name=$dest_pack_name\n";

### 清理以前可能遗留的package和dir
if (-d "bld-release") {
  chdir "bld-release";
  if (-d $dest_dir_name) {
    print "dest_dir_name=$dest_dir_name exists, try remove\n";
    system("rm -r $dest_dir_name") and die("failed to remove dir $dest_dir_name");
  }
  if (-f $dest_pack_name) {
    print "dest_pack_name=$dest_pack_name exists, try remove\n";
    system("rm $dest_pack_name") and die("failed to remove file $dest_pack_name"); 
  }
  chdir "..";
}

### call build.sh
system("./build.sh -t release") and die("failed to build check compile");

### call make package, this may call
chdir "bld-release";
print "=== calling make package to get original mysql package and package name ===\n";
my $make_output = `make package`;
my $ret = $?;
if ($ret != 0) {
  print "make package failed, ret=$ret\n";
  exit $ret;
}

my @lines = split(/\n/, $make_output);
my @output = split(' ', $lines[-1]);
my $package_name = $output[3];
my $tar_name = (split(/\//, $package_name))[-1];
print "=== make package finished tar_name=$tar_name ===\n";

### generate cdb type package, add her.cnf and tmy.conf
print "=== begin to do txsql package work ===\n";
my $to_rename_dir = substr($tar_name, 0, -7);
system("mkdir $dest_dir_name") and die("failed to mkdir for $dest_dir_name");
system("tar xvf $tar_name -C $dest_dir_name") and die("failed to unpack package_name=$package_name into $dest_dir_name");
system("mv $dest_dir_name/$to_rename_dir $dest_dir_name/mysql") and die("failed to rename in $dest_dir_name");
system("cp ../tmy.conf $dest_dir_name") and die("failed to put in tmy.conf");
system("cp ../her.cnf $dest_dir_name") and die("failed to put in her.cnf");
system("tar -zcvf $dest_dir_name.tar.gz $dest_dir_name") and die("failed to package $dest_dir_name");
system("mv $dest_dir_name.tar.gz ..") and die("failed to mv $dest_dir_name");

print "=== finished to do txsql package work, txsql package_name=$dest_dir_name.tar.gz ===\n";
