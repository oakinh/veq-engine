#include <cstdint>
#include <random>

namespace veq::shared {
    inline std::uint64_t generateRandomNumber(std::size_t seed, int min, int max) {
        std::mt19937 gen(seed);

        std::uniform_int_distribution<int> dist(min, max);

        return static_cast<std::uint64_t>(dist(gen));
    }
}
