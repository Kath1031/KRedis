#ifndef SERVER_H
#define SERVER_H

#include <arpa/inet.h>
#include <netinet/ip.h>
#include <sys/socket.h>

#include "public.h"
#include "file.h"
namespace kath
{

    class Server
    {
    private:
        File file_;
    public:
        int MakeSocket()
        {
            int fd = socket(AF_INET, SOCK_STREAM, 0);
            if (fd < 0)
            {
                Err("socket");
            }
            int val = 1;
            setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &val, sizeof(val));

            sockaddr_in addr{};

            addr.sin_addr.s_addr = ntohl(0);
            addr.sin_family = AF_INET;
            addr.sin_port = ntohs(1234);

            int rv = bind(fd, (const sockaddr *)&addr, sizeof(addr));

            if (rv)
            {
                Err("bind");
            }

            rv = listen(fd, SOMAXCONN);

            if (rv)
            {
                Err("bind");
            }

            return fd;
        }
    };
}

#endif