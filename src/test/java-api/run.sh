export LPATH=../../../../eclipsecolorer/libnative

java -Djava.library.path=$LPATH -cp .:../../../../eclipsecolorer/colorer.jar Test
