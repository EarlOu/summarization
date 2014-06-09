find_package(OpenCV REQUIRED)
include_directories(${OpenCV_INCLUDE_DIRS})

add_library(gmm Sensor.cpp OnlineClusterMog.cpp)
target_link_libraries(gmm algorithm ${OpenCV_LIBS})

add_executable(gmm_intra_view gmm_intra_view.cpp)
target_link_libraries(gmm_intra_view gmm ${OpenCV_LIBS})

add_executable(gmm_inter_view gmm_inter_view.cpp)
target_link_libraries(gmm_inter_view gmm database common ${OpenCV_LIBS})
