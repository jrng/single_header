// sh_http_server.h - MIT License
// See end of file for full license

#ifndef __SH_HTTP_SERVER_INCLUDE__
#define __SH_HTTP_SERVER_INCLUDE__

#  ifndef __SH_HASH_INCLUDE__
#    error "sh_http_server.h requires sh_hash.h to be included first"
#  endif

#  ifndef __SH_BASE64_INCLUDE__
#    error "sh_http_server.h requires sh_base64.h to be included first"
#  endif

#  ifndef __SH_STRING_BUILDER_INCLUDE__
#    error "sh_http_server.h requires sh_string_builder.h to be included first"
#  endif

#  include <stdio.h> // TODO: remove

#  if SH_PLATFORM_WINDOWS

#    include <winsock2.h>

#    if !defined(__MINGW32__)
#      pragma comment(lib, "Ws2_32")
#    endif

#  elif SH_PLATFORM_UNIX

#    include <poll.h>
#    include <errno.h>
#    include <fcntl.h>
#    include <unistd.h>
#    include <netinet/in.h>
#    include <sys/socket.h>

#  endif

#  if defined(SH_STATIC) || defined(SH_HTTP_SERVER_STATIC)
#    define SH_HTTP_SERVER_DEF static
#  else
#    define SH_HTTP_SERVER_DEF extern
#  endif

typedef enum
{
    SH_HTTP_PROTOCOL_VERSION_1_0 = 0,
    SH_HTTP_PROTOCOL_VERSION_1_1 = 1,
} ShHttpProtocolVersion;

typedef enum
{
    SH_HTTP_REQUEST_METHOD_GET     = 0,
    SH_HTTP_REQUEST_METHOD_HEAD    = 1,
    SH_HTTP_REQUEST_METHOD_POST    = 2,
    SH_HTTP_REQUEST_METHOD_PUT     = 3,
    SH_HTTP_REQUEST_METHOD_DELETE  = 4,
    SH_HTTP_REQUEST_METHOD_CONNECT = 5,
    SH_HTTP_REQUEST_METHOD_OPTIONS = 6,
    SH_HTTP_REQUEST_METHOD_TRACE   = 7,
    SH_HTTP_REQUEST_METHOD_PATCH   = 8,
} ShHttpRequestMethod;

typedef struct
{
    ShString name;
    ShString value;
} ShHttpHeaderField;

typedef struct
{
    ShHttpRequestMethod method;
    ShString uri;
    ShHttpProtocolVersion protocol_version;

    ShHttpHeaderField *header_fields;
    ShString body;
} ShHttpRequest;

typedef struct
{
    ShString input_buffer;
    ShArena arena;
    ShStringBuilder output_builder;

    bool is_websocket;
    bool close_connection;
    bool is_waiting_for_request;

#  if SH_PLATFORM_UNIX
    int socket;
#  endif
} ShHttpClient;

typedef void (*ShHttpRequestCallback)(ShHttpRequest request, ShStringBuilder *response);

typedef struct
{
    uint16_t client_count;
    uint16_t max_client_count;

    ShHttpClient *clients;

    void *allocation;
    ShAllocator allocator;

    ShArena arena;

    ShHttpRequestCallback handle_request;

#  if SH_PLATFORM_UNIX
    int socket;
#  endif
} ShHttpServer;

// Creates an http server and starts listening to the given port.
SH_HTTP_SERVER_DEF bool sh_http_server_create(ShHttpServer *http_server, ShAllocator allocator, uint16_t port, uint16_t max_client_count, ShHttpRequestCallback handle_request);

SH_HTTP_SERVER_DEF void sh_http_server_run(ShHttpServer *http_server, bool wait_for_event);

SH_HTTP_SERVER_DEF bool sh_http_parse_request(ShHttpRequest *request, ShString request_string);

// Stops and destroys the http server.
SH_HTTP_SERVER_DEF void sh_http_server_destroy(ShHttpServer *http_server);

#endif // __SH_HTTP_SERVER_INCLUDE__

#ifdef SH_HTTP_SERVER_IMPLEMENTATION

#  define INPUT_BUFFER_SIZE ShKiB(4)
#  define ARENA_CAPACITY ShKiB(16)

#  if SH_PLATFORM_UNIX

