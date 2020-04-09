#!/usr/bin/perl -w
#
#  (C) 2010-2012 Alibaba Group Holding Limited.
#
#  This program is free software; you can redistribute it and/or
#  modify it under the terms of the GNU General Public License
#  version 2 as published by the Free Software Foundation.
#
#  Authors:
#    zhuxu@taobao.com  < http://weibo.com/orzdba >
#

use strict;
use Getopt::Long;                             # Usage Info URL:  http://perldoc.perl.org/Getopt/Long.html
use POSIX qw(strftime);                       # Usage Info URL:  http://perldoc.perl.org/functions/localtime.html
use Term::ANSIColor;                          # Usage Info URL:  http://perldoc.perl.org/Term/ANSIColor.html
use Socket;                                   # Get IP info

Getopt::Long::Configure qw(no_ignore_case);   #


# ----------------------------------------------------------------------------------------
# Variables
# ----------------------------------------------------------------------------------------
my %opt;                      # Get options info
my $headline1       = '';
my $headline2       = '';
my $mysql_headline1 = '';
my $mysql_headline2 = '';
my $mycount         = 0;      # to control the print of headline
# Options Flag
#----->
my $timeFlag        = 0;      # -t   : Print current time
my $interval        = 1;      # -i   : Time interval in seconds
my $load            = 0;      # -l   : Print load info
my $cpu             = 0;      # -c   : Print cpu  info
my $swap            = 0;      # -s   : Print swap info
my $disk;                     # -d   : Print disk info
my $mysql           = 0;      # -mysql : Print all of the MySQL status
my $com             = 0;      # -com : Print MySQL Com_xxx status
my $innodb_hit      = 0;      # -hit : Print InnoDB Hit%
my $innodb_rows     = 0;      # -innodb_rows : Print InnoDB Rows Status
my $innodb_pages    = 0;      # -innodb_pages: InnoDB Buffer Pool Pages Status
my $innodb_data     = 0;      # -innodb_data : InnoDB Data Status
my $innodb_log      = 0;      # -innodb_log  : InnoDB Log Status
my $innodb_status   = 0;      # -innodb_status: Show Engine InnoDB Status
my $threads         = 0;      # -T   : Print Threads Status
my $bytes           = 0;      # -B   : Print Bytes Status
my $count;                    # -C   : Times
my $logfile;                  # -L   : logfile
my $logfile_by_day;           # -logfile_by_day : one day a logfile
my $net;                      # -n   : Print net info
my $host;                     # -h   : MySQL's hostname
my $port            = 3306;   # -P   : MySQL's port number
my $passwd          = "";     # -p
my $socket;                   # -S
my $dbrt= 0;                  # -rt
my $tcprstat_dir    = "/tmp";
my $tcprstat_log;
my $tcprstat_lck;
#<-----

# Variables For :
#-----> Get SysInfo (from /proc/stat): CPU
my @sys_cpu1        = (0)x8;
my $total_1         = 0;
#
my $user_diff;
my $system_diff;
my $idle_diff;
my $iowait_diff;
#<----- Get SysInfo (from /proc/stat): CPU

#-----> Get SysInfo (from /proc/diskstats): IO
my @sys_io1         = (0)x15;
#my $not_first      = 0;       # no print first value
#/* Number of processors */
my $ncpu = `grep processor /proc/cpuinfo | wc -l`;
# grep "HZ" -R /usr/include/*
# /usr/include/asm-x86_64/param.h:#define HZ 100
my $HZ = 100;
#<----- Get SysInfo (from /proc/diskstats): IO

#-----> Get SysInfo (from /proc/vmstat): SWAP
my %swap1 =
(
        "pswpin"  => 0,
        "pswpout" => 0
);
my $swap_not_first = 0;
#<----- Get SysInfo (from /proc/vmstat): SWAP

#-----> Get SysInfo (from /proc/net/dev): NET
my %net1 =
(
        "recv"  => 0,
        "send" => 0
);
my $net_not_first = 0;
#<----- Get SysInfo (from /proc/net/dev): NET

#-----> Get MySQL Status
my %mystat1 =
(
        "Com_select" => 0 ,
        "Com_delete" => 0 ,
        "Com_update" => 0 ,
        "Com_insert" => 0,
        "Innodb_buffer_pool_read_requests" => 0,
        "Innodb_rows_inserted" => 0 ,
        "Innodb_rows_updated" => 0 ,
        "Innodb_rows_deleted" => 0 ,
        "Innodb_rows_read" => 0,
        "Threads_created" => 0,
        "Bytes_received" => 0,
        "Bytes_sent" => 0,
        "Innodb_buffer_pool_pages_flushed" => 0,
        "Innodb_data_read" => 0,
        "Innodb_data_reads" => 0,
        "Innodb_data_writes" => 0,
        "Innodb_data_written" => 0,
        "Innodb_os_log_fsyncs" => 0,
        "Innodb_os_log_written" => 0
);
my $not_first  = 0;
#<----- Get MySQL Status

my $LOG_OUT = *STDOUT;

# autoflush
$| = 1;

# handle Ctrl+C
sub catch_zap {
  my $signame = shift;
  if ($dbrt) {
    &rm_logfile("$tcprstat_dir/$tcprstat_log");
    &rm_logfile("$tcprstat_dir/$tcprstat_lck.lck");
  }

  print color ("red");
  print "\nExit Now...\n\n";
  print color ("reset");
  exit;
}
$SIG{INT} = \&catch_zap;


# ----------------------------------------------------------------------------------------
# 0.
# Main()
# ----------------------------------------------------------------------------------------

# clear screen
# print `clear`;

# Get options info
&get_options();

#
my $MYSQL    = qq{mysql -s --skip-column-names -uxx -P$port };
$MYSQL      .= qq{-h$host } if defined $host;
$MYSQL      .= qq{-S$socket } if defined $socket;
$MYSQL      .= qq{-p$passwd } if( defined $passwd and $passwd ne "");
my $TCPRSTAT = "/usr/bin/tcprstat --no-header -t 1 -n 0 -p $port";

&print_title();

while(1) {
  # -C;Times to exits
  if( defined ($count) and $mycount > $count ) {
     if ($dbrt) {
       &rm_logfile("$tcprstat_dir/$tcprstat_log");
       &rm_logfile("$tcprstat_dir/$tcprstat_lck.lck");
     }
    exit;
  }

  # -L;-logfile_by_day
  if ( defined($logfile) and $logfile_by_day ) {
    my $day = strftime ("%Y-%m-%d", localtime);
    my $logfile_day = qq{$logfile.$day};
    unless ( -e $logfile_day ) {
      close LOGFILE_OUT  or die "Can't close!";
      open LOGFILE_OUT,">$logfile_day" or die "Can't open file!";
      $LOG_OUT  = *LOGFILE_OUT;

       &print_title();
       $count = $count - $mycount if defined $count;
       $mycount = 0;
     }
   }

   # Print Headline
   if ( $mycount%15 == 0 ) {
      print $LOG_OUT BLUE(),BOLD(),"$headline1",RESET();
      print $LOG_OUT ON_BLUE(),GREEN(),"$mysql_headline1",RESET() if $mysql;
      print $LOG_OUT "\n";
      print $LOG_OUT BLUE(),UNDERLINE(),BOLD(),"$headline2",RESET();
      print $LOG_OUT GREEN(),UNDERLINE(),"$mysql_headline2",RESET() if $mysql;
      print $LOG_OUT "\n";
   }

    $mycount += 1;

    # (1) Print Current Time
    if ($timeFlag) {
      print $LOG_OUT YELLOW();
      my $nowTime = strftime "%H:%M:%S", localtime;
      print $LOG_OUT "$nowTime",BLUE(),BOLD(),"|",RESET();
    }

    # (2) Print SysInfo
    &get_sysinfo();

    # (3) Print MySQL Status
    &get_mysqlstat();

    # (4) TCPRSTAT
    &get_dbrt() if $dbrt;

    #
    print $LOG_OUT "\n";
    sleep($interval);
}


