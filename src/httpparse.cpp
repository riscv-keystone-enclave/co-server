#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/sendfile.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <sstream>
#include <string>

#include "HTTPParser.hpp"

#define LOG(fmt, ...) \
    printf("[%s:%d]@%s " fmt, __FILE__, __LINE__, __FUNCTION__, ##__VA_ARGS__)

const char http_header[25] = "HTTP/1.1 200 Ok\r\n";

extern "C" void
send_binary(int fd, char image_path[], char head[]);

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

        send_message(path_head, copy_head);
    }
    else
    {
        LOG("ERROR! Method: %s \n", httpRequest.method.c_str());
        co_return;
    }
    co_return;
}

void HTTPParser::send_message(const char *image_path, const char *head)
{
    struct stat stat_buf;
    int fdimg = open(image_path, O_RDONLY);
    if (fdimg < 0)
    {
        LOG("Cannot Open file path : %s with error %d\n", image_path, fdimg);
        char head[500] = "HTTP/1.1 404 Not Found\r\n";
        // m_sock.send({head, strlen(head)});
        send(m_sock.fd(), head, strlen(head), 0);
        return;
    }
    else
    {
        // m_sock.send({head, strlen(head)});
        send(m_sock.fd(), head, strlen(head), 0);

        fstat(fdimg, &stat_buf);
        ssize_t sent_size = 0;
        while (sent_size < stat_buf.st_size)
        {
            ssize_t done_bytes = sendfile(m_sock.fd(), fdimg, NULL, stat_buf.st_size);
            if (done_bytes == -1)
            {
                LOG("Error sending file %s\n", image_path);
                close(fdimg);
                return;
            }
            sent_size = sent_size + done_bytes;
        }

        // LOG("success to send file: %s \n", image_path);
        close(fdimg);

        return;
    }
}
