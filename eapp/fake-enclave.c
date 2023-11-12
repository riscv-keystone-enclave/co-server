#include "fake-enclave.h"

#include "Lock.h"
#include "UMManage-ring-buffer.h"
#include "aes.h"
#include "enclave-printf.h"
#define LOG(fmt, ...)                                              \
    do {                                                           \
        e_printf("[%s:%d@%s] ", __FILE__, __LINE__, __FUNCTION__); \
        e_printf(fmt, ##__VA_ARGS__);                              \
    } while (0)

const uint32_t UM_SIZE = 1024 * 1024;

static uint8_t data[4096];

int uipi_index = 0;

enum ECALL_OCALL_FUNC {
    ECALL_UIPI_INDEX = 0,
    ECALL_ENCRYPT_CBC,
    ECALL_ENCLAVE_EXIT
};

static int
encrypt_ecb(uint8_t* data, uint32_t data_len);

uint64_t
enclave_uintr_handler(struct __uintr_frame* ui_frame, uint64_t irqs) {
    return 0;
}

void
test(void* shared_buffer, notify_normal_func_ptr notify_normal);
void
print_time(op_t op, int64_t total_received, int64_t start_time);

void
fake_enclave(void* shared_buffer, notify_normal_func_ptr notify_normal, int) {
    register_um((uint64_t)shared_buffer, UM_SIZE);
    init_um(true);

    LOG("################################\n");
    LOG("fake enclave, shared_buffer=0x%lx\n", shared_buffer);
    LOG("fake_enclave, uipi_index=%d\n", uipi_index);
    test(shared_buffer, notify_normal);
    LOG("SUCCESSFULLY\n");
}

void
test(void* shared_buffer, notify_normal_func_ptr notify_normal) {
    op_t op = {0};
    while (true) {
        while (get_num_of_ecall_and_oret() == 0)
            ;
        // LOG("hello\n");
        if (read_ecall_or_oret(data, &op)) {
            encrypt_ecb(data, op.param_len);
            async_eret(op.op_id, data, op.param_len);
            // LOG("send back, size=%d\n", op.param_len);
        } else {
            LOG("ERROR! read_ecall_or_oret failed\n");
            return;
        }
    }
}

static int
encrypt_ecb(uint8_t* data, uint32_t data_len) {
    // uint8_t key[] = {0x2b, 0x7e, 0x15, 0x16, 0x28, 0xae, 0xd2, 0xa6,
    //                  0xab, 0xf7, 0x15, 0x88, 0x09, 0xcf, 0x4f, 0x3c};

    // struct AES_ctx ctx;

    // AES_init_ctx(&ctx, key);
    // for (int i = 0; i < data_len / 16; i++)
    //     AES_ECB_encrypt(&ctx, data + i * 16);
    for(uint32_t i = 0; i < data_len; ++i)
    {
        data[i] = (uint8_t)i & 0x3f;
    }
    return 0;
}