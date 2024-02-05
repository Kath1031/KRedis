#include <iostream>
#include <cstdlib>
#include <cstddef>

#include "msg.h"
[[nodiscard]] bool func() {
    return true;
}

int main(){
    Msg("abc");
}