cmake_minimum_required(VERSION 3.10)
project(Server)

set(CMAKE_CXX_STANDARD 17)

include_directories(${CMAKE_SOURCE_DIR}/src)

add_library(Server_lib
	${CMAKE_SOURCE_DIR}/src/Server_lib.cpp)
set_target_properties(Server_lib PROPERTIES
    PUBLIC_HEADER ${CMAKE_SOURCE_DIR}/src/Server_lib.hpp)

add_executable(Run_Server
		${CMAKE_SOURCE_DIR}/src/main.cpp)
		
target_link_libraries(Run_Server
	PRIVATE
        Server_lib)

