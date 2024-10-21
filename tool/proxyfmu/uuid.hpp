
#ifndef ECOS_UUID_HPP
#define ECOS_UUID_HPP

#include <random>
#include <sstream>
#include <string>

inline std::string generate_uuid()
{
    static std::random_device rd;
    static std::mt19937 gen(rd());
    static std::uniform_int_distribution<> dis(0, 15);

    int i;
    std::stringstream ss;
    ss << std::hex;
    for (i = 0; i < 8; i++) {
        ss << dis(gen);
    }

    return ss.str();
}

#endif // ECOS_UUID_HPP
