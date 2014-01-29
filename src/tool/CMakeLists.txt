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

add_executable(random_sample random_sample.cpp)
target_link_libraries(random_sample ${OpenCV_LIBS} common database)

add_executable(uni_sample uniform_sample.cpp)
target_link_libraries(uni_sample ${OpenCV_LIBS} common database)

add_executable(mswave_feature mswave_feature.cpp)
target_link_libraries(mswave_feature ${OpenCV_LIBS} common database)

add_executable(generate_keyframe generate_keyframe.cpp)
target_link_libraries(generate_keyframe ${OpenCV_LIBS} common database)

add_executable(single_kmeans single_kmeans.cpp)
target_link_libraries(single_kmeans ${OpenCV_LIBS} common database)

add_executable(multi_kmeans multi_kmeans.cpp)
target_link_libraries(multi_kmeans ${OpenCV_LIBS} common database)

add_executable(visual_attention visual_attention.cpp)
target_link_libraries(visual_attention ${OpenCV_LIBS} common database)

add_executable(va_summary va_summary.cpp)
target_link_libraries(va_summary ${OpenCV_LIBS} common database)
