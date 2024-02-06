#include <iostream>
#include <cstdlib>
#include <cstddef>

#include "msg.h"
#include "bytes.h"
#include "file.h"

#include "client.h"
[[nodiscard]] bool func() {
    return true;
}

int main(){
    Msg("abc");
    kath::Bytes bytes;
    bytes.AppendStr("abc");
    std::cout<<bytes<<std::endl;

}