static inline bool
_sh_http_server_set_socket_non_blocking(int socket)
{
    int flags;

    if ((flags = fcntl(socket, F_GETFL, 0)) < 0)
    {
        return false;
    }

    flags |= O_NONBLOCK;

    if (fcntl(socket, F_SETFL, flags) != 0)
    {
        return false;
    }

    return true;
}

#  endif

SH_HTTP_SERVER_DEF bool
sh_http_server_create(ShHttpServer *http_server, ShAllocator allocator, uint16_t port, uint16_t max_client_count, ShHttpRequestCallback handle_request)
{
#  if SH_PLATFORM_WINDOWS
    WSADATA winsocket_data;

    if (WSAStartup(MAKEWORD(2, 2), &winsocket_data) != 0)
    {
        return false;
    }

    if ((LOBYTE(winsocket_data.wVersion) != 2) ||
        (HIBYTE(winsocket_data.wVersion) != 2))
    {
        WSACleanup();
        return false;
    }

    assert(!"unimplemented");
#  elif SH_PLATFORM_UNIX
    http_server->socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

    if (http_server->socket < 0)
    {
        return false;
    }

    if (!_sh_http_server_set_socket_non_blocking(http_server->socket))
    {
        close(http_server->socket);
        return false;
    }

    int option_value = 1;

    if (setsockopt(http_server->socket, SOL_SOCKET, SO_REUSEADDR, &option_value, sizeof(option_value)) != 0)
    {
        close(http_server->socket);
        return false;
    }

    struct sockaddr_in server_addr = { 0 };

    server_addr.sin_family      = AF_INET;
    server_addr.sin_port        = htons(port);
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY);

    if (bind(http_server->socket, (struct sockaddr *) &server_addr, sizeof(server_addr)) != 0)
    {
        close(http_server->socket);
        return false;
    }

    if (listen(http_server->socket, 8) != 0)
    {
        close(http_server->socket);
        return false;
    }
#  endif

    http_server->client_count = 0;
    http_server->max_client_count = max_client_count;
    http_server->handle_request = handle_request;

    usize clients_size       =  max_client_count      * sizeof(ShHttpClient);
    usize input_buffers_size =  max_client_count      * INPUT_BUFFER_SIZE;
    usize arenas_size        = (max_client_count + 1) * ARENA_CAPACITY;

    usize total_size = clients_size + input_buffers_size + arenas_size;

    http_server->allocation = sh_alloc(allocator, total_size);
    http_server->allocator = allocator;

    uint8_t *ptr = (uint8_t *) http_server->allocation;

    http_server->clients = (ShHttpClient *) ptr;
    ptr += clients_size;

    // TODO: use smaller arena for global
    sh_arena_init_with_memory(&http_server->arena, ptr, ARENA_CAPACITY);
    ptr += ARENA_CAPACITY;

    for (uint16_t i = 0; i < http_server->max_client_count; i += 1)
    {
        ShHttpClient *client = http_server->clients + i;

        client->input_buffer.count = 0;
        client->input_buffer.data = ptr;
        ptr += INPUT_BUFFER_SIZE;

        sh_arena_init_with_memory(&client->arena, ptr, ARENA_CAPACITY);
        ptr += ARENA_CAPACITY;
    }

    return true;
}

