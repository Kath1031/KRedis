#ifndef EXEC_H
#define EXEC_H

#include "public.h"
#include "bytes.h"
#include "hashtable.h"
#include "heap.h"
// 序列化 serialization
namespace kath
{
    using Cmd = std::vector<std::string>;
    auto OutNil(Bytes &out) -> void
    {
        out.AppendNum(static_cast<uint8_t>(SerType::NIL), 1);
    }

    auto OutStr(Bytes &out, const std::string &str) -> void
    {
        out.AppendNum(static_cast<uint8_t>(SerType::STR), 1);
        out.AppendNum<size_t>(str.size(), 4);
        out.AppendStr(str);
    }

    auto OutInt(Bytes &out, int64_t val) -> void
    {
        out.AppendNum(static_cast<uint8_t>(SerType::INT64), 1);
        out.AppendNum(val, 8);
    }

    auto OutErr(Bytes &out, CmdErr code, const std::string &msg)
    {
        out.AppendNum(static_cast<uint8_t>(SerType::ERR), 1);
        out.AppendNum(static_cast<uint32_t>(code), 4);
        out.AppendNum(msg.size(), 4);
        out.AppendStr(msg);
    }

    auto OutDouble(Bytes &out, double val) -> void
    {
        out.AppendNum(static_cast<uint8_t>(SerType::DOU), 1);
        out.AppendNum(val, 8);
    }
    auto OutArr(Bytes &out, uint32_t n) -> void
    {
        out.AppendNum(static_cast<uint8_t>(SerType::ARR), 1);
        out.AppendNum(n, 4);
    }
    auto OutUpdateArr(Bytes &out, uint32_t n) -> void
    {
        auto type = static_cast<SerType>(out.GetNum<int>(1));
        assert(type == SerType::ARR);
        out.CoverNum(n, 1, 4);
    }

    namespace core
    {
        class CoreException : public std::exception
        {
        public:
            CoreException(CmdErr code, std::string msg)
                : code_(code), msg_(msg) {}
            [[nodiscard]] const char *what() const noexcept override
            {
                return msg_.c_str();
            }
            [[nodiscard]] CmdErr Code() const noexcept { return code_; }

        private:
            CmdErr code_;
            std::string msg_;
        };
        enum class EntryType
        {
            T_STR = 1,
            T_ZSET = 2,
        };
        struct Entry : public HNode
        {
            EntryType type_;
            std::string key_;
            std::string val_;
            // std::shared_ptr<ZSet> zset_;
            size_t heap_index_;
            Entry() = default;
            ~Entry() = default;
            Entry(const std::string &key)
                : HNode(string_hash(key)), type_(EntryType::T_STR), key_(key), heap_index_(0)
            {
            }
            Entry(std::string &key, const std::string &value)
                : HNode(string_hash(key)), type_(EntryType::T_STR), key_(key), val_(value), heap_index_(0)
            {
            }
        };

        HMap m_map{};
        Heap m_heap{};
        using EntryPtr = std::shared_ptr<Entry>;
        NodeCmp EntryEq = [](HNodePtr lhs, HNodePtr rhs) -> bool
        {
            auto le = dyn_cast<Entry, HNode>(lhs);
            auto re = dyn_cast<Entry, HNode>(rhs);
            return lhs->hcode_ == rhs->hcode_ && le->key_ == re->key_;
        };
        auto Scan(Bytes &buf) -> void
        {
            NodeScan node_scan = [](HNodePtr node, void *arg)
            {
                Bytes &buf = *(Bytes *)arg;
                OutStr(buf, dyn_cast<Entry, HNode>(node)->key_);
            };
            m_map.Scan(node_scan, &buf);
        }
        auto Get(const std::string &key) -> std::optional<std::string>
        {
            HNodePtr node = std::make_shared<Entry>(key);
            HNodePtr target = m_map.Loockup(node, EntryEq);
            if (!target)
                return {};
            std::shared_ptr<Entry> entry = dyn_cast<Entry, HNode>(target);
            if (entry->type_ != EntryType::T_STR)
            {
                throw CoreException(CmdErr::ERR_TYPE, "expect string");
            }
            return std::make_optional<std::string>(entry->val_);
        }
        auto Set(const std::string &key, const std::string &value) -> void
        {
            HNodePtr tmp_entry = std::make_shared<Entry>(key);
            HNodePtr target_node = m_map.Loockup(tmp_entry, EntryEq);
            if (target_node != nullptr)
            {
                EntryPtr target_entry = dyn_cast<Entry, HNode>(target_node);
                if (target_entry->type_ != EntryType::T_STR)
                {
                    throw CoreException(CmdErr::ERR_TYPE, "except string");
                }
                target_entry->val_ = value;
            }
            else
            {
                HNodePtr new_entry = std::make_shared<Entry> (key,value);
                m_map.Insert(new_entry);
            }
        }
        auto Del(const std::string &key) -> bool
        {
            std::shared_ptr<HNode> entry = std::make_shared<Entry>(key);
            HNodePtr node = m_map.Pop(entry, EntryEq);
            if (node != nullptr)
            {
                return true;
            }
            return false;
        }

    } // namespace core
    auto CmdEq(const std::string_view word, const std::string_view cmd) -> bool
    {
        return word == cmd;
    }

    auto DoKeys(const Cmd &cmd, Bytes &out) -> void
    {
        OutArr(out, core::m_map.Size());
        core::Scan(out);
    }
    auto DoDel(const Cmd &cmd, Bytes &out) -> void
    {
        OutInt(out, core::Del(cmd[1]));
    }
    auto DoGet(const Cmd &cmd, Bytes &out) -> void
    {
        try
        {
            auto res = core::Get(cmd[1]);
            if (res.has_value())
            {
                OutStr(out, res.value());
            }
            else
            {
                OutNil(out);
            }
        }
        catch (const core::CoreException &e)
        {
            auto code = e.Code();
            std::string msg = e.what();
            OutErr(out, code, msg);
        }
    }
    auto DoSet(const Cmd &cmd, Bytes &out) -> void
    {
        try
        {
            core::Set(cmd[1], cmd[2]);
            OutNil(out);
        }
        catch (const core::CoreException &err)
        {
            auto code = err.Code();
            std::string msg = err.what();
            OutErr(out, code, msg);
        }
    }
    auto interpret(Cmd &cmd, Bytes &out) -> void
    {
        if (cmd.size() == 1 && CmdEq(cmd[0], "key"))
        {
            DoKeys(cmd, out);
        }
        else if (cmd.size() == 2 && CmdEq(cmd[0], "get"))
        {
            DoGet(cmd, out);
        }
        else if (cmd.size() == 2 && CmdEq(cmd[0], "del"))
        {
            DoDel(cmd, out);
        }
        else if (cmd.size() == 3 && CmdEq(cmd[0], "set"))
        {
            DoSet(cmd, out);
        }
    }
}

#endif