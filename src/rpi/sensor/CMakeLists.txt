find_package(OpenCV REQUIRED)
include_directories(${OpenCV_INCLUDE_DIRS})

#add_executable(rpi_sensor EXCLUDE_FROM_ALL rpi_sensor.cpp)
#target_link_libraries(rpi_sensor rpi_opencv rpi_common gmm ${OpenCV_LIBS})
add_executable(rpi_sensor EXCLUDE_FROM_ALL rpi_sensor.cpp)
target_link_libraries(rpi_sensor rpi_common ${OpenCV_LIBS})
