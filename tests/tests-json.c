
#include <libdiscord.h>


int main() {
    ld_set_logging_level(63);
    struct ld_context context;
    ld_init_context(NULL, &context);

    ld_info("%s: testing payload valid", __FILE__);
    struct ld_json_websocket_payload test_payload;
    test_payload.op = LD_GATEWAY_OPCODE_UNKNOWN;
    test_payload.d = NULL;
    ld_json_payload_valid(&test_payload);

    //{"t":"PRESENCES_REPLACE","s":9,"op":0,"d":[]}
    test_payload.op = LD_GATEWAY_OPCODE_DISPATCH;
    test_payload.d = NULL;
    test_payload.t = LD_PRESENCES_REPLACE;
    test_payload.s = 9;
    ld_json_payload_valid(&test_payload);


    return 0;
}