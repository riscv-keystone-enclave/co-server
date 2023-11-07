#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/sendfile.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include <string>

#include "HTTPParser.hpp"

#define LOG(fmt, ...) \
    printf("[%s:%d]@%s " fmt, __FILE__, __LINE__, __FUNCTION__, ##__VA_ARGS__)


const char http_header[25] = "HTTP/1.1 200 Ok\r\n";

extern "C" void
send_binary(int fd, char image_path[], char head[]);


const char *
parse(const char line[], const char symbol[])
{
    char *copy = (char *)malloc(strlen(line) + 1);
    strcpy(copy, line);

    const char *message = "";
    char *token = strtok(copy, symbol);
    int current = 0;

    while (token != NULL)
    {
        token = strtok(NULL, " ");
        if (current == 0)
        {
            message = token;
            if (message == NULL)
            {
                message = "";
            }
            return message;
        }
        current = current + 1;
    }
    free(token);
    free(copy);
    return message;
}

const char *
parse_method(const char line[], const char symbol[])
{
    char *copy = (char *)malloc(strlen(line) + 1);
    strcpy(copy, line);

    const char *message = NULL;
    char *token = strtok(copy, symbol);
    int current = 0;

    while (token != NULL)
    {
        // token = strtok(NULL, " ");
        if (current == 0)
        {
            message = token;
            if (message == NULL)
            {
                message = "";
            }
            return message;
        }
        current = current + 1;
    }
    free(copy);
    free(token);
    return message;
}

const char *
find_token(const char line[], const char symbol[], const char match[])
{
    char *copy = (char *)malloc(strlen(line) + 1);
    strcpy(copy, line);

    const char *message = NULL;
    char *token = strtok(copy, symbol);

    while (token != NULL)
    {
        // LOG("--Token: %s \n", token);

        if (strlen(match) <= strlen(token))
        {
            int match_char = 0;
            for (int i = 0; i < (int)strlen(match); i++)
            {
                if (token[i] == match[i])
                {
                    match_char++;
                }
            }
            if ((size_t)match_char == strlen(match))
            {
                message = token;
                return message;
            }
        }
        token = strtok(NULL, symbol);
    }
    free(copy);
    free(token);
    message = "";
    return message;
}

co_context::task<>
HTTPParser::http_parse(const char *request)
{
    // LOG("read=%d, request message: \n%s \n ", strlen(request), request);
    const char *parse_string_method = parse_method(
        request, " ");

    const char *parse_string =
        parse(request, " ");

    char copy[4096];
    strcpy(copy, parse_string);
    const char *parse_ext =
        parse(copy, "."); // get the file extension such as JPG, jpg

    char copy_head[4096];
    strcpy(copy_head, http_header);

    if (parse_string_method[0] == 'G' && parse_string_method[1] == 'E' &&
        parse_string_method[2] == 'T')
    {
        char path_head[500] = ".";

        if (strlen(parse_string) <= 1)
        {
            strcat(path_head, "/index.html");
            strcat(copy_head, "Content-Type: text/html\r\n\r\n");
        }
        else if ((parse_string[1] == 't' && parse_string[2] == 'e' &&
                  parse_string[3] == 's' && parse_string[4] == 't'))
        {
            strcat(copy_head, "Content-Type: text/plain\r\n\r\n");
            // send_binary(new_socket, NULL, copy_head); // TODO
        }
        else if (
            (parse_ext[0] == 'j' && parse_ext[1] == 'p' &&
             parse_ext[2] == 'g') ||
            (parse_ext[0] == 'J' && parse_ext[1] == 'P' &&
             parse_ext[2] == 'G'))
        {
            // send image to client
            strcat(path_head, parse_string);
            strcat(copy_head, "Content-Type: image/jpeg\r\n\r\n");
        }
        else if (
            parse_ext[0] == 'i' && parse_ext[1] == 'c' && parse_ext[2] == 'o')
        {
            strcat(path_head, "/img/favicon.png");
            strcat(copy_head, "Content-Type: image/vnd.microsoft.icon\r\n\r\n");
        }
        else if (
            parse_ext[0] == 't' && parse_ext[1] == 't' && parse_ext[2] == 'f')
        {
            strcat(path_head, parse_string);
            strcat(copy_head, "Content-Type: font/ttf\r\n\r\n");
        }
        else if (
            parse_ext[strlen(parse_ext) - 2] == 'j' &&
            parse_ext[strlen(parse_ext) - 1] == 's')
        {
            // javascript
            strcat(path_head, parse_string);
            strcat(copy_head, "Content-Type: text/javascript\r\n\r\n");
        }
        else if (
            parse_ext[strlen(parse_ext) - 3] == 'c' &&
            parse_ext[strlen(parse_ext) - 2] == 's' &&
            parse_ext[strlen(parse_ext) - 1] == 's')
        {
            // css
            strcat(path_head, parse_string);
            strcat(copy_head, "Content-Type: text/css\r\n\r\n");
        }
        else if (
            parse_ext[0] == 'w' && parse_ext[1] == 'o' && parse_ext[2] == 'f')
        {
            // Web Open Font Format woff and woff2
            strcat(path_head, parse_string);
            strcat(copy_head, "Content-Type: font/woff\r\n\r\n");
        }
        else if (
            parse_ext[0] == 'm' && parse_ext[1] == '3' && parse_ext[2] == 'u' &&
            parse_ext[3] == '8')
        {
            // Web Open m3u8
            strcat(path_head, parse_string);
            strcat(
                copy_head,
                "Content-Type: application/vnd.apple.mpegurl\r\n\r\n");
        }
        else if (parse_ext[0] == 't' && parse_ext[1] == 's')
        {
            // Web Open ts
            char path_head[500] = ".";
            strcat(path_head, parse_string);
            strcat(copy_head, "Content-Type: video/mp2t\r\n\r\n");
        }
        else
        {
            LOG("Else: %s \n", parse_string);
            strcat(path_head, parse_string);
            strcat(copy_head, "Content-Type: text/plain\r\n\r\n");
        }
        if (strlen(path_head) == 1)
        {
            strcat(path_head, "/index.html");
        }
        send_message(path_head, copy_head);
    }
    else if (
        parse_string_method[0] == 'P' && parse_string_method[1] == 'O' &&
        parse_string_method[2] == 'S' && parse_string_method[3] == 'T')
    {
        const char *find_string = find_token(request, "\r\n", "action");
        strcat(copy_head, "Content-Type: text/plain \r\n\r\n"); //\r\n\r\n
        strcat(copy_head, "User Action: ");
        strcat(copy_head, find_string);
        co_await m_sock.send({copy_head, strlen(copy_head)});
    }
    else
    {
        LOG("ERROR! Method: %s \n", parse_string_method);
        co_return;
    }
    co_return;
}

void
HTTPParser::send_message(char image_path[], char head[])
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
