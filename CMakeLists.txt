cmake_minimum_required(VERSION 3.13)
project(lightcube C)

set(CMAKE_C_STANDARD 11)
set(THREADS_PREFER_PTHREAD_FLAG ON)

find_package(PkgConfig)
find_library(wiringPi_LIB wiringPi)

pkg_check_modules(GSTREAMER REQUIRED gstreamer-1.0)

include_directories(
        ${GLIB_INCLUDE_DIRS}
        ${GSTREAMER_INCLUDE_DIRS}
)
link_directories(
        ${GLIB_INCLUDE_DIRS}
        ${GSTREAMER_INCLUDE_DIRS}
)

add_executable(${PROJECT_NAME} src/main.c src/buttons.c src/buttons.h src/camera.c src/camera.h src/image_camera.c src/image_camera.h src/crosshair.h src/image_destination.c src/image_destination.h)

target_link_libraries(${PROJECT_NAME} ${GSTREAMER_LIBRARIES})
target_link_libraries(${PROJECT_NAME} ${wiringPi_LIB})
