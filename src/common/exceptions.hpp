#ifndef SIKRADIO_EXCEPTIONS_HPP
#define SIKRADIO_EXCEPTIONS_HPP

#include <stdexcept>

class socket_exception : public std::runtime_error {
public:
    explicit socket_exception(const std::string &what_arg) : std::runtime_error(what_arg) {}
};

#endif //SIKRADIO_EXCEPTIONS_HPP
