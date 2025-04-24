// sh_base.h - MIT License
// See end of file for full license

#ifndef __SH_BASE_INCLUDE__
#define __SH_BASE_INCLUDE__

#  define SH_PLATFORM_ANDROID 0
#  define SH_PLATFORM_FREEBSD 0
#  define SH_PLATFORM_WINDOWS 0
#  define SH_PLATFORM_LINUX   0
#  define SH_PLATFORM_MACOS   0

#  define SH_PLATFORM_UNIX    0

#  if defined(__ANDROID__)
#    undef  SH_PLATFORM_ANDROID
#    define SH_PLATFORM_ANDROID 1
#  elif defined(__FreeBSD__)
#    undef  SH_PLATFORM_FREEBSD
#    define SH_PLATFORM_FREEBSD 1
#  elif defined(_WIN32)
#    undef  SH_PLATFORM_WINDOWS
#    define SH_PLATFORM_WINDOWS 1
#  elif defined(__linux__)
#    undef  SH_PLATFORM_LINUX
#    define SH_PLATFORM_LINUX 1
#  elif defined(__APPLE__) && defined(__MACH__)
#    undef  SH_PLATFORM_MACOS
#    define SH_PLATFORM_MACOS 1
#  endif

#  if SH_PLATFORM_ANDROID || SH_PLATFORM_FREEBSD || SH_PLATFORM_LINUX || SH_PLATFORM_MACOS
#    undef  SH_PLATFORM_UNIX
#    define SH_PLATFORM_UNIX 1
#  endif

#  include <assert.h>
#  include <stdarg.h>
#  include <stddef.h>
#  include <stdint.h>
#  include <stdbool.h>

#  if defined(SH_STATIC) || defined(SH_BASE_STATIC)
#    define SH_BASE_DEF static
#  else
#    define SH_BASE_DEF extern
#  endif

typedef ptrdiff_t ssize;
typedef size_t usize;

#  define ShKiB(value) (     (value) * (usize) 1024)
#  define ShMiB(value) (ShKiB(value) * (usize) 1024)
#  define ShGiB(value) (ShMiB(value) * (usize) 1024)

#  define ShArrayCount(arr) (sizeof(arr)/sizeof((arr)[0]))
#  define ShOffsetOf(type, member) (usize) &((type *) 0)->member
#  define ShContainerOf(ptr, type, member) (type *) ((uint8_t *) (ptr) - ShOffsetOf(type, member))

typedef struct ShList ShList;

struct ShList
{
    ShList *prev;
    ShList *next;
};

#  define ShDListInit(sentinel)                      \
    (sentinel)->prev = (sentinel)->next = (sentinel);

#  define ShDListIsEmpty(sentinel)                   \
    ((sentinel)->prev == (sentinel))

#  define ShDListInsertBefore(sentinel, element)     \
    (element)->prev = (sentinel)->prev;              \
    (element)->next = (sentinel);                    \
    (sentinel)->prev->next = (element);              \
    (sentinel)->prev = (element);

#  define ShDListInsertAfter(sentinel, element)      \
    (element)->prev = (sentinel);                    \
    (element)->next = (sentinel)->next;              \
    (sentinel)->next->prev = (element);              \
    (sentinel)->next = (element);

#  define ShDListRemove(element)                     \
    (element)->prev->next = (element)->next;         \
    (element)->next->prev = (element)->prev;         \
    (element)->prev = (element)->next = (element);

typedef struct
{
    usize count;
    uint8_t *data;
} ShString;

#  define ShStringFmt ".*s"
#  define ShStringArg(str) (int) (str).count, (str).data

#  ifdef __cplusplus
#    define ShStringLiteral(str) ShString { sizeof(str) - 1, (uint8_t *) (str) }
#  else
#    define ShStringLiteral(str) (ShString) { sizeof(str) - 1, (uint8_t *) (str) }
#  endif

typedef enum
{
    SH_ALLOCATOR_ACTION_ALLOC   = 0,
    SH_ALLOCATOR_ACTION_REALLOC = 1,
    SH_ALLOCATOR_ACTION_FREE    = 2,
} ShAllocatorAction;

typedef void *(*ShAllocatorFunc)(void *allocator_data, ShAllocatorAction action, usize old_size, usize size, void *ptr);

typedef struct
{
    void *data;
    ShAllocatorFunc func;
} ShAllocator;

#  define sh_alloc_type(allocator, type) (type *) sh_alloc(allocator, sizeof(type))
#  define sh_alloc_array(allocator, type, count) (type *) sh_alloc(allocator, (count) * sizeof(type))

typedef struct
{
    uint8_t *base;
    usize capacity;
    usize occupied;
} ShArena;

typedef struct
{
    ShAllocator allocator;
    usize count;
    usize allocated;
} ShArrayHeader;

