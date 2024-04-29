# 设置cmake最小版本
cmake_minimum_required(VERSION 3.6)

# 设置项目名称
project(Tcp_Relay)

# 设置C++标准11
set(CMAKE_CXX_STANDARD 11)

# 设置输出compile_commands.json
set(CMAKE_EXPORT_COMPILE_COMMANDS on)

include_directories(include)

# set(SOURCE_FILES_TCPSERVER 
#     src/mainreactor.cpp
#     src/subreactor.cpp
#     src/TcpServer.cpp
#     src/sock_item.cpp
# )

# add_library(TcpServer STATIC ${SOURCE_FILES_TCPSERVER})

add_library(Timer STATIC src/timer.cpp)

add_executable(server server.cpp src/RelayServer.cpp src/sock_item.cpp)

target_link_libraries(server Timer)

# 设置gprof选项
# set_target_properties(server PROPERTIES COMPILE_FLAGS "-pg" LINK_FLAGS "-pg")

add_library(TcpClient STATIC src/TcpClient.cpp)

add_executable(client client.cpp)

target_link_libraries(client TcpClient Timer)
