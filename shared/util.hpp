#include <cstdint>
#include <random>

namespace veq::shared {
    inline std::uint64_t generateRandomNumber(std::size_t seed, std::size_t min, std::size_t max) {
        std::mt19937 gen(seed);

        std::uniform_int_distribution<int> dist (min, max);

        return dist(gen);
    }
}
