// C_MAKE_COMPILER_FLAGS = "-std=c99 -Wall -Wextra -pedantic"
#define C_MAKE_IMPLEMENTATION
#include "c_make.h"

C_MAKE_ENTRY()
{
    switch (c_make_target)
    {
        case CMakeTargetSetup:
        {
        } break;

        case CMakeTargetBuild:
        {
            CMakeCommand command = { 0 };

            const char *target_c_compiler = c_make_get_target_c_compiler();

            c_make_command_append(&command, target_c_compiler);
            c_make_command_append_command_line(&command, c_make_get_target_c_flags());
            c_make_command_append_default_compiler_flags(&command, c_make_get_build_type());

            if (!c_make_compiler_is_msvc(target_c_compiler))
            {
                c_make_command_append(&command, "-std=c99", "-Wall", "-Wextra", "-pedantic");
            }

            c_make_command_append(&command, c_make_c_string_concat("-I", c_make_get_source_path()));
            c_make_command_append_output_executable(&command, c_make_c_string_path_concat(c_make_get_build_path(), "main"), c_make_get_target_platform());
            c_make_command_append(&command, c_make_c_string_path_concat(c_make_get_source_path(), "examples", "main.c"));
            c_make_command_append_default_linker_flags(&command, c_make_get_target_architecture());

            if ((c_make_get_target_platform() == CMakePlatformWindows) && !c_make_compiler_is_msvc(target_c_compiler))
            {
                c_make_command_append(&command, "-lws2_32");
            }

            c_make_log(CMakeLogLevelInfo, "compile 'main.c'\n");
            c_make_command_run_and_reset(&command);

            c_make_command_append(&command, target_c_compiler);
            c_make_command_append_command_line(&command, c_make_get_target_c_flags());
            c_make_command_append_default_compiler_flags(&command, c_make_get_build_type());

            if (!c_make_compiler_is_msvc(target_c_compiler))
            {
                c_make_command_append(&command, "-std=c99", "-Wall", "-Wextra", "-pedantic");
            }

            c_make_command_append(&command, c_make_c_string_concat("-I", c_make_get_source_path()));
            c_make_command_append_output_executable(&command, c_make_c_string_path_concat(c_make_get_build_path(), "http_server"), c_make_get_target_platform());
            c_make_command_append(&command, c_make_c_string_path_concat(c_make_get_source_path(), "examples", "http_server.c"));
            c_make_command_append_default_linker_flags(&command, c_make_get_target_architecture());

            if ((c_make_get_target_platform() == CMakePlatformWindows) && !c_make_compiler_is_msvc(target_c_compiler))
            {
                c_make_command_append(&command, "-lws2_32");
            }

            c_make_log(CMakeLogLevelInfo, "compile 'http_server.c'\n");
            c_make_command_run_and_reset(&command);
        } break;

        case CMakeTargetInstall:
        {
        } break;
    }
}
