cmake_minimum_required(VERSION 3.16)
project(lab1_2)

set(CMAKE_CXX_STANDARD 14)

add_executable(lab1_2 main.cpp Daemon.cpp Logger.cpp ConfigLoader.cpp SystemSignalsHandler.cpp)