# ----------------------------------------------------------------------------------------
# 1.
# Func :  print usage
# ----------------------------------------------------------------------------------------
sub print_usage{
  #print BLUE(),BOLD(),<<EOF,RESET();
  print <<EOF;
==========================================================================================
Info  :
        Created By zhuxu\@taobao.com
Usage :
Command line options :

   -H,--help           Print Help Info.
   -i,--interval       Time Interval in Seconds..
   -C,--count          Times.
   -t,--time           Print The Current Time.
   -nocolor            Print NO Color.

   -l,--load           Print Load Info.
   -c,--cpu            Print Cpu  Info.
   -s,--swap           Print Swap Info.
   -d,--disk           Print Disk Info.
   -n,--net            Print Net  Info.

   -h,--host           Host name of MySQL
   -P,--port           Port number of MySQL(default 3306)
   -p,--passwd         Password of MySQL(default null).
   -S,--socket         Socket file of MySQL

   -com                Print MySQL Status(Com_select,Com_insert,Com_update,Com_delete).
   -hit                Print InnoDB Hit%.
   -innodb_rows        Print InnoDB Rows Status(Innodb_rows_inserted/updated/deleted/read).
   -innodb_pages       Print InnoDB Buffer Pool Pages Status(Innodb_buffer_pool_pages_data/free/dirty/flushed)
   -innodb_data        Print InnoDB Data Status(Innodb_data_reads/writes/read/written)
   -innodb_log         Print InnoDB Log  Status(Innodb_os_log_fsyncs/written)
   -innodb_status      Print InnoDB Status from Command: 'Show Engine Innodb Status'
                       (history list/ log unflushed/uncheckpointed bytes/ read views/ queries inside/queued)
   -T,--threads        Print Threads Status(Threads_running,Threads_connected,Threads_created,Threads_cached).
   -rt                 Print MySQL DB RT(us).
   -B,--bytes          Print Bytes received from/send to MySQL(Bytes_received,Bytes_sent).

   -mysql              Print MySQLInfo (include -t,-com,-hit,-T,-B).
   -innodb             Print InnodbInfo(include -t,-innodb_pages,-innodb_data,-innodb_log,-innodb_status)
   -sys                Print SysInfo   (include -t,-l,-c,-s).
   -lazy               Print Info      (include -t,-l,-c,-s,-com,-hit).

   -L,--logfile        Print to Logfile.
   -logfile_by_day     One day a logfile,the suffix of logfile is 'yyyy-mm-dd';
                       and is valid with -L.

Sample :
   shell> nohup ./orzdba -lazy -d sda -C 5 -i 2 -L /tmp/orzdba.log >/dev/null 2>&1 &
==========================================================================================
EOF
  exit;
}

