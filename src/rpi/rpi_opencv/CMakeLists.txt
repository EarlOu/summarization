find_package(OpenCV REQUIRED)
include_directories(${OpenCV_INCLUDE_DIRS})

add_library(rpi_opencv EXCLUDE_FROM_ALL RpiVideoCapture.cpp RpiVideoWriter.cpp)

target_link_libraries(rpi_opencv rpi_hardware ${OpenCV_LIBS})
