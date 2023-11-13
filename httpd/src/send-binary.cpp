#include <stdio.h>

#include <LockFreeQueue.hpp>
#include <array>
#include <co_context/co/channel.hpp>
#include <co_context/co/condition_variable.hpp>
#include <co_context/co/mutex.hpp>
#include <co_context/io_context.hpp>
#include <co_context/task.hpp>
#include <vector>

#include "co_context/net.hpp"

#define LOG(fmt, ...) \
    printf("[%s:%d]@%s " fmt, __FILE__, __LINE__, __FUNCTION__, ##__VA_ARGS__)

using namespace co_context;


typedef std::array<char, 64> DATA;

enum ECALL_OCALL_FUNC {
    ECALL_UIPI_INDEX = 0,
    ECALL_ENCRYPT_CBC,
    ECALL_ENCLAVE_EXIT
};

extern LockFreeQueue<DATA> g_queue_call;
extern LockFreeQueue<DATA> g_queue_ret;

co_context::task<void>
sendBinary(co_context::socket& sock, const char header[]) {
    DATA data;
    memset(data.data(), 0, data.size());

    while (g_queue_call.enqueue(data) == false)[[unlikely]] {
        co_await lazy::yield();
    }

    while(g_queue_ret.dequeue(data) == false){
        co_await lazy::yield();
    }


    for(uint32_t i = 0; i < data.size(); ++i)
    {
        if(data[i] != (uint8_t)i & 0x3f)
        {
            LOG("ERROR! data[%d] = %d\n", i, data[i]);
            break;
        }
    }

    co_await (sock.send({header, strlen(header)}, MSG_MORE) && sock.send(data));
}
