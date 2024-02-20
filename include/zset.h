#ifndef ZSET_H
#define ZSET_H
#include "avl_base.h"
#include "hashtable.h"
#include <string>
namespace kath
{
    struct HKey : public HNode
    {
        std::string name_;
        HKey() = delete;
        virtual ~HKey() = default;
        HKey(const std::string &name) : HNode(string_hash(name)), name_(name) {}
    };
    struct ZNode : public avl::AVLNode, HKey
    {
        // std::string name_;
        double score_;
        ZNode() = delete;
        ~ZNode() = default;
        ZNode(const std::string &name, double score)
            : HKey(name), avl::AVLNode(), score_(score) {}
        friend auto operator<<(std::ostream &os,
                               const ZNode &node) -> std::ostream &
        {
            return os << Class2Str("ZNode", "name", node.name_, "score", node.score_);
        }
    };
    using HKeyPtr = std::shared_ptr<HKey>;
    using ZNodePtr = std::shared_ptr<ZNode>;

    auto ZLess(avl::AVLNodePtr lhs, double score, const std::string &name) -> bool
    {
        ZNodePtr zl = dyn_cast<ZNode, avl::AVLNode>(lhs);
        if (zl->score_ != score)
        {
            return zl->score_ < score;
        }
        return zl->name_ < name;
    }
    auto ZLess(avl::AVLNodePtr lhs, avl::AVLNodePtr rhs)
    {
        ZNodePtr zr = dyn_cast<ZNode, avl::AVLNode>(rhs);
        return ZLess(lhs, zr->score_, zr->name_);
    }

    class ZNodeCollection
    {
    private:
        ZNodePtr data_;
        int64_t length_;

    public:
        class iterator
        {
        private:
            ZNodePtr ptr_;
            int64_t step_;

        public:
            iterator(ZNodePtr ptr, int64_t step) : ptr_(ptr), step_(step) {}
            iterator &operator++()
            {
                ptr_ = dyn_cast<ZNode, avl::AVLNode>(avl::AVLOperate::Offset(ptr_, +1));
                step_++;
                return *this;
            }
            auto operator!=(const iterator &other) const -> bool
            {
                if (!ptr_)
                    return false;
                return step_ < other.step_;
            }
            auto operator*() -> ZNode &
            {
                assert(ptr_);
                return *ptr_;
            }
        }; // class iterator

        ZNodeCollection(ZNodePtr data, int64_t length)
            : data_(data), length_(length) {}
        auto begin() -> iterator { return iterator(data_, 0); }
        auto end() -> iterator { return iterator(data_, length_); }
        auto cbegin() const -> iterator { return iterator(data_, 0); }
        auto cend() const -> iterator { return iterator(data_, length_); }

        int64_t size() const { return length_; }

        friend auto operator<<(std::ostream &os, const ZNodeCollection &znode_collection)
            -> std::ostream &
        {
            return os << Class2Str("ZNodeCollection", "data_", znode_collection.data_,
                                   "length_", znode_collection.length_);
        }
    }; // class ZNodeCollection
    NodeCmp hnode_cmp = [](HNodePtr node, HNodePtr key) -> bool
    {
        if (node->hcode_ != key->hcode_)
        {
            return false;
        }
        ZNodePtr znode_ptr = dyn_cast<ZNode, HNode>(node);
        HKeyPtr hkey_ptr = dyn_cast<HKey, HNode>(node);
        return znode_ptr->name_ == hkey_ptr->name_;
    };
    class ZSet
    {
    private:
        avl::AVLNodePtr avl_root_{nullptr};
        HMap hmap_{};
        auto TreeAdd(ZNodePtr node) -> void
        {
            if (avl_root_ == nullptr)
            {
                avl_root_ = node;
                return;
            }
            avl::AVLNodePtr cur = avl_root_;
            for (;;)
            {
                avl::AVLNodePtr *from = ZLess(node, cur) ? &cur->left_ : &cur->right_;
                if (*from = nullptr)
                {
                    *from = node;
                    node->parent_ = cur;
                    avl_root_ = avl::AVLOperate::Fix(node);
                    break;
                }
                cur = *from;
            }
        }
        auto Lookup(const std::string &name) -> ZNodePtr
        {
            if (avl_root_ == nullptr)
                return nullptr;
            // HKey key(name);
            HKeyPtr key = std::make_shared<HKey>(name);
            HNodePtr found = hmap_.Lookup(key, hnode_cmp);
            if (found == nullptr)
                return {};
            return dyn_cast<ZNode, HNode>(found);
        }

    public:
        ~ZSet() = default;
        auto Add(const std::string &name, double score) -> bool
        {
            ZNodePtr node = Lookup(name);
            if (node == nullptr)
            {
                node = std::make_shared<ZNode>(name, score);
                hmap_.Insert(node);
                TreeAdd(node);
                return true;
            }
            Update(node, score);
            return false;
        }
        auto Update(ZNodePtr node, double score) -> void
        {
            if (node->score_ == score)
            {
                return;
            }
            avl_root_ = avl::AVLOperate::Delete(node);
            TreeAdd(std::make_shared<ZNode>(node->name_, score));
        }
        auto Find(const std::string &name) -> std::optional<double>
        {
            if (avl_root_ == nullptr)
                return std::optional<double>();
            HKeyPtr key = std::make_shared<HKey>(name);
            HNodePtr found = hmap_.Lookup(key, hnode_cmp);
            if (found == nullptr)
                return std::optional<double>();
            return dyn_cast<ZNode, HNode>(found)->score_;
        }
        auto Pop(const std::string &name) -> bool
        {
            if (avl_root_ == nullptr)
                return false;
            HKeyPtr key = std::make_shared<HKey>(name);
            HNodePtr found = hmap_.Pop(key, hnode_cmp);
            if (found == nullptr)
                return false;
            ZNodePtr znode_ptr = dyn_cast<ZNode, HNode>(found);
            avl_root_ = avl::AVLOperate::Delete(znode_ptr);
            return true;
        }
        auto Query(const std::string &name, double score, int64_t offset) -> ZNodePtr
        {
            avl::AVLNodePtr found = nullptr;
            avl::AVLNodePtr cur = avl_root_;
            while (cur)
            {
                if (ZLess(cur, score, name))
                {
                    cur = cur->right_;
                }
                else
                {
                    found = cur;
                    cur = cur->left_;
                }
            }
            if (found != nullptr)
                found = avl::AVLOperate::Offset(found, offset);
            return found ? dyn_cast<ZNode, avl::AVLNode>(found) : nullptr;
        }

        auto Query(const std::string &name,
                   double score, int64_t offset, int64_t limit) -> ZNodeCollection
        {
            avl::AVLNodePtr found = nullptr;
            avl::AVLNodePtr cur = avl_root_;
            while (cur)
            {
                if (ZLess(cur, score, name))
                {
                    cur = cur->right_;
                }
                else
                {
                    found = cur;
                    cur = cur->left_;
                }
            }
            if (found)
                found = avl::AVLOperate::Offset(found, offset);
            return found ? ZNodeCollection(dyn_cast<ZNode, avl::AVLNode>(found), limit)
                         : ZNodeCollection(nullptr, 0);
        }
    };
}

#endif