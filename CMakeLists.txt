project(sikradio)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_FLAGS "-Wall -O2")

add_executable(sikradio-sender src/sender.cpp src/sender/sender.hpp src/common/exceptions.hpp src/common/types.hpp src/common/socket_utils.hpp src/sender/client_socket.hpp)
target_link_libraries(sikradio-sender boost_program_options pthread)
add_executable(sikradio-receiver src/receiver.cpp src/receiver/receiver.hpp src/receiver/server_socket.hpp src/common/socket_utils.hpp src/receiver/audio_buffer.hpp)
target_link_libraries(sikradio-receiver boost_program_options pthread)
