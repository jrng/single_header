// sh_json.h - MIT License
// See end of file for full license

#ifndef __SH_JSON_INCLUDE__
#define __SH_JSON_INCLUDE__

#  ifndef __SH_BASE_INCLUDE__
#    error "sh_json.h requires sh_base.h to be included first"
#  endif

#  if defined(SH_STATIC) || defined(SH_JSON_STATIC)
#    define SH_JSON_DEF static
#  else
#    define SH_JSON_DEF extern
#  endif

typedef enum
{
    SH_JSON_ELEMENT_FLAG_HAS_NAME             = (1 << 0),
    SH_JSON_ELEMENT_FLAG_NAME_NEEDS_ESCAPING  = (1 << 1),
    SH_JSON_ELEMENT_FLAG_VALUE_NEEDS_ESCAPING = (1 << 2),
} ShJsonElementFlags;

typedef struct
{
    uint32_t flags;
    ShString name;

    union
    {
        ShString string;
        int64_t number_integer;
        double number_float;
        bool boolean;
    } value;
} ShJsonElement;

typedef struct
{
    uint8_t depth;
    bool comma_after_last_element;
    uint64_t is_object;
    uint64_t is_first;

    usize index;
    ShString input;
} ShJsonParser;

typedef enum
{
    SH_JSON_RESULT_ARRAY_BEGIN    = 0,
    SH_JSON_RESULT_ARRAY_END      = 1,
    SH_JSON_RESULT_OBJECT_BEGIN   = 2,
    SH_JSON_RESULT_OBJECT_END     = 3,
    SH_JSON_RESULT_STRING         = 4,
    SH_JSON_RESULT_NUMBER_INTEGER = 5,
    SH_JSON_RESULT_NUMBER_FLOAT   = 6,
    SH_JSON_RESULT_BOOLEAN        = 7,
    SH_JSON_RESULT_NULL           = 8,
    SH_JSON_RESULT_END_OF_STREAM  = 64,
    SH_JSON_RESULT_ERROR          = 128,
} ShJsonResult;

SH_JSON_DEF ShString sh_json_escape_string(ShAllocator allocator, ShString str);
SH_JSON_DEF void sh_json_parser_init(ShJsonParser *parser, ShString json_str);
SH_JSON_DEF ShJsonResult sh_json_parser_get_next_element(ShJsonParser *parser, ShJsonElement *element);

#endif // __SH_JSON_INCLUDE__

#ifdef SH_JSON_IMPLEMENTATION

SH_JSON_DEF ShString
sh_json_escape_string(ShAllocator allocator, ShString str)
{
    ShString result = ShStringEmpty;

    if (str.count > 0)
    {
        result.data = sh_alloc_array(allocator, uint8_t, str.count);

        bool escaping = false;
        uint8_t *dst = result.data;

        for (usize i = 0; i < str.count; i += 1)
        {
            if (escaping)
            {
                switch (str.data[i])
                {
                    case '"':  *dst++ = '"';  break;
                    case '\\': *dst++ = '\\'; break;
                    case '/':  *dst++ = '/';  break;
                    case 'b':  *dst++ = '\b'; break;
                    case 'f':  *dst++ = '\f'; break;
                    case 'n':  *dst++ = '\n'; break;
                    case 'r':  *dst++ = '\r'; break;
                    case 't':  *dst++ = '\t'; break;
                    case 'u':
                    {
                        if ((i + 4) < str.count)
                        {
                            uint8_t c0 = str.data[i + 1];
                            uint8_t c1 = str.data[i + 2];
                            uint8_t c2 = str.data[i + 3];
                            uint8_t c3 = str.data[i + 4];

                            uint32_t v0 = 0, v1 = 0, v2 = 0, v3 = 0;

                            if ((c0 >= '0') && (c0 <= '9'))
                            {
                                v0 = c0 - '0';
                            }
                            else if (((c0 >= 'a') && (c0 <= 'f')) ||
                                     ((c0 >= 'A') && (c0 <= 'F')))
                            {
                                v0 = 9 + (c0 & 0x7);
                            }

                            if ((c1 >= '0') && (c1 <= '9'))
                            {
                                v1 = c1 - '0';
                            }
                            else if (((c1 >= 'a') && (c1 <= 'f')) ||
                                     ((c1 >= 'A') && (c1 <= 'F')))
                            {
                                v1 = 9 + (c1 & 0x7);
                            }

                            if ((c2 >= '0') && (c2 <= '9'))
                            {
                                v2 = c2 - '0';
                            }
                            else if (((c2 >= 'a') && (c2 <= 'f')) ||
                                     ((c2 >= 'A') && (c2 <= 'F')))
                            {
                                v2 = 9 + (c2 & 0x7);
                            }

                            if ((c3 >= '0') && (c3 <= '9'))
                            {
                                v3 = c3 - '0';
                            }
                            else if (((c3 >= 'a') && (c3 <= 'f')) ||
                                     ((c3 >= 'A') && (c3 <= 'F')))
                            {
                                v3 = 9 + (c3 & 0x7);
                            }

                            uint32_t codepoint = (v0 << 12) | (v1 << 8) | (v2 << 4) | v3;
                            dst += sh_utf8_encode(ShMakeString(4, dst), 0, codepoint);

                            i += 4;
                        }
                    } break;
                }

                escaping = false;
            }
            else
            {
                if (str.data[i] == '\\')
                {
                    escaping = true;
                }
                else
                {
                    *dst++ = str.data[i];
                }
            }
        }

        result.count = dst - result.data;
    }

    return result;
}

