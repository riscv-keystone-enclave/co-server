#include <edge_call.h>
#include <keystone.h>

#include <span>

#include "LockFreeQueue.hpp"

#define LOG(fmt, ...)                                            \
    do {                                                         \
        printf("[%s:%d@%s] ", __FILE__, __LINE__, __FUNCTION__); \
        printf(fmt, ##__VA_ARGS__);                              \
    } while (0)


typedef std::array<char, 64> DATA;

LockFreeQueue<DATA> g_queue_call(1024 * 1024 * 8);
LockFreeQueue<DATA> g_queue_ret(1024 * 1024 * 8);

Keystone::Enclave enclave;


constexpr const int UM_SIZE = 1024 * 1024;

enum ocall_ids {
    OCALL_PRINT_STRING = 1,
    OCALL_HTTP_ENCRYPT,
};

void
print_string_wrapper(void* buffer);

static void
http_encrypt_wrapper(void* buffer);

void
enclave_main(int argc, char** argv) {
    Keystone::Params params;
    params.setSimulated(false);

    params.setFreeMemSize(1024 * 1024 * 8);
    params.setUntrustedMem(DEFAULT_UNTRUSTED_PTR, UM_SIZE);

    enclave.init(argv[1], argv[2], params);

    enclave.registerOcallDispatch(incoming_call_dispatch);

    /* We must specifically register functions we want to export to the
   enclave. */
    register_call(OCALL_PRINT_STRING, print_string_wrapper);
    register_call(OCALL_HTTP_ENCRYPT, http_encrypt_wrapper);

    edge_call_init_internals(
        (uintptr_t)enclave.getSharedBuffer(), enclave.getSharedBufferSize());



    printf("[runner]: running enclave\n");

    enclave.run();

    printf("[runner]: enclave exit\n");

    return;
}

unsigned long
print_string(char* str) {
    // printf("Enclave said: \"%s\"", str);
    return 13;
}

void
print_string_wrapper(void* buffer) {
    /* Parse and validate the incoming call data */
    struct edge_call* edge_call = (struct edge_call*)buffer;
    uintptr_t call_args;
    unsigned long ret_val;
    size_t arg_len;
    if (edge_call_args_ptr(edge_call, &call_args, &arg_len) != 0) {
        edge_call->return_data.call_status = CALL_STATUS_BAD_OFFSET;
        return;
    }

    /* Pass the arguments from the eapp to the exported ocall function */
    ret_val = print_string((char*)call_args);

    /* Setup return data from the ocall function */
    uintptr_t data_section = edge_call_data_ptr();
    memcpy((void*)data_section, &ret_val, sizeof(unsigned long));
    if (edge_call_setup_ret(
            edge_call, (void*)data_section, sizeof(unsigned long))) {
        edge_call->return_data.call_status = CALL_STATUS_BAD_PTR;
    } else {
        edge_call->return_data.call_status = CALL_STATUS_OK;
    }

    /* This will now eventually return control to the enclave */
    return;
}

static void
http_encrypt_wrapper(void* buffer) {
    /* Parse and validate the incoming call data */
    struct edge_call* edge_call = (struct edge_call*)buffer;
    uintptr_t call_args;
    unsigned long ret_val = 0;
    size_t arg_len;
    if (edge_call_args_ptr(edge_call, &call_args, &arg_len) != 0) {
        LOG("edge_call_args_ptr failed\n");
        edge_call->return_data.call_status = CALL_STATUS_BAD_OFFSET;
        return;
    }

    // LOG("receive size=%d\n", arg_len);
    DATA data;

    if (data.size() < arg_len) [[unlikely]] {
        LOG("data.size() < arg_len\n");
        edge_call->return_data.call_status = CALL_STATUS_BAD_PTR;
        return;
    }
    memcpy((void*)data.data(), (void*)call_args, arg_len);

    while(!g_queue_ret.enqueue(data));

    while(!g_queue_call.dequeue(data));

    uintptr_t data_section = edge_call_data_ptr();
    memcpy((void*)data_section, data.data(), data.size());
    if (edge_call_setup_ret(edge_call, (void*)data_section, data.size()))
        [[unlikely]] {
        edge_call->return_data.call_status = CALL_STATUS_BAD_PTR;
    } else {
        edge_call->return_data.call_status = CALL_STATUS_OK;
    }
    return;
}
