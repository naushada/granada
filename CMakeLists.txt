cmake_minimum_required(VERSION 3.16.3)
project(microservice)

add_subdirectory(mongodbc)

include_directories(/usr/local/include/mongocxx/v_noabi)
include_directories(/usr/local/include/bsoncxx/v_noabi)
include_directories(mongodbc/inc)


include_directories(inc)
include_directories(/usr/local/ACE_TAO-7.0.0/include)
include_directories(/usr/local/openssl-1.1.1-stable/include)

link_directories(/usr/local/ACE_TAO-7.0.0/lib)
link_directories(/usr/local/openssl-1.1.1-stable/lib)

#Adding Complier flags
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++17 -g -Wall -Wextra")

#However, the file(GLOB...) allows for wildcard additions:
file(GLOB SOURCES "src/*.cc")

add_executable(uniservice ${SOURCES})
target_link_libraries(uniservice pthread)
target_link_libraries(uniservice ACE)
target_link_libraries(uniservice ACE_SSL)
target_link_libraries(uniservice libssl.a libcrypto.a dl)
#target_link_libraries(uniservice libcrypto.a)
target_link_libraries(uniservice mongodbcxx)
