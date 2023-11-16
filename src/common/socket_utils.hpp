#ifndef SIKRADIO_SOCKET_UTILS_HPP
#define SIKRADIO_SOCKET_UTILS_HPP

#include <string>
#include <netdb.h>
#include <bits/socket.h>
#include <cstring>
#include "exceptions.hpp"

#define MIN_BYTES 17

struct sockaddr_in get_socket_address(const std::string &address_str, uint16_t port) {
    struct addrinfo hints{};
    memset(&hints, 0, sizeof(struct addrinfo));
    hints.ai_family = AF_INET; // IPv4
    hints.ai_socktype = SOCK_DGRAM;
    hints.ai_protocol = IPPROTO_UDP;

    struct addrinfo *address_result;
    int err = getaddrinfo(address_str.c_str(), nullptr, &hints, &address_result);
    if (err != 0) {
        throw socket_exception("getaddrinfo: " + std::string(gai_strerror(err)));
    }

    struct sockaddr_in socket_address{};
    socket_address.sin_family = AF_INET;
    socket_address.sin_addr.s_addr = ((struct sockaddr_in *) address_result->ai_addr)->sin_addr.s_addr;
    socket_address.sin_port = htons(port);

    freeaddrinfo(address_result);

    return socket_address;
}

void verify_connection_result(ssize_t bytes) {
    if (bytes == -1) {
        throw socket_exception("");
    }
    if (bytes < MIN_BYTES) {
        throw socket_exception("");
    }
}

#endif //SIKRADIO_SOCKET_UTILS_HPP
