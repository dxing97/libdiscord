//
// Created by dxing97 on 11/10/18.
//

#include <libdiscord.h>
int main(int argc, char *argv[]) {

    struct ld_rest_request request;
    void * ret;
    ret = ld_rest_init_request(&request);
    if(ret == NULL) {
        return 1;
    }

    return 0;
}
