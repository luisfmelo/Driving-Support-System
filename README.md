# Driving-Support-System
SEAI 2016/2017

## permission to access serial port (linux)
sudo usermod -a -G dialout <username>
sudo chmod a+rw /dev/tty*

## Compile library
g++ -c lib_equipaD.cpp

## Compile & Execute TCP server
g++ server.cpp -lpthread -o server
./server <port>

## Compile & Execute TCP client
### C++ TCP client
g++ client.cpp -o client
### C++ with serial port connection TCP client
g++ serial_client.cpp lib_equipaD.o -o client
./client <host> <port>

