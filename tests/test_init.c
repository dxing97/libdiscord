//
// Created by dxing97 on 5/30/20.
//
#include <libdiscord.h>
#include "libdiscord.h"



int main() {
    lde_code ret = ld_init(LD_ERR | LD_WARN | LD_NOTICE | LD_INFO | LD_DEBUG);
    return ret;
}