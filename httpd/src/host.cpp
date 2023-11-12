#include "UMManage-ring-buffer.h"
#include "UINTRRAII.hpp"

#include <edge_call.h>
#include <keystone.h>

#define LOG(fmt, ...)                                            \
    do {                                                         \
        printf("[%s:%d@%s] ", __FILE__, __LINE__, __FUNCTION__); \
        printf(fmt, ##__VA_ARGS__);                              \
    } while (0)

// defined in send-binary.cpp
uint64_t
normal_uintr_handler(struct __uintr_frame* ui_frame, uint64_t irqs);



Keystone::Enclave enclave;
UINTRAII enclave_uintr;
UINTRAII normal_uintr;

const int sender_id_for_enclave = 2;
const int sender_id_for_normal = 1;

constexpr const int UM_SIZE = 1024 * 1024;

void
notify_enclave(int uipi_index) {
    // uipi_send(uipi_index);
}



void
host_uintr_init() {
    if (!normal_uintr.registerUintrHandler(
            (void*)normal_uintr_handler, false)) {
        LOG("ERROR! normal_uintr.registerUintrHandler\n");
        return;
    }
    printf(
        "[runner]: utvec=0x%lx, uscratch=0x%lx\n", csr_read(CSR_UTVEC),
        csr_read(CSR_USCRATCH));
    if (!normal_uintr.create(sender_id_for_normal)) {
        LOG("ERROR! normal_uintr.create\n");
        return;
    }
    while (enclave_uintr.getUintrFd() < 0) {
        // wait for enclave_ui_fd to be created
    }
    if (!normal_uintr.registerUintrReceiver(enclave_uintr.getUintrFd())) {
        LOG("ERROR! normal_uintr.registerUintrReceiver\n");
        return;
    }

    // 必须要保证 enclave 的缓冲区已经分配好了
    // 前面的 enclave_uintr.getUintrFd() 的死循环可以保证
    // 因为 enclave_uintr 是在 enclvae缓冲区创建好之后才创建的
    register_um((uint64_t)enclave.getSharedBuffer(), UM_SIZE);
    init_um(false);
}

void
enclave_main(int argc, char** argv) {
    Keystone::Params params;
    params.setSimulated(false);

    params.setFreeMemSize(1024 * 1024 * 8);
    params.setUntrustedMem(DEFAULT_UNTRUSTED_PTR, UM_SIZE);

    enclave.init(argv[1], argv[2], params);

    enclave.registerOcallDispatch(incoming_call_dispatch);

    edge_call_init_internals(
        (uintptr_t)enclave.getSharedBuffer(), enclave.getSharedBufferSize());

    if (!enclave_uintr.registerUintrHandler(NULL, true)) {
        LOG("ERROR! enclave_uintr.registerUintrHandler\n");
        return;
    }

    if (!enclave_uintr.create(sender_id_for_enclave)) {
        LOG("ERROR! enclave_uintr.create\n");
        return;
    }

    while (normal_uintr.getUintrFd() < 0) {
        // wait for uintrfd to be created
    }
    if (!enclave_uintr.registerUintrReceiver(normal_uintr.getUintrFd())) {
        LOG("ERROR! enclave_uintr.registerUintrReceiver\n");
        return;
    }

    printf("[runner]: running enclave\n");

    enclave.run();

    printf("[runner]: enclave exit\n");

    return;
}