SH_HTTP_SERVER_DEF void
sh_http_server_run(ShHttpServer *http_server, bool wait_for_event)
{
    sh_arena_clear(&http_server->arena);
    ShAllocator temp_allocator = sh_arena_get_allocator(&http_server->arena);

    uint16_t *clients_to_delete = NULL;
    sh_array_init(clients_to_delete, 4, temp_allocator);

    uint16_t current_client_count = http_server->client_count;

#  if SH_PLATFORM_UNIX
    struct pollfd *sockets = sh_alloc_array(temp_allocator, struct pollfd, current_client_count + 1);

    sockets[0].fd      = http_server->socket;
    sockets[0].events  = POLLIN;
    sockets[0].revents = 0;

    for (uint16_t i = 0; i < current_client_count; i += 1)
    {
        ShHttpClient *client = http_server->clients + i;

        sockets[i + 1].fd      = client->socket;
        sockets[i + 1].events  = client->is_waiting_for_request ? POLLIN : POLLOUT;
        sockets[i + 1].revents = 0;
    }

    int timeout = 0;

    if (wait_for_event)
    {
        timeout = -1;
    }

    int ret = poll(sockets, current_client_count + 1, timeout);

    if (ret > 0)
    {
        if (sockets[0].revents & POLLIN)
        {
            struct sockaddr_in client_addr;
            socklen_t client_addr_len = sizeof(client_addr);

            int client_socket = accept(http_server->socket, (struct sockaddr *) &client_addr, &client_addr_len);

            if (client_socket >= 0)
            {
                if (_sh_http_server_set_socket_non_blocking(client_socket) &&
                    (http_server->client_count < http_server->max_client_count))
                {
                    uint16_t client_index = http_server->client_count;
                    http_server->client_count += 1;

                    ShHttpClient *client = http_server->clients + client_index;

                    client->input_buffer.count = 0;
                    client->socket = client_socket;
                    client->is_websocket = false;
                    client->close_connection = true;
                    client->is_waiting_for_request = true;
                }
                else
                {
                    fprintf(stderr, "error: too many clients\n");
                    close(client_socket);
                }
            }
        }

        for (uint16_t i = 0; i < current_client_count; i += 1)
        {
            ShHttpClient *client = http_server->clients + i;

            if (sockets[i + 1].revents & POLLIN)
            {
                sh_arena_clear(&client->arena);

                ssize_t ret;
                bool has_request = true;

                for (;;)
                {
                    ret = read(client->socket, client->input_buffer.data + client->input_buffer.count, INPUT_BUFFER_SIZE - client->input_buffer.count);
                    fprintf(stderr, " read = %zd\n", ret);

                    if (ret < 0)
                    {
                        if (errno != EAGAIN)
                        {
                            has_request = false;
                        }

                        break;
                    }
                    else if (ret == 0)
                    {
                        fprintf(stderr, "POLLIN read = 0\n");
                        close(client->socket);
                        *sh_array_append(clients_to_delete) = i;
                        has_request = false;

                        break;
                    }
                    else
                    {
                        client->input_buffer.count += ret;
                    }
                }

                if (has_request)
                {
                    fprintf(stderr, "has request - is_websocket = %s\n",
                            client->is_websocket ? "true" : "false");

                    ShAllocator allocator = sh_arena_get_allocator(&client->arena);

                    sh_string_builder_init(&client->output_builder, allocator);

                    ShHttpRequest request;
                    request.header_fields = NULL;
                    sh_array_init(request.header_fields, 16, allocator);

                    if (sh_http_parse_request(&request, client->input_buffer))
                    {
                        if (request.protocol_version == SH_HTTP_PROTOCOL_VERSION_1_1)
                        {
                            client->close_connection = false;
                        }
                        else if (request.protocol_version == SH_HTTP_PROTOCOL_VERSION_1_0)
                        {
                            client->close_connection = true;
                        }

                        ShHttpHeaderField *connection_field = NULL;
                        ShHttpHeaderField *websocket_version_field = NULL;
                        ShHttpHeaderField *websocket_key_field = NULL;
                        ShHttpHeaderField *upgrade_field = NULL;

                        // TODO: header field names are case-insensitive
                        for (usize j = 0; j < sh_array_count(request.header_fields); j += 1)
                        {
                            ShHttpHeaderField *header_field = request.header_fields + j;

                            if (sh_string_equal(header_field->name, ShStringLiteral("Connection")))
                            {
                                connection_field = header_field;
                            }
                            else if (sh_string_equal(header_field->name, ShStringLiteral("Sec-WebSocket-Version")))
                            {
                                websocket_version_field = header_field;
                            }
                            else if (sh_string_equal(header_field->name, ShStringLiteral("Sec-WebSocket-Key")))
                            {
                                websocket_key_field = header_field;
                            }
                            else if (sh_string_equal(header_field->name, ShStringLiteral("Upgrade")))
                            {
                                upgrade_field = header_field;
                            }
                        }

                        if (connection_field)
                        {
                            ShString value = connection_field->value;

                            while (value.count)
                            {
                                ShString val = sh_string_trim(sh_string_split_left_on_char(&value, ','));

                                if (sh_string_equal(val, ShStringLiteral("close")))
                                {
                                    client->close_connection = true;
                                }
                                else if (sh_string_equal(val, ShStringLiteral("keep-alive")))
                                {
                                    client->close_connection = false;
                                }
                                else if (sh_string_equal(val, ShStringLiteral("Upgrade")))
                                {
                                    // TODO: has to be GET and >=HTTP/1.1
                                    if (websocket_version_field && sh_string_equal(websocket_version_field->value, ShStringLiteral("13")) &&
                                        upgrade_field && sh_string_equal(upgrade_field->value, ShStringLiteral("websocket")) && websocket_key_field)
                                    {
                                        ShString websocket_key = websocket_key_field->value;
                                        ShString websocket_magic = ShStringLiteral("258EAFA5-E914-47DA-95CA-C5AB0DC85B11");

                                        ShString websocket_concat = sh_string_concat_n(temp_allocator, 2, websocket_key, websocket_magic);
                                        ShSha1 websocket_sha1 = sh_hash_sha1(websocket_concat.count, websocket_concat.data);
                                        ShString websocket_accept = sh_base64_encode(temp_allocator, ShArrayCount(websocket_sha1.hash), websocket_sha1.hash);

                                        sh_string_builder_append_string(&client->output_builder, ShStringLiteral("HTTP/1.1 101 Switching Protocols\r\n"));
                                        sh_string_builder_append_string(&client->output_builder, ShStringLiteral("Upgrade: websocket\r\n"));
                                        sh_string_builder_append_string(&client->output_builder, ShStringLiteral("Connection: Upgrade\r\n"));
                                        sh_string_builder_append_string(&client->output_builder, ShStringLiteral("Sec-WebSocket-Accept: "));
                                        sh_string_builder_append_string(&client->output_builder, websocket_accept);
                                        sh_string_builder_append_string(&client->output_builder, ShStringLiteral("\r\n"));
                                        sh_string_builder_append_string(&client->output_builder, ShStringLiteral("\r\n"));

                                        client->is_websocket = true;
                                    }
                                }
                            }
                        }

                        if ((sh_string_builder_get_size(&client->output_builder) == 0) &&
                            http_server->handle_request)
                        {
                            http_server->handle_request(request, &client->output_builder);
                        }

                        if (sh_string_builder_get_size(&client->output_builder) == 0)
                        {
                            ShString body = ShStringLiteral(
                                "<!doctype html>\n"
                                "<html>\n"
                                "<head>\n"
                                "  <title>500 Internal Server Error</title>\n"
                                "</head>\n"
                                "<body>\n"
                                "  <h1>500 Internal Server Error</h1>\n"
                                "  <p>There was no response generated. Maybe there is no callback registered for handling requests.</p>\n"
                                "</body>\n"
                                "</html>\n"
                            );

                            sh_string_builder_append_string(&client->output_builder, ShStringLiteral("HTTP/1.1 500 Internal Server Error\r\n"));
                            sh_string_builder_append_string(&client->output_builder, ShStringLiteral("Server: ShHttpServer\r\n"));
                            sh_string_builder_append_string(&client->output_builder, ShStringLiteral("Content-Type: text/html; charset=utf-8\r\n"));
                            sh_string_builder_append_string(&client->output_builder, ShStringLiteral("Content-Length: "));
                            sh_string_builder_append_number(&client->output_builder, body.count, 0, '0', 10, false);
                            sh_string_builder_append_string(&client->output_builder, ShStringLiteral("\r\n"));
                            sh_string_builder_append_string(&client->output_builder, ShStringLiteral("\r\n"));
                            sh_string_builder_append_string(&client->output_builder, body);
                        }
                    }

                    client->is_waiting_for_request = false;
                    client->input_buffer.count = 0;
                }
            }
            else if (sockets[i + 1].revents & POLLOUT)
            {
                // TODO: do better
                ShStringBuffer *buffer = client->output_builder.first_buffer;

                write(client->socket, buffer->data, buffer->occupied);

                client->is_waiting_for_request = true;

                if (client->close_connection)
                {
                    fprintf(stderr, "POLLOUT close_connection\n");
                    close(client->socket);
                    *sh_array_append(clients_to_delete) = i;
                }
            }
            else if (sockets[i + 1].revents & (POLLERR | POLLHUP))
            {
                fprintf(stderr, "POLLHUP\n");
                close(client->socket);
                *sh_array_append(clients_to_delete) = i;
            }
        }
    }
#  else
    assert(!"unimplemented");
#  endif

    usize count = sh_array_count(clients_to_delete);

    for (usize i = 0; i < count; i += 1)
    {
        http_server->client_count -= 1;
        uint16_t index = clients_to_delete[i];

        if (index < http_server->client_count)
        {
            http_server->clients[index] = http_server->clients[http_server->client_count];
        }
    }
}

