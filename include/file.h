#ifndef KFILE_H
#define KFILE_H

#include <errno.h>
#include <fcntl.h>

#include "bytes.h"
namespace kath
{
    void FDSetNB(int fd)
    {
        auto errno_backup = errno;
        errno = 0;

        int flag = fcntl(fd, F_GETFL, 0);
        if (errno != 0)
        {
            // todo 抛出异常
        }

        flag |= O_NONBLOCK;

        fcntl(fd, F_SETFL, flag);

        if (errno != 0)
        {
            // todo 抛出异常
        }
    }

    class File
    {
    private:
        /* data */
    public:
        File(/* args */);
        ~File();
    };


}

#endif
