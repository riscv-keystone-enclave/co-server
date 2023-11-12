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



void fatal_error(const char *syscall)
{
    perror(syscall);
    std::terminate();
}

co_context::task<void>
sendBinary(co_context::socket &sock, const char header[]);

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
HTTPParser::httpParse(const char *request)
{
    HttpRequest http_request = parseHttpRequest(request);

    char copy_head[4096];
    strcpy(copy_head, http_header);

    if (http_request.method == "GET")
    {
        char path_head[500] = ".";

        if (http_request.path.size() <= 1)
        {
            strcat(path_head, "/index.html");
            strcat(copy_head, "Content-Type: text/html\r\n\r\n");
        }
        else if (http_request.path == "/test")
        {
            strcat(copy_head, "Content-Type: text/plain\r\n\r\n");
            // LOG("access to /test\n");
            co_await sendBinary(m_sock, copy_head);
            co_return;
        }
        else if (http_request.extension == "jpg" || http_request.extension == "JPG")
        {
            // send image to client
            strcat(path_head, http_request.path.c_str());
            strcat(copy_head, "Content-Type: image/jpeg\r\n\r\n");
        }
        else if (http_request.extension == "ico")
        {
            strcat(path_head, "/img/favicon.png");
            strcat(copy_head, "Content-Type: image/vnd.microsoft.icon\r\n\r\n");
        }
        else if ("ttf" == http_request.extension)
        {
            strcat(path_head, http_request.path.c_str());
            strcat(copy_head, "Content-Type: font/ttf\r\n\r\n");
        }
        else if (http_request.extension == "js")
        {
            // javascript
            strcat(path_head, http_request.path.c_str());
            strcat(copy_head, "Content-Type: text/javascript\r\n\r\n");
        }
        else if (http_request.extension == "css")
        {
            // css
            strcat(path_head, http_request.path.c_str());
            strcat(copy_head, "Content-Type: text/css\r\n\r\n");
        }
        else if (http_request.extension.contains("wof"))
        {
            // Web Open Font Format woff and woff2
            strcat(path_head, http_request.path.c_str());
            strcat(copy_head, "Content-Type: font/woff\r\n\r\n");
        }
        else if ("m3u8" == http_request.extension)
        {
            // Web Open m3u8
            strcat(path_head, http_request.path.c_str());
            strcat(
                copy_head,
                "Content-Type: application/vnd.apple.mpegurl\r\n\r\n");
        }
        else if ("ts" == http_request.extension)
        {
            // Web Open ts
            strcat(path_head, http_request.path.c_str());
            strcat(copy_head, "Content-Type: video/mp2t\r\n\r\n");
        }
        else
        {
            LOG("Else: %s \n", http_request.path.c_str());
            strcat(path_head, http_request.path.c_str());
            strcat(copy_head, "Content-Type: text/plain\r\n\r\n");
        }

        co_await response(path_head, copy_head);
    }
    else
    {
        LOG("ERROR! Method: %s \n", http_request.method.c_str());
        co_return;
    }
    co_return;
}

co_context::task<> HTTPParser::response(const char *image_path,
                                            const char *header)
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
            m_sock.send({header, strlen(header)}, MSG_MORE) && m_sock.send(content_buf));
    }
}
