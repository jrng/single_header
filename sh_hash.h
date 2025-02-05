// sh_hash.h - MIT License
// See end of file for full license

#ifndef __SH_HASH_INCLUDE__
#define __SH_HASH_INCLUDE__

#  ifndef __SH_BASE_INCLUDE__
#    error "sh_hash.h requires sh_base.h to be included first"
#  endif

#  if defined(SH_STATIC) || defined(SH_HASH_STATIC)
#    define SH_HASH_DEF static
#  else
#    define SH_HASH_DEF extern
#  endif

typedef struct
{
    uint8_t hash[20];
} ShSha1;

SH_HASH_DEF ShSha1 sh_hash_sha1(usize size, void const *data);
SH_HASH_DEF bool sh_hash_sha1_equal(ShSha1 *a, ShSha1 *b);

#endif // __SH_HASH_INCLUDE__

#ifdef SH_HASH_IMPLEMENTATION

#define sh_hash_sha1_round(_data)                                         \
    w[0]  = ((uint32_t) _data[0]  << 24) | ((uint32_t) _data[1]  << 16) | \
            ((uint32_t) _data[2]  <<  8) | ((uint32_t) _data[3]  <<  0);  \
    w[1]  = ((uint32_t) _data[4]  << 24) | ((uint32_t) _data[5]  << 16) | \
            ((uint32_t) _data[6]  <<  8) | ((uint32_t) _data[7]  <<  0);  \
    w[2]  = ((uint32_t) _data[8]  << 24) | ((uint32_t) _data[9]  << 16) | \
            ((uint32_t) _data[10] <<  8) | ((uint32_t) _data[11] <<  0);  \
    w[3]  = ((uint32_t) _data[12] << 24) | ((uint32_t) _data[13] << 16) | \
            ((uint32_t) _data[14] <<  8) | ((uint32_t) _data[15] <<  0);  \
    w[4]  = ((uint32_t) _data[16] << 24) | ((uint32_t) _data[17] << 16) | \
            ((uint32_t) _data[18] <<  8) | ((uint32_t) _data[19] <<  0);  \
    w[5]  = ((uint32_t) _data[20] << 24) | ((uint32_t) _data[21] << 16) | \
            ((uint32_t) _data[22] <<  8) | ((uint32_t) _data[23] <<  0);  \
    w[6]  = ((uint32_t) _data[24] << 24) | ((uint32_t) _data[25] << 16) | \
            ((uint32_t) _data[26] <<  8) | ((uint32_t) _data[27] <<  0);  \
    w[7]  = ((uint32_t) _data[28] << 24) | ((uint32_t) _data[29] << 16) | \
            ((uint32_t) _data[30] <<  8) | ((uint32_t) _data[31] <<  0);  \
    w[8]  = ((uint32_t) _data[32] << 24) | ((uint32_t) _data[33] << 16) | \
            ((uint32_t) _data[34] <<  8) | ((uint32_t) _data[35] <<  0);  \
    w[9]  = ((uint32_t) _data[36] << 24) | ((uint32_t) _data[37] << 16) | \
            ((uint32_t) _data[38] <<  8) | ((uint32_t) _data[39] <<  0);  \
    w[10] = ((uint32_t) _data[40] << 24) | ((uint32_t) _data[41] << 16) | \
            ((uint32_t) _data[42] <<  8) | ((uint32_t) _data[43] <<  0);  \
    w[11] = ((uint32_t) _data[44] << 24) | ((uint32_t) _data[45] << 16) | \
            ((uint32_t) _data[46] <<  8) | ((uint32_t) _data[47] <<  0);  \
    w[12] = ((uint32_t) _data[48] << 24) | ((uint32_t) _data[49] << 16) | \
            ((uint32_t) _data[50] <<  8) | ((uint32_t) _data[51] <<  0);  \
    w[13] = ((uint32_t) _data[52] << 24) | ((uint32_t) _data[53] << 16) | \
            ((uint32_t) _data[54] <<  8) | ((uint32_t) _data[55] <<  0);  \
    w[14] = ((uint32_t) _data[56] << 24) | ((uint32_t) _data[57] << 16) | \
            ((uint32_t) _data[58] <<  8) | ((uint32_t) _data[59] <<  0);  \
    w[15] = ((uint32_t) _data[60] << 24) | ((uint32_t) _data[61] << 16) | \
            ((uint32_t) _data[62] <<  8) | ((uint32_t) _data[63] <<  0);  \
                                                                          \
    for (int32_t i = 16; i < 80; i += 1)                                  \
    {                                                                     \
        w[i] = (w[i - 3] ^ w[i - 8] ^ w[i - 14] ^ w[i - 16]);             \
        w[i] = (w[i] >> 31) | (w[i] << 1);                                \
    }                                                                     \
                                                                          \
    uint32_t a = h0;                                                      \
    uint32_t b = h1;                                                      \
    uint32_t c = h2;                                                      \
    uint32_t d = h3;                                                      \
    uint32_t e = h4;                                                      \
                                                                          \
    uint32_t k = 0x5a827999;                                              \
                                                                          \
    for (int i = 0; i < 20; i += 1)                                       \
    {                                                                     \
        uint32_t f = (b & c) | (~b & d);                                  \
        uint32_t temp = ((a >> 27) | (a << 5)) + f + e + k + w[i];        \
        e = d;                                                            \
        d = c;                                                            \
        c = ((b >> 2) | (b << 30));                                       \
        b = a;                                                            \
        a = temp;                                                         \
    }                                                                     \
                                                                          \
    k = 0x6ed9eba1;                                                       \
                                                                          \
    for (int i = 20; i < 40; i += 1)                                      \
    {                                                                     \
        uint32_t f = b ^ c ^ d;                                           \
        uint32_t temp = ((a >> 27) | (a << 5)) + f + e + k + w[i];        \
        e = d;                                                            \
        d = c;                                                            \
        c = ((b >> 2) | (b << 30));                                       \
        b = a;                                                            \
        a = temp;                                                         \
    }                                                                     \
                                                                          \
    k = 0x8f1bbcdc;                                                       \
                                                                          \
    for (int i = 40; i < 60; i += 1)                                      \
    {                                                                     \
        uint32_t f = (b & c) | (b & d) | (c & d);                         \
        uint32_t temp = ((a >> 27) | (a << 5)) + f + e + k + w[i];        \
        e = d;                                                            \
        d = c;                                                            \
        c = ((b >> 2) | (b << 30));                                       \
        b = a;                                                            \
        a = temp;                                                         \
    }                                                                     \
                                                                          \
    k = 0xca62c1d6;                                                       \
                                                                          \
    for (int i = 60; i < 80; i += 1)                                      \
    {                                                                     \
        uint32_t f = b ^ c ^ d;                                           \
        uint32_t temp = ((a >> 27) | (a << 5)) + f + e + k + w[i];        \
        e = d;                                                            \
        d = c;                                                            \
        c = ((b >> 2) | (b << 30));                                       \
        b = a;                                                            \
        a = temp;                                                         \
    }                                                                     \
                                                                          \
    h0 += a;                                                              \
    h1 += b;                                                              \
    h2 += c;                                                              \
    h3 += d;                                                              \
    h4 += e;

