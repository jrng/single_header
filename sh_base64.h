// sh_base64.h - MIT License
// See end of file for full license

#ifndef __SH_BASE64_INCLUDE__
#define __SH_BASE64_INCLUDE__

#  ifndef __SH_BASE_INCLUDE__
#    error "sh_base64.h requires sh_base.h to be included first"
#  endif

#  if defined(SH_STATIC) || defined(SH_BASE64_STATIC)
#    define SH_BASE64_DEF static
#  else
#    define SH_BASE64_DEF extern
#  endif

SH_BASE64_DEF ShString sh_base64_encode(ShAllocator allocator, usize size, void const *data);

#endif // __SH_BASE64_INCLUDE__

#ifdef SH_BASE64_IMPLEMENTATION

SH_BASE64_DEF ShString
sh_base64_encode(ShAllocator allocator, usize size, void const *data)
{
    const uint8_t chars[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

    ShString result;

    result.count = 4 * ((size + 2) / 3);
    result.data = sh_alloc_array(allocator, uint8_t, result.count);

    result.data[result.count - 2] = '=';
    result.data[result.count - 1] = '=';

    uint8_t *src = (uint8_t *) data;
    uint8_t *dst = result.data;

    while (size >= 3)
    {
        uint8_t a = src[0];
        uint8_t b = src[1];
        uint8_t c = src[2];

        dst[0] = chars[a >> 2];
        dst[1] = chars[((a & 0x3) << 4) | (b >> 4)];
        dst[2] = chars[((b & 0xf) << 2) | (c >> 6)];
        dst[3] = chars[c & 0x3f];

        size -= 3;
        src += 3;
        dst += 4;
    }

    if (size > 0)
    {
        uint8_t a = src[0];
        uint8_t b = 0;

        if (size > 1)
        {
            b = src[1];
            dst[2] = chars[(b & 0xf) << 2];
        }

        dst[1] = chars[((a & 0x3) << 4) | (b >> 4)];
        dst[0] = chars[a >> 2];
    }

    return result;
}

#endif // SH_BASE64_IMPLEMENTATION

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
