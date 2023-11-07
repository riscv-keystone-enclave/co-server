#include "co_context/net.hpp"
#include "HTTPParser.hpp"
#include <algorithm>

using namespace co_context;
using namespace std;

class HTTPParser;

task<> session(int sockfd){
    co_context::socket sock{sockfd};
    HTTPParser parser(sock);

    char buf[8192];
    int nr = co_await sock.recv(buf);
    co_await parser.http_parse(buf);
}

task<void> server(const uint16_t port)
{
    printf("Server started on port %d\n", port);
    acceptor ac{inet_address{port}};
    for (int sock; (sock = co_await ac.accept()) >= 0;) {
        co_spawn(session(sock));
    }
}

int main(int argc, char** argv)
{
    chdir("/home/wang/Documents/co-server/public");

    io_context ctx;
    ctx.co_spawn(server(8080));
    ctx.start();
    ctx.join();
    return 0;
}
