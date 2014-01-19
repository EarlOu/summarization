find_package(OpenCV REQUIRED)
include_directories(${OpenCV_INCLUDE_DIRS})

add_executable(old_event_player old_event_player.cpp)
target_link_libraries(old_event_player ${OpenCV_LIBS})

add_executable(frame_diff frame_diff.cpp)
target_link_libraries(frame_diff ${OpenCV_LIBS})
