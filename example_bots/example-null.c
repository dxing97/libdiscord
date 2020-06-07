//
// Created by dxing97 on 6/6/20.
// Opens connection to discord and stays online
//
#include <stdio.h>
#include <libdiscord.h>
#include <signal.h>

int signaled = 0;

void int_handler(int i) {
    signaled = 1;
}

int main(int argc, char *argv[]) {
    char *bot_token = NULL;

    if(argc == 1) {
        printf("missing bot token\n");
        return 0;
    }

    signal(SIGINT, int_handler);

    ld_init(LLL_DEBUG | LLL_INFO | LLL_NOTICE | LLL_WARN | LLL_ERR);

    struct ld_context_options opts;
    memset(&opts, 0, sizeof(struct ld_context_options));

    opts.bot_token = argv[1];

    struct ld_context context;
    ld_context_init(&opts, &context);

    while(!signaled) {
        ld_service(&context, 50);
    }
    lwsl_info("received interrupt, cleaning up");

    ld_context_destroy(&context);

    return 0;
}