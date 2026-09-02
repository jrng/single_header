#define SH_BASE_IMPLEMENTATION
#define SH_STRING_BUILDER_IMPLEMENTATION
#define SH_PLATFORM_IMPLEMENTATION
#define SH_JSON_IMPLEMENTATION

#include "sh_base.h"
#include "sh_string_builder.h"
#include "sh_platform.h"
#include "sh_json.h"

#include <stdio.h>

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

#if 0

static void
print_json_element(ShJsonResult result, ShJsonElement *element)
{
    ShString element_type_name = ShStringEmpty;

    switch (result)
    {
        case SH_JSON_RESULT_ARRAY_BEGIN:    element_type_name = ShStringLiteral("SH_JSON_RESULT_ARRAY_BEGIN");    break;
        case SH_JSON_RESULT_ARRAY_END:      element_type_name = ShStringLiteral("SH_JSON_RESULT_ARRAY_END");      break;
        case SH_JSON_RESULT_OBJECT_BEGIN:   element_type_name = ShStringLiteral("SH_JSON_RESULT_OBJECT_BEGIN");   break;
        case SH_JSON_RESULT_OBJECT_END:     element_type_name = ShStringLiteral("SH_JSON_RESULT_OBJECT_END");     break;
        case SH_JSON_RESULT_STRING:         element_type_name = ShStringLiteral("SH_JSON_RESULT_STRING");         break;
        case SH_JSON_RESULT_NUMBER_INTEGER: element_type_name = ShStringLiteral("SH_JSON_RESULT_NUMBER_INTEGER"); break;
        case SH_JSON_RESULT_NUMBER_FLOAT:   element_type_name = ShStringLiteral("SH_JSON_RESULT_NUMBER_FLOAT");   break;
        case SH_JSON_RESULT_BOOLEAN:        element_type_name = ShStringLiteral("SH_JSON_RESULT_BOOLEAN");        break;
        case SH_JSON_RESULT_NULL:           element_type_name = ShStringLiteral("SH_JSON_RESULT_NULL");           break;
        case SH_JSON_RESULT_END_OF_STREAM:  element_type_name = ShStringLiteral("SH_JSON_RESULT_END_OF_STREAM");  break;
        case SH_JSON_RESULT_ERROR:          element_type_name = ShStringLiteral("SH_JSON_RESULT_ERROR");          break;
    }

    if (element->flags & SH_JSON_ELEMENT_FLAG_HAS_NAME)
    {
        fprintf(stderr, "    %" ShStringFmt ", key = '%" ShStringFmt "'", ShStringArg(element_type_name), ShStringArg(element->name));
    }
    else
    {
        fprintf(stderr, "    %" ShStringFmt, ShStringArg(element_type_name));
    }

    switch (result)
    {
        case SH_JSON_RESULT_ARRAY_BEGIN:    fprintf(stderr, "\n"); break;
        case SH_JSON_RESULT_ARRAY_END:      fprintf(stderr, "\n"); break;
        case SH_JSON_RESULT_OBJECT_BEGIN:   fprintf(stderr, "\n"); break;
        case SH_JSON_RESULT_OBJECT_END:     fprintf(stderr, "\n"); break;
        case SH_JSON_RESULT_STRING:         fprintf(stderr, ", value = '%" ShStringFmt "'\n", ShStringArg(element->value.string)); break;
        case SH_JSON_RESULT_NUMBER_INTEGER: fprintf(stderr, ", value = %lld\n", element->value.number_integer);                    break;
        case SH_JSON_RESULT_NUMBER_FLOAT:   fprintf(stderr, ", value = %f\n", element->value.number_float);                        break;
        case SH_JSON_RESULT_BOOLEAN:        fprintf(stderr, ", value = %s\n", element->value.boolean ? "true" : "false");          break;
        case SH_JSON_RESULT_NULL:           fprintf(stderr, ", value = null\n"); break;
        case SH_JSON_RESULT_END_OF_STREAM:  fprintf(stderr, "\n"); break;
        case SH_JSON_RESULT_ERROR:          fprintf(stderr, "\n"); break;
    }
}

#endif

int main(void)
{
    ShAllocator allocator;
    allocator.data = NULL;
    allocator.func = c_default_allocator_func;

    ShThreadContext *thread_context = sh_thread_context_create(allocator, ShMiB(1));

    fprintf(stderr, "--- Success cases:\n");

    for (uint32_t i = 1; i <= 2; i += 1)
    {
        ShTemporaryMemory temp_memory = sh_begin_temporary_memory(thread_context, 0, NULL);

        ShString filename = sh_string_path_concat(thread_context, temp_memory.allocator,
                                                  ShStringLiteral("data"), ShStringLiteral("json"),
                                                  ShStringLiteral("success"),
                                                  sh_string_formated(thread_context, temp_memory.allocator, ShStringLiteral("%02u.json"), i));

        ShString json_str = ShStringEmpty;

        if (sh_read_entire_file(thread_context, allocator, filename, &json_str))
        {
            ShJsonParser parser;
            sh_json_parser_init(&parser, json_str);

            for (;;)
            {
                ShJsonElement elem;

                ShJsonResult result = sh_json_parser_get_next_element(&parser, &elem);

                // print_json_element(result, &elem);

                if (result == SH_JSON_RESULT_END_OF_STREAM)
                {
                    fprintf(stderr, "  '%" ShStringFmt "'  OK\n", ShStringArg(filename));
                    break;
                }

                if (result == SH_JSON_RESULT_ERROR)
                {
                    fprintf(stderr, "  '%" ShStringFmt "'  ERROR should have succeeded\n", ShStringArg(filename));
                    break;
                }
            }
        }
        else
        {
            fprintf(stderr, "  '%" ShStringFmt "'  error: could not load file\n", ShStringArg(filename));
        }

        sh_end_temporary_memory(temp_memory);
    }

    fprintf(stderr, "--- Failure cases:\n");

    for (uint32_t i = 1; i <= 2; i += 1)
    {
        ShTemporaryMemory temp_memory = sh_begin_temporary_memory(thread_context, 0, NULL);

        ShString filename = sh_string_path_concat(thread_context, temp_memory.allocator,
                                                  ShStringLiteral("data"), ShStringLiteral("json"),
                                                  ShStringLiteral("failure"),
                                                  sh_string_formated(thread_context, temp_memory.allocator, ShStringLiteral("%02u.json"), i));

        ShString json_str = ShStringEmpty;

        if (sh_read_entire_file(thread_context, allocator, filename, &json_str))
        {
            ShJsonParser parser;
            sh_json_parser_init(&parser, json_str);

            for (;;)
            {
                ShJsonElement elem;

                ShJsonResult result = sh_json_parser_get_next_element(&parser, &elem);

                // print_json_element(result, &elem);

                if (result == SH_JSON_RESULT_END_OF_STREAM)
                {
                    fprintf(stderr, "  '%" ShStringFmt "'  ERROR should have failed\n", ShStringArg(filename));
                    break;
                }

                if (result == SH_JSON_RESULT_ERROR)
                {
                    fprintf(stderr, "  '%" ShStringFmt "'  OK\n", ShStringArg(filename));
                    break;
                }
            }
        }
        else
        {
            fprintf(stderr, "  '%" ShStringFmt "'  error: could not load file\n", ShStringArg(filename));
        }

        sh_end_temporary_memory(temp_memory);
    }

    sh_thread_context_destroy(thread_context);

    return 0;
}
