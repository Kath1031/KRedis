#ifndef KFILE_H
#define KFILE_H

#include <errno.h>
#include <fcntl.h>
#include <unistd.h>

#include "bytes.h"
namespace kath
{
    auto FDSetNB(int fd) -> void
    {
        auto errno_backup = errno;
        errno = 0;

        int flag = fcntl(fd, F_GETFL, 0);
        if (errno != 0)
        {
            Err("fcntl err");
        }

        flag |= O_NONBLOCK;

        fcntl(fd, F_SETFL, flag);

        if (errno != 0)
        {
            Err("fcmtl err");
        }
        errno = errno_backup;
    }

    class File
    {
    private:
        int fd_;

    public:
        File(int fd = -1) : fd_(fd){};
        ~File();
        File(File &&other) : fd_(other.fd_) { other.fd_ = -1; }
        File(File &) = delete;
        File &operator=(const File &) = delete;
        [[nodiscard]] auto Data() const -> int { return fd_; }
        auto SetData(int fd) -> void { fd_ = fd; }
        auto SetNb() -> void { FDSetNB(fd_); }

        auto Check() const -> bool
        {
            auto errno_backup = errno;
            errno = 0;
            fcntl(fd_, F_GETFL, 0);
            if (errno)
            {
                Err("file check fail");
                return false;
            }
            errno = errno_backup;
            return true;
        }

        // 从bytes中以阻塞模式写入fd，默认是全部写出
        auto WriteByte_b(Bytes &bytes, size_t len = -1) -> bool
        {
            // 阻塞IO
            len = std::min(len, bytes.data_.size() - bytes.pos_);
            // 轮询，直到读入
            while (len)
            {
                ssize_t write_len = write(fd_, bytes.data_.data() + bytes.pos_, len);

                assert((write_len >= 0) && (static_cast<size_t>(write_len) <= len));

                bytes.pos_ += write_len;
                len -= write_len;
            }
            return true;
        }
        // 从fd中以阻塞模式写入bytes
        auto ReadByte_b(Bytes &bytes, size_t len) -> bool
        {
            // 阻塞IO

            // 准备好足够的空间用来从fd中读数据
            bytes.data_.resize(bytes.Size() + len);

            while (len)
            {
                ssize_t read_len = read(fd_, bytes.data_.data() + bytes.data_.size() - len, len);
                if (read_len <= 0)
                {
                    bytes.data_.resize(bytes.data_.size() - len);
                    // 具体是不是读到EOF需要结合errno的值判断
                    return false;
                }
                assert(static_cast<size_t>(read_len) <= len);
                len -= static_cast<size_t>(read_len);
            }
            return true;
        }

        // 从bytes中以非阻塞模式写入fd
        // 返回值含义   1:失败   0:写完    -1:阻塞
        auto WriteByte_nb(Bytes &bytes) -> int
        {
            ssize_t write_len = 0;
            do
            {
                write_len = write(fd_, bytes.data_.data() + bytes.pos_, bytes.Size() - bytes.pos_);
            } while (write_len < 0 && errno == EINTR);
            if (write_len < 0)
            {
                if (errno == EAGAIN)
                {
                    return -1;
                }
                return 1;
            }
            bytes.pos_ += write_len;
            assert(bytes.pos_ <= bytes.Size());
            return bytes.pos_ != bytes.Size();
        }

        // 从fd中以非阻塞模式写入bytes
        // 返回值含义   1:失败   0:写成功    -1:阻塞    2:读到EOF
        auto ReadByte_nb(Bytes &bytes, size_t len) -> int
        {
            size_t last_size = bytes.Size();
            ssize_t read_len = 0;
            bytes.data_.resize(bytes.Size() + len);

            do
            {
                read_len = read(fd_, bytes.data_.data() + last_size, len);
            } while (read_len < 0 && errno == EINTR);

            if(read_len >0 ){
                bytes.data_.resize(last_size + read_len);
                return 0;
            }

            bytes.data_.resize(last_size);
            if(read_len == 0) {
                return 2;
            }
            if (read_len < 0 &&errno == EAGAIN)
            {
                return -1;
            }
            return 1;
        }
    };

}

#endif