SH_HASH_DEF ShSha1
sh_hash_sha1(usize size, void const *data)
{
    ShSha1 result;

    uint32_t h0 = 0x67452301;
    uint32_t h1 = 0xefcdab89;
    uint32_t h2 = 0x98badcfe;
    uint32_t h3 = 0x10325476;
    uint32_t h4 = 0xc3d2e1f0;

    uint32_t w[80];

    uint64_t size_in_bits = 8 * size;
    uint8_t *at = (uint8_t *) data;

    while (size >= 64)
    {
        sh_hash_sha1_round(at);
        at += 64;
        size -= 64;
    }

    uint8_t chunk[64];

    usize i = 0;

    for (; i < size; i += 1)
    {
        chunk[i] = at[i];
    }

    chunk[i] = 0x80;
    i += 1;

    if (size >= 56)
    {
        for (; i < 64; i += 1)
        {
            chunk[i] = 0;
        }

        sh_hash_sha1_round(chunk);

        i = 0;
    }

    for (; i < 56; i += 1)
    {
        chunk[i] = 0;
    }

    chunk[56] = (uint8_t) (size_in_bits >> 56);
    chunk[57] = (uint8_t) (size_in_bits >> 48);
    chunk[58] = (uint8_t) (size_in_bits >> 40);
    chunk[59] = (uint8_t) (size_in_bits >> 32);
    chunk[60] = (uint8_t) (size_in_bits >> 24);
    chunk[61] = (uint8_t) (size_in_bits >> 16);
    chunk[62] = (uint8_t) (size_in_bits >>  8);
    chunk[63] = (uint8_t) (size_in_bits >>  0);

    sh_hash_sha1_round(chunk);

    result.hash[0]  = (uint8_t) (h0 >> 24);
    result.hash[1]  = (uint8_t) (h0 >> 16);
    result.hash[2]  = (uint8_t) (h0 >>  8);
    result.hash[3]  = (uint8_t) (h0 >>  0);
    result.hash[4]  = (uint8_t) (h1 >> 24);
    result.hash[5]  = (uint8_t) (h1 >> 16);
    result.hash[6]  = (uint8_t) (h1 >>  8);
    result.hash[7]  = (uint8_t) (h1 >>  0);
    result.hash[8]  = (uint8_t) (h2 >> 24);
    result.hash[9]  = (uint8_t) (h2 >> 16);
    result.hash[10] = (uint8_t) (h2 >>  8);
    result.hash[11] = (uint8_t) (h2 >>  0);
    result.hash[12] = (uint8_t) (h3 >> 24);
    result.hash[13] = (uint8_t) (h3 >> 16);
    result.hash[14] = (uint8_t) (h3 >>  8);
    result.hash[15] = (uint8_t) (h3 >>  0);
    result.hash[16] = (uint8_t) (h4 >> 24);
    result.hash[17] = (uint8_t) (h4 >> 16);
    result.hash[18] = (uint8_t) (h4 >>  8);
    result.hash[19] = (uint8_t) (h4 >>  0);

    return result;
}

#undef sh_hash_sha1_round

SH_HASH_DEF bool
sh_hash_sha1_equal(ShSha1 *a, ShSha1 *b)
{
    for (unsigned int i = 0; i < ShArrayCount(a->hash); i += 1)
    {
        if (a->hash[i] != b->hash[i])
        {
            return false;
        }
    }

    return true;
}

#endif // SH_HASH_IMPLEMENTATION

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
