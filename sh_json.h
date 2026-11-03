// sh_json.h - MIT License
// See end of file for full license

#ifndef __SH_JSON_INCLUDE__
#define __SH_JSON_INCLUDE__

#  ifndef __SH_BASE_INCLUDE__
#    error "sh_json.h requires sh_base.h to be included first"
#  endif

#  include <stdio.h> // TODO: remove

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

typedef enum
{
    SH_JSON_FLAG_ALLOW_SINGLE_LINE_COMMENTS = (1 << 0),
    SH_JSON_FLAG_ALLOW_MULTI_LINE_COMMENTS  = (1 << 1),
    SH_JSON_FLAG_ALLOW_TRAILING_COMMAS      = (1 << 2),

    // If parsing fails don't free all nodes that were already created.
    // This is useful if the memory allocator passed is an arena,
    // which can just be resetted.
    SH_JSON_FLAG_DONT_FREE_NODES_ON_FAILURE = (1 << 3),

    SH_JSON_FLAG_SPEC_STANDARD              = 0,
    SH_JSON_FLAG_SPEC_JSONC                 = SH_JSON_FLAG_ALLOW_SINGLE_LINE_COMMENTS |
                                              SH_JSON_FLAG_ALLOW_MULTI_LINE_COMMENTS |
                                              SH_JSON_FLAG_ALLOW_TRAILING_COMMAS,
} ShJsonFlags;

typedef enum
{
    SH_JSON_VALUE_TYPE_ARRAY          = 0,
    SH_JSON_VALUE_TYPE_OBJECT         = 1,
    SH_JSON_VALUE_TYPE_STRING         = 2,
    SH_JSON_VALUE_TYPE_NUMBER_INTEGER = 3,
    SH_JSON_VALUE_TYPE_NUMBER_FLOAT   = 4,
    SH_JSON_VALUE_TYPE_BOOLEAN        = 5,
    SH_JSON_VALUE_TYPE_NULL           = 6,
} ShJsonValueType;

typedef enum
{
    SH_JSON_NODE_FLAG_NAME_NEEDS_ESCAPING  = (1 << 0),
    SH_JSON_NODE_FLAG_VALUE_NEEDS_ESCAPING = (1 << 1),
} ShJsonNodeFlags;

typedef struct ShJsonNode ShJsonNode;

struct ShJsonNode
{
    ShJsonValueType type;
    uint32_t flags;
    ShString name;
    uint32_t count;

    ShList list;

    union
    {
        ShString string;
        int64_t number_integer;
        double number_float;
        bool boolean;
        ShList children;
    } value;
};

typedef struct
{
    uint8_t depth;
    bool comma_after_last_element;
    uint64_t is_object;
    uint64_t is_first;

    uint32_t flags;
    uint32_t line;
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
SH_JSON_DEF ShJsonNode *sh_json_parse(ShAllocator allocator, ShString json_str, uint32_t flags);
SH_JSON_DEF void sh_json_free_node(ShAllocator allocator, ShJsonNode *node);

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
    parser->line = 1;
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

        if (c == '\n')  parser->line += 1;

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

static bool
_sh_json_parse_string(ShJsonParser *parser, ShString *str, bool *needs_escaping)
{
    if (!_sh_json_matches(parser, '"'))
    {
        fprintf(stderr, "line %u: error: expected string.\n", parser->line);
        return false;
    }

    str->count = 0;
    str->data  = parser->input.data + parser->index;

    bool escaping = false;
    *needs_escaping = false;

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
                    fprintf(stderr, "line %u: error: expected \\u to be followed by a 4-digit hex number.\n", parser->line);
                    return false;
                }

                parser->index += 4;
            }
            else
            {
                fprintf(stderr, "line %u: error: unknown escape character '%c'.\n", parser->line, c);
                return false;
            }

            escaping = false;
        }
        else
        {
            if (c == '\\')
            {
                escaping = true;
                *needs_escaping = true;
            }
            else if (c == '"')
            {
                str->count = (parser->input.data + parser->index) - str->data;
                break;
            }
        }

        parser->index += 1;
    }

    if (!_sh_json_matches(parser, '"'))
    {
        fprintf(stderr, "line %u: error: string is missing closing '\"'.\n", parser->line);
        return false;
    }

    return true;
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
                    if (is_signed)
                    {
                        float_value = -float_value;
                    }

                    element->value.number_float = float_value;
                    return SH_JSON_RESULT_NUMBER_FLOAT;
                }
                else
                {
                    if (is_signed)
                    {
                        integer_value = -integer_value;
                    }

                    element->value.number_integer = integer_value;
                    return SH_JSON_RESULT_NUMBER_INTEGER;
                }
            } break;
        }
    }

    return SH_JSON_RESULT_ERROR;
}

static inline ShJsonNode *
_sh_json_alloc_node(ShAllocator allocator, ShJsonValueType type)
{
    ShJsonNode *node = sh_alloc_type(allocator, ShJsonNode);

    node->type         = type;
    node->flags        = 0;
    node->name         = ShStringEmpty;
    node->count        = 0;
    node->list.prev    = NULL;
    node->list.next    = NULL;
    node->value.string = ShStringEmpty;

    return node;
}

