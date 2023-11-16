COMPILER = g++
CPP_FLAGS = -std=c++20 -Wall -O2
LINK_FLAGS = -lpthread -lboost_program_options

.PHONY: all clean

all: sikradio-sender sikradio-receiver

sikradio-sender: src/sender.cpp
	$(COMPILER) $(CPP_FLAGS) $< $(LINK_FLAGS) -o $@

sikradio-receiver: src/receiver.cpp
	$(COMPILER) $(CPP_FLAGS) $< $(LINK_FLAGS) -o $@

clean:
	rm -f sikradio-sender sikradio-receiver
