find_package(OpenCV REQUIRED)
include_directories(${OpenCV_INCLUDE_DIRS})

add_executable(random_sample random_sample.cpp)
target_link_libraries(random_sample ${OpenCV_LIBS} common database)

add_executable(uni_sample uniform_sample.cpp)
target_link_libraries(uni_sample ${OpenCV_LIBS} common database)

add_executable(single_kmeans single_kmeans.cpp)
target_link_libraries(single_kmeans ${OpenCV_LIBS} common database)

add_executable(multi_kmeans multi_kmeans.cpp)
target_link_libraries(multi_kmeans ${OpenCV_LIBS} common database)

add_executable(visual_attention visual_attention.cpp)
target_link_libraries(visual_attention ${OpenCV_LIBS} common database)

add_executable(va_summary va_summary.cpp)
target_link_libraries(va_summary ${OpenCV_LIBS} common database)

