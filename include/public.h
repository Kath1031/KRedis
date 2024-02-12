#ifndef PUBLIC_H
#define PUBLIC_H

#include <cassert>
#include <ctime>
#include <memory>

#include "msg.h"


namespace kath {
    inline int server_port = 1234;
    enum class SerType{
        NIL = 0, 
        ERR, //错误码
        STR,
        INT64,
        DOU,
        ARR,
    };


    #define container_of(ptr, type, member)                    \
    ({                                                     \
        const typeof(((type *)0)->member) *__mptr = (ptr); \
        (type *)((char *)__mptr - offsetof(type, member)); \
    })
}

#endif