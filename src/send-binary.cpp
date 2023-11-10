#include "co_context/net.hpp"
#include <co_context/co/condition_variable.hpp>
#include <co_context/co/mutex.hpp>
#include <co_context/io_context.hpp>
#include <co_context/task.hpp>

#include <ThreadSafeQueue.hpp>

#include <array>

using namespace co_context;

// std::mutex mutex_;

// co_context::mutex m;
// co_context::condition_variable cv;
// bool ready = false;
// ThreadSafeQueue<std::span<uint8_t>> queue1, queue2;

// task<>
// processData()
// {
//     while (true)
//     {
//         std::span<uint8_t> buffer;
//         queue1.wait_and_pop(buffer);
//         {
//             for(uint32_t i = 0; i < buffer.size(); i++)
//             {
//                 buffer[i] = (uint8_t)i & 0x3f;
//             }
//         }
//         {
//             co_await m.lock();
//             ready = true;
//         }
//     }
// }

co_context::task<void>
sendBinary(co_context::socket &sock, const char header[])
{
    std::array<char, 64> buffer;
    // queue1.push(buffer);



    co_await (
        sock.send({header, strlen(header)}, MSG_MORE) && sock.send(buffer));
}