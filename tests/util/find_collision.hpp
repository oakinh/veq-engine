#pragma once
#include <utility>
#include <cstddef>
#include <cstdint>
#include <vector>
#include <optional>
#include <array>
#include <stdexcept>

inline std::pair<std::uint64_t, std::uint64_t> findCollision(std::size_t capacity) {
    std::vector<std::optional<std::uint64_t>> first_key(capacity);

    for (std::uint64_t key {}; key <= capacity; ++key) {
        std::size_t index { std::hash<std::uint64_t>{}(key) % capacity };

        if (first_key[index].has_value()) {
            return std::pair{ *first_key[index], key };
        }

        first_key[index] = key;

    }
    throw std::logic_error("No collision found");
}
