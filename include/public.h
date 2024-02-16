#ifndef PUBLIC_H
#define PUBLIC_H

#include <cassert>
#include <ctime>
#include <memory>

#include "msg.h"

namespace kath
{
    inline int server_port = 1234;
    enum class SerType
    {
        NIL = 0,
        ERR, // 错误码
        STR,
        INT64,
        DOU,
        ARR,
    };
    enum class CmdErr
    {
        Err_UNKNOWN = 1,
        ERR_2Big,
        ERR_TYPE,
        ERR_ARG,
    };
    enum class ConnState
    {
        STATE_REQ = 0,
        STATE_RES,
        STATE_END,
    };

#define container_of(ptr, type, member)                    \
    ({                                                     \
        const typeof(((type *)0)->member) *__mptr = (ptr); \
        (type *)((char *)__mptr - offsetof(type, member)); \
    })

    inline auto GetMonotonicUsec() -> uint64_t
    {
        timespec tv = {0, 0};
        clock_gettime(CLOCK_MONOTONIC, &tv);
        return static_cast<uint64_t>(tv.tv_sec * 1000000 + tv.tv_nsec / 1000);
    }
}

#endif