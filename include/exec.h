#ifndef EXEC_H
#define EXEC_H

#include "public.h"
#include "bytes.h"
#include "hashtable.h"
#include "zset.h"
#include "heap.h"
// 序列化 serialization
inline auto str2dbl(const std::string &str, double &out) -> bool
{
    char *endp = nullptr;
    out = strtod(str.c_str(), &endp);
    return endp == str.c_str() + str.size() && !std::isnan(out);
}
static auto str2int(const std::string &str, int64_t &out) -> bool
{
    char *endp = nullptr;
    out = strtoll(str.c_str(), &endp, 10);
    return endp == str.c_str() + str.size();
}
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
            std::shared_ptr<ZSet> zset_;
            size_t heap_index_;
            Entry() = default;
            ~Entry() { SetTTL(-1); };
            Entry(const std::string &key)
                : HNode(string_hash(key)), type_(EntryType::T_STR), key_(key), zset_{nullptr}, heap_index_(0)
            {
            }
            Entry(const std::string &key, const std::string &value)
                : HNode(string_hash(key)), type_(EntryType::T_STR),
                  key_(key), val_(value), zset_{nullptr}, heap_index_(0)
            {
            }
            auto SetTTL(int64_t ttl_ms) -> void
            {
                if (ttl_ms < 0 && heap_index_ != 0)
                {
                    size_t pos = heap_index_;
                }
                else if (ttl_ms >= 0)
                {
                    size_t pos = heap_index_;
                    uint64_t now_time = GetMonotonicUsec() + ttl_ms * 1000;
                    if (pos == 0)
                    {
                        m_heap.Push(now_time, &heap_index_);
                    }
                    else
                    {
                        m_heap.Set(pos, now_time);
                    }
                }
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
            HNodePtr target = m_map.Lookup(node, EntryEq);
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
            HNodePtr target_node = m_map.Lookup(tmp_entry, EntryEq);
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
                HNodePtr new_entry = std::make_shared<Entry>(key, value);
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

        auto GetZsetEntry(const std::string &key) -> EntryPtr
        {
            EntryPtr entry_ptr = std::make_shared<Entry>(key);
            HNodePtr hnode = m_map.Lookup(entry_ptr, EntryEq);
            if (hnode == nullptr)
                return nullptr;

            EntryPtr ent = dyn_cast<Entry, HNode>(hnode);
            if (ent->type_ == EntryType::T_STR)
                return nullptr;
            return ent;
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
    auto DoZAdd(Cmd &cmd, Bytes &out) -> void
    {
        double score = 0;
        if (!str2dbl(cmd[2], score))
        {
            return OutErr(out, CmdErr::ERR_ARG, "expect fp number");
        }

        core::EntryPtr key = std::make_shared<core::Entry>(cmd[1]);
        HNodePtr hnode = core::m_map.Lookup(key, core::EntryEq);
        core::EntryPtr ent = nullptr;
        if (hnode == nullptr)
        {
            ent = std::make_shared<core::Entry>(std::move(key->key_), "", key->hcode_);
            ent->type_ = core::EntryType::T_ZSET;
            ent->zset_ = std::make_shared<ZSet>();
            core::m_map.Insert(ent);
        }
        else
        {
            ent = dyn_cast<core::Entry, HNode>(hnode);
            if (ent->type_ != core::EntryType::T_ZSET)
            {
                return OutErr(out, CmdErr::ERR_TYPE, "expect zset");
            }
        }
        const std::string &name = cmd[3];
        auto ok = ent->zset_->Add(name, score);
        OutInt(out, static_cast<int64_t>(ok));
    }
    auto DoZRem(Cmd &cmd, Bytes &out) -> void
    {
        core::EntryPtr ent = core::GetZsetEntry(cmd[1]);
        if (!ent)
        {
            OutNil(out);
            return;
        }
        const std::string &name = cmd[2];
        auto ok = ent->zset_->Pop(name);
        OutInt(out, (int64_t)ok);
    }
    auto DoZScore(Cmd &cmd, Bytes &out) -> void
    {
        core::EntryPtr entry_ptr = core::GetZsetEntry(cmd[1]);
        if (!entry_ptr)
        {
            OutNil(out);
            return;
        }
        const std::string &name = cmd[2];
        auto res = entry_ptr->zset_->Find(name);
        if (res.has_value())
        {
            OutDouble(out, res.value());
        }
        else
        {
            OutNil(out);
        }
    }
    auto DoZQuery(const Cmd &cmd, Bytes &out) -> void
    {
        double score = 0;
        if (!str2dbl(cmd[2], score))
        {
            return OutErr(out, CmdErr::ERR_ARG, "expect fp number");
        }
        const std::string &name = cmd[3];
        int64_t offset = 0;
        int64_t limit = 0;
        if(!str2int(cmd[4],offset)) {
            return OutErr(out,CmdErr::ERR_ARG,"expect int");
        }
        if(!str2int(cmd[5],limit)) {
            return OutErr(out,CmdErr::ERR_ARG,"expect int");
        }

        // get the zset
        core::EntryPtr ent_ptr = core::GetZsetEntry(cmd[1]);
        if (!ent_ptr) {
            OutErr(out,CmdErr::ERR_TYPE, "expect zset");
            return ;
        }
        if(limit &1) limit++;
        limit >>=1;
        auto ite = ent_ptr->zset_->Query(name,score,offset,limit);

        uint32_t n =0;
        Bytes buff;
        for (auto sam :ite) {
            OutStr(buff,sam.name_);
            OutDouble(buff,sam.score_);
            n+=2;
        }
        OutArr(out,n);
        out.AppendBytes(std::move(buff));
    }
    auto Interpret(Cmd &cmd, Bytes &out) -> void
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
        else if (cmd.size() == 4 && CmdEq(cmd[0], "zadd"))
        {
            DoZAdd(cmd, out);
        }
        else if (cmd.size() == 3 && CmdEq(cmd[0], "zrem"))
        {
            DoZRem(cmd, out);
        }
        else if (cmd.size() == 3 && CmdEq(cmd[0], "zscore"))
        {
            DoZScore(cmd, out);
        }
        else if (cmd.size() == 6 && CmdEq(cmd[0], "zquery"))
        {
            DoZQuery(cmd, out);
        }
    }
}

#endif