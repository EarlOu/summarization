add_library(rpi EXCLUDE_FROM_ALL RpiCamReader.cpp RpiHardwareEncoder.cpp)

target_link_libraries(rpi ilclient -L/usr/lib/uv4l/uv4lext/armv6l -luv4lext -Wl,-rpath,'/usr/lib/uv4l/uv4lext/armv6l')
