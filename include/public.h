#ifndef PUBLIC_H
#define PUBLIC_H

#include <cassert>

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
}

#endif