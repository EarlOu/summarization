find_package(OpenCV REQUIRED)
include_directories(${OpenCV_INCLUDE_DIRS})

add_executable(compress_domain compress_domain.cpp)
target_link_libraries(compress_domain ${OpenCV_LIBS})
