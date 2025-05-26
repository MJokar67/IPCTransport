# IPCTransport
This article attempts to demonstrate a modern solution for utilizing IPC in the C++ language.

## How to build

cd IPCTransport

git submodule update --init --recursive

cmake -B build -DCMAKE_BUILD_TYPE=Debug

cmake --Build build

## Run tests

cd build

./tests/ipc_tests 

