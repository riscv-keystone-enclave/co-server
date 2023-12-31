#include "co_context/net.hpp"
#include "HTTPParser.hpp"
#include <algorithm>
#include <thread>

using namespace co_context;
using namespace std;

constexpr uint16_t port = 8081;
constexpr uint32_t worker_num = 4;
io_context server_ctx;
io_context worker[worker_num];

#define LOG(fmt, ...) \
    printf("[%s:%d]@%s " fmt, __FILE__, __LINE__, __FUNCTION__, ##__VA_ARGS__)


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
    LOG("Server started on port %d\n", port);
    uint32_t turn = 0;
    acceptor ac{inet_address{port}};
    for (int sock; (sock = co_await ac.accept()) >= 0;)
    {
	    // LOG("NEW SOCKET\n");
        worker[turn].co_spawn(session(sock));
        turn = (turn + 1) % worker_num;
    }
}


int main(int argc, char **argv)
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


    for(auto &ctx : worker)
    {
        ctx.start();
    }



    server_ctx.join();
    return 0;
}
