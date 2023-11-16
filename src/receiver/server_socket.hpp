#ifndef SIKRADIO_SERVER_SOCKET_HPP
#define SIKRADIO_SERVER_SOCKET_HPP

#include <string>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <cstring>
#include <unistd.h>
#include "../common/exceptions.hpp"
#include "../common/types.hpp"
#include "../common/socket_utils.hpp"

#ifndef UDP_DATAGRAM_DATA_LEN_MAX
#define UDP_DATAGRAM_DATA_LEN_MAX 65535
#endif

class server_socket {
private:
    int fd;
    bool is_bound;
    std::string src_addr_str;
    uint16_t data_port;
    byte_t buffer[UDP_DATAGRAM_DATA_LEN_MAX] = {0};
    uint32_t src_addr{};

    audio_package_t receive_audio_package() {
        memset(&buffer, 0, sizeof(buffer));
        struct sockaddr_in recv_addr_struct{};
        socklen_t len = sizeof(recv_addr_struct);
        ssize_t bytes_received = recvfrom(fd, buffer, sizeof(buffer), 0, (struct sockaddr *) &recv_addr_struct, &len);
        verify_connection_result(bytes_received);
        if (src_addr != ntohl(recv_addr_struct.sin_addr.s_addr)) {
            throw socket_exception("");
        }
        audio_package_t audio_package(buffer, buffer + bytes_received);
        return audio_package;
    }

public:
    server_socket(std::string src_addr_str, uint16_t data_port)
            : fd(-1), is_bound(false), src_addr_str(std::move(src_addr_str)), data_port(data_port) {}

    void bind_socket() {
        if (is_bound) return;
        fd = socket(AF_INET, SOCK_DGRAM, 0);
        if (fd < 0) {
            throw socket_exception("server_socket: " + std::string(strerror(errno)));
        }

        src_addr = ntohl(get_socket_address(src_addr_str, data_port).sin_addr.s_addr);

        sockaddr_in server_address{};
        server_address.sin_family = AF_INET;
        server_address.sin_addr.s_addr = htonl(INADDR_ANY);
        server_address.sin_port = htons(data_port);
        if (bind(fd, (sockaddr *) &server_address, sizeof(server_address)) < 0) {
            throw socket_exception("bind: " + std::string(strerror(errno)));
        }

        is_bound = true;
    }

    audio_package_t force_receive_audio_package() {
        while (true) {
            try {
                return receive_audio_package();
            } catch (socket_exception &e) {
                // Try to receive audio package until success.
            }
        }
    }

    void close_server_socket() {
        if (fd >= 0) close(fd);
        is_bound = false;
    }

    ~server_socket() {
        close_server_socket();
    }
};

#endif //SIKRADIO_SERVER_SOCKET_HPP
