cmake_minimum_required(VERSION 2.8)
project(udpgen)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -g3")

set(SOURCE_FILES main.cpp rate_limiter.cpp udp_generator.cpp syslog_generator.cpp trap_generator.cpp netflow5_generator.cpp)
add_executable(udpgen ${SOURCE_FILES})
target_link_libraries(udpgen netsnmp netsnmpagent netsnmphelpers pthread)
