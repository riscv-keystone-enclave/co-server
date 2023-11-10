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
// io_context client_ctx;

#define LOG(fmt, ...) \
    printf("[%s:%d]@%s " fmt, __FILE__, __LINE__, __FUNCTION__, ##__VA_ARGS__)

extern void processData();

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

// task<> client()
// {
//     constexpr std::string_view host = "127.0.0.1";
//     using Socket = co_context::socket;
//     inet_address addr;
//     if (inet_address::resolve(host, port, addr))
//     {
//         while (true)
//         {
//             co_await timeout(std::chrono::seconds{100});

//             Socket sock{Socket::create_tcp(addr.family())};
//             LOG("connect...\n");
//             int res = co_await sock.connect(addr);
//             if (res < 0)
//             {
//                 log::e("%s\n", strerror(-res));
//                 ::exit(0);
//             }
//             LOG("connect...OK\n");

//             // http request for test
//             string_view http_request = "GET /test HTTP/1.1\r\n";
//             co_await sock.send(http_request);
//             LOG("send http request\n");
//             // rescv http response
//             char buf[8192];
//             int nr = co_await sock.recv(buf);
//             LOG("recv http response, size = %d, content = \n%s\n", nr, buf);
//         }
//     }
//     else
//     {
//         LOG("resolve failed\n");
//     }
// }

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

    thread t(processData);

    // client_ctx.co_spawn(client());
    // client_ctx.start();

    // client_ctx.join();
    server_ctx.join();
    t.join();
    return 0;
}
