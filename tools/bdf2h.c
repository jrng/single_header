#define SH_BASE_IMPLEMENTATION
#include "sh_base.h"
#define SH_STRING_BUILDER_IMPLEMENTATION
#include "sh_string_builder.h"
#define SH_IMAGE_BMP_IMPLEMENTATION
#include "sh_image_bmp.h"
#define SH_PLATFORM_IMPLEMENTATION
#include "sh_platform.h"

#include <stdio.h>

typedef struct
{
    uint32_t width;
    uint32_t height;
    uint32_t stride;
    uint8_t *pixels;

    uint32_t x;
    uint32_t y;
    uint32_t y_max;
} Texture;

typedef struct
{
    uint16_t x, y;
} Point;

typedef struct
{
    uint32_t codepoint;
    uint16_t x_advance;
    int16_t x_offset;
    int16_t y_offset;
    uint16_t bound_width;
    uint16_t bound_height;
    uint16_t x;
    uint16_t y;
} Glyph;

static Point
texture_allocate_glyph(Texture *texture, uint32_t width, uint32_t height)
{
    Point result = { texture->width, texture->height };

    if ((texture->x + width) > texture->width)
    {
        texture->x = 0;
        texture->y = texture->y_max;
    }

    if (((texture->x + width) <= texture->width) && ((texture->y + height) <= texture->height))
    {
        result.x = texture->x;
        result.y = texture->y;

        texture->x += width;

        if ((texture->y + height) > texture->y_max)
        {
            texture->y_max = texture->y + height;
        }
    }

    return result;
}

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
print_help(const char *program_name)
{
    fprintf(stderr, "usage: %s [--help | -h] [--prefix <prefix> | -p <prefix>] [-o <output-header-file>] <input-bdf-file>\n", program_name);
}

