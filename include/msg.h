#ifndef MSG_H
#define MSG_H
#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <sstream>
#include <optional>
#include <bitset>
namespace notice
{
    void MSG(const char *file, const char *func, int line, const char *msg)
    {
        std::fprintf(stderr, "msg in %s:%d func:%s() :%s \n", file, line, func, msg);
    }

    void ERR(const char *file, const char *func, int line, const char *msg)
    {
        std::fprintf(stderr, "err in %s:%d func:%s() :%s \n", file, line, func, msg);
        exit(1);
    }

}
#define Err(msg) notice::ERR(__FILE__, __func__, __LINE__, msg)

#define Msg(msg) notice::MSG(__FILE__, __func__, __LINE__, msg)

namespace std
{
    auto operator<<(std::ostream &os, const std::byte &b) -> std::ostream &
    {
        return os << std::bitset<8>(std::to_integer<int>(b));
    }
    template <typename T>
    auto operator<<(typename std::enable_if<std::is_enum<T>::value, std::ostream>::type &os,
                    const T &e) -> std::ostream
    {
        return os << "\033[93m" << typeid(e).name() << "\033[0m::\033[36m"
                  << static_cast<typename std::underlying_type<T>::type>(e) << "\033[0m";
    }
    template <typename T>
    auto operator<<(std::ostream &os, const std::optional<T> &op) -> std::ostream &
    {
        if (op.has_value())
        {
            return os << op.value();
        }
        return os << "nil";
    }
}

auto Class2Str(const std::string &str) -> std::string
{
    if (str.size())
        return "\033[33m" + str + "\033[0m";
    return ")";
}

template <typename T1, typename T2, typename... Args>
auto Class2Str(const std::string &str, T1 &&arg1, T2 &&arg2, Args &&...args) -> std::string
{
    static_assert(
        sizeof...(args) % 2 == 0, "The number of arguments must be even.");
    std::ostringstream ss;
    if (!str.empty())
    {
        ss << "\033[33m" << str << "\033[0m( ";
    }
    ss << "\033[34m" << arg1 << "\033[0m"
       << " = " << arg2 << " ";
    ss << Class2Str("", std::forward<Args>(args)...);
    return ss.str();
}

#endif