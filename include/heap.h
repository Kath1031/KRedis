#ifndef HEAP_H
#define HEAP_H

#include <vector>
#include "public.h"

namespace kath
{
    struct HeapItem
    {
        uint64_t val_{0};
        size_t *ref_{nullptr};
        HeapItem() = default;
        HeapItem(uint64_t val, size_t *ref) : val_(val), ref_(ref) {}
        bool operator<(const HeapItem &other) const { return val_ < other.val_; }
        bool operator>(const HeapItem &other) const { return val_ > other.val_; }
        bool operator<=(const HeapItem &other) const { return val_ <= other.val_; }
        bool operator>=(const HeapItem &other) const { return val_ >= other.val_; }
        void ChangeRef(size_t pos) const { *ref_ = pos; }

        // friend std::ostream &operator<<(std::ostream &os,const HeapItem  &node) {
        //     return os << Class2Str("HeapItem ","val",node.val_,"ref",node.ref_);
        // }
    };

    // idx start at 0
    // parent (i+1)/2-1
    // left_son i*2+1
    // right_son i*2+2
    class Heap
    {
    private:
        std::vector<HeapItem> data_;
        static auto LeftSon(size_t index) -> size_t
        {
            return index * 2 + 1;
        }
        static auto RightSon(size_t index) -> size_t
        {
            return index * 2 + 2;
        }
        static auto Parent(size_t index) -> size_t
        {
            return (index + 1) / 2 - 1;
        }
        auto Up(size_t cur) -> void
        {
            assert(cur < data_.size());

            auto cur_item = data_[cur];
            std::size_t next;
            while (cur)
            {
                next = Parent(cur);
                if (data_[next] <= cur_item)
                    break;
                data_[cur] = data_[next];
                data_[cur].ChangeRef(cur);
                cur = next;
            }
            data_[cur] = cur_item;
            data_[cur].ChangeRef(cur);
        }
        auto Down(size_t cur) -> void
        {
            assert(cur < data_.size());
            std::size_t next;
            auto cur_item = data_[cur];

            while (LeftSon(cur) < data_.size())
            {
                next = LeftSon(cur);
                if (RightSon(cur) < data_.size() && data_[RightSon(cur)] < data_[next])
                {
                    next = RightSon(cur);
                }
                if (data_[cur] <= data_[next])
                {
                    break;
                }
                data_[cur] = data_[next];
                data_[cur].ChangeRef(cur);
                cur = next;
            }
            data_[cur].ChangeRef(cur);
        }

        auto Update(size_t pos) -> void
        {
            if (pos && data_[Parent(pos)] > data_[pos])
            {
                Up(pos);
            }
            else
            {
                Down(pos);
            }
        }

    public:
        Heap() {}

        friend std::ostream &operator<<(std::ostream &os, const Heap &heap)
        {
            // todo
            Msg("还没实现");

            return os;
        }

        [[nodiscard]] auto Empty() const -> bool { return data_.empty(); }
        auto Get(size_t pos) const
        {
            assert(pos < data_.size());
            return data_[pos].val_;
        }

        auto GetMinRef() const -> size_t *
        {
            assert(!Empty());
            return data_.front().ref_;
        }

        auto Del(size_t pos) -> void
        {
            data_[pos] = data_.back();
            data_.pop_back();
            if (pos < data_.size())
            {
                Update(pos);
            }
        }
        auto Push(uint64_t val, size_t *entry_heap_idx) -> void
        {
            data_.emplace_back(HeapItem{val, entry_heap_idx});
            Update(data_.size() - 1);
        }

        auto Set(size_t pos, uint64_t val) -> void
        {
            data_[pos].val_ = val;
            Update(pos);
        }

        // 偷懒用了递归的写法，换成迭代其实也很简单，不过这样做并不重要就没改了
        auto Check(size_t pos = 0) -> bool
        {
            bool flag = true;
            if (LeftSon(pos) < data_.size())
            {
                assert(data_[pos] <= data_[LeftSon(pos)]);
                flag &= Check(LeftSon(pos));
            }
            if (RightSon(pos) < data_.size())
            {
                assert(data_[pos] <= data_[RightSon(pos)]);
                flag &= Check(RightSon(pos));
            }
            return pos;
        }
    };

}

#endif