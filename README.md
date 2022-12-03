# KDS Semestral Project

## Description

This project is a simple implementation of a file transfer protocol built on top of UDP.

## Building

To build the project run following commands in the root directory of the project:

    cmake -Bbuild -H.
    cmake --build build --target all

## Running

To run the project run following commands in the separate terminals:

#### NetDerper:

    ./NetDerper/NetDerper.CLI

#### Sender:

    cd ./build
    ./Sender

#### Receiver:

    cd ./build
    ./Receiver

