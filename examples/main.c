// cc -std=c99 -o main main.c
#define SH_IMPLEMENTATION
#define SH_STATIC
#include "sh.h"

#include <stdio.h>
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
check_sha1(usize size, char const *str, ShSha1 *expected_sha1)
{
    ShSha1 sha1 = sh_hash_sha1(size, str);

    printf("sha1(\"%.*s\") = ", (int) size, str);

    for (usize i = 0; i < ShArrayCount(sha1.hash); i += 1)
    {
        printf("%02x", sha1.hash[i]);
    }

    printf("\n");

    if (!sh_hash_sha1_equal(&sha1, expected_sha1))
    {
        printf("  !!! exptected ");
        for (usize i = 0; i < ShArrayCount(expected_sha1->hash); i += 1)
        {
            printf("%02x", expected_sha1->hash[i]);
        }
        printf("\n");
    }
}

int main(void)
{
    ShAllocator allocator;
    allocator.data = NULL;
    allocator.func = c_default_allocator_func;

    {
        printf("--- sh_hash -------------\n");

        const char empty_string[] = ""; // da39a3ee5e6b4b0d3255bfef95601890afd80709
        ShSha1 empty_string_sha1_expected = {{ 0xda, 0x39, 0xa3, 0xee, 0x5e, 0x6b, 0x4b, 0x0d, 0x32, 0x55, 0xbf, 0xef, 0x95, 0x60, 0x18, 0x90, 0xaf, 0xd8, 0x07, 0x09 }};
        check_sha1(sizeof(empty_string) - 1, empty_string, &empty_string_sha1_expected);

        const char string2[] = "The quick brown fox jumps over the lazy dog"; // 2fd4e1c67a2d28fced849ee1bb76e7391b93eb12
        ShSha1 string2_sha1_expected = {{ 0x2f, 0xd4, 0xe1, 0xc6, 0x7a, 0x2d, 0x28, 0xfc, 0xed, 0x84, 0x9e, 0xe1, 0xbb, 0x76, 0xe7, 0x39, 0x1b, 0x93, 0xeb, 0x12 }};
        check_sha1(sizeof(string2) - 1, string2, &string2_sha1_expected);

        printf("\n");
    }

    {
        printf("--- sh_base64 -----------\n");

        const char hello_world[] = "Hello World!"; // SGVsbG8gV29ybGQh
        ShString hello_world_base64 = sh_base64_encode(allocator, sizeof(hello_world) - 1, hello_world);
        ShString hello_world_base64_expected = ShStringLiteral("SGVsbG8gV29ybGQh");

        printf("base64(\"%s\") = \"%" ShStringFmt "\"\n", hello_world, ShStringArg(hello_world_base64));

        if (!sh_string_equal(hello_world_base64, hello_world_base64_expected))
        {
            printf("  !!! expected \"%" ShStringFmt "\"\n", ShStringArg(hello_world_base64_expected));
        }

        sh_free(allocator, hello_world_base64.data);

        printf("\n");
    }

    return 0;
}
