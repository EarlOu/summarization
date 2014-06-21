find_package(OpenCV REQUIRED)
include_directories(${OpenCV_INCLUDE_DIRS})

add_executable(rpi_server EXCLUDE_FROM_ALL rpi_server.cpp)
target_link_libraries(rpi_server rpi_common ${OpenCV_LIBS})

add_executable(rpi_post_proc EXCLUDE_FROM_ALL rpi_post_proc.cpp)
target_link_libraries(rpi_post_proc rpi_common ${OpenCV_LIBS})
