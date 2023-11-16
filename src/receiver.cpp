#include <boost/program_options.hpp>
#include <iostream>
#include "receiver/receiver.hpp"

namespace po = boost::program_options;

int main(int argc, char *argv[]) {
    po::options_description desc("Allowed options");
    desc.add_options()
            (",a", po::value<std::string>()->required(), "DEST_ADDR")
            (",P", po::value<uint16_t>()->default_value(27519), "DATA_PORT")
            (",b", po::value<size_t>()->default_value(65536), "BSIZE");

    po::variables_map vm;
    try {
        po::store(po::parse_command_line(argc, argv, desc), vm);
        po::notify(vm);
    } catch (po::error &e) {
        std::cerr << e.what() << std::endl;
        std::cerr << desc << std::endl;
        exit(EXIT_FAILURE);
    }

    if (vm["-b"].as<size_t>() == 0) {
        std::cerr << "ERROR: BSIZE must be a positive number." << std::endl;
        exit(EXIT_FAILURE);
    }
    receiver receiver(vm["-a"].as<std::string>(), vm["-P"].as<uint16_t>(), vm["-b"].as<size_t>());
    receiver.receive();

    return 0;
}
