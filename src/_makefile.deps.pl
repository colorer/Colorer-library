
open DEPS, ">_dependencies.dep";

foreach(`ls *.d`){

  foreach(`less $_`){
    s/^(.*?\.)o/\$\(obj_output_slash\)$1\$\(obj\)/g;
    s/\.o/\.\$\(obj\)/g;
    s/(\w\:|\/cygdrive\/\w)?\/usr.*?(\s|$)/ /g;
    s/^\s*\\\s*$//;
    s/\.\.\/shared\/..\/zlib\//\$(unzipdir)/g;
    s/\.\.\/shared\//\$(shared_path)/g;
    print DEPS $_;
  };

  print STDERR $_;
  print DEPS "\n\n";

};
close DEPS;