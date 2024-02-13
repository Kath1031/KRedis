#ifndef CONNECT_H
#define CONNECT_H

#include <poll.h>

#include "public.h"
#include "file.h"
#include "list.h"

namespace kath
{
    class Conn
    {
        friend class Server;

    private:
        File file_;
        ConnState state_;
        Bytes rbuf_, wbuf_;

        uint64_t idle_start_;
        DList idle_node_;

    public:
        auto Check() -> void { file_.Check(); }
        explicit Conn(File &&f, ConnState conn_state)
            : file_(std::move(f)),
              state_{conn_state},
              rbuf_{},
              wbuf_{},
              idle_start_{GetMonotonicUsec()},
              idle_node_{}
        {
            file_.SetNb();
        }
        auto GetFd() const -> int { return file_.Data(); }

        auto GetEvent() const -> short int
        {
            if(state_ == ConnState::STATE_REQ) return POLLIN;
            if(state_ == ConnState::STATE_RES) return POLLOUT;
            assert(false);
            return 0;
        }

        auto IsEnd() const -> bool {return state_ == ConnState::STATE_END;}

    };
}

#endif