// sh_base.h - MIT License
// See end of file for full license

#ifndef __SH_BASE_INCLUDE__
#define __SH_BASE_INCLUDE__

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
#  define ShStringLiteral(str) { sizeof(str) - 1, (uint8_t *) (str) }

typedef enum
{
    SH_ALLOCATOR_ACTION_ALLOC   = 0,
    SH_ALLOCATOR_ACTION_REALLOC = 1,
    SH_ALLOCATOR_ACTION_FREE    = 2,
} ShAllocatorAction;

typedef void *(*ShAllocatorFunc)(void *allocator_data, ShAllocatorAction action, usize size, void *ptr);

typedef struct
{
    void *data;
    ShAllocatorFunc func;
} ShAllocator;

#  define sh_alloc_array(allocator, type, count) (type *) sh_alloc(allocator, (count) * sizeof(type))

SH_BASE_DEF void *sh_alloc(ShAllocator allocator, usize size);
SH_BASE_DEF void *sh_realloc(ShAllocator allocator, void *ptr, usize size);
SH_BASE_DEF void sh_free(ShAllocator allocator, void *ptr);

SH_BASE_DEF bool sh_string_equal(ShString a, ShString b);

#endif // __SH_BASE_INCLUDE__

#ifdef SH_BASE_IMPLEMENTATION

SH_BASE_DEF void *
sh_alloc(ShAllocator allocator, usize size)
{
    return allocator.func(allocator.data, SH_ALLOCATOR_ACTION_ALLOC, size, 0);
}

SH_BASE_DEF void *
sh_realloc(ShAllocator allocator, void *ptr, usize size)
{
    return allocator.func(allocator.data, SH_ALLOCATOR_ACTION_REALLOC, size, ptr);
}

SH_BASE_DEF void
sh_free(ShAllocator allocator, void *ptr)
{
    allocator.func(allocator.data, SH_ALLOCATOR_ACTION_FREE, 0, ptr);
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
