// sh_draw.h - MIT License
// See end of file for full license

#ifndef __SH_DRAW_INCLUDE__
#define __SH_DRAW_INCLUDE__

#  ifndef __SH_BASE_INCLUDE__
#    error "sh_draw.h requires sh_base.h to be included first"
#  endif

#  if defined(SH_STATIC) || defined(SH_DRAW_STATIC)
#    define SH_DRAW_DEF static
#  else
#    define SH_DRAW_DEF extern
#  endif

SH_DRAW_DEF void sh_image_clear(ShImage image, ShColor clear_color);

#endif // __SH_DRAW_INCLUDE__

#ifdef SH_DRAW_IMPLEMENTATION

SH_DRAW_DEF void
sh_image_clear(ShImage image, ShColor clear_color)
{
    assert(image.format != SH_PIXEL_FORMAT_NONE);

    float r = clear_color.r;
    float g = clear_color.g;
    float b = clear_color.b;
    float a = clear_color.a;

    if (r < 0.0f)  r = 0.0f;
    if (g < 0.0f)  g = 0.0f;
    if (b < 0.0f)  b = 0.0f;
    if (a < 0.0f)  a = 0.0f;

    if (r > 1.0f)  r = 1.0f;
    if (g > 1.0f)  g = 1.0f;
    if (b > 1.0f)  b = 1.0f;
    if (a > 1.0f)  a = 1.0f;

    switch (image.format)
    {
        case SH_PIXEL_FORMAT_NONE: break;

        case SH_PIXEL_FORMAT_GRAY1:
        case SH_PIXEL_FORMAT_GRAY2:
        case SH_PIXEL_FORMAT_GRAY4:
        case SH_PIXEL_FORMAT_GRAY8:
        {
            int32_t x_max;
            uint8_t clear_color_u8;

            float l = (r + g + b) / 3.0f;

            if (image.format == SH_PIXEL_FORMAT_GRAY1)
            {
                x_max = (image.width + 7) / 8;
                clear_color_u8 = 0xFF * (uint8_t) (l * 1.0f + 0.5f);
            }
            else if (image.format == SH_PIXEL_FORMAT_GRAY2)
            {
                x_max = (image.width + 3) / 4;
                clear_color_u8 = 0x55 * (uint8_t) (l * 3.0f + 0.5f);
            }
            else if (image.format == SH_PIXEL_FORMAT_GRAY4)
            {
                x_max = (image.width + 1) / 2;
                clear_color_u8 = 0x11 * (uint8_t) (l * 15.0f + 0.5f);
            }
            else
            {
                assert(image.format == SH_PIXEL_FORMAT_GRAY8);

                x_max = image.width;
                clear_color_u8 = (uint8_t) (l * 255.0f + 0.5f);
            }

            uint8_t *row = (uint8_t *) image.pixels;

            for (int32_t y = 0; y < image.height; y += 1)
            {
                uint8_t *pixel = row;

                for (int32_t x = 0; x < x_max; x += 1)
                {
                    *pixel = clear_color_u8;
                    pixel += 1;
                }

                row += image.stride;
            }
        } break;

        case SH_PIXEL_FORMAT_RGBA8:
        case SH_PIXEL_FORMAT_BGRA8:
        {
            uint32_t clear_color_u32;

            if (image.format == SH_PIXEL_FORMAT_RGBA8)
            {
                ((uint8_t *) &clear_color_u32)[0] = (uint8_t) (r * 255.0f + 0.5f);
                ((uint8_t *) &clear_color_u32)[1] = (uint8_t) (g * 255.0f + 0.5f);
                ((uint8_t *) &clear_color_u32)[2] = (uint8_t) (b * 255.0f + 0.5f);
                ((uint8_t *) &clear_color_u32)[3] = (uint8_t) (a * 255.0f + 0.5f);
            }
            else
            {
                assert(image.format == SH_PIXEL_FORMAT_BGRA8);

                ((uint8_t *) &clear_color_u32)[0] = (uint8_t) (b * 255.0f + 0.5f);
                ((uint8_t *) &clear_color_u32)[1] = (uint8_t) (g * 255.0f + 0.5f);
                ((uint8_t *) &clear_color_u32)[2] = (uint8_t) (r * 255.0f + 0.5f);
                ((uint8_t *) &clear_color_u32)[3] = (uint8_t) (a * 255.0f + 0.5f);
            }

            uint8_t *row = (uint8_t *) image.pixels;

            for (int32_t y = 0; y < image.height; y += 1)
            {
                uint32_t *pixel = (uint32_t *) row;

                for (int32_t x = 0; x < image.width; x += 1)
                {
                    *pixel = clear_color_u32;
                    pixel += 1;
                }

                row += image.stride;
            }
        } break;

        default: assert(!"unimplemented"); break;
    }
}

#endif // SH_DRAW_IMPLEMENTATION

/*
MIT License

Copyright (c) 2026 Julius Range-Lüdemann

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
