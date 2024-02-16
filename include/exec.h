#ifndef EXEC_H
#define EXEC_H

#include "public.h"
#include "bytes.h"
#include "hashtable.h"
#include "heap.h"
// 序列化 serialization
namespace kath
{
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
        HMap m_map{};
        Heap m_heap{};
        enum class EntryType
        {
            T_STR = 1,
            T_ZSET = 2,
        };
        struct Entry
        {
            HNode node_;
            EntryType type_;
            std::string key_;
            std::string val_;
            // std::shared_ptr<ZSet> zset_;
            size_t heap_index;
            Entry() = default;
            Entry(const std::string &key)
                : type_(EntryType::T_STR), node_(string_hash(key)), key_(key), val_{}, heap_index(0)
            {
            }
        };
        NodeCmp EntryEq = [](HNodePtr lhs, HNodePtr rhs) -> bool
        {
            Entry *le = container_of(lhs.get(), Entry, node_);
            Entry *re = container_of(rhs.get(), Entry, node_);
            return lhs->hcode_ == rhs->hcode_ && le->key_ == re->key_;
        };
        auto Scan(Bytes &buf) -> void
        {
            NodeScan node_scan = [](HNodePtr node, void *arg)
            {
                Bytes &buf = *(Bytes *)arg;
                OutStr(buf, container_of(node.get(), Entry, node_)->key_);
            };
            m_map.Scan(node_scan, &buf);
        }

        auto Del(const std::string &key) -> bool
        {
            Entry entry(key);
            HNodePtr node = m_map.Pop(&entry.node_,EntryEq);

            return false;
        }

    } // namespace core

    void DoKeys(const std::vector<std::string> &cmd, Bytes &out)
    {
        OutArr(out, core::m_map.Size());
        core::Scan(out);
    }
    void DoDel(const std::vector<std::string> &cmd, Bytes &out)
    {
        OutInt(out, core::Del(cmd[1]));
    }
}

#endif