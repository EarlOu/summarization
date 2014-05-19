find_package(OpenCV REQUIRED)
include_directories(${OpenCV_INCLUDE_DIRS})

AUX_SOURCE_DIRECTORY(./ dir)
add_library(algorithm ${dir})
