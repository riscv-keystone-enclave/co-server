#ifndef __FAKE_H__
#define __FAKE_H__

#include <stdbool.h>
#include <stdint.h>
#include "uintr.h"

#ifdef __x86_64__
#include <sys/time.h>
#include <time.h>
#endif

#ifdef __cplusplus
extern "C" {
#endif

typedef void (*notify_normal_func_ptr)(int);
typedef void (*notify_enclave_func_ptr)(int);


uint64_t
enclave_uintr_handler(struct __uintr_frame* ui_frame, uint64_t irqs);

void
fake_enclave(
    void* shared_buffer, notify_normal_func_ptr notify_normal, int pid);

/**
 * @brief
 * 关于程序中的计时问题的一些基本概念参阅：https://en.wikipedia.org/wiki/Elapsed_real_time
 * https://stackoverflow.com/questions/556405/what-do-real-user-and-sys-mean-in-the-output-of-time1
 * 这里实现的是计算实际的运行时间.返回值以毫秒为单位。但是值得注意的是，该程序的返回值以什么时间为基准计算的是
 * 不确定的。因此只能用来计算时间差。根据两次相减的结果可以得到wall time
 * @return 以毫秒为单位
 */
static inline int64_t
getCurrentMillisecs() {
#ifdef __riscv
    uint64_t cycle;
    asm volatile("rdcycle %0" : "=r"(cycle));
    return cycle;
#elif defined(__x86_64__)
    struct timeval tv;
    int ret = gettimeofday(&tv, NULL);
    if (ret == 0) {
        return tv.tv_sec * 1000 + tv.tv_usec / 1000;
    }
    return 0;
#endif
}


#ifdef __cplusplus
}
#endif

#endif  // __FAKE_H__