# ----------------------------------------------------------------------------------------
# 2.
# Func : get options and set option flag
# ----------------------------------------------------------------------------------------
sub get_options{
  # Get options info
  GetOptions(\%opt,
    'H|help',           # OUT : print help info
    'i|interval=i',     # IN  : time(second) interval
    't|time',           # OUT : print current time
    'sys',              # OUT : print SysInfo (include -l,-c,-s)
    'l|load',           # OUT : print load info
    'c|cpu',            # OUT : print cpu  info
    'd|disk=s',         # IN  : print disk info
    'n|net=s',          # IN  : print info
    's|swap',           # OUT : print swap info
    'com',              # OUT : print mysql status
    'innodb_rows',      # OUT : Print Innodb Rows Status
    'innodb_pages',     # OUT : Print Innodb Buffer Pool Pages Status
    'innodb_data',      # OUT : Print Innodb Data Status
    'innodb_log',       # OUT : Print Innodb Log  Status
    'innodb_status',    # OUT : Print Innodb Status from Command: 'Show Engine Innodb Status'
    'innodb',           # OUT : Print Innodb Info
    'T|threads',        # OUT : Print Threads Status
    'B|bytes',          # OUT : Print Bytes Status
    'rt',               # OUT : Print MySQL DB RT
    'hit',              # OUT : Print Innodb Hit%
    'mysql',            # OUT : Print mysql info
    'h|host=s',         # IN  : host
    'P|port=i',         # IN  : port
    'p|passwd=s',       # IN  : password
    'S|socket=s',       # IN  : socket
    'C|count=i',        # IN  : times
    'L|logfile=s',      # IN  : path of logfile
    'logfile_by_day',   # IN  : one day a logfile
    'lazy',             # OUT : Print Info (include -t,-l,-c,-s,-m,-hit).
    'nocolor',          # OUT : print no color
    ) or print_usage();

    if (!scalar(%opt)) {
      &print_usage();
    }

    # Handle for options
    $opt{'H'}   and print_usage();
    $opt{'i'}   and $interval = $opt{'i'};
    $opt{'t'}   and $timeFlag = 1;
    $opt{'C'}   and $count = $opt{'C'};
    $opt{'l'}   and $load = 1;
    $opt{'c'}   and $cpu = 1;
    $opt{'d'}   and $disk = $opt{'d'};
    $opt{'n'}   and $net  = $opt{'n'};
    $opt{'T'}   and $threads = 1 and $mysql = 1;
    $opt{'B'}   and $bytes = 1 and $mysql = 1;
    $opt{'rt'}  and $dbrt = 1 and $mysql = 1 ;
    $opt{'com'} and $com = 1 and $mysql = 1;
    $opt{'hit'} and $innodb_hit = 1 and $mysql = 1;
    $opt{'s'}   and $swap = 1;
    $opt{'h'}   and $host = $opt{'h'};
    $opt{'P'}   and $port = $opt{'P'};
    $opt{'p'}   and $passwd = $opt{'p'};
    $opt{'S'}   and $socket = $opt{'S'};
    $opt{'sys'} and $load= 1 and $cpu=1 and $timeFlag=1 and $swap = 1;
    $opt{'innodb_rows'}   and $innodb_rows = 1   and $mysql = 1;
    $opt{'innodb_pages'}  and $innodb_pages = 1  and $mysql = 1;
    $opt{'innodb_data'}   and $innodb_data = 1   and $mysql = 1;
    $opt{'innodb_log'}    and $innodb_log = 1    and $mysql = 1;
    $opt{'innodb_status'} and $innodb_status = 1 and $mysql = 1;

    # -lazy (include -t,-l,-c,-m,-s,-hit)
    $opt{'lazy'} and $timeFlag = 1 and $load=1 and $cpu=1 and $swap = 1 and $com=1 and $innodb_hit = 1 and $mysql = 1;

    # -mysql
    $opt{'mysql'} and $timeFlag = 1 and $com=1 and $innodb_hit = 1 and $threads=1 and $bytes = 1 and $mysql = 1;

    # -innodb
    $opt{'innodb'} and $timeFlag = 1 and $innodb_pages = 1 and $innodb_data = 1 and $innodb_log = 1 and $innodb_status = 1 and $mysql = 1;

    $opt{'logfile_by_day'} and $logfile_by_day = $opt{'logfile_by_day'};
    # -L
    if ( defined $opt{'L'} ) {
      $logfile = $opt{'L'};
      if ( defined $logfile_by_day ) {
        my $day = strftime ("%Y-%m-%d", localtime);
        my $logfile_day = qq{$logfile.$day};
        open LOGFILE_OUT,">$logfile_day" or die "Can't open file!";
      } else {
        open LOGFILE_OUT,">$logfile" or die "Can't open file!";
      }
      $LOG_OUT  = *LOGFILE_OUT;
    }
    # color control
    my $HAS_COLOR = (defined $opt{'L'} or defined $opt{'nocolor'}) ? 0:1;
    if ($HAS_COLOR) {
      import Term::ANSIColor ':constants';
    } else {
      *RESET     = sub { };
      *YELLOW    = sub { };
      *RED       = sub { };
      *GREEN     = sub { };
      *BLUE      = sub { };
      *WHITE     = sub { };
      *BOLD      = sub { };
      *MAGENTA   = sub { };
      *ON_BLUE   = sub { };
      *UNDERLINE = sub { };
    }

    # Init Headline
    if($timeFlag){
      $headline1 = "-------- ";
      $headline2 = "  time  |";
    }
    if($load){
      $headline1 .= "-----load-avg---- ";
      $headline2 .= "  1m    5m   15m |";
    }
    if($cpu){
      $headline1 .= "---cpu-usage--- ";
      $headline2 .= "usr sys idl iow|";
    }
    if($swap){
      $headline1 .= "---swap--- ";
      $headline2 .= "   si   so|";
    }
    if($net){
      $headline1 .= "----net(B)---- ";
      $headline2 .= "   recv   send|";
    }
    if($disk){
      $headline1 .= "-------------------------io-usage----------------------- ";
      $headline2 .= "   r/s    w/s    rkB/s    wkB/s  queue await svctm \%util|";
    }
    if($com){
      $mysql_headline1 .= "                    -QPS- -TPS-";
      $mysql_headline2 .= "  ins   upd   del    sel   iud|";
    }
    if($innodb_hit){
      $mysql_headline1 .= "         -Hit%- ";
      $mysql_headline2 .= "     lor    hit|";
    }
    if($innodb_rows){
      $mysql_headline1 .= "---innodb rows status--- ";
      $mysql_headline2 .= "  ins   upd   del   read|";
    }
    if($innodb_pages){
      $mysql_headline1 .= "---innodb bp pages status-- ";
      $mysql_headline2 .= "   data   free  dirty flush|";
     }
    if($innodb_data){
      $mysql_headline1 .= "-----innodb data status---- ";
      $mysql_headline2 .= " reads writes  read written|";
    }
    if($innodb_log){
      $mysql_headline1 .= "--innodb log-- ";
      $mysql_headline2 .= "fsyncs written|";
    }
    if($innodb_status){
      $mysql_headline1 .= "  his --log(byte)--  read ---query--- ";
      $mysql_headline2 .= " list uflush  uckpt  view inside  que|";
    }
    if($threads){
      $mysql_headline1 .= "------threads------ ";
      $mysql_headline2 .= " run  con  cre  cac|";
    }
    if($bytes){
      $mysql_headline1 .= "-----bytes---- ";
      $mysql_headline2 .= "   recv   send|";
    }
    if($dbrt){
      $mysql_headline1 .= "--------tcprstat(us)-------- ";
      $mysql_headline2 .= "  count    avg 95-avg 99-avg|";
    }
}

sub print_title {
  #-----> Just to Print
  print $LOG_OUT GREEN();
  print $LOG_OUT <<EOF;

.=================================================.
|       Welcome to use the orzdba tool !          |
|          Yep...Chinese English~                 |
EOF
  print $LOG_OUT GREEN(),"'=============== ";
  print $LOG_OUT RED(),"Date : ",strftime ("%Y-%m-%d", localtime);
  print $LOG_OUT GREEN()," ==============='"."\n\n";
  print $LOG_OUT RESET();
  #<----- Just to print

  # Get Hostname and IP
  chomp (my $hostname = `hostname` );
  my $ip = inet_ntoa((gethostbyname($hostname))[4]);
  print $LOG_OUT RED(), "HOST: ",YELLOW(),$hostname,RED(),"   IP: ",YELLOW(),$ip,RESET(),"\n";

  $TCPRSTAT .= " -l $ip";

  # Get MYSQL DB Name and Variables
  if ($mysql) {
    my $mysqldb_sql = qq{$MYSQL -e 'show databases' | grep -iwvE "information_schema|mysql|test" | tr "\n" "|"};
    my $db_name = `$mysqldb_sql`;
    chop($db_name);
    print $LOG_OUT RED(),"DB  : ",YELLOW(),$db_name,RESET(),"\n";

    # Get MySQL Variables
    my $mysql = qq{$MYSQL -e 'show variables where Variable_name in ("sync_binlog","max_connections","max_user_connections","max_connect_errors","table_open_cache","table_definition_cache","thread_cache_size","binlog_format","open_files_limit","max_binlog_size","max_binlog_cache_size")'};
    open MYSQL_VARIABLES,"$mysql|" or die "Can't connect to mysql!";
    print $LOG_OUT RED(),"Var : ",RESET();
    &print_vars();
    print $LOG_OUT "\n\n      ";

    $mysql = qq{$MYSQL -e 'show variables where Variable_name in ("innodb_flush_log_at_trx_commit","innodb_flush_method","innodb_buffer_pool_size","innodb_max_dirty_pages_pct","innodb_log_buffer_size","innodb_log_file_size","innodb_log_files_in_group","innodb_thread_concurrency","innodb_file_per_table","innodb_adaptive_hash_index","innodb_open_files","innodb_io_capacity","innodb_read_io_threads","innodb_write_io_threads","innodb_adaptive_flushing","innodb_lock_wait_timeout","innodb_log_files_in_group")'};
    open MYSQL_VARIABLES,"$mysql|" or die "Can't connect to mysql!";
    &print_vars();

    sub print_vars {
      my $cnt = 0;
      while (my $line = <MYSQL_VARIABLES>) {
        chomp($line);
        my($key,$value) = split(/\s+/,$line);
        if ($key eq 'innodb_buffer_pool_size' or $key eq 'innodb_log_file_size' or $key eq 'innodb_log_buffer_size' or $key eq 'max_binlog_cache_size' or $key eq 'max_binlog_size' ) {
          print $LOG_OUT MAGENTA(),"$key",WHITE(),"[";
          $value/1024/1024/1024>=1 ? print $LOG_OUT $value/1024/1024/1024,"G" : ($value/1024/1024>1 ? print $LOG_OUT $value/1024/1024,"M" : print $LOG_OUT $value) ;
            print $LOG_OUT "] ",RESET();
        } else {
          print $LOG_OUT MAGENTA(),"$key",WHITE(),"[$value] ",RESET();
        }
        $cnt += 1;
        print $LOG_OUT "\n      " if $cnt%3 == 0;
      }
    }
    close MYSQL_VARIABLES or die "Can't close!";
    print $LOG_OUT "\n";
  }
  print $LOG_OUT "\n";
}

