#cmake_minimum_required(VERSION 2.8.9)
#project(microservice)

# Specify the minimum version you require.
#find_package (mongoc-1.0 REQUIRED)

include_directories(inc)
include_directories(/usr/include/mongo)
include_directories(/usr/include/libbson-1.0)
include_directories(/usr/include/libmongoc-1.0)

link_directories(/usr/lib/x86_64-linux-gnu)

#Adding Complier flags
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++17 -g -Wall -Wextra")

#However, the file(GLOB...) allows for wildcard additions:
file(GLOB SOURCES "src/*.cc")

add_library(mongodbc STATIC ${SOURCES})
#target_link_libraries(mongo_client mongoc-1.0 bson-1.0)
#target_link_libraries (mongodbc libbson-1.0.so)
#target_link_libraries (mongodbc libmongoc-1.0.so)
#target_link_libraries (dbc-service PRIVATE mongo::mongoc_shared)

