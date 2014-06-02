find_package(OpenCV REQUIRED)
include_directories(${OpenCV_INCLUDE_DIRS})

add_executable(mmr mmr.cpp)
target_link_libraries(mmr common database)

add_executable(tree tree.cpp)
target_link_libraries(tree algorithm ${OpenCV_LIBS})

add_executable(compress_domain compress_domain.cpp)
target_link_libraries(compress_domain ${OpenCV_LIBS})
