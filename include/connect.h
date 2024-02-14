#ifndef CONNECT_H
#define CONNECT_H

#include <poll.h>
#include <vector>
#include <string>

#include "public.h"
#include "file.h"
#include "list.h"
#include "excute.h"

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
            if (state_ == ConnState::STATE_REQ)
                return POLLIN;
            if (state_ == ConnState::STATE_RES)
                return POLLOUT;
            assert(false);
            return 0;
        }

        auto IsEnd() const -> bool { return state_ == ConnState::STATE_END; }

        auto StartConnectionIO(DList *head)
        {
            // todo idle_node
            idle_start_ = GetMonotonicUsec();
            idle_node_.Detach();
            head->InsertFront(&idle_node_);
            ConnectionIO();
        }

        auto ConnectionIO() -> void
        {
            if (state_ == ConnState::STATE_REQ)
            {
                StateRequest();
            }
            else if (state_ == ConnState::STATE_RES)
            {
                StateResponse();
            }
            else
            {
                assert(0);
            }
        }

        auto StateRequest() -> void
        {
            while (TryFillBuffer())
                ;
        }

        auto TryFillBuffer() -> bool
        {
            do
            {
                auto rv = file_.ReadByte_nb(rbuf_, 4);
                switch (rv)
                {
                case -1:
                    Msg("nb read() error");
                    state_ = ConnState::STATE_END;
                    return false;
                case 0:
                    break;
                case 1:
                    return false;
                case 2:
                    rbuf_.Size() ? Msg("unexpected EOF") : Msg("EOF");
                    state_ = ConnState::STATE_END;
                    return false;
                default:
                    break;
                }
                auto len = rbuf_.GetNum<uint32_t>(4);
                rv = file_.ReadByte_nb(rbuf_, len);

                switch (rv)
                {
                case -1:
                    Msg("nb read() error");
                    state_ = ConnState::STATE_END;
                    return false;
                case 0:
                    break;
                case 1:
                    return false;
                case 2:
                    rbuf_.Size() ? Msg("unexpected EOF") : Msg("EOF");
                    state_ = ConnState::STATE_END;
                    return false;
                default:
                    break;
                }
            } while (TryOneRequest());
            return state_ == ConnState::STATE_REQ;
        }

        auto TryOneRequest() -> bool
        {
            std::vector<std::string> cmd;
            auto ok = ParseReq(rbuf_, cmd);
            if (!ok)
            {
                Msg("bad req");
                state_ = ConnState::STATE_END;
                return false;
            }

            // 数据从Bytes中拷贝进cmd
            Bytes out;
            interpret(cmd, out);

            wbuf_.AppendNum(out.Size(), 4);
            wbuf_.AppendBytes(std::move(out));

            state_ = ConnState::STATE_RES;

            StateResponse();

            return state_ == ConnState::STATE_REQ;
        }

        auto StateResponse() -> void
        {
            while (TryFlushBuffer())
                ;
        }

        auto TryFlushBuffer() -> bool
        {
            auto rv = file_.WriteByte_nb(wbuf_);
            switch (rv)
            {
            case -1:
                Msg("nb write() error");
                state_ = ConnState::STATE_END;
                return false;
            case 0:
                break;
            case 1:
                return false;

            default:
                break;
            }
            if (wbuf_.IsReadEnd())
            {
                state_ = ConnState::STATE_REQ;
                rbuf_.Clear();
                wbuf_.Clear();
                return false;
            }
        }
    };
}

#endif