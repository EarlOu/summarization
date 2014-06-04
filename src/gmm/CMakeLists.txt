find_package(OpenCV REQUIRED)
include_directories(${OpenCV_INCLUDE_DIRS})

add_executable(gmm_single_view single_view.cpp)
target_link_libraries(gmm_single_view ${OpenCV_LIBS} algorithm)

add_executable(gmm_multi_view multi_view.cpp Server.cpp util.cpp VideoSensor.cpp)
target_link_libraries(gmm_multi_view ${OpenCV_LIBS} algorithm common database)

add_library(gmm Sensor.cpp)
target_link_libraries(gmm algorithm ${OpenCV_LIBS})

add_executable(gmm_single_view2 single_view2.cpp)
target_link_libraries(gmm_single_view2 gmm ${OpenCV_LIBS})