#  define sh_array_header(array) ((ShArrayHeader *) (array) - 1)
#  define sh_array_count(array) ((array) ? sh_array_header(array)->count : 0)
#  define sh_array_allocated(array) ((array) ? sh_array_header(array)->allocated : 0)
#  define sh_array_append(array) ((array) ? (sh_array_ensure_space(array), (array) + sh_array_header(array)->count++) : 0)

#  ifdef __cplusplus
#    define sh_array_init(array, initial_allocated, allocator)  \
        ((array) = (decltype(+(array))) sh_array_grow(array, initial_allocated, sizeof(*(array)), allocator))
#    define sh_array_ensure_space(array) \
        (((sh_array_count(array) + 1) > sh_array_allocated(array)) ? \
            (array) = (decltype(+(array))) sh_array_grow(array, 2 * sh_array_allocated(array), sizeof(*(array)), ShAllocator {0, 0}) : 0)
#  else
#    define sh_array_init(array, initial_allocated, allocator)  \
        ((array) = sh_array_grow(array, initial_allocated, sizeof(*(array)), allocator))
#    define sh_array_ensure_space(array) \
        (((sh_array_count(array) + 1) > sh_array_allocated(array)) ? \
            (array) = sh_array_grow(array, 2 * sh_array_allocated(array), sizeof(*(array)), (ShAllocator) {0, 0}) : 0)
#  endif

SH_BASE_DEF void sh_arena_init_with_memory(ShArena *arena, void *memory, usize memory_size);
SH_BASE_DEF void sh_arena_allocate(ShArena *arena, usize capacity, ShAllocator allocator);
SH_BASE_DEF void sh_arena_clear(ShArena *arena);
SH_BASE_DEF void *sh_arena_alloc(ShArena *arena, usize size);
SH_BASE_DEF void *sh_arena_realloc(ShArena *arena, void *ptr, usize old_size, usize size);
SH_BASE_DEF ShAllocator sh_arena_get_allocator(ShArena *arena);

SH_BASE_DEF void *sh_array_grow(void *array, usize new_allocated, usize item_size, ShAllocator allocator);

SH_BASE_DEF void *sh_alloc(ShAllocator allocator, usize size);
SH_BASE_DEF void *sh_realloc(ShAllocator allocator, void *ptr, usize old_size, usize size);
SH_BASE_DEF void sh_free(ShAllocator allocator, void *ptr);

SH_BASE_DEF bool sh_string_equal(ShString a, ShString b);

SH_BASE_DEF ShString sh_string_concat_n(ShAllocator allocator, usize n, ...);

SH_BASE_DEF ShString sh_string_trim(ShString str);
SH_BASE_DEF ShString sh_string_split_left(ShString *str, uint8_t c);
SH_BASE_DEF ShString sh_string_split_left_http_line(ShString *str);

#endif // __SH_BASE_INCLUDE__

#ifdef SH_BASE_IMPLEMENTATION

SH_BASE_DEF void
sh_arena_init_with_memory(ShArena *arena, void *memory, usize memory_size)
{
    arena->base = (uint8_t *) memory;
    arena->capacity = memory_size;
    arena->occupied = 0;
}

SH_BASE_DEF void
sh_arena_allocate(ShArena *arena, usize capacity, ShAllocator allocator)
{
    arena->base = (uint8_t *) sh_alloc(allocator, capacity);
    arena->capacity = capacity;
    arena->occupied = 0;
}

SH_BASE_DEF void
sh_arena_clear(ShArena *arena)
{
    arena->occupied = 0;
}

SH_BASE_DEF void *
sh_arena_alloc(ShArena *arena, usize size)
{
    void *result = 0;

    const usize alignment = 8;
    const usize alignment_mask = alignment - 1;

    usize alignment_offset = 0;
    usize next_pointer = (usize) (arena->base + arena->occupied);

    if (next_pointer & alignment_mask)
    {
        alignment_offset = alignment - (next_pointer & alignment_mask);
    }

    usize effective_size = size + alignment_offset;

    if ((arena->occupied + effective_size) <= arena->capacity)
    {
        result = arena->base + arena->occupied + alignment_offset;
        arena->occupied += effective_size;
    }

    return result;
}

SH_BASE_DEF void *
sh_arena_realloc(ShArena *arena, void *ptr, usize old_size, usize size)
{
    void *result = ptr;

    if (size > old_size)
    {
        // TODO: optimize: don't copy if right next to each other
        result = sh_arena_alloc(arena, size);

        if (result)
        {
            uint8_t *dst = (uint8_t *) result;
            uint8_t *src = (uint8_t *) ptr;

            while (old_size--) *dst++ = *src++;
        }
    }

    return result;
}

static void *
_sh_arena_allocator_func(void *allocator_data, ShAllocatorAction action, usize old_size, usize size, void *ptr)
{
    void *result = 0;
    ShArena *arena = (ShArena *) allocator_data;

    switch (action)
    {
        case SH_ALLOCATOR_ACTION_ALLOC:   result = sh_arena_alloc(arena, size);                  break;
        case SH_ALLOCATOR_ACTION_REALLOC: result = sh_arena_realloc(arena, ptr, old_size, size); break;
        case SH_ALLOCATOR_ACTION_FREE:    /* there is no free for arena */                       break;
    }

    return result;
}

