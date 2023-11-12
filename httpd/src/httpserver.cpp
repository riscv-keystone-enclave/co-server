#include "co_context/net.hpp"
#include <co_context/co/condition_variable.hpp>
#include <co_context/co/mutex.hpp>
#include "HTTPParser.hpp"
#include <algorithm>
#include <thread>

using namespace co_context;
using namespace std;

constexpr uint32_t worker_num = 4;
co_context::io_context server_ctx;
std::array<co_context::io_context, worker_num> workers;

constexpr uint16_t port = 8081;

#define LOG(fmt, ...) \
    printf("[%s:%d]@%s " fmt, __FILE__, __LINE__, __FUNCTION__, ##__VA_ARGS__)

// defined in host.cpp
void
host_uintr_init();



task<> session(int sockfd)
{
    co_context::socket sock{sockfd};
    defer _{[sockfd]
            {
                ::close(sockfd);
            }}; // 在当前作用域结束时被执行

    HTTPParser parser(sock);

    char buf[512];
    int nr = co_await sock.recv(buf);
    if (nr <= 0) [[unlikely]]
    {
        if (nr < 0)
        {
            log::e("Bad recv\n");
        }
        co_return;
    }

    co_await parser.httpParse(buf);
}

task<void> server(const uint16_t port)
{
    host_uintr_init();

    LOG("Server started on port %d\n", port);
    uint32_t turn = 0;
    acceptor ac{inet_address{port}};
    for (int sock; (sock = co_await ac.accept()) >= 0;)
    {
        // LOG("accept success\n");
        workers[turn].co_spawn(session(sock));
        turn = (turn + 1) % worker_num;
    }
}

void httpd_main()
{
    int res = chdir("/keystone/co-server/public");
    if (res < 0)
    {
        perror("chdir");
    }
    else
    {
        LOG("chdir success\n");
    }

    server_ctx.co_spawn(server(port));
    server_ctx.start();


    for(auto &ctx : workers)
    {
        ctx.start();
    }



    server_ctx.join();
}
