// #ifndef AVL_H
// #define AVL_H



// #include "avl_base.h"
// // 值类型暂时用的uint32

// namespace kath::avl
// {
//     class AVLData;
//     using AVLDataPtr = std::shared_ptr<AVLData>;

//     class AVLData : public AVLNode
//     {
//     public:
//         uint32_t val_{0};
//         AVLData(uint32_t val) : val_(val) {}
//     };
//     class AVL
//     {
//     public:
//         AVLNodePtr root_;
//         static auto GetVal(AVLNodePtr node) -> uint32_t
//         {
//             return dynamic_cast<AVLData *>(node.get())->val_;
//         }
//         auto Insert(uint32_t val) -> void
//         {
//             AVLNodePtr data = std::make_shared<AVLData>(AVLData(val));
//             if (root_ == nullptr)
//             {
//                 root_ = data;
//                 return;
//             }

//             AVLNodePtr last = nullptr;
//             AVLNodePtr cur = root_;

//             while (cur != nullptr)
//             {
//                 last = cur;
//                 uint32_t last_val = GetVal(last);
//                 cur = (val < last_val) ? last->left_ : last->right_;
//             }

//             if (val < GetVal(last))
//             {
//                 last->left_ = data;
//             }
//             else
//             {
//                 last->right_ = data;
//             }
//             data->parent_ = last;
//             AVLOperate::Fix(data);
//         }
//         auto Del(uint32_t val) -> bool
//         {
//             AVLNodePtr cur = root_;
//             while (cur != nullptr)
//             {
//                 uint32_t cur_val = GetVal(cur);
//                 if (cur_val == val) {
//                     break;
//                 }
//                 cur = (val<cur_val) ? cur->left_ : cur->right_;
//             }
//             if(cur == nullptr) {
//                 return false;
//             }
//             AVLOperate::Delete(cur);
//             return true;
//         }
//     };
// }

// #endif