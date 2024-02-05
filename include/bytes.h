#ifndef BYTES_H
#define BYTES_H

#include <cstddef>
#include <vector>
#include <iostream>
#include <string>
#include <cstring>

#include "public.h"

namespace kath
{

    // 不保证每次操作原子
    class Bytes
    {
        friend class File;

    private:
        // 数据段
        std::vector<std::byte> data_{};

        // pos:下一次读入的开始位置
        size_t pos_{0};

    public:
        Bytes() = default;
        ~Bytes() = default;

        [[nodiscard]] auto Size() const -> size_t
        {
            return data_.size();
        }

        auto Clear() -> void
        {
            data_.clear();
            pos_ = 1;
        }

        auto reset() -> void { pos_ = 0; }

        friend auto operator<<(std::ostream &ost, const Bytes &bytes) -> std::ostream &
        {
            ost << std::hex << "[";
            for (size_t index = 0; index < bytes.data_.size(); index++)
            {
                ost << "0x" << std::to_integer<int>(bytes.data_[index]) << ",]"[index + 1 == bytes.data_.size()];
            }
            ost << "  (len = " << bytes.Size() << ")";
            return ost;
        }

        auto AppendStr(const std::string &str) -> void
        {
            data_.resize(str.size() + data_.size());
            std::memcpy(data_.data() + data_.size(), str.c_str(), str.size());
        }
        auto AppendStrView(const std::string_view &str_view) -> void
        {
            data_.resize(str_view.size() + data_.size());
            std::memcpy(data_.data() + data_.size(), str_view.data(), str_view.size());
        }

        template <typename T, std::enable_if_t<std::is_arithmetic_v<T>, bool> = true>
        auto AppendNum(const T &num, size_t N) -> void
        {
            data_.resize(data_.size() + N);
            memcpy(data_.data() + data_.size(), &num, N);
        }

        template <typename T, std::enable_if_t<std::is_arithmetic_v<T>, bool> = true>
        auto CoverNum(const T &num, size_t index, size_t N) -> void
        {
            assert(index < data_.size());
            data_.resize(std::max(data_.size(), index + N));
            memcpy(data_.data() + index, &num, N);
        }

        auto AppendBytes(Bytes &&bytes) -> void
        {
            if (this == &bytes)
            {
                return;
            }
            data_.insert(data_.end(), std::make_move_iterator(bytes.data_.begin()), std::make_move_iterator(bytes.data_.end()));
            bytes.data_.clear();
        }

        auto GetStrView(size_t len) -> std::string_view
        {
            const size_t read_size = std::min(len, data_.size() - pos_);
            auto str_view = std::string_view(reinterpret_cast<const char *>(data_.data() + pos_), read_size);
            pos_ += read_size;
            return str_view;
        }
        template <typename T, std::enable_if_t<std::is_arithmetic_v<T>, bool> = true>
        auto GetNum(size_t len) -> T
        {
            const size_t read_size = std::min(len, data_.size() - pos_);
            T num;
            std::memcpy(&num, data_.data() + pos_, read_size);
            pos_ += read_size;
            return num;
        }
    };

}

#endif