# ----------------------------------------------------------------------------------------
# 3.
# Func : get sys performance info
# ----------------------------------------------------------------------------------------
sub get_sysinfo{
  # 1. Get SysInfo (from /proc/loadavg): Load
  if($load){
    open PROC_LOAD,"</proc/loadavg" or die "Can't open file(/proc/loadavg)!";
    if ( defined (my $line = <PROC_LOAD>) ){
      chomp($line);
      #print $line;
      my @sys_load = split(/\s+/,$line);
      $sys_load[0]>$ncpu ? print $LOG_OUT RED() : print $LOG_OUT WHITE();
      printf $LOG_OUT "%5.2f",$sys_load[0] and print $LOG_OUT RESET();
      $sys_load[1]>$ncpu ? print $LOG_OUT RED() : print $LOG_OUT WHITE();
      printf $LOG_OUT " %5.2f",$sys_load[1] and print $LOG_OUT RESET();
      $sys_load[2]>$ncpu ? print $LOG_OUT RED() : print $LOG_OUT WHITE();
      printf $LOG_OUT " %5.2f",$sys_load[2] and print $LOG_OUT RESET();
      print $LOG_OUT BLUE(),BOLD(),"|",RESET();
    }
    close PROC_LOAD or die "Can't close file(/proc/loadavg)!";
  }
  # 2. Get SysInfo (from /proc/stat): CPU
  if($cpu or $disk) {
    open PROC_CPU,"</proc/stat" or die "Can't open file(/proc/stat)!";
    if ( defined (my $line = <PROC_CPU>) ){ # use "if" instead of "while" to read first line
      chomp($line);
      my @sys_cpu2 = split(/\s+/,$line);
      # line format :     (http://blog.csdn.net/nineday/archive/2007/12/11/1928847.aspx)
      # cpu   1-user  2-nice  3-system 4-idle   5-iowait  6-irq   7-softirq
      # cpu   628808  1642    61861    24978051 22640     349     3086        0
      my $total_2 =$sys_cpu2[1]+$sys_cpu2[2]+$sys_cpu2[3]+$sys_cpu2[4]+$sys_cpu2[5]+$sys_cpu2[6]+$sys_cpu2[7];

      # my $user_diff   = int ( ($sys_cpu2[1] - $sys_cpu1[1]) / ($total_2 - $total_1) * 100 + 0.5 );
      # my $system_diff = int ( ($sys_cpu2[3] - $sys_cpu1[3]) / ($total_2 - $total_1) * 100 + 0.5 );
      # my $idle_diff   = int ( ($sys_cpu2[4] - $sys_cpu1[4]) / ($total_2 - $total_1) * 100 + 0.5 );
      # my $iowait_diff = int ( ($sys_cpu2[5] - $sys_cpu1[5]) / ($total_2 - $total_1) * 100 + 0.5 );
      #printf "%3d %3d %3d %3d",$user_diff,$system_diff,$idle_diff,$iowait_diff;

      $user_diff        = $sys_cpu2[1] + $sys_cpu2[2] - $sys_cpu1[1] - $sys_cpu1[2] ;
      $system_diff = $sys_cpu2[3] + $sys_cpu2[6] + $sys_cpu2[7] - $sys_cpu1[3] - $sys_cpu1[6] - $sys_cpu1[7];
      $idle_diff        = $sys_cpu2[4] - $sys_cpu1[4] ;
      $iowait_diff      = $sys_cpu2[5] - $sys_cpu1[5] ;
      my $user_diff_1   = int ( $user_diff / ($total_2 - $total_1) * 100 + 0.5 );
      my $system_diff_1 = int ( $system_diff / ($total_2 - $total_1) * 100 + 0.5 );
      my $idle_diff_1   = int ( $idle_diff / ($total_2 - $total_1) * 100 + 0.5 );
      my $iowait_diff_1 = int ( $iowait_diff / ($total_2 - $total_1) * 100 + 0.5 );

      if ($cpu) {
        # printf "%3d %3d %3d %3d",$user_diff_1,$system_diff_1,$idle_diff_1,$iowait_diff_1;
        $user_diff_1>10   ? print $LOG_OUT RED() : print $LOG_OUT GREEN();
        printf $LOG_OUT "%3d",$user_diff_1 and print $LOG_OUT RESET();
        $system_diff_1>10   ? print $LOG_OUT RED() : print $LOG_OUT WHITE();
        printf $LOG_OUT " %3d",$system_diff_1 and print $LOG_OUT RESET();
        print $LOG_OUT WHITE() ;
        printf $LOG_OUT " %3d",$idle_diff_1;
        $iowait_diff_1>10 ? print $LOG_OUT RED() : print $LOG_OUT GREEN();
        printf $LOG_OUT " %3d",$iowait_diff_1;
        # if ($iowait_diff_1>10) {
        #       print RED();
        #       printf "%3d",$iowait_diff_1;
        # } else {
        #       print GREEN();
        #       printf "%3d",$iowait_diff_1;
        # }
        print $LOG_OUT BLUE(),BOLD(),"|",RESET();
      }

      # Keep Last Status
      # print @sys_cpu1; print '<->';
      @sys_cpu1 = @sys_cpu2;
      $total_1  = $total_2;
      # print @sys_cpu2;
    }
    close PROC_CPU or die "Can't close file(/proc/stat)!";
  }

  # 3. Get SysInfo (from /proc/vmstat): SWAP
  # Detail Info : http://www.linuxinsight.com/proc_vmstat.html
  if($swap) {
    my %swap2;
    open PROC_VMSTAT,"cat /proc/vmstat | grep -E \"pswpin|pswpout\" |" or die "Can't open file(/proc/vmstat)!";
    while (my $line = <PROC_VMSTAT>) {
      chomp($line);
      my($key,$value) = split(/\s+/,$line);
      $swap2{"$key"}= $value;
    }
    if ($swap_not_first) {
      ($swap2{"pswpin"} - $swap1{"pswpin"})>0  ? print $LOG_OUT RED() : print $LOG_OUT WHITE();
      printf $LOG_OUT " %4d",($swap2{"pswpin"} - $swap1{"pswpin"})/$interval;
      ($swap2{"pswpout"} - $swap1{"pswpout"})>0 ? print $LOG_OUT RED() : print $LOG_OUT WHITE();
      printf $LOG_OUT " %4d",($swap2{"pswpout"} - $swap1{"pswpout"})/$interval;
      print $LOG_OUT BLUE(),BOLD(),"|",RESET();
    } else {
      print $LOG_OUT WHITE();
      printf $LOG_OUT " %4d %4d",0,0;
      print $LOG_OUT BLUE(),BOLD(),"|",RESET();
    }
    close PROC_VMSTAT or die "Can't close file(/proc/vmstat)!";

    # Keep Last Status
    %swap1 = %swap2;
    $swap_not_first += 1;
  }

  # 4. Get SysInfo (from /proc/net/dev): NET
  if($net) {
    open PROC_NET,"cat /proc/net/dev | grep \"\\b$net\\b\" | " or die "Can't open file(/proc/net/dev)!";
    if ( defined (my $line = <PROC_NET>) ) {
      chomp($line);
      my @net = split(/\s+|:/,$line);
      my %net2 = (
        "recv" => $net[2],
        "send" => $net[10]
      );
      #print "$net2{recv},$net2{send},$net1{recv},$net1{send}";
      if ($net_not_first) {
        #print join('*',@net);
        my $diff_recv = ( $net2{"recv"} - $net1{"recv"} ) / $interval;
        my $diff_send = ( $net2{"send"} - $net1{"send"} ) / $interval;

        $diff_recv/1024/1024 > 1 ? print $LOG_OUT RED() : print $LOG_OUT WHITE();
        $diff_recv/1024/1024 > 1 ?
          printf $LOG_OUT "%6.1fm",($diff_recv/1024/1024):
          printf $LOG_OUT "%7s",($diff_recv/1024 > 1 ? int($diff_recv/1024 + 0.5)."k":$diff_recv);
        print $LOG_OUT RESET();

        $diff_send/1024/1024 > 1 ? print $LOG_OUT RED() : print $LOG_OUT WHITE();
        $diff_send/1024/1024 > 1 ?
           printf $LOG_OUT "%6.1fm",($diff_send/1024/1024):
           printf $LOG_OUT "%7s",($diff_send/1024 > 1 ? int($diff_send/1024 + 0.5)."k":$diff_send);
        print $LOG_OUT RESET();

        print $LOG_OUT BLUE(),BOLD(),"|",RESET();
      } else {
        print $LOG_OUT WHITE();
        printf $LOG_OUT " %6d %6d",0,0;
        print $LOG_OUT BLUE(),BOLD(),"|",RESET();
      }
      close PROC_NET or die "Can't close file(/proc/net/dev)!";

      # Keep Last Status
      %net1 = %net2;
      $net_not_first += 1;

    } else {
      print $LOG_OUT RED();
      print $LOG_OUT "\nERROR! Please set the right net info!\n";
      print $LOG_OUT RESET();
      exit;
    }

  }

  # 5. Get SysInfo (from /proc/diskstats): IO
  if($disk) {
    # Detail IO Info :
    # (1) http://www.mjmwired.net/kernel/Documentation/iostats.txt
    # (2) http://www.linuxinsight.com/iostat_utility.html
    # (3) source code --> http://www.linuxinsight.com/files/iostat-2.2.tar.gz
    my $deltams = 1000.0 * ( $user_diff + $system_diff + $idle_diff + $iowait_diff ) / $ncpu / $HZ ;
    # Shell Command : cat /proc/diskstats  | grep "\bsda\b"
    open PROC_IO,"cat /proc/diskstats  | grep \"\\b$disk\\b\" |" or die "Can't open file(/proc/diskstats)!";
    if ( defined (my $line = <PROC_IO>) ) {
      chomp($line);
      # iostat --> line format :
      # 0               1        2        3     4      5        6     7        8          9      10     11
      # Device:         rrqm/s   wrqm/s   r/s   w/s    rkB/s    wkB/s avgrq-sz avgqu-sz   await  svctm  %util
      # sda               0.05    12.44  0.42  7.60     5.67    80.15    21.42     0.04    4.63   0.55   0.44
      my @sys_io2 = split(/\s+/,$line);

      my $rd_ios     = $sys_io2[4]  - $sys_io1[4];  #/* Read I/O operations */
      my $rd_merges  = $sys_io2[5]  - $sys_io1[5];  #/* Reads merged */
      my $rd_sectors = $sys_io2[6]  - $sys_io1[6];  #/* Sectors read */
      my $rd_ticks   = $sys_io2[7]  - $sys_io1[7];  #/* Time in queue + service for read */
      my $wr_ios     = $sys_io2[8]  - $sys_io1[8];  #/* Write I/O operations */
      my $wr_merges  = $sys_io2[9]  - $sys_io1[9];  #/* Writes merged */
      my $wr_sectors = $sys_io2[10] - $sys_io1[10]; #/* Sectors written */
      my $wr_ticks   = $sys_io2[11] - $sys_io1[11]; #/* Time in queue + service for write */
      my $ticks      = $sys_io2[13] - $sys_io1[13]; #/* Time of requests in queue */
      my $aveq       = $sys_io2[14] - $sys_io1[14]; #/* Average queue length */

      my $n_ios;        #/* Number of requests */
      my $n_ticks;      #/* Total service time */
      my $n_kbytes;     #/* Total kbytes transferred */
      my $busy;         #/* Utilization at disk       (percent) */
      my $svc_t;        #/* Average disk service time */
      my $wait;         #/* Average wait */
      my $size;         #/* Average request size */
      my $queue;        #/* Average queue */
      $n_ios    = $rd_ios + $wr_ios;
      $n_ticks  = $rd_ticks + $wr_ticks;
      $n_kbytes = ( $rd_sectors + $wr_sectors) / 2.0;
      $queue    = $aveq/$deltams;
      $size     = $n_ios ? $n_kbytes / $n_ios : 0.0;
      $wait     = $n_ios ? $n_ticks / $n_ios : 0.0;
      $svc_t    = $n_ios ? $ticks / $n_ios : 0.0;
      $busy     = 100.0 * $ticks / $deltams;  #/* percentage! */
      if ($busy > 100.0) {
        $busy = 100.0;
      }
      #
      my $rkbs     = (1000.0 * $rd_sectors/$deltams /2) ;
      my $wkbs     = (1000.0 * $wr_sectors/$deltams /2) ;

      # r/s  w/s
      my $rd_ios_s = (1000.0 * $rd_ios/$deltams) ;
      my $wr_ios_s = (1000.0 * $wr_ios/$deltams) ;

      # printf "%7.1f %7.1f %5.1f %6.1f %5.1f %5.1f",$rkbs,$wkbs,$queue,$wait,$svc_t,$busy ;
      # color print wait/svc_t/busy info
      print $LOG_OUT WHITE();
      printf $LOG_OUT "%7.1f%7.1f",$rd_ios_s,$wr_ios_s and print $LOG_OUT RESET();

      $rkbs > 1024 ? print $LOG_OUT RED() : print $LOG_OUT WHITE();
      printf $LOG_OUT "%8.1f",$rkbs and print $LOG_OUT RESET();
      $wkbs > 1024 ? print $LOG_OUT RED() : print $LOG_OUT WHITE();
      printf $LOG_OUT " %8.1f",$wkbs and print $LOG_OUT RESET();
      print $LOG_OUT WHITE() ;
      printf $LOG_OUT " %5.1f",$queue;
      $wait>5  ? print $LOG_OUT RED() : print $LOG_OUT GREEN();
      printf $LOG_OUT " %6.1f",$wait and print $LOG_OUT RESET();
      $svc_t>5 ? print $LOG_OUT RED() : print $LOG_OUT WHITE();
      printf $LOG_OUT " %5.1f",$svc_t and print $LOG_OUT RESET();
      $busy>80 ? print $LOG_OUT RED() : print $LOG_OUT GREEN();
      printf $LOG_OUT " %5.1f",$busy and print $LOG_OUT RESET();
      print $LOG_OUT BLUE(),BOLD(),"|",RESET();

      # Keep Last Status
      @sys_io1 = @sys_io2;

      close PROC_IO or die "Can't close file(/proc/diskstats)!";
    } else {
      print $LOG_OUT RED();
      print $LOG_OUT "\nERROR! Please set the right disk info!\n";
      print $LOG_OUT RESET();
      exit;
    }
  }

  # END !
}


