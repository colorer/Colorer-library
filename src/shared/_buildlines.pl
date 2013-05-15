
open F, ">makefile.lines";

traverse('.');

print F "\n\n\n\n";

traverse('../zlib');

close F;


sub traverse
{
    my $dir = shift;
    my @list = `ls $dir`;
    foreach $entry (@list){
        chomp $entry;
        traverse($dir.'/'.$entry) if (-d $dir.'/'.$entry);
        next unless ($entry =~ /^(.*)\.(c|cpp)$/);
        if (-f $dir."/".$entry){
           print F <<X
\$(obj_output_slash)$1.\$(obj):
	\$(CPP) \$(CPPFLAGS) \$(shared_path)/$dir/$entry

X
        }
    }

}

