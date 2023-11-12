#include "fake-enclave.h"
#include "enclave-printf.h"
#include "uintr.h"
#include "syscall.h"

#define LOG(fmt, ...)                                            \
    do {                                                         \
        e_printf("[%s:%d@%s] ", __FILE__, __LINE__, __FUNCTION__); \
        e_printf(fmt, ##__VA_ARGS__);                              \
    } while (0)

void notify_normal(int uipi_index);

void register_sender();

int
main() {
    if(uintr_register_receiver(enclave_uintr_handler, true))
    {
        LOG("uintr_register_receiver failed\n");
        EAPP_RETURN(1);
    }
    register_sender();

    uint64_t shared_buffer      = 0;
    uint64_t shared_buffer_size = 0;
    get_shared_buffer(&shared_buffer, &shared_buffer_size);

    LOG(
        "&shared_buffer=0x%lx, shared_buffer=0x%lx\n", &shared_buffer,
        shared_buffer);

    fake_enclave(shared_buffer, notify_normal, 0);

    EAPP_RETURN(0);
}

void notify_normal(int uipi_index)
{
    // uipi_send(uipi_index);
}

void register_sender()
{
    SYSCALL_0(__NR_uintr_register_sender);
}
