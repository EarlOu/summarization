find_package(OpenCV REQUIRED)
include_directories(${OpenCV_INCLUDE_DIRS})

add_executable(rpisensor sensor.cpp summarization.cpp OnlineClusterMog.cpp)
SET(sensor_dep ${OpenCV_LIBS})

if (SIMULATE)
    target_link_libraries(sensor ${sensor_dep})
else()
    target_link_libraries(sensor ${sensor_dep} rpi_opencv)
endif()

add_executable(gmm_single_view gmm_single_view.cpp summarization.cpp OnlineClusterMog.cpp)
target_link_libraries(gmm_single_view ${OpenCV_LIBS})
