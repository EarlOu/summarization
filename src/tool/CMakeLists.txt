find_package(OpenCV REQUIRED)
include_directories(${OpenCV_INCLUDE_DIRS})

add_executable(frame_diff frame_diff.cpp)
target_link_libraries(frame_diff ${OpenCV_LIBS})

add_executable(event_player event_player.cpp)
target_link_libraries(event_player ${OpenCV_LIBS} common database)

add_executable(concat_single_view_skim concat_single_view_skim.cpp)
target_link_libraries(concat_single_view_skim common database)
