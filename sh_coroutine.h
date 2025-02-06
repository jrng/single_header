// sh_coroutine.h - MIT License
// See end of file for full license

#ifndef __SH_COROUTINE_INCLUDE__
#define __SH_COROUTINE_INCLUDE__

#  ifndef __SH_BASE_INCLUDE__
#    error "sh_hash.h requires sh_base.h to be included first"
#  endif

#  include <ucontext.h>

#  if defined(SH_STATIC) || defined(SH_COROUTINE_STATIC)
#    define SH_COROUTINE_DEF static
#  else
#    define SH_COROUTINE_DEF extern
#  endif

typedef struct
{
    ShList list;
    ucontext_t context;
} ShCoroutine;

typedef struct
{
    ShList scheduled;
    ShList active;
    ShList free;

    ucontext_t finish_context;

    ShAllocator allocator;
} ShCoroutineContext;

// Initializes the coroutine system. It registers the calling context as a coroutine,
// so it can use all the coroutine functions. This should be called exactly once.
SH_COROUTINE_DEF void sh_coroutine_init(ShCoroutineContext *ctx, ShAllocator allocator);

// Creates a new coroutine. When the coroutine gets scheduled it will start as if the
// function f was called with the coroutine context and custom data passed in.
// This can be called from every coroutine.
SH_COROUTINE_DEF void sh_coroutine_create(ShCoroutineContext *ctx, void (*f)(ShCoroutineContext *, void *), void *data);

// This will yield the execution of the calling coroutine and schedule
// another coroutine.
SH_COROUTINE_DEF void sh_coroutine_yield(ShCoroutineContext *ctx);

// Exit the calling coroutine and destroys it.
SH_COROUTINE_DEF void sh_coroutine_exit(ShCoroutineContext *ctx);

#endif // __SH_COROUTINE_INCLUDE__

#ifdef SH_COROUTINE_IMPLEMENTATION

static void
_sh_coroutine_schedule(ShCoroutineContext *ctx, ShCoroutine *current_coroutine)
{
    if (ShDListIsEmpty(&ctx->scheduled))
    {
        ShList *first = ctx->active.next;

        ShDListRemove(&ctx->active);
        ShDListInsertBefore(first, &ctx->scheduled);
    }

    assert(!ShDListIsEmpty(&ctx->scheduled));

    ShCoroutine *next_coroutine = ShContainerOf(ctx->scheduled.next, ShCoroutine, list);

    swapcontext(&current_coroutine->context, &next_coroutine->context);
}

SH_COROUTINE_DEF void
sh_coroutine_init(ShCoroutineContext *ctx, ShAllocator allocator)
{
    ShDListInit(&ctx->scheduled);
    ShDListInit(&ctx->active);
    ShDListInit(&ctx->free);

    ctx->allocator = allocator;

    getcontext(&ctx->finish_context);

    ctx->finish_context.uc_stack.ss_size = ShKiB(16);
    ctx->finish_context.uc_stack.ss_sp   = sh_alloc(ctx->allocator, ctx->finish_context.uc_stack.ss_size);
    ctx->finish_context.uc_link          = 0;

    makecontext(&ctx->finish_context, (void (*)(void)) sh_coroutine_exit, 1, ctx);

    ShCoroutine *main_coroutine = sh_alloc_type(ctx->allocator, ShCoroutine);

    getcontext(&main_coroutine->context);

    ShDListInsertBefore(&ctx->scheduled, &main_coroutine->list);
}

SH_COROUTINE_DEF void
sh_coroutine_create(ShCoroutineContext *ctx, void (*f)(ShCoroutineContext *, void *), void *data)
{
    if (ShDListIsEmpty(&ctx->free))
    {
        ShCoroutine *coroutine = sh_alloc_type(ctx->allocator, ShCoroutine);

        getcontext(&coroutine->context);

        coroutine->context.uc_stack.ss_size = ShMiB(1);
        coroutine->context.uc_stack.ss_sp   = sh_alloc(ctx->allocator, coroutine->context.uc_stack.ss_size);
        coroutine->context.uc_link          = &ctx->finish_context;

        ShDListInsertBefore(&ctx->free, &coroutine->list);
    }

    ShCoroutine *coroutine = ShContainerOf(ctx->free.next, ShCoroutine, list);

    ShDListRemove(&coroutine->list);

    makecontext(&coroutine->context, (void (*)(void)) f, 2, ctx, data);

    ShDListInsertBefore(&ctx->active, &coroutine->list);
}

SH_COROUTINE_DEF void
sh_coroutine_yield(ShCoroutineContext *ctx)
{
    assert(!ShDListIsEmpty(&ctx->scheduled));

    ShCoroutine *current_coroutine = ShContainerOf(ctx->scheduled.next, ShCoroutine, list);

    ShDListRemove(&current_coroutine->list);
    ShDListInsertBefore(&ctx->active, &current_coroutine->list);

    _sh_coroutine_schedule(ctx, current_coroutine);
}

SH_COROUTINE_DEF void
sh_coroutine_exit(ShCoroutineContext *ctx)
{
    assert(!ShDListIsEmpty(&ctx->scheduled));

    ShCoroutine *current_coroutine = ShContainerOf(ctx->scheduled.next, ShCoroutine, list);

    ShDListRemove(&current_coroutine->list);
    ShDListInsertBefore(&ctx->free, &current_coroutine->list);

    _sh_coroutine_schedule(ctx, current_coroutine);
}

#endif // SH_COROUTINE_IMPLEMENTATION

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
