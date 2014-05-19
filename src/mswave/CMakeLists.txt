find_package(OpenCV REQUIRED)
include_directories(${OpenCV_INCLUDE_DIRS})

add_executable(mswave_feature mswave_feature.cpp)
target_link_libraries(mswave_feature ${OpenCV_LIBS} common database)
