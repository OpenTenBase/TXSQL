
# be very specific with the regex, we have got a format to prove:
--replace_regex  /The io time taken by this query \([0-9]* us\)/The io time taken by this query ( us)/   /The time taken to write the network this time\([0-9]{0,10} us\)/The time taken to write the network this time( us)/  /mysql version is [0-9]\.[0-9]\.[0-9][0-9]/mysql version is  /  /txsql version is [0-9]{8}/txsql version is  /    /send signal process id is [0-9]+/send signal process id is:  /  /send signal user id is [0-9]+/send signal user id is:  /   /[0-9][0-9][0-9][0-9]-[0-9][0-9]-[0-9][0-9]T[0-9][0-9]:[0-9][0-9]:[0-9][0-9]\.[0-9][0-9][0-9][0-9][0-9][0-9][-+Z][0-9:]* *[0-9]* *\[/DATE_TIME [/ /thread: [0-9]*,/thread:    #,/ /Needed [0-9][0-9]* bytes/Needed N bytes/ 

perl;
use strict;
use File::stat;
my $file= $ENV{'GREP_FILE'} or die("grep file not set");

my $pattern= $ENV{'GREP_PATTERN'} or die("pattern is not set");
my $stime= $ENV{'GREP_START'} or die("start time is not set");
my $max_line= $ENV{'GREP_MAX_LINE'} or die("grep line number is not set");
my $line_number= 0;

open(FILE, "$file") or die("Unable to open $file: $!");
while (<FILE>) {
  my $line = $_;
  if ($line =~ /$pattern/) {
    $line =~ /([0-9][0-9][0-9][0-9])-([0-9][0-9])-([0-9][0-9])T([0-9][0-9]):([0-9][0-9]):([0-9][0-9])\.([0-9][0-9][0-9][0-9][0-9][0-9])[-+Z][0-9:]* *[0-9]* *?(\[.*)/;
    my $ts=$1.$2.$3.$4.$5.$6.$7;
    my $tx=$8;

    if ($ts >= $stime and $line_number < $max_line) {
      print "$line";
      $line_number+= 1;
    }
  }
}
close(FILE);

EOF
