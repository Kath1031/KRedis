#ifndef MSG_H
#define MSG_H
#include <cstdio>
#include <cstdlib>
namespace kath
{
    void MSG(const char *file, const char *func, int line, const char *msg)
    {
        std::fprintf(stderr, "msg in %s:%d func:%s() :%s \n", file, line, func, msg);
    }

    void ERR(const char *file, const char *func, int line, const char *msg){
        std::fprintf(stderr, "err in %s:%d func:%s() :%s \n", file, line, func, msg);
        exit(1);
    }

}
#define Err(msg) kath::ERR(__FILE__, __func__, __LINE__, msg)

#define Msg(msg) kath::MSG(__FILE__, __func__, __LINE__, msg)

#endif