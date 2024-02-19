#ifndef ZSET_H
#define ZSET_H
#include "avl_base.h"
#include "hashtable.h"
#include <string>
namespace kath
{

    struct ZNode : public avl::AVLNode, HNode
    {
        std::string name_;
        double score_;
        ZNode() = delete;
        friend auto operator<<(std::ostream &os,
                               const ZNode &node) -> std::ostream &
        {
            return os << Class2Str("ZNode", "name", node.name_, "score", node.score_);
        }
    };
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
    class ZSet{

    };
}

#endif