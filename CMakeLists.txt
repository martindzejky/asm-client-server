project(AsmServerClient)


# meta
cmake_minimum_required(VERSION 3.6)
set(CMAKE_C_STANDARD 11)


# source files
file(GLOB SOURCE_FILES *.c)


# build executable
add_executable(AsmServerClient ${SOURCE_FILES})
