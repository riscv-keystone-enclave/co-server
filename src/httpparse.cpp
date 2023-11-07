#include <sstream>
#include <string>
#include <string_view>
#include "HTTPParser.hpp"

using namespace std::string_view_literals;

#define LOG(fmt, ...) \
    printf("[%s:%d]@%s " fmt, __FILE__, __LINE__, __FUNCTION__, ##__VA_ARGS__)

const char http_header[25] = "HTTP/1.1 200 Ok\r\n";

constexpr std::string_view http_404_content =
    "HTTP/1.0 404 Not Found\r\n"
    "Content-type: text/html\r\n"
    "\r\n"
    "<html>"
    "<head>"
    "<title>ZeroHTTPd: Not Found</title>"
    "</head>"
    "<body>"
    "<h1>Not Found (404)</h1>"
    "<p>Your client is asking for an object that was not found on this server.</p>"
    "</body>"
    "</html>"sv;

extern "C" void
send_binary(int fd, char image_path[], char head[]);

void fatal_error(const char *syscall)
{
    perror(syscall);
    std::terminate();
}

co_context::task<void> prep_file_contents(
    std::string_view file_path, off_t file_size, std::span<char> content_buffer)
{
    int fd;

    fd = ::open(file_path.data(), O_RDONLY);
    if (fd < 0)
    {
        fatal_error("read");
    }

    /* We should really check for short reads here */
    int ret = co_await co_context::lazy::read(fd, content_buffer, 0);

    if (ret < file_size) [[unlikely]]
    {
        co_context::log::w("Encountered a short read.\n");
    }

    ::close(fd);
}

struct HttpRequest
{
    std::string method;
    std::string path;
    std::string extension;
};

HttpRequest parseHttpRequest(const std::string &header)
{
    HttpRequest request;

    std::istringstream iss(header);
    std::string requestLine;
    std::getline(iss, requestLine); // 读取请求行

    std::istringstream requestLineStream(requestLine);
    requestLineStream >> request.method >> request.path; // 解析请求方法和文件路径

    // 获取文件拓展名
    size_t dotPos = request.path.find_last_of('.');
    if (dotPos != std::string::npos)
    {
        request.extension = request.path.substr(dotPos + 1);
    }

    return request;
}

co_context::task<>
HTTPParser::http_parse(const char *request)
{
    HttpRequest httpRequest = parseHttpRequest(request);

    char copy_head[4096];
    strcpy(copy_head, http_header);

    if (httpRequest.method == "GET")
    {
        char path_head[500] = ".";

        if (httpRequest.path.size() <= 1)
        {
            strcat(path_head, "/index.html");
            strcat(copy_head, "Content-Type: text/html\r\n\r\n");
        }
        else if (httpRequest.path == "test")
        {
            strcat(copy_head, "Content-Type: text/plain\r\n\r\n");
            // send_binary(new_socket, NULL, copy_head); // TODO
        }
        else if (httpRequest.extension == "jpg" || httpRequest.extension == "JPG")
        {
            // send image to client
            strcat(path_head, httpRequest.path.c_str());
            strcat(copy_head, "Content-Type: image/jpeg\r\n\r\n");
        }
        else if (httpRequest.extension == "ico")
        {
            strcat(path_head, "/img/favicon.png");
            strcat(copy_head, "Content-Type: image/vnd.microsoft.icon\r\n\r\n");
        }
        else if ("ttf" == httpRequest.extension)
        {
            strcat(path_head, httpRequest.path.c_str());
            strcat(copy_head, "Content-Type: font/ttf\r\n\r\n");
        }
        else if (httpRequest.extension == "js")
        {
            // javascript
            strcat(path_head, httpRequest.path.c_str());
            strcat(copy_head, "Content-Type: text/javascript\r\n\r\n");
        }
        else if (httpRequest.extension == "css")
        {
            // css
            strcat(path_head, httpRequest.path.c_str());
            strcat(copy_head, "Content-Type: text/css\r\n\r\n");
        }
        else if (httpRequest.extension.contains("wof"))
        {
            // Web Open Font Format woff and woff2
            strcat(path_head, httpRequest.path.c_str());
            strcat(copy_head, "Content-Type: font/woff\r\n\r\n");
        }
        else if ("m3u8" == httpRequest.extension)
        {
            // Web Open m3u8
            strcat(path_head, httpRequest.path.c_str());
            strcat(
                copy_head,
                "Content-Type: application/vnd.apple.mpegurl\r\n\r\n");
        }
        else if ("ts" == httpRequest.extension)
        {
            // Web Open ts
            char path_head[500] = ".";
            strcat(path_head, httpRequest.path.c_str());
            strcat(copy_head, "Content-Type: video/mp2t\r\n\r\n");
        }
        else
        {
            LOG("Else: %s \n", httpRequest.path.c_str());
            strcat(path_head, httpRequest.path.c_str());
            strcat(copy_head, "Content-Type: text/plain\r\n\r\n");
        }

        co_await send_message(path_head, copy_head);
    }
    else
    {
        LOG("ERROR! Method: %s \n", httpRequest.method.c_str());
        co_return;
    }
    co_return;
}

co_context::task<> HTTPParser::send_message(const char *image_path,
                                            const char *head)
{
    struct stat stat_buf;
    if (-1 == stat(image_path, &stat_buf) ||
        !S_ISREG(stat_buf.st_mode))
    {
        co_await m_sock.send(http_404_content);
    }
    else
    {
        std::vector<char> content_buf{};
        content_buf.resize(stat_buf.st_size);
        co_await prep_file_contents(image_path, stat_buf.st_size, content_buf);
        co_await (
            m_sock.send({head, strlen(head)}, MSG_MORE) && m_sock.send(content_buf));
    }
}
