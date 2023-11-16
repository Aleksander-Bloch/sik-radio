#ifndef SIKRADIO_SENDER_HPP
#define SIKRADIO_SENDER_HPP

#include "../common/types.hpp"
#include "client_socket.hpp"

class sender {
private:
    client_socket socket;
    size_t audio_data_size;
    audio_id_t session_id;
public:
    sender(std::string dest_addr, uint16_t data_port, size_t audio_data_size)
            : socket(std::move(dest_addr), data_port),
              audio_data_size(audio_data_size), session_id(time(nullptr)) {}

    void transmit_audio_data() {
        try {
            socket.open_connection();
        } catch(socket_exception &e) {
            socket.close_client_socket();
            std::cerr << "ERROR: Failed to connect socket, " << e.what() << std::endl;
            exit(EXIT_FAILURE);
        }

        audio_package_t audio_package(audio_data_size + 2 * sizeof(audio_id_t));
        byte_t *buffer_audio_data_offset = audio_package.data() + 2 * sizeof(audio_id_t);
        audio_id_t first_byte_num = 0;
        while (true) {
            std::cin.read(buffer_audio_data_offset, (std::streamsize) audio_data_size);
            auto read_bytes = std::cin.gcount();
            if (read_bytes != (long) audio_data_size) {
                break;
            }
            audio_id_t network_session_id = htobe64(session_id);
            audio_id_t network_first_byte_num = htobe64(first_byte_num);
            std::memcpy(audio_package.data(), &network_session_id, sizeof(audio_id_t));
            std::memcpy(audio_package.data() + sizeof(audio_id_t), &network_first_byte_num, sizeof(audio_id_t));
            socket.force_send_audio_package(audio_package);
            first_byte_num += audio_data_size;
        }
    }
};

#endif //SIKRADIO_SENDER_HPP
