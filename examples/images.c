#define SH_BASE_IMPLEMENTATION
#define SH_DRAW_IMPLEMENTATION
#define SH_STRING_BUILDER_IMPLEMENTATION
#define SH_PLATFORM_IMPLEMENTATION
#define SH_IMAGE_BMP_IMPLEMENTATION

#include "sh_base.h"
#include "sh_draw.h"
#include "sh_string_builder.h"
#include "sh_platform.h"
#include "sh_image_bmp.h"

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
draw_image(ShImage image)
{
    sh_image_clear(image, ShMakeColor(1.0f, 1.0f, 1.0f, 1.0f));
    sh_image_stroke_line(image, 15, 20, 24, 16, ShMakeColor(1.0f, 0.0f, 0.0f, 1.0f));
    sh_image_stroke_line(image, 24, 16, 32, 20, ShMakeColor(1.0f, 0.0f, 0.0f, 1.0f));
    sh_image_stroke_line(image, 15, 25, 24, 21, ShMakeColor(0.0f, 1.0f, 0.0f, 1.0f));
    sh_image_stroke_line(image, 24, 21, 32, 25, ShMakeColor(0.0f, 1.0f, 0.0f, 1.0f));
    sh_image_stroke_line(image, 15, 30, 24, 26, ShMakeColor(0.0f, 0.0f, 1.0f, 1.0f));
    sh_image_stroke_line(image, 24, 26, 32, 30, ShMakeColor(0.0f, 0.0f, 1.0f, 1.0f));
}

int main(void)
{
    ShAllocator allocator;
    allocator.data = NULL;
    allocator.func = c_default_allocator_func;

    ShThreadContext *thread_context = sh_thread_context_create(allocator, ShMiB(1));

    ShImage image;

    {
        image = sh_allocate_image(allocator, SH_PIXEL_FORMAT_GRAY1, 48, 48);

        draw_image(image);

        ShStringBuilder builder;
        sh_string_builder_init(&builder, allocator);

        sh_image_encode_bmp(&builder, image);

        sh_write_entire_file(thread_context, ShStringLiteral("GRAY1.bmp"), &builder);

        sh_free(allocator, image.pixels);
    }

    {
        image = sh_allocate_image(allocator, SH_PIXEL_FORMAT_GRAY4, 48, 48);

        draw_image(image);

        ShStringBuilder builder;
        sh_string_builder_init(&builder, allocator);

        sh_image_encode_bmp(&builder, image);

        sh_write_entire_file(thread_context, ShStringLiteral("GRAY4.bmp"), &builder);

        sh_free(allocator, image.pixels);
    }

    {
        image = sh_allocate_image(allocator, SH_PIXEL_FORMAT_GRAY8, 48, 48);

        draw_image(image);

        ShStringBuilder builder;
        sh_string_builder_init(&builder, allocator);

        sh_image_encode_bmp(&builder, image);

        sh_write_entire_file(thread_context, ShStringLiteral("GRAY8.bmp"), &builder);

        sh_free(allocator, image.pixels);
    }

    {
        image = sh_allocate_image(allocator, SH_PIXEL_FORMAT_RGBA8, 48, 48);

        draw_image(image);

        ShStringBuilder builder;
        sh_string_builder_init(&builder, allocator);

        sh_image_encode_bmp(&builder, image);

        sh_write_entire_file(thread_context, ShStringLiteral("RGBA8.bmp"), &builder);

        sh_free(allocator, image.pixels);
    }

    {
        image = sh_allocate_image(allocator, SH_PIXEL_FORMAT_BGRA8, 48, 48);

        draw_image(image);

        ShStringBuilder builder;
        sh_string_builder_init(&builder, allocator);

        sh_image_encode_bmp(&builder, image);

        sh_write_entire_file(thread_context, ShStringLiteral("BGRA8.bmp"), &builder);

        sh_free(allocator, image.pixels);
    }

    sh_thread_context_destroy(thread_context);

    return 0;
}
