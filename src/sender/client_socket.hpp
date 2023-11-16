#ifndef SIKRADIO_CLIENT_SOCKET_HPP
#define SIKRADIO_CLIENT_SOCKET_HPP

#include <string>
#include <unistd.h>
#include "../common/socket_utils.hpp"
#include "../common/types.hpp"

class client_socket {
private:
    int fd;
    bool is_connected;
    std::string dest_addr;
    uint16_t data_port;
public:
    client_socket(std::string dest_addr, uint16_t data_port)
            : fd(-1), is_connected(false), dest_addr(std::move(dest_addr)), data_port(data_port) {}

    void open_connection() {
        if (is_connected) return;
        struct sockaddr_in send_addr = get_socket_address(dest_addr, data_port);

        fd = socket(AF_INET, SOCK_DGRAM, 0);
        if (fd < 0) {
            throw socket_exception("audio_socket: " + std::string(strerror(errno)));
        }

        int err = connect(fd, (struct sockaddr *) &send_addr, (socklen_t)
        sizeof(send_addr));
        if (err < 0) {
            throw socket_exception("connect: " + std::string(strerror(errno)));
        }

        is_connected = true;
    }

    void send_audio_package(audio_package_t &package) const {
        ssize_t bytes_sent = send(fd, package.data(), package.size(), 0);
        verify_connection_result(bytes_sent);
    }

    void force_send_audio_package(audio_package_t &package) const {
        while (true) {
            try {
                send_audio_package(package);
                break;
            } catch (socket_exception &e) {
                // Try to send audio package until success.
            }
        }
    }

    void close_client_socket() {
        if (fd >= 0) close(fd);
        is_connected = false;
    }

    ~client_socket() {
        close_client_socket();
    }
};

#endif //SIKRADIO_CLIENT_SOCKET_HPP
