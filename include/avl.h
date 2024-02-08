#ifndef AVL_H
#define AVL_H

/*
参考以下博客
https://oi-wiki.org/ds/avl/
https://en.wikipedia.org/wiki/AVL_tree

在AVLNode中可以不存父亲，需要使用的时候存一下就好。但可能会使得代码更臃肿，所以这里没有多想，直接写了。


*/

#include "public.h"

namespace kath
{
    struct AVLNode;
    using AVLNodePtr = std::shared_ptr<AVLNode>;
    using AVLNodeWPtr = std::weak_ptr<AVLNode>;
    struct AVLNode
    {
    public:
        uint32_t depth_{1};
        uint32_t size_{1};
        AVLNodePtr left_;
        AVLNodePtr right_;
        AVLNodeWPtr parent_; // 为了使共享指针的拓扑图无环，所以父节点用weak_ptr存

    public:
        static auto Depth(AVLNodePtr node) -> uint32_t { return (node == nullptr) ? 0 : node->depth_; }
        static auto Size(AVLNodePtr node) -> uint32_t { return (node == nullptr) ? 0 : node->size_; }
        auto Update() -> void
        {
            depth_ = 1 + std::max(Depth(left_), Depth(right_));
            size_ = 1 + Size(left_) + Size(right_);
        }

        // 返回node是当前节点的左节点还是右节点
        auto From(AVLNodePtr node) -> AVLNodePtr &
        {
            if (node == this->left_)
                return left_;
            return right_;
        }
    };

    class AVL
    {
    public:
        /*
        左旋样例
            2           4
           / \         / \
          1   4  ==>  2   5
             / \     / \
            3   5   1   3
        */
        // 左旋
        static auto RotateLeft(AVLNodePtr cur_fa) -> AVLNodePtr
        {
            AVLNodePtr new_fa = cur_fa->right_;
            if (new_fa->left_ != nullptr)
            {
                new_fa->left_->parent_ = cur_fa;
            }
            cur_fa->right_ = new_fa->left_;
            new_fa->left_ = cur_fa;
            new_fa->parent_ = cur_fa->parent_;
            cur_fa->parent_ = new_fa;
            cur_fa->Update();
            new_fa->Update();
            return new_fa;
        }
        // 右旋
        static auto RotateRight(AVLNodePtr cur_fa) -> AVLNodePtr
        {
            auto new_fa = cur_fa->left_;
            if (new_fa->right_ != nullptr)
            {
                new_fa->right_->parent_ = cur_fa;
            }
            cur_fa->left_ = new_fa->right_;
            new_fa->right_ = cur_fa;
            new_fa->parent_ = cur_fa->parent_;
            cur_fa->parent_ = new_fa;
            cur_fa->Update();
            new_fa->Update();
            return new_fa;
        }

        // 如果h(left) - h(right) ==2
        // 则需要进行旋转操作使得整棵树平衡
        static auto FixLeft(AVLNodePtr subtree_root) -> AVLNodePtr
        {
            if (AVLNode::Depth(subtree_root->left_->left_) < AVLNode::Depth(subtree_root->left_->right_))
            {
                subtree_root->left_ = RotateLeft(subtree_root->left_);
            }
            return RotateRight(subtree_root);
        }
        // 如果h(right) - h(left) ==2
        // 则需要进行旋转操作使得整棵树平衡
        static auto FixRight(AVLNodePtr subtree_root) -> AVLNodePtr
        {
            if (AVLNode::Depth(subtree_root->right_->right_) < AVLNode::Depth(subtree_root->right_->left_))
            {
                subtree_root->right_ = RotateRight(subtree_root->right_);
            }
            return RotateLeft(subtree_root);
        }

        static auto Fix(AVLNodePtr node) -> AVLNodePtr
        {
            for (;;)
            {
                node->Update();
                uint32_t l = AVLNode::Depth(node->left_);
                uint32_t r = AVLNode::Depth(node->right_);
                auto p = node->parent_.lock();
                auto node_backup = node;
                if (l + 2 == r)
                {
                    node = FixRight(node);
                }
                else if (l == r + 2)
                {
                    node = FixLeft(node);
                }
                if (p == nullptr)
                {
                    return node;
                }
                p->From(node_backup) = node;
                node = p;
            }
        }

        static auto Delete(AVLNodePtr node) -> AVLNodePtr
        {
            if (node->right_ == nullptr)
            {
                AVLNodePtr parent = node->parent_.lock();
                if (node->left_ != nullptr)
                {
                    node->left_->parent_ = parent;
                }
                if (parent != nullptr)
                {
                    ((parent->left_ == node) ? parent->left_ : parent->right_) = node->left_;
                    return Fix(parent);
                }
                return node->left_;
            }
            else
            {
                AVLNodePtr victim = node->right_;
                while (victim->left_ != nullptr)
                {
                    victim = victim->left_;
                }
                AVLNodePtr root = Delete(victim);

                *victim = *node;
                if (victim->left_ != nullptr)
                {
                    victim->left_->parent_ = victim;
                }
                if (victim->right_ != nullptr)
                {
                    victim->right_->parent_ = victim;
                }

                if (AVLNodePtr parent = node->parent_.lock())
                {
                    ((parent->left_ == node) ? parent->left_ : parent->right_) = victim;
                    return Fix(parent);
                }
                return victim;
            }
        }
    };

    class AVLContainer
    {
    };

}

#endif