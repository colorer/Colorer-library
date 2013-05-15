#!/usr/bin/perl
print "Content-type: text/html\n\n";
system "colorer -h $ENV{PATH_TRANSLATED}";
