#ifndef SERVER_H
#define SERVER_H

#include <arpa/inet.h>
#include <netinet/ip.h>
#include <sys/socket.h>
#include <unordered_map>
#include "public.h"
#include "file.h"
#include "connect.h"
namespace kath
{
    const uint64_t k_idle_timeout_ms = 5 * 1000;

    class Server
    {
    private:
        File file_;
        DList head_;
        std::unordered_map<int, std::shared_ptr<Conn>> fd2conn_;

    public:
        Server() : file_(MakeSocket()) {}
        static auto MakeSocket() -> int
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

        auto AddNewConn(std::shared_ptr<Conn> conn) -> void
        {
            auto conn_fd = conn->GetFd();
            assert(fd2conn_.find(conn_fd) != fd2conn_.end() ||
                   fd2conn_[conn_fd] == nullptr);
            fd2conn_[conn_fd] = std::move(conn);
        }
        
        auto DelConn(std::shared_ptr<Conn> conn) ->void {
            fd2conn_.erase(conn->GetFd());
            conn->idle_node_.Detach();
        }

        auto NextTimerMS() -> uint32_t {
            uint64_t now_us = GetMonotonicUsec();
            uint64_t next_us = std::numeric_limits<uint64_t>::max();

            if(!head_.Empty()) {
                Conn *next = container_of(head_.next_,Conn,idle_node_);
                next_us - std::min(next_us,next->idle_start_ +k_idle_timeout_ms * 1000 );
            }
            // todo 实现heap
            // if(!heap.Empty()) {
            //     next_us = std::min(next_us,heap.get_min());
            // }
            if(next_us == std::numeric_limits<uint64_t>::max())
                return 10000;
            if(next_us <= now_us)
                return 0;
            return static_cast<uint32_t>((next_us - now_us)/1000);
        }
        void join() {
            file_.SetNb();
            std::vector<struct pollfd> poll_args;
            for(;;) {
                poll_args.clear();
                struct pollfd pfd = {file_.Data(),POLLIN,0};
                poll_args.push_back(pfd);

                for(const auto &[fd,conn] : fd2conn_) {
                    if(!conn) continue;
                    struct pollfd pfd = {};
                    pfd.fd = conn->GetFd();
                    pfd.events = conn->GetEvent() | POLLERR;
                    poll_args.push_back(pfd);
                }

                int timeout_ms = static_cast<int> (NextTimerMS());
            }
        }
    };
}

#endif