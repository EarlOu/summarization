find_package(OpenCV REQUIRED)
include_directories(${OpenCV_INCLUDE_DIRS})

add_executable(old_event_player old_event_player.cpp)
target_link_libraries(old_event_player ${OpenCV_LIBS})

add_executable(frame_diff frame_diff.cpp)
target_link_libraries(frame_diff ${OpenCV_LIBS})

add_executable(event_player event_player.cpp)
target_link_libraries(event_player ${OpenCV_LIBS} common database)

add_executable(eval_keyframe eval_keyframe.cpp)
target_link_libraries(eval_keyframe common database)

add_executable(eval_single_view_skim eval_single_view_skim.cpp)
target_link_libraries(eval_single_view_skim common database)

add_executable(eval_multi_view_skim eval_multi_view_skim.cpp)
target_link_libraries(eval_multi_view_skim common database)

add_executable(concat_single_view_skim concat_single_view_skim.cpp)
target_link_libraries(concat_single_view_skim common database)
