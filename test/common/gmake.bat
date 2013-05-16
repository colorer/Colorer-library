gcc -I../../shared/ -g -c deps.cpp
gcc -I../../shared/ -g -c test-vector.cpp
gcc -g deps.o test-vector.o -lstdc++ -o test-vector.exe