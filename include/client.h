#ifndef CLIENT_H
#define CLIENT_H

#include <arpa/inet.h>
#include <netinet/ip.h>
#include <sys/socket.h>
#include <memory>
#include <vector>
#include <string>

#include "public.h"
#include "file.h"
namespace kath
{
    class Client
    {
    private:
        File fd_;

    public:
        Client() : fd_(GenerateSocket()) {}
        static int GenerateSocket()
        {
            int fd = socket(AF_INET, SOCK_STREAM, 0);
            if (fd < 0)
            {
                Err("socket create");
            }

            sockaddr_in server = {};
            server.sin_family = AF_INET;
            server.sin_port = htons(server_port);
            server.sin_addr.s_addr = ntohl(INADDR_LOOPBACK);

            int rv = connect(fd, reinterpret_cast<const sockaddr *>(&server), sizeof(server));
            if (rv)
            {
                Err("connect");
            }
            return fd;
        }

        void Send(std::vector<std::string> cmds)
        {
            Bytes buff;
            size_t size_sum = 4;
            for (auto &cmd : cmds)
            {
                size_sum += cmd.size() + 4;
            }
            buff.AppendNum(size_sum, 4);
            buff.AppendNum(cmds.size(), 4);
            for (auto &cmd : cmds)
            {
                buff.AppendNum(cmd.size(), 4);
                buff.AppendStr(cmd);
            }
            auto ok = fd_.WriteByte_b(buff);
            assert(ok);
        }
        void Handle(Bytes &buff, const std::string &pre = "")
        {
            auto type = static_cast<SerType>(buff.GetNum<int>(1));
            double dou_val;
            size_t arr_len;
            int str_len;
            std::string_view str_view;
            std::string_view msg;
            switch (type)
            {
            case SerType::NIL:
                std::cout << pre << "[nil]\n";
                break;
            case SerType::ARR:
                arr_len = buff.GetNum<size_t>(4);
                std::cout << pre << "[arr]: len = " << arr_len << std::endl;
                for (int index = 0; index < arr_len; index++)
                {
                    Handle(buff, pre + "-");
                }
                break;
            case SerType::DOU:
                dou_val = buff.GetNum<double>(8);
                std::cout << pre << "[dou]: " << dou_val << std::endl;
                break;
            case SerType::ERR:
                str_len = buff.GetNum<int>(4);
                msg = buff.GetStrView(str_len);
                std::cout << pre << "[err]: " << msg << std::endl;
                break;
            case SerType::INT64:
                std::cout << pre << "[int]: " << buff.GetNum<int64_t>(8) << std::endl;
                break;
            case SerType::STR:
                str_len = buff.GetNum<int>(4);
                str_view = buff.GetStrView(str_len);
                std::cout << pre << "[str]: " << str_view << std::endl;
            default:
                Msg("type dont find");
                break;
            }
        }
        void Receive()
        {
            Bytes buff;
            bool ok = fd_.ReadByte_b(buff, 4);
            assert(ok);
            size_t size_sum = buff.GetNum<size_t>(4);
            ok = fd_.ReadByte_b(buff, size_sum);
            assert(ok);
            Handle(buff);
        }
    };
}

#endif