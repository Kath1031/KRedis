#ifndef EXCUTE_H
#define EXCUTE_H

#include <vector>
#include <string>

#include "bytes.h"
namespace kath
{
    auto ParseReq(Bytes &data, std::vector<std::string> &cmd) -> bool
    {
        if (data.IsReadEnd())
            return false;

        auto cmd_num = data.GetNum<size_t>(4);

        while (cmd_num--)
        {
            auto cmd_len = data.GetNum<size_t>(4);
            cmd.emplace_back(data.GetStrView(cmd_len));
        }

        return true;
    }

    auto interpret(std::vector<std::string> &cmd ,Bytes &out) -> void {

    }
}

#endif