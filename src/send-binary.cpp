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

typedef std::array<char, size> DATA;



void
processData(DATA& data)
{
    for(int i = 0; i < data.size(); ++i)
    {
        data[i] = (char)i & 0x3f;
    }
}

co_context::task<void>
sendBinary(co_context::socket &sock, const char header[])
{
    DATA data;
    processData(data);
    co_await (
        sock.send({header, strlen(header)}, MSG_MORE) && sock.send(data));
}