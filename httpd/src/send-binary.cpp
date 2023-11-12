#include <stdio.h>

#include <LockFreeQueue.hpp>
#include <array>
#include <co_context/co/channel.hpp>
#include <co_context/co/condition_variable.hpp>
#include <co_context/co/mutex.hpp>
#include <co_context/io_context.hpp>
#include <co_context/task.hpp>
#include <vector>

#include "UMManage-ring-buffer.h"
#include "co_context/net.hpp"

#define LOG(fmt, ...) \
    printf("[%s:%d]@%s " fmt, __FILE__, __LINE__, __FUNCTION__, ##__VA_ARGS__)

using namespace co_context;

constexpr const size_t size = 64;

typedef std::array<char, size> DATA;

co_context::condition_variable cv;
co_context::mutex cv_m;

std::atomic_bool has_ret{false};

enum ECALL_OCALL_FUNC {
    ECALL_UIPI_INDEX = 0,
    ECALL_ENCRYPT_CBC,
    ECALL_ENCLAVE_EXIT
};

co_context::task<void>
sendBinary(co_context::socket& sock, const char header[]) {
    DATA data;
    // memset(data.data(), 0, data.size());
    op_t op = {0};

    // LOG("enter sendBinary\n");
    if (has_ret) {
        LOG("has_ret = true\n");
        has_ret = false;
        auto lk = co_await cv_m.lock_guard();
        cv.notify_all();
    }

    {
        auto lk = co_await cv_m.lock_guard();

        // LOG("try to async_ecall\n");
        // 多线程，需要加锁发送
        while (async_ecall(ECALL_ENCRYPT_CBC, data.data(), data.size()) < 0) {
            if (get_num_of_ocall_and_eret() > 0) [[likely]] {
                // ecall 失败，容量已满,通知处理返回数据
                cv.notify_all();
            }
            co_await lazy::yield();
        }
    }
    // LOG("async_ecall success\n");

    {
        auto lk = co_await cv_m.lock_guard();
        if (get_num_of_ocall_and_eret() <= 0) {
            // 当前没有返回数据则挂起当前协程
            co_await cv.wait(
                cv_m, [] { return get_num_of_ocall_and_eret() > 0; });
        }

        // 由于工作在多个线程中，所以需要加锁
        while (!read_ocall_or_eret((uint8_t*)data.data(), &op)) [[unlikely]] {
            // 读取返回数据失败，挂起当前协程
            co_await lazy::yield();
        }
    }

    // LOG("read_ocall_or_eret success\n");

    // 测试通过
    // for(uint32_t i = 0; i < data.size(); ++i)
    // {
    //     if(data[i] != (uint8_t)i & 0x3f)
    //     {
    //         LOG("ERROR! data[%d] = %d\n", i, data[i]);
    //     }
    // }

    co_await (sock.send({header, strlen(header)}, MSG_MORE) && sock.send(data));
}

uint64_t
normal_uintr_handler(struct __uintr_frame* ui_frame, uint64_t irqs) {
    has_ret = true;
    LOG("receive uintr handler from enclave\n");
    return 0;
}