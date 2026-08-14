// sh_image_bmp.h - MIT License
// See end of file for full license

#ifndef __SH_IMAGE_BMP_INCLUDE__
#define __SH_IMAGE_BMP_INCLUDE__

#  ifndef __SH_STRING_BUILDER_INCLUDE__
#    error "sh_image_bmp.h requires sh_string_builder.h to be included first"
#  endif

#  if defined(SH_STATIC) || defined(SH_IMAGE_BMP_STATIC)
#    define SH_IMAGE_BMP_DEF static
#  else
#    define SH_IMAGE_BMP_DEF extern
#  endif

SH_IMAGE_BMP_DEF void sh_image_encode_bmp(ShStringBuilder *string_builder, ShImage image);

#endif // __SH_IMAGE_BMP_INCLUDE__

#ifdef SH_IMAGE_BMP_IMPLEMENTATION

SH_IMAGE_BMP_DEF void
sh_image_encode_bmp(ShStringBuilder *string_builder, ShImage image)
{
    sh_string_builder_append_string(string_builder, ShStringLiteral("BM"));
    sh_string_builder_append_u32le(string_builder, 0); // file size (can be 0 for uncompressed)
    sh_string_builder_append_u16le(string_builder, 0); // reserved
    sh_string_builder_append_u16le(string_builder, 0); // reserved

    switch (image.format)
    {
        case SH_PIXEL_FORMAT_NONE: break;

        case SH_PIXEL_FORMAT_GRAY1:
        case SH_PIXEL_FORMAT_GRAY2: // NOTE: this is offically not supported
        case SH_PIXEL_FORMAT_GRAY4:
        case SH_PIXEL_FORMAT_GRAY8:
        {
            int32_t bits_per_pixel = sh_get_bits_per_pixel(image.format);
            int32_t stride = (((bits_per_pixel * image.width) + 31) / 32) * 4;
            int32_t number_of_colors = 1 << bits_per_pixel;

            sh_string_builder_append_u32le(string_builder, 14 + 40 + (number_of_colors * 4)); // pixel data offset
            sh_string_builder_append_u32le(string_builder, 40);                    // bitmap header size
            sh_string_builder_append_u32le(string_builder, image.width);           // image width
            sh_string_builder_append_u32le(string_builder, -image.height);         // image height
            sh_string_builder_append_u16le(string_builder, 1);                     // number of color planes
            sh_string_builder_append_u16le(string_builder, bits_per_pixel);        // bits per pixel
            sh_string_builder_append_u32le(string_builder, 0);                     // compression
            sh_string_builder_append_u32le(string_builder, stride * image.height); // image size
            sh_string_builder_append_u32le(string_builder, 0);                     // horizontal resolution
            sh_string_builder_append_u32le(string_builder, 0);                     // vertical resolution
            sh_string_builder_append_u32le(string_builder, number_of_colors);      // number of colors in the palette
            sh_string_builder_append_u32le(string_builder, 0);                     // number of important colors

            for (int32_t i = 0; i < number_of_colors; i += 1)
            {
                float f = (float) i / (float) (number_of_colors - 1);
                uint8_t value = (uint8_t) (f * 255.0f + 0.5f);
                sh_string_builder_append_u8(string_builder, value);
                sh_string_builder_append_u8(string_builder, value);
                sh_string_builder_append_u8(string_builder, value);
                sh_string_builder_append_u8(string_builder, 0);
            }

            int32_t x_max;

            if (image.format == SH_PIXEL_FORMAT_GRAY1)
            {
                x_max = (image.width + 7) / 8;
            }
            else if (image.format == SH_PIXEL_FORMAT_GRAY2)
            {
                x_max = (image.width + 3) / 4;
            }
            else if (image.format == SH_PIXEL_FORMAT_GRAY4)
            {
                x_max = (image.width + 1) / 2;
            }
            else
            {
                assert(image.format == SH_PIXEL_FORMAT_GRAY8);
                x_max = image.width;
            }

            uint8_t *row = (uint8_t *) image.pixels;

            for (int32_t y = 0; y < image.height; y += 1)
            {
                for (int32_t x = 0; x < x_max; x += 1)
                {
                    sh_string_builder_append_u8(string_builder, row[x]);
                }

                for (int32_t x = x_max; x < stride; x += 1)
                {
                    sh_string_builder_append_u8(string_builder, 0);
                }

                row += image.stride;
            }
        } break;

        case SH_PIXEL_FORMAT_RGBA8:
        case SH_PIXEL_FORMAT_BGRA8:
        {
            sh_string_builder_append_u32le(string_builder, 14 + 108);                       // pixel data offset
            sh_string_builder_append_u32le(string_builder, 108);                            // bitmap header size
            sh_string_builder_append_u32le(string_builder, image.width);                    // image width
            sh_string_builder_append_u32le(string_builder, -image.height);                  // image height
            sh_string_builder_append_u16le(string_builder, 1);                              // number of color planes
            sh_string_builder_append_u16le(string_builder, 32);                             // bits per pixel
            sh_string_builder_append_u32le(string_builder, 3);                              // compression
            sh_string_builder_append_u32le(string_builder, image.width * image.height * 4); // image size
            sh_string_builder_append_u32le(string_builder, 0);                              // horizontal resolution
            sh_string_builder_append_u32le(string_builder, 0);                              // vertical resolution
            sh_string_builder_append_u32le(string_builder, 0);                              // number of colors in the palette
            sh_string_builder_append_u32le(string_builder, 0);                              // number of important colors

            if (image.format == SH_PIXEL_FORMAT_RGBA8)
            {
                sh_string_builder_append_u32le(string_builder, 0x000000FF); // red mask
                sh_string_builder_append_u32le(string_builder, 0x0000FF00); // green mask
                sh_string_builder_append_u32le(string_builder, 0x00FF0000); // blue mask
            }
            else
            {
                sh_string_builder_append_u32le(string_builder, 0x00FF0000); // red mask
                sh_string_builder_append_u32le(string_builder, 0x0000FF00); // green mask
                sh_string_builder_append_u32le(string_builder, 0x000000FF); // blue mask
            }

            sh_string_builder_append_u32le(string_builder, 0xFF000000); // alpha mask

            sh_string_builder_append_u32le(string_builder, 1); // color space type
            sh_string_builder_append_u32le(string_builder, 0); // red point x
            sh_string_builder_append_u32le(string_builder, 0); // red point y
            sh_string_builder_append_u32le(string_builder, 0); // red point z
            sh_string_builder_append_u32le(string_builder, 0); // green point x
            sh_string_builder_append_u32le(string_builder, 0); // green point y
            sh_string_builder_append_u32le(string_builder, 0); // green point z
            sh_string_builder_append_u32le(string_builder, 0); // blue point x
            sh_string_builder_append_u32le(string_builder, 0); // blue point y
            sh_string_builder_append_u32le(string_builder, 0); // blue point z
            sh_string_builder_append_u32le(string_builder, 0); // gamma red scale
            sh_string_builder_append_u32le(string_builder, 0); // gamma green scale
            sh_string_builder_append_u32le(string_builder, 0); // gamma blue scale

            uint8_t *row = (uint8_t *) image.pixels;

            for (int32_t y = 0; y < image.height; y += 1)
            {
                uint8_t *pixel = row;

                for (int32_t x = 0; x < image.width; x += 1)
                {
                    sh_string_builder_append_u8(string_builder, pixel[0]);
                    sh_string_builder_append_u8(string_builder, pixel[1]);
                    sh_string_builder_append_u8(string_builder, pixel[2]);
                    sh_string_builder_append_u8(string_builder, pixel[3]);
                    pixel += 4;
                }

                row += image.stride;
            }
        } break;

        default: assert(!"unimplemented"); break;
    }
}

#endif // SH_IMAGE_BMP_IMPLEMENTATION

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
