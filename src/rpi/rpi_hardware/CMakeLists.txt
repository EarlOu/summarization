add_library(rpi_hardware EXCLUDE_FROM_ALL RpiCamReader.cpp RpiHardwareEncoder.cpp)

target_link_libraries(rpi_hardware ilclient -L/usr/lib/uv4l/uv4lext/armv6l -luv4lext -Wl,-rpath,'/usr/lib/uv4l/uv4lext/armv6l')
