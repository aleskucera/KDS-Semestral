cmake_minimum_required(VERSION 3.23)
project(KDS_Semestral C)

set(CMAKE_C_STANDARD 11)

add_executable(Sender sender.c communication.c communication.h crc.c crc.h sha256.c sha256.h utils.c utils.h main.h)
add_executable(Receiver receiver.c communication.c communication.h crc.c crc.h sha256.c sha256.h utils.c utils.h main.h)
add_executable(Test test.c sha256.h sha256.c)