# ----------------------------------------------------------------------------------------
# 4.
# Func : get mysql status
# ----------------------------------------------------------------------------------------
sub get_mysqlstat{
  if($mysql) {
    my %mystat2 ;
    my $mysql = qq{$MYSQL -e 'show global status where Variable_name in ("Com_select","Com_insert","Com_update","Com_delete","Innodb_buffer_pool_read_requests","Innodb_buffer_pool_reads","Innodb_rows_inserted","Innodb_rows_updated","Innodb_rows_deleted","Innodb_rows_read","Threads_running","Threads_connected","Threads_cached","Threads_created","Bytes_received","Bytes_sent","Innodb_buffer_pool_pages_data","Innodb_buffer_pool_pages_free","Innodb_buffer_pool_pages_dirty","Innodb_buffer_pool_pages_flushed","Innodb_data_reads","Innodb_data_writes","Innodb_data_read","Innodb_data_written","Innodb_os_log_fsyncs","Innodb_os_log_written")'};
    #print YELLOW(),$mysql,RESET();
    open MYSQL_STAT,"$mysql|" or die "Can't connect to mysql!";
    while (my $line = <MYSQL_STAT>) {
      chomp($line);
      my($key,$value) = split(/\s+/,$line);
      $mystat2{"$key"}=$value;
    }
    close MYSQL_STAT or die "Can't close!";

    if ($not_first) {
      my $insert_diff = ( $mystat2{"Com_insert"} - $mystat1{"Com_insert"} ) / $interval;
      my $update_diff = ( $mystat2{"Com_update"} - $mystat1{"Com_update"} ) / $interval;
      my $delete_diff = ( $mystat2{"Com_delete"} - $mystat1{"Com_delete"} ) / $interval;
      my $select_diff = ( $mystat2{"Com_select"} - $mystat1{"Com_select"} ) / $interval;
      my $read_request = ( $mystat2{"Innodb_buffer_pool_read_requests"} - $mystat1{"Innodb_buffer_pool_read_requests"} ) / $interval;
      my $read         = ( $mystat2{"Innodb_buffer_pool_reads"} - $mystat1{"Innodb_buffer_pool_reads"} ) / $interval;

      my $innodb_rows_inserted_diff = ( $mystat2{"Innodb_rows_inserted"} - $mystat1{"Innodb_rows_inserted"} ) / $interval;
      my $innodb_rows_updated_diff  = ( $mystat2{"Innodb_rows_updated"}  - $mystat1{"Innodb_rows_updated"}  ) / $interval;
      my $innodb_rows_deleted_diff  = ( $mystat2{"Innodb_rows_deleted"}  - $mystat1{"Innodb_rows_deleted"}  ) / $interval;
      my $innodb_rows_read_diff     = ( $mystat2{"Innodb_rows_read"}     - $mystat1{"Innodb_rows_read"}     ) / $interval;

      my $innodb_bp_pages_flushed_diff= ( $mystat2{"Innodb_buffer_pool_pages_flushed"} - $mystat1{"Innodb_buffer_pool_pages_flushed"} ) / $interval;

      my $innodb_data_reads_diff    = ( $mystat2{"Innodb_data_reads"}    - $mystat1{"Innodb_data_reads"}     ) / $interval;
      my $innodb_data_writes_diff   = ( $mystat2{"Innodb_data_writes"}   - $mystat1{"Innodb_data_writes"}    ) / $interval;
      my $innodb_data_read_diff     = ( $mystat2{"Innodb_data_read"}     - $mystat1{"Innodb_data_read"}      ) / $interval;
      my $innodb_data_written_diff  = ( $mystat2{"Innodb_data_written"}  - $mystat1{"Innodb_data_written"}   ) / $interval;

      my $innodb_os_log_fsyncs_diff = ( $mystat2{"Innodb_os_log_fsyncs"} - $mystat1{"Innodb_os_log_fsyncs"}  ) / $interval;
      my $innodb_os_log_written_diff= ( $mystat2{"Innodb_os_log_written"}- $mystat1{"Innodb_os_log_written"} ) / $interval;

      my $threads_created_diff      = ( $mystat2{"Threads_created"}      - $mystat1{"Threads_created"}      ) / $interval;

      my $bytes_received_diff       = ( $mystat2{"Bytes_received"}       - $mystat1{"Bytes_received"}       ) / $interval;
      my $bytes_sent_diff           = ( $mystat2{"Bytes_sent"}           - $mystat1{"Bytes_sent"}           ) / $interval;

      if ($com) {
        # Com_insert # Com_update # Com_delete
        print $LOG_OUT WHITE();
        printf $LOG_OUT "%5d %5d %5d",$insert_diff,$update_diff,$delete_diff;
        print $LOG_OUT YELLOW();
        # Com_select
        printf $LOG_OUT " %6d",$select_diff;
        # Total TPS
        printf $LOG_OUT " %5d",$insert_diff+$update_diff+$delete_diff;
        print $LOG_OUT GREEN(),"|",RESET();
      }

      if ($innodb_hit) {
        # Innodb_buffer_pool_read_requests
        print $LOG_OUT WHITE();
        printf $LOG_OUT " %7d",$read_request;
        # Hit% : (Innodb_buffer_pool_read_requests - Innodb_buffer_pool_reads) / Innodb_buffer_pool_read_requests * 100%
        if ($read_request) {
          my $hit = ($read_request-$read)/$read_request*100;
          $hit>99 ? print $LOG_OUT GREEN() : print $LOG_OUT RED();
          printf $LOG_OUT " %6.2f",$hit;
        } else {
          print $LOG_OUT GREEN()," 100.00";
        }
        print $LOG_OUT GREEN(),"|",RESET();
      }

      if ($innodb_rows) {
        # Innodb_rows_inserted,Innodb_rows_updated,Innodb_rows_deleted,Innodb_rows_read
        print $LOG_OUT WHITE();
        printf $LOG_OUT "%5d %5d %5d %6d",$innodb_rows_inserted_diff,$innodb_rows_updated_diff,$innodb_rows_deleted_diff,$innodb_rows_read_diff;
        print $LOG_OUT GREEN(),"|",RESET();
      }

      if ($innodb_pages) {
        # Innodb_buffer_pool_pages_data/free/dirty/flushed
        print $LOG_OUT WHITE();
        printf $LOG_OUT "%7d %6d ",$mystat2{"Innodb_buffer_pool_pages_data"},$mystat2{"Innodb_buffer_pool_pages_free"};
        print $LOG_OUT YELLOW();
        printf $LOG_OUT "%6d %5d",$mystat2{"Innodb_buffer_pool_pages_dirty"},$innodb_bp_pages_flushed_diff;
        print $LOG_OUT GREEN(),"|",RESET();
      }

      if ($innodb_data) {
        # Innodb_data_reads/writes/read/written
        print $LOG_OUT WHITE();
        printf $LOG_OUT "%6d %6d ",$innodb_data_reads_diff,$innodb_data_writes_diff;

        $innodb_data_read_diff/1024/1024 > 9 ? print $LOG_OUT RED() : print $LOG_OUT WHITE();
        $innodb_data_read_diff/1024/1024 > 1 ?
        printf $LOG_OUT "%5.1fm",($innodb_data_read_diff/1024/1024):
        printf $LOG_OUT "%6s",($innodb_data_read_diff/1024 > 1 ? int($innodb_data_read_diff/1024 + 0.5)."k":$innodb_data_read_diff);

        $innodb_data_written_diff/1024/1024 > 9 ? print $LOG_OUT RED() : print $LOG_OUT WHITE();
        $innodb_data_written_diff/1024/1024 > 1 ?
        printf $LOG_OUT " %5.1fm",($innodb_data_written_diff/1024/1024):
        printf $LOG_OUT " %6s",($innodb_data_written_diff/1024 > 1 ? int($innodb_data_written_diff/1024 + 0.5)."k":$innodb_data_written_diff);

        print $LOG_OUT GREEN(),"|",RESET();
      }

      if ($innodb_log) {
        # Innodb_os_log_fsyncs/written
        print $LOG_OUT WHITE();
        printf $LOG_OUT "%6d ",$innodb_os_log_fsyncs_diff;

        $innodb_os_log_written_diff/1024/1024 > 1 ? print $LOG_OUT RED() : print $LOG_OUT YELLOW();
        $innodb_os_log_written_diff/1024/1024 > 1 ?
        printf $LOG_OUT "%6.1fm",($innodb_os_log_written_diff/1024/1024):
        printf $LOG_OUT "%7s",($innodb_os_log_written_diff/1024 > 1 ? int($innodb_os_log_written_diff/1024 + 0.5)."k":$innodb_os_log_written_diff);

        print $LOG_OUT GREEN(),"|",RESET();
     }

     if ($innodb_status) {
       my %innodb_status = &get_innodb_status();
       print $LOG_OUT WHITE();
       printf $LOG_OUT "%5d ",$innodb_status{"history_list"};

       print $LOG_OUT YELLOW();
       $innodb_status{"unflushed_log"}/1024/1024 > 1 ?
       printf $LOG_OUT "%5.1fm ",($innodb_status{"unflushed_log"}/1024/1024):
       printf $LOG_OUT "%6s ",($innodb_status{"unflushed_log"}/1024 > 1 ? int($innodb_status{"unflushed_log"}/1024 + 0.5)."k":$innodb_status{"unflushed_log"});
       $innodb_status{"uncheckpointed_bytes"}/1024/1024 > 1 ?
       printf $LOG_OUT "%6.1fm",($innodb_status{"uncheckpointed_bytes"}/1024/1024):
       printf $LOG_OUT "%7s",($innodb_status{"uncheckpointed_bytes"}/1024 > 1 ? int($innodb_status{"uncheckpointed_bytes"}/1024 + 0.5)."k":$innodb_status{"uncheckpointed_bytes"});

       print $LOG_OUT WHITE();
       printf $LOG_OUT "%5d %5d %5d",$innodb_status{"read_views"},$innodb_status{"queries_inside"},$innodb_status{"queries_queued"};

       print $LOG_OUT GREEN(),"|",RESET();
     }

     if ($threads) {
       print $LOG_OUT WHITE();
       printf $LOG_OUT "%4d %4d %4d %4d",$mystat2{"Threads_running"},$mystat2{"Threads_connected"},$threads_created_diff,$mystat2{"Threads_cached"};
       print $LOG_OUT GREEN(),"|",RESET();
     }

     if ($bytes) {
       print $LOG_OUT WHITE();
       $bytes_received_diff/1024/1024 > 1 ?
       printf $LOG_OUT "%6.1fm",($bytes_received_diff/1024/1024):
       printf $LOG_OUT "%7s",($bytes_received_diff/1024 > 1 ? int($bytes_received_diff/1024 + 0.5)."k":$bytes_received_diff);
       $bytes_sent_diff/1024/1024 > 1 ?
       printf $LOG_OUT "%6.1fm",($bytes_sent_diff/1024/1024):
       printf $LOG_OUT "%7s",($bytes_sent_diff/1024 > 1 ? int($bytes_sent_diff/1024 + 0.5)."k":$bytes_sent_diff);
       print $LOG_OUT GREEN(),"|",RESET();
     }

   } else{
     if ($com) {
       print $LOG_OUT WHITE();
       printf $LOG_OUT "%5d %5d %5d %6d %5d",0,0,0,0,0;
       print $LOG_OUT GREEN(),"|",RESET();
     }

     if ($innodb_hit) {
       print $LOG_OUT WHITE();
       printf $LOG_OUT " %7d %6.2f",0,100;
       print $LOG_OUT GREEN(),"|",RESET();
     }

     if ($innodb_rows) {
       print $LOG_OUT WHITE();
       printf $LOG_OUT "%5d %5d %5d %6d",0,0,0,0;
       print $LOG_OUT GREEN(),"|",RESET();
     }

     if ($innodb_pages) {
       print $LOG_OUT WHITE();
       printf $LOG_OUT "%7d %6d %6d %5d",0,0,0,0;
       print $LOG_OUT GREEN(),"|",RESET();
     }

     if ($innodb_data) {
       print $LOG_OUT WHITE();
       printf $LOG_OUT "%6d %6d %6d %6d",0,0,0,0;
       print $LOG_OUT GREEN(),"|",RESET();
     }

     if ($innodb_log) {
       print $LOG_OUT WHITE();
       printf $LOG_OUT "%6d %7d",0,0;
       print $LOG_OUT GREEN(),"|",RESET();
     }

     if ($innodb_status) {
       print $LOG_OUT WHITE();
       printf $LOG_OUT "%5d %6d %6d %5d %5d %5d",0,0,0,0,0,0;
       print $LOG_OUT GREEN(),"|",RESET();
     }

     if ($threads) {
       print $LOG_OUT WHITE();
       printf $LOG_OUT "%4d %4d %4d %4d",0,0,0,0;
       print $LOG_OUT GREEN(),"|",RESET();
     }

      if ($bytes) {
        print $LOG_OUT WHITE();
        printf $LOG_OUT " %6d %6d",0,0;
        print $LOG_OUT GREEN(),"|",RESET();
      }
    }

    # Keep Last Status
    %mystat1 = %mystat2;
    $not_first += 1;
  }
}


