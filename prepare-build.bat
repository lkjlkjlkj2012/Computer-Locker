chdir user-code
mkdir build
chdir ..
::compile resources
windres resources.rc -o user-code/build/resources.o
::compile core code
g++ -c locker.cpp -o user-code/build/locker.o --std=c++11
