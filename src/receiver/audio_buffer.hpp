#ifndef SIKRADIO_AUDIO_BUFFER_HPP
#define SIKRADIO_AUDIO_BUFFER_HPP

#include <deque>
#include <optional>
#include <mutex>
#include <set>
#include <iostream>
#include "../common/types.hpp"

enum class buffer_state {
    NO_SESSION, WAITING, READABLE
};

class audio_buffer {
private:
    size_t size_in_bytes; // bsize from command argument
    size_t audio_data_capacity{}; // floor(bsize / psize)
    buffer_state state{buffer_state::NO_SESSION};
    std::deque<std::optional<audio_data_t>> audio_data_buffer;
    std::deque<audio_id_t> audio_id_buffer; // Stores first_byte_nums to identify audio packages.
    audio_id_t byte_zero{}; // first_byte_num of the first package received when state is NO_SESSION.
    size_t audio_data_size{}; // psize deduced from the size of the first received package.
    std::mutex buffer_mutex{};
    std::set<audio_id_t> missed_audio_data_ids; // Stores missed ids to write to stderr.

    void save_new_audio_data(const audio_data_t &audio_data, audio_id_t audio_id) {
        if (audio_id_buffer.empty()) {
            audio_data_buffer.emplace_back(audio_data);
            audio_id_buffer.emplace_back(audio_id);
            return;
        }
        for (auto missed_id = audio_id_buffer.back() + audio_data_size;
             missed_id < audio_id; missed_id += audio_data_size) {
            if (audio_id_buffer.size() == audio_data_capacity) {
                if (!audio_data_buffer.front().has_value()) {
                    missed_audio_data_ids.erase(audio_id_buffer.front());
                }
                // Make space for expected new audio data.
                audio_data_buffer.pop_front();
                audio_id_buffer.pop_front();
            }
            audio_data_buffer.emplace_back(std::nullopt);
            audio_id_buffer.emplace_back(missed_id);
            missed_audio_data_ids.insert(missed_id);
        }
        if (audio_id_buffer.size() == audio_data_capacity) {
            if (!audio_data_buffer.front().has_value()) {
                missed_audio_data_ids.erase(audio_id_buffer.front());
            }
            audio_data_buffer.pop_front();
            audio_id_buffer.pop_front();
        }
        audio_data_buffer.emplace_back(std::make_optional(audio_data));
        audio_id_buffer.emplace_back(audio_id);
    }

    void save_missed_audio_data(const audio_data_t &audio_data, audio_id_t audio_id) {
        size_t audio_data_position = (audio_id - audio_id_buffer.front()) / audio_data_size;
        audio_data_buffer[audio_data_position] = audio_data;
        missed_audio_data_ids.erase(audio_id);
    }

public:
    explicit audio_buffer(size_t size_in_bytes) : size_in_bytes(size_in_bytes) {}

    void reset() {
        std::lock_guard lock(buffer_mutex);
        audio_data_buffer.clear();
        audio_id_buffer.clear();
        missed_audio_data_ids.clear();
        state = buffer_state::NO_SESSION;
    }

    void write_audio_data(audio_data_t &audio_data, audio_id_t audio_id) {
        std::lock_guard lock(buffer_mutex);
        if (state == buffer_state::NO_SESSION) {
            byte_zero = audio_id;
            audio_data_size = audio_data.size();
            audio_data_capacity = size_in_bytes / audio_data_size;
            state = buffer_state::WAITING;
        }
        if (audio_id >= byte_zero + audio_data_capacity * audio_data_size * 3 / 4) {
            state = buffer_state::READABLE;
        }
        if (audio_id_buffer.empty() || audio_id > audio_id_buffer.back()) {
            save_new_audio_data(audio_data, audio_id);
        } else if (audio_id >= audio_id_buffer.front() && audio_id <= audio_id_buffer.back()) {
            save_missed_audio_data(audio_data, audio_id);
        }
        // If above conditions are not met, then the package is too old to store.

        // Print diagnostic data about missed packages.
        auto missed_ids_bound_it = missed_audio_data_ids.upper_bound(audio_id);
        for (auto missed_id_it = missed_audio_data_ids.begin();
            missed_id_it != missed_ids_bound_it; missed_id_it++) {
            std::cerr << "BEFORE " << audio_id << " EXPECTED " << *missed_id_it << std::endl;
        }
    }

    std::optional<audio_data_t> read_audio_data() {
        std::lock_guard lock(buffer_mutex);
        if (state != buffer_state::READABLE || audio_data_buffer.empty()) {
            return std::nullopt;
        }
        auto audio_data = audio_data_buffer.front();
        if (!audio_data.has_value()) {
            missed_audio_data_ids.erase(audio_id_buffer.front());
        }
        audio_data_buffer.pop_front();
        audio_id_buffer.pop_front();
        return audio_data;
    }
};

#endif //SIKRADIO_AUDIO_BUFFER_HPP
