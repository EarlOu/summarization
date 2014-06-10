add_library(rpi_common EXCLUDE_FROM_ALL NetUtil.cpp ConcurrentQueue.cpp)

target_link_libraries(rpi_common)