SH_JSON_DEF void
sh_json_parser_init(ShJsonParser *parser, ShString json_str)
{
    parser->depth = 0;
    parser->comma_after_last_element = false;
    parser->is_object = 0;
    parser->is_first = 1;
    parser->index = 0;
    parser->input = json_str;
}

static inline bool
_sh_json_matches(ShJsonParser *parser, uint8_t c)
{
    if ((parser->index < parser->input.count) &&
        (parser->input.data[parser->index] == c))
    {
        parser->index += 1;
        return true;
    }

    return false;
}

static inline void
_sh_json_skip_whitespaces(ShJsonParser *parser)
{
    while (parser->index < parser->input.count)
    {
        uint8_t c = parser->input.data[parser->index];

        if ((c != ' ') && (c != '\n') && (c != '\r') && (c != '\t'))
        {
            break;
        }

        parser->index += 1;
    }
}

static inline void
_sh_json_handle_comma(ShJsonParser *parser)
{
    if (parser->depth > 0)
    {
        _sh_json_skip_whitespaces(parser);
        parser->comma_after_last_element = _sh_json_matches(parser, ',');
    }
    else
    {
        parser->comma_after_last_element = false;
    }
}

SH_JSON_DEF ShJsonResult
sh_json_parser_get_next_element(ShJsonParser *parser, ShJsonElement *element)
{
    element->flags = 0;
    element->name = ShStringEmpty;
    element->value.string = ShStringEmpty;

    _sh_json_skip_whitespaces(parser);

    if (parser->index >= parser->input.count)
    {
        if ((parser->depth > 0) || (parser->is_first & 1))
        {
            return SH_JSON_RESULT_ERROR;
        }
        else
        {
            return SH_JSON_RESULT_END_OF_STREAM;
        }
    }

    if (_sh_json_matches(parser, ']'))
    {
        if ((parser->depth == 0) || (parser->is_object & (1 << (parser->depth - 1))))
        {
            return SH_JSON_RESULT_ERROR;
        }

        if (parser->comma_after_last_element)
        {
            return SH_JSON_RESULT_ERROR;
        }

        parser->depth -= 1;

        _sh_json_handle_comma(parser);

        return SH_JSON_RESULT_ARRAY_END;
    }
    else if (_sh_json_matches(parser, '}'))
    {
        if ((parser->depth == 0) || !(parser->is_object & (1 << (parser->depth - 1))))
        {
            return SH_JSON_RESULT_ERROR;
        }

        if (parser->comma_after_last_element)
        {
            return SH_JSON_RESULT_ERROR;
        }

        parser->depth -= 1;

        _sh_json_handle_comma(parser);

        return SH_JSON_RESULT_OBJECT_END;
    }

    if (parser->is_first & (1 << parser->depth))
    {
        parser->is_first &= ~(1 << parser->depth);
    }
    else if (!parser->comma_after_last_element)
    {
        return SH_JSON_RESULT_ERROR;
    }

    if ((parser->depth > 0) && (parser->is_object & (1 << (parser->depth - 1))))
    {
        if (!_sh_json_matches(parser, '"'))
        {
            return SH_JSON_RESULT_ERROR;
        }

        element->name.data = parser->input.data + parser->index;
        element->flags |= SH_JSON_ELEMENT_FLAG_HAS_NAME;

        bool escaping = false;

        while (parser->index < parser->input.count)
        {
            uint8_t c = parser->input.data[parser->index];

            if (escaping)
            {
                if ((c == '"') || (c == '\\') || (c == '/') || (c == 'b') ||
                    (c == 'f') || (c == 'n') || (c == 'r') || (c == 't'))
                {
                }
                else if (c == 'u')
                {
                    if (((parser->index + 5) > parser->input.count) ||
                        !sh_unicode_is_hexdigit(parser->input.data[parser->index + 1]) ||
                        !sh_unicode_is_hexdigit(parser->input.data[parser->index + 2]) ||
                        !sh_unicode_is_hexdigit(parser->input.data[parser->index + 3]) ||
                        !sh_unicode_is_hexdigit(parser->input.data[parser->index + 4]))
                    {
                        return SH_JSON_RESULT_ERROR;
                    }

                    parser->index += 4;
                }
                else
                {
                    return SH_JSON_RESULT_ERROR;
                }

                escaping = false;
            }
            else
            {
                if (c == '\\')
                {
                    escaping = true;
                    element->flags |= SH_JSON_ELEMENT_FLAG_NAME_NEEDS_ESCAPING;
                }
                else if (c == '"')
                {
                    element->name.count = (parser->input.data + parser->index) - element->name.data;
                    break;
                }
            }

            parser->index += 1;
        }

        if (!_sh_json_matches(parser, '"'))
        {
            return SH_JSON_RESULT_ERROR;
        }

        _sh_json_skip_whitespaces(parser);

        if (!_sh_json_matches(parser, ':'))
        {
            return SH_JSON_RESULT_ERROR;
        }

        _sh_json_skip_whitespaces(parser);
    }

    if (parser->index < parser->input.count)
    {
        uint8_t c = parser->input.data[parser->index];

        switch (c)
        {
            case '[':
            {
                if (parser->depth >= 64)
                {
                    return SH_JSON_RESULT_ERROR;
                }

                parser->is_object &= ~(1 << parser->depth);
                parser->depth += 1;
                parser->index += 1;
                parser->is_first |= (1 << parser->depth);
                parser->comma_after_last_element = false;

                return SH_JSON_RESULT_ARRAY_BEGIN;
            } break;

            case '{':
            {
                if (parser->depth >= 64)
                {
                    return SH_JSON_RESULT_ERROR;
                }

                parser->is_object |= (1 << parser->depth);
                parser->depth += 1;
                parser->index += 1;
                parser->is_first |= (1 << parser->depth);
                parser->comma_after_last_element = false;

                return SH_JSON_RESULT_OBJECT_BEGIN;
            } break;

            case '"':
            {
                parser->index += 1;

                element->value.string.data = parser->input.data + parser->index;

                bool escaping = false;

                while (parser->index < parser->input.count)
                {
                    c = parser->input.data[parser->index];

                    if (escaping)
                    {
                        if ((c == '"') || (c == '\\') || (c == '/') || (c == 'b') ||
                            (c == 'f') || (c == 'n') || (c == 'r') || (c == 't'))
                        {
                        }
                        else if (c == 'u')
                        {
                            if (((parser->index + 5) > parser->input.count) ||
                                !sh_unicode_is_hexdigit(parser->input.data[parser->index + 1]) ||
                                !sh_unicode_is_hexdigit(parser->input.data[parser->index + 2]) ||
                                !sh_unicode_is_hexdigit(parser->input.data[parser->index + 3]) ||
                                !sh_unicode_is_hexdigit(parser->input.data[parser->index + 4]))
                            {
                                return SH_JSON_RESULT_ERROR;
                            }

                            parser->index += 4;
                        }
                        else
                        {
                            return SH_JSON_RESULT_ERROR;
                        }

                        escaping = false;
                    }
                    else
                    {
                        if (c == '\\')
                        {
                            escaping = true;
                            element->flags |= SH_JSON_ELEMENT_FLAG_VALUE_NEEDS_ESCAPING;
                        }
                        else if (c == '"')
                        {
                            element->value.string.count = (parser->input.data + parser->index) - element->value.string.data;
                            parser->index += 1;

                            _sh_json_handle_comma(parser);

                            return SH_JSON_RESULT_STRING;
                        }
                    }

                    parser->index += 1;
                }
            } break;

            case 't':
            {
                ShString str = ShMakeString(parser->input.count - parser->index, parser->input.data + parser->index);

                if (sh_string_starts_with(str, ShStringLiteral("true")))
                {
                    parser->index += ShStringLiteral("true").count;
                    element->value.boolean = true;

                    _sh_json_handle_comma(parser);

                    return SH_JSON_RESULT_BOOLEAN;
                }
            } break;

            case 'f':
            {
                ShString str = ShMakeString(parser->input.count - parser->index, parser->input.data + parser->index);

                if (sh_string_starts_with(str, ShStringLiteral("false")))
                {
                    parser->index += ShStringLiteral("false").count;
                    element->value.boolean = false;

                    _sh_json_handle_comma(parser);

                    return SH_JSON_RESULT_BOOLEAN;
                }
            } break;

            case 'n':
            {
                ShString str = ShMakeString(parser->input.count - parser->index, parser->input.data + parser->index);

                if (sh_string_starts_with(str, ShStringLiteral("null")))
                {
                    parser->index += ShStringLiteral("null").count;

                    _sh_json_handle_comma(parser);

                    return SH_JSON_RESULT_NULL;
                }
            } break;

            default:
            {
                bool is_signed = false;

                if (c == '-')
                {
                    is_signed = true;
                    parser->index += 1;
                }

                if ((parser->index >= parser->input.count) ||
                    !sh_unicode_is_digit(parser->input.data[parser->index]))
                {
                    return SH_JSON_RESULT_ERROR;
                }

                c = parser->input.data[parser->index];
                parser->index += 1;

                bool is_float = false;
                int64_t integer_value = c - '0';

                if (c != '0')
                {
                    while ((parser->index < parser->input.count) && sh_unicode_is_digit(parser->input.data[parser->index]))
                    {
                        c = parser->input.data[parser->index];
                        parser->index += 1;
                        integer_value = (10 * integer_value) + (c - '0');
                    }
                }

                if (is_signed)
                {
                    integer_value = -integer_value;
                }

                double float_value = (double) integer_value;

                if (_sh_json_matches(parser, '.'))
                {
                    is_float = true;

                    if ((parser->index >= parser->input.count) ||
                        !sh_unicode_is_digit(parser->input.data[parser->index]))
                    {
                        return SH_JSON_RESULT_ERROR;
                    }

                    double factor = 1.0 / 10.0;

                    while ((parser->index < parser->input.count) && sh_unicode_is_digit(parser->input.data[parser->index]))
                    {
                        c = parser->input.data[parser->index];
                        parser->index += 1;
                        float_value += factor * (double) (c - '0');
                        factor /= 10.0;
                    }
                }

                if (_sh_json_matches(parser, 'e') || _sh_json_matches(parser, 'E'))
                {
                    is_float = true;
                    bool exponent_is_signed = false;

                    if (_sh_json_matches(parser, '-'))
                    {
                        exponent_is_signed = true;
                    }
                    else if (_sh_json_matches(parser, '+'))
                    {
                    }

                    if ((parser->index >= parser->input.count) ||
                        !sh_unicode_is_digit(parser->input.data[parser->index]))
                    {
                        return SH_JSON_RESULT_ERROR;
                    }

                    uint64_t exponent = 0;

                    while ((parser->index < parser->input.count) && sh_unicode_is_digit(parser->input.data[parser->index]))
                    {
                        c = parser->input.data[parser->index];
                        parser->index += 1;
                        exponent = (10 * exponent) + (c - '0');
                    }

                    if (exponent_is_signed)
                    {
                        for (uint64_t i = 0; i < exponent; i += 1)
                        {
                            float_value /= 10.0;
                        }
                    }
                    else
                    {
                        for (uint64_t i = 0; i < exponent; i += 1)
                        {
                            float_value *= 10.0;
                        }
                    }
                }

                _sh_json_handle_comma(parser);

                if (is_float)
                {
                    element->value.number_float = float_value;
                    return SH_JSON_RESULT_NUMBER_FLOAT;
                }
                else
                {
                    element->value.number_integer = integer_value;
                    return SH_JSON_RESULT_NUMBER_INTEGER;
                }
            } break;
        }
    }

    return SH_JSON_RESULT_ERROR;
}

#endif // SH_JSON_IMPLEMENTATION

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
