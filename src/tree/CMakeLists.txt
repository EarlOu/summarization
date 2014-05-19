find_package(OpenCV REQUIRED)
include_directories(${OpenCV_INCLUDE_DIRS})

add_executable(tree tree.cpp)
target_link_libraries(tree ${OpenCV_LIBS} algorithm)
