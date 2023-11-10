#include "co_context/net.hpp"
#include <co_context/co/condition_variable.hpp>
#include <co_context/co/mutex.hpp>
#include <co_context/io_context.hpp>
#include <co_context/task.hpp>
#include <co_context/co/channel.hpp>

#include <LockFreeQueue.hpp>

#include <array>
#include <vector>

#include <stdio.h>

#define LOG(fmt, ...) \
    printf("[%s:%d]@%s " fmt, __FILE__, __LINE__, __FUNCTION__, ##__VA_ARGS__)


using namespace co_context;

constexpr const size_t size = 64;

typedef std::vector<char> DATA;

std::vector<DATA> datas(1000, DATA(size, '\0'));

LockFreeQueue<DATA> q1(100), q2(100);

void
processData()
{
    while (true)
    {
        DATA data;
        while (!q1.dequeue(data))
            ;

        // LOG("processData\n");
        for (auto &c : data)
        {
            c = c & 0x0f;
        }

        while(!q2.enqueue(std::move(data)))
            ;
    }
}

co_context::task<void>
sendBinary(co_context::socket &sock, const char header[])
{
    DATA data;
    {
        while (!q1.enqueue(std::move(data)))
            ;
    }

    while (!q2.dequeue(data))
        ;

    co_await (
        sock.send({header, strlen(header)}, MSG_MORE) && sock.send(data));
}