SH_JSON_DEF void
sh_json_free_node(ShAllocator allocator, ShJsonNode *node)
{
    if ((node->type == SH_JSON_VALUE_TYPE_ARRAY) || (node->type == SH_JSON_VALUE_TYPE_OBJECT))
    {
        ShDListForEach(ShList, child, &node->value.children)
        {
            sh_json_free_node(allocator, ShContainerOf(child, ShJsonNode, list));
        }
    }

    sh_free(allocator, node);
}

static ShJsonNode *
_sh_json_parse_value(ShAllocator allocator, ShJsonParser *parser)
{
    _sh_json_skip_whitespaces(parser);

    if (parser->index >= parser->input.count)
    {
        fprintf(stderr, "line %u: error: expected value.\n", parser->line);
        return NULL;
    }

    ShJsonNode *node = NULL;

    uint8_t c = parser->input.data[parser->index];

    switch (c)
    {
        case '[':
        {
            parser->index += 1;

            _sh_json_skip_whitespaces(parser);

            node = _sh_json_alloc_node(allocator, SH_JSON_VALUE_TYPE_ARRAY);
            ShDListInit(&node->value.children);

            if (!_sh_json_matches(parser, ']'))
            {
                ShJsonNode *child = _sh_json_parse_value(allocator, parser);

                if (!child)
                {
                    if (!(parser->flags & SH_JSON_FLAG_DONT_FREE_NODES_ON_FAILURE))
                    {
                        sh_json_free_node(allocator, node);
                    }

                    return NULL;
                }

                node->count += 1;
                ShDListInsertBefore(&node->value.children, &child->list);

                while (_sh_json_matches(parser, ','))
                {
                    child = _sh_json_parse_value(allocator, parser);

                    if (!child)
                    {
                        if (!(parser->flags & SH_JSON_FLAG_DONT_FREE_NODES_ON_FAILURE))
                        {
                            sh_json_free_node(allocator, node);
                        }

                        return NULL;
                    }

                    node->count += 1;
                    ShDListInsertBefore(&node->value.children, &child->list);
                }

                if (!_sh_json_matches(parser, ']'))
                {
                    fprintf(stderr, "line %u: error: array is missing closing ']'.\n", parser->line);

                    if (!(parser->flags & SH_JSON_FLAG_DONT_FREE_NODES_ON_FAILURE))
                    {
                        sh_json_free_node(allocator, node);
                    }

                    return NULL;
                }
            }
        } break;

        case '{':
        {
            parser->index += 1;

            _sh_json_skip_whitespaces(parser);

            node = _sh_json_alloc_node(allocator, SH_JSON_VALUE_TYPE_OBJECT);
            ShDListInit(&node->value.children);

            if (!_sh_json_matches(parser, '}'))
            {
                _sh_json_skip_whitespaces(parser);

                ShString name;
                bool name_needs_escaping;

                if (!_sh_json_parse_string(parser, &name, &name_needs_escaping))
                {
                    if (!(parser->flags & SH_JSON_FLAG_DONT_FREE_NODES_ON_FAILURE))
                    {
                        sh_json_free_node(allocator, node);
                    }

                    return NULL;
                }

                _sh_json_skip_whitespaces(parser);

                if (!_sh_json_matches(parser, ':'))
                {
                    fprintf(stderr, "line %u: error: expected ':' after object member name.\n", parser->line);

                    if (!(parser->flags & SH_JSON_FLAG_DONT_FREE_NODES_ON_FAILURE))
                    {
                        sh_json_free_node(allocator, node);
                    }

                    return NULL;
                }

                ShJsonNode *child = _sh_json_parse_value(allocator, parser);

                if (!child)
                {
                    if (!(parser->flags & SH_JSON_FLAG_DONT_FREE_NODES_ON_FAILURE))
                    {
                        sh_json_free_node(allocator, node);
                    }

                    return NULL;
                }

                child->name = name;

                if (name_needs_escaping)
                {
                    child->flags |= SH_JSON_NODE_FLAG_NAME_NEEDS_ESCAPING;
                }

                node->count += 1;
                ShDListInsertBefore(&node->value.children, &child->list);

                while (_sh_json_matches(parser, ','))
                {
                    _sh_json_skip_whitespaces(parser);

                    if (!_sh_json_parse_string(parser, &name, &name_needs_escaping))
                    {
                        if (!(parser->flags & SH_JSON_FLAG_DONT_FREE_NODES_ON_FAILURE))
                        {
                            sh_json_free_node(allocator, node);
                        }

                        return NULL;
                    }

                    _sh_json_skip_whitespaces(parser);

                    if (!_sh_json_matches(parser, ':'))
                    {
                        fprintf(stderr, "line %u: error: expected ':' after object member name.\n", parser->line);

                        if (!(parser->flags & SH_JSON_FLAG_DONT_FREE_NODES_ON_FAILURE))
                        {
                            sh_json_free_node(allocator, node);
                        }

                        return NULL;
                    }

                    child = _sh_json_parse_value(allocator, parser);

                    if (!child)
                    {
                        if (!(parser->flags & SH_JSON_FLAG_DONT_FREE_NODES_ON_FAILURE))
                        {
                            sh_json_free_node(allocator, node);
                        }

                        return NULL;
                    }

                    child->name = name;

                    if (name_needs_escaping)
                    {
                        child->flags |= SH_JSON_NODE_FLAG_NAME_NEEDS_ESCAPING;
                    }

                    node->count += 1;
                    ShDListInsertBefore(&node->value.children, &child->list);
                }

                if (!_sh_json_matches(parser, '}'))
                {
                    fprintf(stderr, "line %u: error: object is missing closing '}'.\n", parser->line);

                    if (!(parser->flags & SH_JSON_FLAG_DONT_FREE_NODES_ON_FAILURE))
                    {
                        sh_json_free_node(allocator, node);
                    }

                    return NULL;
                }
            }
        } break;

        case '"':
        {
            ShString str;
            bool needs_escaping;

            if (_sh_json_parse_string(parser, &str, &needs_escaping))
            {
                node = _sh_json_alloc_node(allocator, SH_JSON_VALUE_TYPE_STRING);

                if (needs_escaping)
                {
                    node->flags |= SH_JSON_NODE_FLAG_VALUE_NEEDS_ESCAPING;
                }

                node->value.string = str;
            }
        } break;

        case 't':
        {
            ShString str = ShMakeString(parser->input.count - parser->index, parser->input.data + parser->index);

            if (sh_string_starts_with(str, ShStringLiteral("true")))
            {
                parser->index += ShStringLiteral("true").count;
                node = _sh_json_alloc_node(allocator, SH_JSON_VALUE_TYPE_BOOLEAN);
                node->value.boolean = true;
            }
            else
            {
                fprintf(stderr, "line %u: error: expected value.\n", parser->line);
            }
        } break;

        case 'f':
        {
            ShString str = ShMakeString(parser->input.count - parser->index, parser->input.data + parser->index);

            if (sh_string_starts_with(str, ShStringLiteral("false")))
            {
                parser->index += ShStringLiteral("false").count;
                node = _sh_json_alloc_node(allocator, SH_JSON_VALUE_TYPE_BOOLEAN);
                node->value.boolean = false;
            }
            else
            {
                fprintf(stderr, "line %u: error: expected value.\n", parser->line);
            }
        } break;

        case 'n':
        {
            ShString str = ShMakeString(parser->input.count - parser->index, parser->input.data + parser->index);

            if (sh_string_starts_with(str, ShStringLiteral("null")))
            {
                parser->index += ShStringLiteral("null").count;
                node = _sh_json_alloc_node(allocator, SH_JSON_VALUE_TYPE_NULL);
            }
            else
            {
                fprintf(stderr, "line %u: error: expected value.\n", parser->line);
            }
        } break;

        case '-':
        case '0': case '1': case '2': case '3': case '4':
        case '5': case '6': case '7': case '8': case '9':
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
                fprintf(stderr, "line %u: error: expected number.\n", parser->line);
                return NULL;
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

            double float_value = (double) integer_value;

            if (_sh_json_matches(parser, '.'))
            {
                is_float = true;

                if ((parser->index >= parser->input.count) ||
                    !sh_unicode_is_digit(parser->input.data[parser->index]))
                {
                    fprintf(stderr, "line %u: error: expected digits after '.'.\n", parser->line);
                    return NULL;
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
                    fprintf(stderr, "line %u: error: expected digits after 'e/E'.\n", parser->line);
                    return NULL;
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

            if (is_float)
            {
                if (is_signed)
                {
                    float_value = -float_value;
                }

                node = _sh_json_alloc_node(allocator, SH_JSON_VALUE_TYPE_NUMBER_FLOAT);
                node->value.number_float = float_value;
            }
            else
            {
                if (is_signed)
                {
                    integer_value = -integer_value;
                }

                node = _sh_json_alloc_node(allocator, SH_JSON_VALUE_TYPE_NUMBER_INTEGER);
                node->value.number_integer = integer_value;
            }
        } break;

        default:
        {
            fprintf(stderr, "line %u: error: expected value.\n", parser->line);
        } break;
    }

    _sh_json_skip_whitespaces(parser);

    return node;
}

SH_JSON_DEF ShJsonNode *
sh_json_parse(ShAllocator allocator, ShString json_str, uint32_t flags)
{
    // TODO: should we reuse the ShJsonParser or create a separate struct?
    // TODO: store the allocator on the parser
    ShJsonParser parser;
    parser.flags = flags;
    parser.line = 1;
    parser.index = 0;
    parser.input = json_str;

    return _sh_json_parse_value(allocator, &parser);
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
