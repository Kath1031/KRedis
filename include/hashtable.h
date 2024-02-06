#ifndef HASHTABLE_H
#define HASHTABLE_H
#include "public.h"
namespace kath
{
    struct HNode{
        HNode *next_{nullptr};
        uint64_t hcode_; // cached hash value
        HNode()=delete;
        HNode(uint64_t hcode):next_(nullptr),hcode_(hcode){}
    };

    class HTab{
        friend class Hmap;
    };
    class HMap{

    };
} // namespace kath


#endif