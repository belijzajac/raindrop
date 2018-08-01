cmake_minimum_required(VERSION 3.12)
project(raindrop)

set(CMAKE_CXX_STANDARD 14)

add_executable(raindrop main.cpp FetchWeather.cpp FetchWeather.h Conditions.h Config.cpp Config.h Exception.h)
target_link_libraries(raindrop curl)