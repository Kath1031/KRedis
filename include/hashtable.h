#ifndef HASHTABLE_H
#define HASHTABLE_H
#include <vector>
#include <iostream>
#include <functional>
#include <optional>

#include "public.h"

// 本身并不保证线程安全，需要用户自行保证
namespace kath
{
    struct HNode;
    using HNodePtr = std::shared_ptr<HNode>;
    using HNodeWPtr = std::weak_ptr<HNode>;

    using NodeCmp = std::function<bool(HNodePtr, HNodePtr)>; // 并不保证Cmp的参数非空指针
    using NodeScan = std::function<void(HNodePtr, void *)>;
    using NodeDispose = std::function<void(HNodePtr)>;

    const size_t HASH_TABLE_INIT_SIZE = 4;
    const size_t Load_FACTOR_MAX = 4;
    const size_t K_RESIZING_WORK = 128;

    struct HNode
    {
        HNodePtr next_;
        size_t hcode_; // cached hash value
        HNode() = delete;
        HNode(size_t hcode) : next_(nullptr), hcode_(hcode) {}
    };
    class HTab
    {
        friend class HMap;

    private:
        std::vector<HNodePtr> table_;
        size_t mask_;
        size_t size_;

    public:
        // n必须是 是2^k
        HTab(size_t n) : mask_(n - 1), size_(0), table_{n}
        {
            std::cout << "HTab:" << n << " table_.size():" << table_.size() << std::endl;
        }
        auto GetLoadFactor() const -> size_t { return size_ / (mask_ + 1); }
        auto CheckLoadFactor() const -> bool { return size_ && GetLoadFactor() >= Load_FACTOR_MAX; }

        auto GetIndex(size_t hcode) const -> size_t { return hcode & mask_; }
        auto Insert(HNodePtr node) -> void
        {
            size_t pos = node->hcode_ & mask_;
            node->next_ = table_[pos];
            table_[pos] = node;
            size_++;
        }

        // 返回key的上一个节点
        // 如果不存在上一个节点则返回空指针
        auto LookupPre(HNodePtr key, NodeCmp cmp) -> HNodePtr
        {
            size_t index = GetIndex(key->hcode_);
            auto pre_node = table_[index];
            while (pre_node != nullptr)
            {
                if (cmp(pre_node->next_, key))
                    return pre_node;
                pre_node = pre_node->next_;
            }
            return nullptr;
        }
        //单纯用来查找的
        auto Lookup(HNodePtr key, NodeCmp cmp) -> HNodePtr
        {
            size_t index = GetIndex(key->hcode_);
            auto cur_node = table_[index];
            while (cur_node != nullptr)
            {
                if (cmp(cur_node, key))
                    return cur_node;
                cur_node = cur_node->next_;
            }
            return nullptr;
        }

        // 参数是需要删除的节点的上一个节点
        auto DetachByPre(HNodePtr pre_node) -> HNodePtr
        {
            if (pre_node == nullptr)
            {
                return nullptr;
            }
            auto node = pre_node->next_;
            pre_node->next_ = pre_node->next_->next_;
            --size_;
            return node;
        }
        auto DetachFront(size_t index) -> HNodePtr
        {
            auto cur_node = table_[index];
            table_[index] = cur_node->next_;
            --size_;
            return cur_node;
        }
        // 删除key这个节点
        auto Detach(HNodePtr key, NodeCmp cmp) -> HNodePtr
        {
            auto pre_node = LookupPre(key, cmp);
            if (pre_node == nullptr)
            {
                size_t index = GetIndex(key->hcode_);
                if (cmp(key, table_[index]))
                {
                    return DetachFront(index);
                }
                return nullptr;
            }
            return DetachByPre(pre_node);
        }

        auto Scan(NodeScan node_scan, void *extra) const -> void
        {
            if (!size_)
                return;
            for (auto cur_node : table_)
            {
                while (cur_node != nullptr)
                {
                    node_scan(cur_node, extra);
                    cur_node = cur_node->next_;
                }
            }
        }

        auto Dispose(NodeDispose node_dispose) -> void
        {
            if (!size_)
                return;
            for (auto cur_node : table_)
            {
                while (cur_node != nullptr)
                {
                    auto next_node = cur_node->next_;
                    node_dispose(cur_node);
                    cur_node = next_node;
                }
            }
        }
    };
    class HMap
    {
    private:
        HTab ht1_{HASH_TABLE_INIT_SIZE}, ht2_{HASH_TABLE_INIT_SIZE};
        size_t resizing_pos{0};

    public:
        HMap() = default;
        auto Size() const -> size_t { return ht1_.size_ + ht2_.size_; }

        auto ResizingHlep() -> void
        {
            if (!ht2_.size_)
            {
                return;
            }
            size_t nwork = 0;
            while (nwork < K_RESIZING_WORK && ht2_.size_ != 0)
            {
                if (ht2_.table_[resizing_pos] == nullptr)
                {
                    ++resizing_pos;
                    continue;
                }
                ++nwork;
                ht1_.Insert(ht2_.DetachFront(resizing_pos));
            }
        }
        auto Resizing() -> void
        {
            assert(ht2_.size_ == 0);
            ht2_ = std::move(ht1_);
            ht1_ = std::move(ht2_.size_ * 2);
            resizing_pos = 0;
        }
        auto Insert(HNodePtr node) -> void
        {
            ht1_.Insert(node);

            if (ht1_.CheckLoadFactor())
            {
                Resizing();
            }
            ResizingHlep();
        }
        auto Loockup(HNodePtr key, NodeCmp cmp) -> HNodePtr
        {
            ResizingHlep();
            auto res = ht1_.Lookup(key, cmp);
            if (res == nullptr)
                res = ht2_.Lookup(key, cmp);
            return res;
        }
        auto Pop(HNodePtr key, NodeCmp cmp) -> HNodePtr
        {
            if(ht1_.Lookup(key,cmp)!=nullptr) {
                return ht1_.Detach(key,cmp);
            }
            return ht2_.Detach(key,cmp);
        }
        auto Scan(NodeScan node_scan, void *extra) -> void
        {
            ht1_.Scan(node_scan, extra);
            ht2_.Scan(node_scan, extra);
        }
        auto Dispose(NodeDispose node_dispose) -> void
        {
            ht1_.Dispose(node_dispose);
            ht2_.Dispose(node_dispose);
        }
    };
    uint64_t string_hash(const std::string &str) {
        uint64_t hash = 0;
        for(auto &c:str) {
            hash = hash*257 + static_cast<uint8_t>(c);
        }
        return hash;
    }
} // namespace kath

#endif