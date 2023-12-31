#pragma once

#include "co_context/net.hpp"

class HTTPParser
{
private:
    co_context::socket &m_sock;

public:
    explicit HTTPParser(co_context::socket &sock) : m_sock(sock) {}
    virtual ~HTTPParser() {}

    co_context::task<>
    httpParse(const char *request);

    co_context::task<> response(const char *image_path, const char *head);
};