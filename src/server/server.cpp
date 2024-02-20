#include <iostream>
#include <cstdlib>
#include <cstddef>

#include "msg.h"
#include "bytes.h"
#include "file.h"
#include "hashtable.h"
#include "avl_base.h"
#include "connect.h"

#include "server.h"
#include "heap.h"
#include "rand.h"

#include "client.h"
#include "exec.h"
#include "zset.h"
[[nodiscard]] bool func() {
    return true;
}

int main(){
    Msg("abc");
    kath::Bytes bytes;
    bytes.AppendStr("abc");
    std::cout<<bytes<<std::endl;

}