#define SH_BASE_IMPLEMENTATION
#include "sh_base.h"
#define SH_HASH_IMPLEMENTATION
#include "sh_hash.h"
#define SH_BASE64_IMPLEMENTATION
#include "sh_base64.h"
#define SH_STRING_BUILDER_IMPLEMENTATION
#include "sh_string_builder.h"
#define SH_HTTP_SERVER_IMPLEMENTATION
#include "sh_http_server.h"

#include <stdlib.h>

void *c_default_allocator_func(void *allocator_data, ShAllocatorAction action, usize old_size, usize size, void *ptr)
{
    (void) allocator_data;
    (void) old_size;

    void *result = NULL;

    switch (action)
    {
        case SH_ALLOCATOR_ACTION_ALLOC:   result = malloc(size);       break;
        case SH_ALLOCATOR_ACTION_REALLOC: result = realloc(ptr, size); break;
        case SH_ALLOCATOR_ACTION_FREE:    free(ptr);                   break;
    }

    return result;
}

static void
handle_http_request(ShHttpRequest request, ShStringBuilder *output)
{
    if (sh_string_equal(request.uri, ShStringLiteral("/")))
    {
        ShString body = ShStringLiteral("<!doctype html><html><head><title>Hello</title></head><body><h2>Hello</h2></body></html>");

        sh_string_builder_append_string(output, ShStringLiteral("HTTP/1.1 200 OK\r\n"));
        sh_string_builder_append_formated(output, ShStringLiteral("Content-Length: %zu\r\n"), body.count);
        sh_string_builder_append_string(output, ShStringLiteral("\r\n"));
        sh_string_builder_append_string(output, body);
    }
    else
    {
        ShString body = ShStringLiteral("<!doctype html><html><head><title>Not Found</title></head><body><h2>Hello</h2></body></html>");

        sh_string_builder_append_string(output, ShStringLiteral("HTTP/1.1 404 Not Found\r\n"));
        sh_string_builder_append_formated(output, ShStringLiteral("Content-Length: %zu\r\n"), body.count);
        sh_string_builder_append_string(output, ShStringLiteral("\r\n"));
        sh_string_builder_append_string(output, body);
    }
}

int main(void)
{
    ShAllocator allocator;
    allocator.data = NULL;
    allocator.func = c_default_allocator_func;

    ShHttpServer http_server;

    if (!sh_http_server_create(&http_server, allocator, 8080, 10, handle_http_request))
    {
        return -1;
    }

    ShThreadContext *thread_context = sh_thread_context_create(allocator, ShMiB(1));

    while (true)
    {
        sh_http_server_run(thread_context, &http_server, true);
    }

    sh_http_server_destroy(&http_server);
    sh_thread_context_destroy(thread_context);

    return 0;
}
