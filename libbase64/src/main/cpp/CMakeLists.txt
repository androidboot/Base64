cmake_minimum_required(VERSION 3.4.1)


add_library(base64
        SHARED

        Base64.cpp
        base64-lib.cpp)

find_library(log-lib log)


target_link_libraries(
        base64

        ${log-lib})