#ifndef RAND_H
#define RAND_H

#include <chrono>
#include <random>
#include <string>

#include "public.h"
namespace randgen
{
    auto RandNum(int l, int r) -> int
    {
        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_int_distribution<> dis(l, r);
        return dis(gen);
    }
    inline const char rand_base[] = "0123456789\
                                   ABCDEFGHIJKLMNOPQRSTUVWXYZ\
                                   abcdefghijklmnopqrstuvwxyz";
    inline constexpr size_t RAND_BASE_LEN = 10 + 26 + 26;
    auto RandStrByLen(size_t len) -> std::string
    {
        std::string rand_str ;
        while(len--) {
            rand_str += rand_base[RandNum(0,RAND_BASE_LEN)];
        }
        return rand_str;
    }

    inline auto RandStr() -> std::string {
        return RandStrByLen(RandNum(5,42));
    }
}

#endif