int main(int argument_count, char **arguments)
{
    if (argument_count < 2)
    {
        print_help(arguments[0]);
        return 0;
    }

    ShString prefix = ShStringEmpty;
    ShString input_filename = ShStringEmpty;
    ShString output_filename = ShStringEmpty;

    for (int i = 1; i < argument_count; i += 1)
    {
        ShString argument = ShCString(arguments[i]);

        if (sh_string_equal(argument, ShStringLiteral("--help")) ||
            sh_string_equal(argument, ShStringLiteral("-h")))
        {
            print_help(arguments[0]);
            return 0;
        }
        else if (sh_string_equal(argument, ShStringLiteral("-o")))
        {
            if ((i + 1) < argument_count)
            {
                i += 1;
                output_filename = ShCString(arguments[i]);
            }
        }
        else if (sh_string_equal(argument, ShStringLiteral("--prefix")) ||
                 sh_string_equal(argument, ShStringLiteral("-p")))
        {
            if ((i + 1) < argument_count)
            {
                i += 1;
                prefix = ShCString(arguments[i]);
            }
        }
        else
        {
            input_filename = argument;
        }
    }

    if (input_filename.count == 0)
    {
        print_help(arguments[0]);
        return 0;
    }

    ShAllocator allocator;
    allocator.data = NULL;
    allocator.func = c_default_allocator_func;

    ShThreadContext *thread_context = sh_thread_context_create(allocator, ShMiB(1));

    ShString contents = ShStringEmpty;

    if (!sh_read_entire_file(thread_context, allocator, input_filename, &contents))
    {
        fprintf(stderr, "error: could not read file '%" ShStringFmt "'\n", ShStringArg(input_filename));
        return -1;
    }

    uint32_t size = 0;
    int64_t ascent = 0, descent = 0;

    ShString font_family = ShStringEmpty;
    ShString font_weight = ShStringEmpty;

    bool logged_allocation_failure = false;

    Texture texture;
    texture.width = 512;
    texture.height = 512;
    texture.stride = (texture.width + 7) / 8;
    texture.pixels = sh_alloc_array(allocator, uint8_t, texture.stride * texture.height);
    texture.x = 0;
    texture.y = 0;
    texture.y_max = 0;

    uint32_t total = texture.stride * texture.height;

    for (uint32_t i = 0; i < total; i += 1)
    {
        texture.pixels[i] = 0;
    }

    Glyph *glyphs = NULL;
    sh_array_init(glyphs, 128, allocator);

    while (contents.count)
    {
        ShString line = sh_string_split_left_on_char(&contents, '\n');
        ShString keyword = sh_string_trim(sh_string_split_left_on_char(&line, ' '));
        line = sh_string_trim(line);

        // SIZE PointSize Xres Yres
        if (sh_string_equal(keyword, ShStringLiteral("SIZE")))
        {
            if (size == 0)
            {
                int64_t value;
                uint32_t y_res = 72;

                if (sh_parse_integer(&line, &value) && (value >= 0) && (value <= 0xFFFFFFFF))
                {
                    size = (uint32_t) value;
                }

                line = sh_string_trim(line);

                sh_parse_integer(&line, &value);

                line = sh_string_trim(line);

                if (sh_parse_integer(&line, &value) && (value >= 0) && (value <= 0xFFFFFFFF))
                {
                    y_res = (uint32_t) value;
                }

                size = ((size * y_res) + 36) / 72;
            }
        }
        // STARTPROPERTIES n
        else if (sh_string_equal(keyword, ShStringLiteral("STARTPROPERTIES")))
        {
            while (contents.count)
            {
                line = sh_string_split_left_on_char(&contents, '\n');
                keyword = sh_string_trim(sh_string_split_left_on_char(&line, ' '));
                line = sh_string_trim(line);

                // FONT_ASCENT ascent - X Window extension
                if (sh_string_equal(keyword, ShStringLiteral("FONT_ASCENT")))
                {
                    sh_parse_integer(&line, &ascent);
                }
                // FONT_DESCENT descent - X Window extension
                else if (sh_string_equal(keyword, ShStringLiteral("FONT_DESCENT")))
                {
                    sh_parse_integer(&line, &descent);
                }
                // FAMILY_NAME font_family - X Window extension
                else if (sh_string_equal(keyword, ShStringLiteral("FAMILY_NAME")))
                {
                    if (sh_string_starts_with(line, ShStringLiteral("\"")))
                    {
                        line.count -= 1;
                        line.data  += 1;

                        size_t index = 0;

                        while ((index < line.count) && (line.data[index] != '"'))
                        {
                            index += 1;
                        }

                        // TODO: handle spaces in family name
                        font_family.count = index;
                        font_family.data = line.data;

                        font_family = sh_string_ascii_to_lower(allocator, font_family);
                    }
                }
                // WEIGHT_NAME font_weight - X Window extension
                else if (sh_string_equal(keyword, ShStringLiteral("WEIGHT_NAME")))
                {
                    if (sh_string_starts_with(line, ShStringLiteral("\"")))
                    {
                        line.count -= 1;
                        line.data  += 1;

                        size_t index = 0;

                        while ((index < line.count) && (line.data[index] != '"'))
                        {
                            index += 1;
                        }

                        // TODO: handle spaces in weight name
                        font_weight.count = index;
                        font_weight.data = line.data;

                        font_weight = sh_string_ascii_to_lower(allocator, font_weight);
                    }
                }
                // PIXEL_SIZE size
                else if (sh_string_equal(keyword, ShStringLiteral("PIXEL_SIZE")))
                {
                    int64_t value;

                    if (sh_parse_integer(&line, &value) && (value >= 0) && (value <= 0xFFFFFFFF))
                    {
                        size = (uint32_t) value;
                    }
                }
                else if (sh_string_equal(keyword, ShStringLiteral("ENDPROPERTIES")))
                {
                    break;
                }
            }
        }
        // STARTCHAR string
        else if (sh_string_equal(keyword, ShStringLiteral("STARTCHAR")))
        {
            Glyph *glyph = sh_array_append(glyphs);

            glyph->codepoint = 0;
            glyph->x_advance = 0;
            glyph->x_offset = 0;
            glyph->y_offset = 0;
            glyph->bound_width = 0;
            glyph->bound_height = 0;
            glyph->x = 0;
            glyph->y = 0;

            while (contents.count)
            {
                line = sh_string_split_left_on_char(&contents, '\n');
                keyword = sh_string_trim(sh_string_split_left_on_char(&line, ' '));
                line = sh_string_trim(line);

                // ENCODING codepoint
                if (sh_string_equal(keyword, ShStringLiteral("ENCODING")))
                {
                    int64_t encoding;

                    if (sh_parse_integer(&line, &encoding) && (encoding >= 0) && (encoding <= 0xFFFFFFFF))
                    {
                        glyph->codepoint = (uint32_t) encoding;
                    }
                }
                // DWIDTH dwx0 dwy0
                else if (sh_string_equal(keyword, ShStringLiteral("DWIDTH")))
                {
                    int64_t advance;

                    if (sh_parse_integer(&line, &advance) && (advance >= 0) && (advance <= 0xFFFF))
                    {
                        glyph->x_advance = (uint16_t) advance;
                    }
                }
                // BBX BBw BBh BBxoff0x BByoff0y
                else if (sh_string_equal(keyword, ShStringLiteral("BBX")))
                {
                    int64_t bound_width, bound_height, x_offset, y_offset;

                    if (sh_parse_integer(&line, &bound_width) && (bound_width >= 0) && (bound_width <= 0xFFFF))
                    {
                        glyph->bound_width = (uint16_t) bound_width;
                    }

                    line = sh_string_trim(line);

                    if (sh_parse_integer(&line, &bound_height) && (bound_height >= 0) && (bound_height <= 0xFFFF))
                    {
                        glyph->bound_height = (uint16_t) bound_height;
                    }

                    line = sh_string_trim(line);

                    if (sh_parse_integer(&line, &x_offset) && ((uint64_t) x_offset >= 0xFFFFFFFFFFFF8000) && (x_offset <= 0x7FFF))
                    {
                        glyph->x_offset = (int16_t) x_offset;
                    }

                    line = sh_string_trim(line);

                    if (sh_parse_integer(&line, &y_offset) && ((uint64_t) y_offset >= 0xFFFFFFFFFFFF8000) && (y_offset <= 0x7FFF))
                    {
                        glyph->y_offset = (int16_t) y_offset;
                    }
                }
                // BITMAP
                else if (sh_string_equal(keyword, ShStringLiteral("BITMAP")))
                {
                    Point uv = texture_allocate_glyph(&texture, glyph->bound_width, glyph->bound_height);

                    uint8_t *dst_row = texture.pixels + (uv.y * texture.stride);

                    uint16_t row_stride = (glyph->bound_width + 7) / 8;

                    for (uint16_t y = 0; y < glyph->bound_height; y += 1)
                    {
                        line = sh_string_split_left_on_char(&contents, '\n');

                        if ((uv.x < texture.width) && (uv.y < texture.height))
                        {
                            usize index = 0;
                            uint16_t dst_x = 0;
                            uint16_t width = 8;

                            for (uint16_t i = 0; i < row_stride; i += 1)
                            {
                                uint8_t byte = 0;

                                for (int j = 0; j < 2; j += 1)
                                {
                                    if (index < line.count)
                                    {
                                        uint8_t c = line.data[index];

                                        if ((c >= '0') && (c <= '9'))
                                        {
                                            byte = (byte << 4) | (c - '0');
                                        }
                                        else if ((c >= 'A') && (c <= 'F'))
                                        {
                                            byte = (byte << 4) | (c - 'A' + 10);
                                        }

                                        index += 1;
                                    }
                                }

                                if (width > glyph->bound_width)
                                {
                                    width = glyph->bound_width;
                                }

                                for (int j = 0; dst_x < width; dst_x += 1, j += 1)
                                {
                                    uint16_t x = uv.x + dst_x;

                                    if ((0x80 >> j) & byte)
                                    {
                                        dst_row[x / 8] |= 0x80 >> (x % 8);
                                    }
                                    else
                                    {
                                        dst_row[x / 8] &= ~(0x80 >> (x % 8));
                                    }
                                }

                                width += 8;
                            }

                            dst_row += texture.stride;
                        }
                    }

                    if ((uv.x < texture.width) && (uv.y < texture.height))
                    {
                        glyph->x = uv.x;
                        glyph->y = uv.y;
                    }
                    else if (!logged_allocation_failure)
                    {
                        fprintf(stderr, "error: texture with size %u x %u is not big enough to hold all glyphs\n", texture.width, texture.height);
                        logged_allocation_failure = true;
                    }
                }
                else if (sh_string_equal(keyword, ShStringLiteral("ENDCHAR")))
                {
                    break;
                }
            }
        }
        // ENDFONT
        else if (sh_string_equal(keyword, ShStringLiteral("ENDFONT")))
        {
            break;
        }
    }

    if (prefix.count == 0)
    {
        prefix = sh_string_formated(thread_context, allocator, ShStringLiteral("%" ShStringFmt "_%u_%" ShStringFmt "_"), ShStringArg(font_family), size, ShStringArg(font_weight));
    }

    if (output_filename.count == 0)
    {
        output_filename = sh_string_formated(thread_context, allocator, ShStringLiteral("%" ShStringFmt "_%u_%" ShStringFmt ".h"), ShStringArg(font_family), size, ShStringArg(font_weight));
    }

    texture.height = texture.y_max;

    ShStringBuilder sb;
    sh_string_builder_init(&sb, allocator);

    sh_string_builder_append_formated(&sb, ShStringLiteral("uint8_t %" ShStringFmt "texture_data[] = {\n"), ShStringArg(prefix));

    uint8_t *row = texture.pixels;

    for (uint32_t y = 0; y < texture.height; y += 1)
    {
        for (uint32_t i = 0; i < texture.stride; i += 1)
        {
            sh_string_builder_append_formated(&sb, ShStringLiteral("%u,"), row[i]);
        }

        sh_string_builder_append_string(&sb, ShStringLiteral("\n"));

        row += texture.stride;
    }

    sh_string_builder_append_string(&sb, ShStringLiteral("};\n\n"));

    sh_string_builder_append_formated(&sb, ShStringLiteral("ShGlyph %" ShStringFmt "glyphs[] = {"), ShStringArg(prefix));

    for (size_t i = 0; i < sh_array_count(glyphs); i += 1)
    {
        Glyph *glyph = glyphs + i;

        if ((i % 4) == 0)
        {
            sh_string_builder_append_string(&sb, ShStringLiteral("\n   "));
        }

        sh_string_builder_append_formated(&sb, ShStringLiteral(" { %u, %u, %d, %d, %u, %u, %u, %u },"),
                                          glyph->codepoint, glyph->x_advance, glyph->x_offset,
                                          glyph->y_offset, glyph->bound_width, glyph->bound_height,
                                          glyph->x, glyph->y);
    }

    sh_string_builder_append_string(&sb, ShStringLiteral("\n};\n\n"));

    sh_string_builder_append_formated(&sb, ShStringLiteral("ShFont %" ShStringFmt "font = { %u, %d, %d, %u, %u, %u, %" ShStringFmt "glyphs, %" ShStringFmt "texture_data };\n"),
                                      ShStringArg(prefix), (uint16_t) size, (int16_t) ascent, (int16_t) descent,
                                      texture.width, texture.height,  sh_array_count(glyphs), ShStringArg(prefix), ShStringArg(prefix));

    sh_write_entire_file(thread_context, output_filename, &sb);

    sh_thread_context_destroy(thread_context);

    return 0;
}