# ----------------------------------------------------------------------------------------
# 5.
# Func : get db rt
# ----------------------------------------------------------------------------------------
sub get_dbrt {
  eval { require File::Lockfile;  };
  if ($@) {
    print $LOG_OUT RED(),"\n\n[ERROR] need File::Lockfile !\n",RESET();
    exit;
  }

  $tcprstat_lck = "orzdba_tcprstat.$$";
  my $lockfile = File::Lockfile->new($tcprstat_lck,"$tcprstat_dir");
  if ($lockfile->check) {
    open TCPRSTAT_LOG,"tail -n 1 $tcprstat_dir/$tcprstat_log | " or die "Can't open file $tcprstat_log!";

    my ($timestamp,$count,$max,$min,$avg,$med,$stddev,$max_95,$avg_95,$std_95,$max_99,$avg_99,$std_99) ;
    while (my $line = <TCPRSTAT_LOG>) {
      chomp($line);
      ($timestamp,$count,$max,$min,$avg,$med,$stddev,$max_95,$avg_95,$std_95,$max_99,$avg_99,$std_99) = split(/\s+/,$line);

      print $LOG_OUT WHITE() ;
      printf $LOG_OUT " %6d",$count;

      # $avg $avg_95 $avg_99;
      $avg >10000 ? print $LOG_OUT RED() : print $LOG_OUT GREEN();
      printf $LOG_OUT " %6d",$avg and print $LOG_OUT RESET();
      $avg_95 >10000 ? print $LOG_OUT RED() : print $LOG_OUT GREEN();
      printf $LOG_OUT " %6d",$avg_95 and print $LOG_OUT RESET();
      $avg_99 >10000 ? print $LOG_OUT RED() : print $LOG_OUT GREEN();
      printf $LOG_OUT " %6d",$avg_99 and print $LOG_OUT RESET();

      print $LOG_OUT GREEN(),"|",RESET();
    }

    close TCPRSTAT_LOG or die "Can't close! $!";

    unless (defined($timestamp)) {
      print $LOG_OUT WHITE() ;
      printf $LOG_OUT " %6d %6d %6d %6d",0,0,0,0 and print $LOG_OUT RESET();
      print $LOG_OUT GREEN(),"|",RESET();
    }
  } else {
    $lockfile->write();

    #        local $SIG{CHLD} = 'IGNORE';
    defined ( my $pid = fork() ) or die "Can't fork: $!\n";
    unless ($pid) {
      my $tcprstat = qq{$TCPRSTAT > $tcprstat_dir/orzdba_tcprstat.$$.log};
      exec($tcprstat);
      exit;
    }
    $tcprstat_log = "orzdba_tcprstat.$pid.log";

    print $LOG_OUT WHITE() ;
    printf $LOG_OUT " %6d %6d %6d %6d",0,0,0,0 and print $LOG_OUT RESET();
    print $LOG_OUT GREEN(),"|",RESET();

    #waitpid($pid,0);
    #$lockfile->remove;
  }
}


