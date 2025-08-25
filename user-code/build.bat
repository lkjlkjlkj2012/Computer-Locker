::compile your key file
g++ -c key.cpp -o build/key.o --std=c++11
::link to make an executable file
g++ build/locker.o build/key.o build/resources.o -o build/locker.exe -mwindows