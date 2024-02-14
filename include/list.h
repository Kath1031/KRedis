#ifndef LIST_H
#define LIST_H

namespace kath
{
    struct DList
    {
        DList *prev_;
        DList *next_;
        // 初始化为自环
        DList() : prev_(this), next_(this) {}
        [[nodiscard]] auto Empty() const -> bool { return next_ == this; }

        void Detach() const
        {
            prev_->next_ = next_;
            next_->prev_ = prev_;
        }
        void InsertFront(DList *node)
        {
            node->prev_ = prev_;
            node->next_ = this;
            prev_->next_ = node;
            prev_ = node;
        }
    };
}

#endif