# ----------------------------------------------------------------------------------------
# 6.
# Func : remove logfile of tcprstat
# ----------------------------------------------------------------------------------------
sub rm_logfile {
  my ($file) = @_;
  if ( -e $file) {
    #print "rm $file\n";
    unlink $file ;
  }
}


# ----------------------------------------------------------------------------------------
# 7.
# Func : Get Innodb Status from Command: 'Show Engine Innodb Status'
# ----------------------------------------------------------------------------------------
sub get_innodb_status {
  my $mysql = qq{ $MYSQL -e 'show engine innodb status'};
  open MYSQL_STAT,"$mysql|" or die "Can't connect to mysql!";
  my @result ;
  my %innodb_status;
  my $line = <MYSQL_STAT> ;
  @result = split(/\\n/,$line);
  close MYSQL_STAT or die "Can't close!";

  # http://code.google.com/p/mysql-cacti-templates/source/browse/trunk/scripts/ss_get_mysql_stats.php
  foreach (@result) {
    # ------------
    # TRANSACTIONS
    # ------------
    # Trx id counter 64AFBCC1B
    # Purge done for trx's n:o < 64AFBCAD4 undo n:o < 0
    # History list length 23
    if ( index($_,"History list length") != -1) {
      my @tmp = split(/\s+/,$_);
      $innodb_status{"history_list"} = $tmp[3];
    }
    # ---
    # LOG
    # ---
    # Log sequence number 6712509083974
    # Log flushed up to   6712508972870
    # Last checkpoint at  6709615343735
    # 0 pending log writes, 0 pending chkp writes
    # 2556962847 log i/o's done, 509.12 log i/o's/second
    elsif ( index($_,"Log sequence number") != -1 ) {
      my @tmp = split(/\s+/,$_);
      $innodb_status{"log_bytes_written"} = $tmp[3];
    } elsif ( index($_,"Log flushed up to") != -1 ) {
      my @tmp = split(/\s+/,$_);
      $innodb_status{"log_bytes_flushed"} = $tmp[4];
    } elsif ( index($_,"Last checkpoint at") != -1 ) {
      my @tmp = split(/\s+/,$_);
      $innodb_status{"last_checkpoint"} = $tmp[3];
    }
    # --------------
    # ROW OPERATIONS
    # --------------
    # 2 queries inside InnoDB, 0 queries in queue
    # 2 read views open inside InnoDB
    # Main thread process no. 7969, id 1191348544, state: sleeping
    # Number of rows inserted 287921794, updated 733493588, deleted 30775703, read 2351464150250
    # 5.10 inserts/s, 29.38 updates/s, 0.02 deletes/s, 51322.87 reads/s
    elsif ( index($_,"queries inside InnoDB") != -1 ) {
      my @tmp = split(/\s+/,$_);
      $innodb_status{"queries_inside"} = $tmp[0];
      $innodb_status{"queries_queued"} = $tmp[4];
    } elsif ( index($_,"read views open inside InnoDB") != -1 ) {
      my @tmp = split(/\s+/,$_);
      $innodb_status{"read_views"} = $tmp[0];
    }

    # elsif ( index($_,"") != -1 ) {
    #        my @tmp = split(/\s+/,$_);
    #        $innodb_status{""} = $tmp[3];
    # }
  }
  $innodb_status{"unflushed_log"}        = $innodb_status{"log_bytes_written"} - $innodb_status{"log_bytes_flushed"} ;
  $innodb_status{"uncheckpointed_bytes"} = $innodb_status{"log_bytes_written"} - $innodb_status{"last_checkpoint"};

  return %innodb_status;
}
