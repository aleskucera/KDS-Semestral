cmake_minimum_required(VERSION 3.23)
project(KDS_Semestral C)

set(CMAKE_C_STANDARD 11)

add_executable(Server server.c image.h image.c communication.c communication.h crc.c crc.h)
add_executable(Client client.c image.h image.c communication.c communication.h crc.c crc.h)
add_executable(Test test.c test.c communication.h communication.c crc.c crc.h)