SH_BASE_DEF ShAllocator
sh_arena_get_allocator(ShArena *arena)
{
    ShAllocator allocator;
    allocator.data = arena;
    allocator.func = _sh_arena_allocator_func;
    return allocator;
}

SH_BASE_DEF void *
sh_array_grow(void *array, usize allocated, usize item_size, ShAllocator allocator)
{
    ShArrayHeader *array_header;

    if (array)
    {
        ShArrayHeader *old_array_header = sh_array_header(array);

        usize old_allocated = old_array_header->allocated;
        usize old_size = sizeof(ShArrayHeader) + (old_allocated * item_size);

        if (allocated > old_allocated)
        {
            usize size = sizeof(ShArrayHeader) + (allocated * item_size);

            array_header = (ShArrayHeader *) sh_realloc(old_array_header->allocator, old_array_header, old_size, size);
            array_header->allocated = allocated;
        }
        else
        {
            array_header = old_array_header;
        }
    }
    else
    {
        if (allocated < 4)
        {
            allocated = 4;
        }

        usize size = sizeof(ShArrayHeader) + (allocated * item_size);

        array_header = (ShArrayHeader *) sh_alloc(allocator, size);

        array_header->count = 0;
        array_header->allocated = allocated;
        array_header->allocator = allocator;
    }

    return array_header + 1;
}

SH_BASE_DEF void *
sh_alloc(ShAllocator allocator, usize size)
{
    return allocator.func(allocator.data, SH_ALLOCATOR_ACTION_ALLOC, 0, size, 0);
}

SH_BASE_DEF void *
sh_realloc(ShAllocator allocator, void *ptr, usize old_size, usize size)
{
    return allocator.func(allocator.data, SH_ALLOCATOR_ACTION_REALLOC, old_size, size, ptr);
}

SH_BASE_DEF void
sh_free(ShAllocator allocator, void *ptr)
{
    allocator.func(allocator.data, SH_ALLOCATOR_ACTION_FREE, 0, 0, ptr);
}

SH_BASE_DEF bool
sh_string_equal(ShString a, ShString b)
{
    if (a.count != b.count)
    {
        return false;
    }

    for (usize i = 0; i < a.count; i += 1)
    {
        if (a.data[i] != b.data[i])
        {
            return false;
        }
    }

    return true;
}

SH_BASE_DEF ShString
sh_string_concat_n(ShAllocator allocator, usize n, ...)
{
    ShString result = { 0, 0 };
    ShString *strings = sh_alloc_array(allocator, ShString, n);

    va_list args;
    va_start(args, n);

    for (usize i = 0; i < n; i += 1)
    {
        strings[i] = va_arg(args, ShString);
        result.count += strings[i].count;
    }

    va_end(args);

    result.data = sh_alloc_array(allocator, uint8_t, result.count);

    uint8_t *at = result.data;

    for (usize i = 0; i < n; i += 1)
    {
        ShString str = strings[i];

        for (usize j = 0; j < str.count; j += 1)
        {
            *at++ = str.data[j];
        }
    }

    sh_free(allocator, strings);

    return result;
}

SH_BASE_DEF ShString
sh_string_trim(ShString str)
{
    while (str.count && ((str.data[str.count - 1] == ' ') ||
                         (str.data[str.count - 1] == '\t') ||
                         (str.data[str.count - 1] == '\r') ||
                         (str.data[str.count - 1] == '\n')))
    {
        str.count -= 1;
    }

    while (str.count && ((str.data[0] == ' ') || (str.data[0] == '\t') ||
                         (str.data[0] == '\r') || (str.data[0] == '\n')))
    {
        str.count -= 1;
        str.data += 1;
    }

    return str;
}

SH_BASE_DEF ShString
sh_string_split_left(ShString *str, uint8_t c)
{
    usize index = 0;

    while (index < str->count)
    {
        if (str->data[index] == c)
        {
            break;
        }

        index += 1;
    }

    ShString result;
    result.count = index;
    result.data = str->data;

    if (index < str->count)
    {
        str->count -= index + 1;
        str->data += index + 1;
    }
    else
    {
        str->count -= index;
        str->data += index;
    }

    return result;
}

SH_BASE_DEF ShString
sh_string_split_left_http_line(ShString *str)
{
    usize index = 0;

    while (index < str->count)
    {
        if ((str->data[index] == '\r') &&
            ((index + 1) < str->count) &&
            (str->data[index + 1] == '\n'))
        {
            break;
        }

        index += 1;
    }

    ShString result;
    result.count = index;
    result.data = str->data;

    if (index < str->count)
    {
        str->count -= index + 2;
        str->data += index + 2;
    }
    else
    {
        str->count -= index;
        str->data += index;
    }

    return result;
}

#endif // SH_BASE_IMPLEMENTATION

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
