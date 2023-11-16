#ifndef SIKRADIO_RECEIVER_HPP
#define SIKRADIO_RECEIVER_HPP

#include <iostream>
#include <thread>
#include "server_socket.hpp"
#include "audio_buffer.hpp"

class receiver {
private:
    server_socket socket;
    audio_id_t session_id;
    audio_buffer buffer;
public:
    receiver(std::string src_addr, uint16_t data_port, size_t buffer_size)
            : socket(std::move(src_addr), data_port), session_id(0), buffer(buffer_size) {}

    [[noreturn]] void receive_audio_data() {
        try {
            socket.bind_socket();
        } catch (const socket_exception &e) {
            socket.close_server_socket();
            std::cerr << "ERROR: Failed to bind socket, " << e.what() << std::endl;
            exit(EXIT_FAILURE);
        }

        while (true) {
            auto audio_package = socket.force_receive_audio_package();
            audio_id_t received_session_id = be64toh(*((audio_id_t *) audio_package.data()));
            if (received_session_id < session_id) {
                continue;
            }
            if (received_session_id > session_id) {
                session_id = received_session_id;
                buffer.reset();
            }
            audio_id_t received_first_byte_num = be64toh(*((audio_id_t *) (audio_package.data() + sizeof(audio_id_t))));
            audio_data_t audio_data(audio_package.begin() + 2 * sizeof(audio_id_t), audio_package.end());
            buffer.write_audio_data(audio_data, received_first_byte_num);
        }
    }

    [[noreturn]] void stream_audio_data() {
        while (true) {
            auto audio_data = buffer.read_audio_data();
            if (audio_data.has_value()) {
                std::cout.write(audio_data.value().data(), (std::streamsize) audio_data.value().size());
            }
        }
    }

    void receive() {
        std::thread data_receiver(&receiver::receive_audio_data, this);
        data_receiver.detach();
        stream_audio_data();
    }
};

#endif //SIKRADIO_RECEIVER_HPP
