# Purpose:
#    Simple search with Perl for asserting that a pattern is not in some file.
# 
# Created: 2011-11-11 mleich
# Modified: 2014-04-26 hvadodar
# Modified: 2021-01-04 mortyzhou
#

perl;
    use strict;
    my $search_file=           $ENV{'SEARCH_FILE'}           or die "SEARCH_FILE not set";
    my $search_pattern=        $ENV{'SEARCH_PATTERN'}        or die "SEARCH_PATTERN not set";
    my $file_content=          "";
    my $found=                  0;
    open(FILE, "$search_file") or die("Unable to open '$search_file': $!\n");
    while ( read(FILE, $file_content, 50000, 0) ) {
          if ( $file_content =~ m{$search_pattern} ) {
             $found= 1;
             last;
          }
    }
    close(FILE);
    if ( ($found == 1) || ($file_content =~ m{$search_pattern}) ) {
       die("# ERROR: The file '$search_file' contains the unexpected pattern  $search_pattern\n->$file_content<-\n");
    }
EOF