SH_HTTP_SERVER_DEF bool
sh_http_parse_request(ShHttpRequest *request, ShString request_string)
{
    ShString request_line = sh_string_split_left(&request_string, ShStringLiteral("\r\n"));

    ShString request_method = sh_string_split_left_on_char(&request_line, ' ');
    ShString request_uri = sh_string_split_left_on_char(&request_line, ' ');
    ShString protocol_version = request_line;

    request->method = SH_HTTP_REQUEST_METHOD_GET;
    request->uri = request_uri;

    if (sh_string_equal(request_method, ShStringLiteral("GET")))
    {
        request->method = SH_HTTP_REQUEST_METHOD_GET;
    }
    else if (sh_string_equal(request_method, ShStringLiteral("HEAD")))
    {
        request->method = SH_HTTP_REQUEST_METHOD_HEAD;
    }
    else if (sh_string_equal(request_method, ShStringLiteral("POST")))
    {
        request->method = SH_HTTP_REQUEST_METHOD_POST;
    }
    else if (sh_string_equal(request_method, ShStringLiteral("PUT")))
    {
        request->method = SH_HTTP_REQUEST_METHOD_PUT;
    }
    else if (sh_string_equal(request_method, ShStringLiteral("DELETE")))
    {
        request->method = SH_HTTP_REQUEST_METHOD_DELETE;
    }
    else if (sh_string_equal(request_method, ShStringLiteral("CONNECT")))
    {
        request->method = SH_HTTP_REQUEST_METHOD_CONNECT;
    }
    else if (sh_string_equal(request_method, ShStringLiteral("OPTIONS")))
    {
        request->method = SH_HTTP_REQUEST_METHOD_OPTIONS;
    }
    else if (sh_string_equal(request_method, ShStringLiteral("TRACE")))
    {
        request->method = SH_HTTP_REQUEST_METHOD_TRACE;
    }
    else if (sh_string_equal(request_method, ShStringLiteral("PATCH")))
    {
        request->method = SH_HTTP_REQUEST_METHOD_PATCH;
    }
    else
    {
        // TODO: send error response
        return false;
    }

    request->protocol_version = SH_HTTP_PROTOCOL_VERSION_1_1;

    if (sh_string_equal(protocol_version, ShStringLiteral("HTTP/1.1")))
    {
        request->protocol_version = SH_HTTP_PROTOCOL_VERSION_1_1;
    }
    else if (sh_string_equal(protocol_version, ShStringLiteral("HTTP/1.0")))
    {
        request->protocol_version = SH_HTTP_PROTOCOL_VERSION_1_0;
    }
    else
    {
        // TODO: send error response
        return false;
    }

    for (;;)
    {
        ShString header_field = sh_string_split_left(&request_string, ShStringLiteral("\r\n"));

        if (header_field.count == 0)
        {
            break;
        }

        ShString field_name = sh_string_split_left_on_char(&header_field, ':');
        ShString field_value = header_field;

        if ((field_value.count > 0) && (field_value.data[0] == ' '))
        {
            field_value.count -= 1;
            field_value.data += 1;
        }

        ShHttpHeaderField *field = sh_array_append(request->header_fields);

        field->name = field_name;
        field->value = field_value;
    }

    request->body = request_string;

    return true;
}

SH_HTTP_SERVER_DEF void
sh_http_server_destroy(ShHttpServer *http_server)
{
#  if SH_PLATFORM_UNIX
    close(http_server->socket);
#  else
    assert(!"unimplemented");
#  endif

    sh_free(http_server->allocator, http_server->allocation);
}

#  undef ARENA_CAPACITY
#  undef INPUT_BUFFER_SIZE

#endif // SH_HTTP_SERVER_IMPLEMENTATION

/*
MIT License

Copyright (c) 2025 Julius Range-Lüdemann

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/
