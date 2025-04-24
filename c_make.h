// c_make.h - MIT License
// See end of file for full license

// TODO:
// - support android as a platform
// - add logging api
// - documentation
// - improve c_make_command_to_string()
// - improve c_make_command_append_command_line()
// - support comments in config file

#ifndef __C_MAKE_INCLUDE__
#define __C_MAKE_INCLUDE__

#define C_MAKE_PLATFORM_ANDROID 0
#define C_MAKE_PLATFORM_FREEBSD 0
#define C_MAKE_PLATFORM_WINDOWS 0
#define C_MAKE_PLATFORM_LINUX   0
#define C_MAKE_PLATFORM_MACOS   0
#define C_MAKE_PLATFORM_WEB     0

#if defined(__ANDROID__)
#  undef C_MAKE_PLATFORM_ANDROID
#  define C_MAKE_PLATFORM_ANDROID 1
#elif defined(__FreeBSD__)
#  undef C_MAKE_PLATFORM_FREEBSD
#  define C_MAKE_PLATFORM_FREEBSD 1
#elif defined(_WIN32)
#  undef C_MAKE_PLATFORM_WINDOWS
#  define C_MAKE_PLATFORM_WINDOWS 1
#elif defined(__linux__)
#  undef C_MAKE_PLATFORM_LINUX
#  define C_MAKE_PLATFORM_LINUX 1
#elif defined(__APPLE__) && defined(__MACH__)
#  undef C_MAKE_PLATFORM_MACOS
#  define C_MAKE_PLATFORM_MACOS 1
#elif defined(__wasm__)
#  undef C_MAKE_PLATFORM_WEB
#  define C_MAKE_PLATFORM_WEB 1
#endif

#define C_MAKE_ARCHITECTURE_AMD64   0
#define C_MAKE_ARCHITECTURE_AARCH64 0
#define C_MAKE_ARCHITECTURE_RISCV64 0
#define C_MAKE_ARCHITECTURE_WASM32  0
#define C_MAKE_ARCHITECTURE_WASM64  0

#if C_MAKE_PLATFORM_WINDOWS
#  if defined(__MINGW32__)
#    if defined(__x86_64__)
#      undef C_MAKE_ARCHITECTURE_AMD64
#      define C_MAKE_ARCHITECTURE_AMD64 1
#    elif defined(__aarch64__)
#      undef C_MAKE_ARCHITECTURE_AARCH64
#      define C_MAKE_ARCHITECTURE_AARCH64 1
#    elif defined(__riscv) && (__riscv_xlen == 64)
#      undef C_MAKE_ARCHITECTURE_RISCV64
#      define C_MAKE_ARCHITECTURE_RISCV64 1
#    endif
#  else
#    if defined(_M_AMD64)
#      undef C_MAKE_ARCHITECTURE_AMD64
#      define C_MAKE_ARCHITECTURE_AMD64 1
#    endif
#  endif
#elif C_MAKE_PLATFORM_ANDROID || C_MAKE_PLATFORM_FREEBSD || C_MAKE_PLATFORM_LINUX || C_MAKE_PLATFORM_MACOS
#  if defined(__x86_64__)
#    undef C_MAKE_ARCHITECTURE_AMD64
#    define C_MAKE_ARCHITECTURE_AMD64 1
#  elif defined(__aarch64__)
#    undef C_MAKE_ARCHITECTURE_AARCH64
#    define C_MAKE_ARCHITECTURE_AARCH64 1
#  elif defined(__riscv) && (__riscv_xlen == 64)
#    undef C_MAKE_ARCHITECTURE_RISCV64
#    define C_MAKE_ARCHITECTURE_RISCV64 1
#  endif
#elif C_MAKE_PLATFORM_WEB
#  if defined(__wasm32__)
#    undef C_MAKE_ARCHITECTURE_WASM32
#    define C_MAKE_ARCHITECTURE_WASM32 1
#  elif defined(__wasm64__)
#    undef C_MAKE_ARCHITECTURE_WASM64
#    define C_MAKE_ARCHITECTURE_WASM64 1
#  endif
#endif

#define _CMakeStr(str) #str
#define CMakeStr(str) _CMakeStr(str)

#define CMakeArrayCount(array) (sizeof(array)/sizeof((array)[0]))

#define CMakeNArgs(...) __CMakeNArgs(_CMakeNArgs(__VA_ARGS__, 15, 14, 13, 12, 11, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0))
#define __CMakeNArgs(x) x
#define _CMakeNArgs(_1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, n, ...) n

#define CMakeStringLiteral(str) c_make_make_string((char *) (str), sizeof(str) - 1)
#define CMakeCString(str) c_make_make_string((char *) (str), c_make_get_c_string_length(str))

#define c_make_string_concat(...) c_make_string_concat_va(CMakeNArgs(__VA_ARGS__), __VA_ARGS__)

#define c_make_c_string_concat(...) c_make_c_string_concat_va(CMakeNArgs(__VA_ARGS__), __VA_ARGS__)
#define c_make_c_string_path_concat(...) c_make_c_string_path_concat_va(CMakeNArgs(__VA_ARGS__), __VA_ARGS__)

#define c_make_command_append(command, ...) c_make_command_append_va(command, CMakeNArgs(__VA_ARGS__), __VA_ARGS__ )

#if defined(C_MAKE_STATIC)
#  define C_MAKE_DEF static
#else
#  define C_MAKE_DEF extern
#endif

#if C_MAKE_PLATFORM_WINDOWS

#  define UNICODE
#  define _UNICODE
#  define NOMINMAX
#  include <windows.h>

typedef HANDLE CMakeProcessId;

#  define CMakeInvalidProcessId INVALID_HANDLE_VALUE

#elif C_MAKE_PLATFORM_ANDROID || C_MAKE_PLATFORM_FREEBSD || C_MAKE_PLATFORM_LINUX || C_MAKE_PLATFORM_MACOS

#  include <sys/wait.h>

typedef pid_t CMakeProcessId;

#  define CMakeInvalidProcessId (-1)

#endif

#if C_MAKE_PLATFORM_FREEBSD
#  include <sys/sysctl.h>
#elif C_MAKE_PLATFORM_MACOS
#  include <libproc.h>
#endif

#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#else
#  include <stdbool.h>
#endif

typedef enum CMakeTarget
{
    CMakeTargetSetup   = 0,
    CMakeTargetBuild   = 1,
    CMakeTargetInstall = 2,
} CMakeTarget;

#if !defined(C_MAKE_NO_ENTRY_POINT)

#  define C_MAKE_ENTRY() void _c_make_entry_(CMakeTarget c_make_target)

C_MAKE_ENTRY();

#endif // !defined(C_MAKE_NO_ENTRY_POINT)

typedef enum CMakeLogLevel
{
    CMakeLogLevelRaw     = 0,
    CMakeLogLevelInfo    = 1,
    CMakeLogLevelWarning = 2,
    CMakeLogLevelError   = 3,
} CMakeLogLevel;

typedef enum CMakePlatform
{
    CMakePlatformAndroid = 0,
    CMakePlatformFreeBsd = 1,
    CMakePlatformWindows = 2,
    CMakePlatformLinux   = 3,
    CMakePlatformMacOs   = 4,
    CMakePlatformWeb     = 5,
} CMakePlatform;

typedef enum CMakeArchitecture
{
    CMakeArchitectureUnknown = 0,
    CMakeArchitectureAmd64   = 1,
    CMakeArchitectureAarch64 = 2,
    CMakeArchitectureRiscv64 = 3,
    CMakeArchitectureWasm32  = 4,
    CMakeArchitectureWasm64  = 5,
} CMakeArchitecture;

typedef enum CMakeBuildType
{
    CMakeBuildTypeDebug    = 0,
    CMakeBuildTypeRelDebug = 1,
    CMakeBuildTypeRelease  = 2,
} CMakeBuildType;

typedef struct CMakeMemory
{
    size_t used;
    size_t allocated;
    void *base;
} CMakeMemory;

typedef struct CMakeString
{
    size_t count;
    char *data;
} CMakeString;

#define CMakeStringFmt ".*s"
#define CMakeStringArg(str) (int) (str).count, (str).data

typedef struct CMakeCommand
{
    size_t count;
    size_t allocated;
    const char **items;
} CMakeCommand;

typedef struct CMakeConfigValue
{
    bool is_valid;
    const char *val;
} CMakeConfigValue;

typedef struct CMakeConfigEntry
{
    CMakeString key;
    CMakeString value;
} CMakeConfigEntry;

typedef struct CMakeConfig
{
    size_t count;
    size_t allocated;
    CMakeConfigEntry *items;
} CMakeConfig;

typedef struct CMakeProcess
{
    CMakeProcessId id;
    bool exited;
    bool succeeded;
} CMakeProcess;

typedef struct CMakeProcessGroup
{
    size_t count;
    size_t allocated;
    CMakeProcess *items;
} CMakeProcessGroup;

typedef struct CMakeContext
{
    bool verbose;
    bool did_fail;
    bool sequential;

    CMakePlatform target_platform;
    CMakeArchitecture target_architecture;
    CMakeBuildType build_type;

    const char *build_path;
    const char *source_path;

    CMakeConfig config;
    CMakeMemory private_memory;
    CMakeMemory public_memory;

    CMakeProcessGroup process_group;

    bool shell_initialized;

    const char *reset;
    const char *color_black;
    const char *color_red;
    const char *color_green;
    const char *color_yellow;
    const char *color_blue;
    const char *color_magenta;
    const char *color_cyan;
    const char *color_white;
    const char *color_bright_black;
    const char *color_bright_red;
    const char *color_bright_green;
    const char *color_bright_yellow;
    const char *color_bright_blue;
    const char *color_bright_magenta;
    const char *color_bright_cyan;
    const char *color_bright_white;
} CMakeContext;

typedef struct CMakeWindowsSoftwarePackage
{
    const char *version;
    const char *root_path;
} CMakeWindowsSoftwarePackage;

static inline CMakeString
c_make_make_string(void *data, size_t count)
{
    CMakeString result;
    result.count = count;
    result.data = (char *) data;
    return result;
}

static inline size_t
c_make_get_c_string_length(const char *str)
{
    size_t length = 0;
    while (*str++) length += 1;
    return length;
}

static inline CMakePlatform
c_make_get_host_platform(void)
{
#if C_MAKE_PLATFORM_ANDROID
    return CMakePlatformAndroid;
#elif C_MAKE_PLATFORM_FREEBSD
    return CMakePlatformFreeBsd;
#elif C_MAKE_PLATFORM_WINDOWS
    return CMakePlatformWindows;
#elif C_MAKE_PLATFORM_LINUX
    return CMakePlatformLinux;
#elif C_MAKE_PLATFORM_MACOS
    return CMakePlatformMacOs;
#elif C_MAKE_PLATFORM_WEB
    return CMakePlatformWeb;
#endif
}

static inline CMakeArchitecture
c_make_get_host_architecture(void)
{
#if C_MAKE_ARCHITECTURE_AMD64
    return CMakeArchitectureAmd64;
#elif C_MAKE_ARCHITECTURE_AARCH64
    return CMakeArchitectureAarch64;
#elif C_MAKE_ARCHITECTURE_RISCV64
    return CMakeArchitectureRiscv64;
#elif C_MAKE_ARCHITECTURE_WASM32
    return CMakeArchitectureWasm32;
#elif C_MAKE_ARCHITECTURE_WASM64
    return CMakeArchitectureWasm64;
#else
    return CMakeArchitectureUnknown;
#endif
}

static inline const char *
c_make_get_platform_name(CMakePlatform platform)
{
    const char *name = "";

    switch (platform)
    {
        case CMakePlatformAndroid: name = "android"; break;
        case CMakePlatformFreeBsd: name = "freebsd"; break;
        case CMakePlatformWindows: name = "windows"; break;
        case CMakePlatformLinux:   name = "linux";   break;
        case CMakePlatformMacOs:   name = "macos";   break;
        case CMakePlatformWeb:     name = "web";     break;
    }

    return name;
}

static inline const char *
c_make_get_architecture_name(CMakeArchitecture architecture)
{
    const char *name = "";

    switch (architecture)
    {
        case CMakeArchitectureUnknown: name = "unknown"; break;
        case CMakeArchitectureAmd64:   name = "amd64";   break;
        case CMakeArchitectureAarch64: name = "aarch64"; break;
        case CMakeArchitectureRiscv64: name = "riscv64"; break;
        case CMakeArchitectureWasm32:  name = "wasm32";  break;
        case CMakeArchitectureWasm64:  name = "wasm64";  break;
    }

    return name;
}

C_MAKE_DEF void c_make_set_failed(bool failed);
C_MAKE_DEF bool c_make_get_failed(void);
C_MAKE_DEF void c_make_log(CMakeLogLevel log_level, const char *format, ...);

C_MAKE_DEF void *c_make_memory_allocate(CMakeMemory *memory, size_t size);
C_MAKE_DEF void *c_make_memory_reallocate(CMakeMemory *memory, void *old_ptr, size_t old_size, size_t new_size);
C_MAKE_DEF size_t c_make_memory_get_used(CMakeMemory *memory);
C_MAKE_DEF void c_make_memory_set_used(CMakeMemory *memory, size_t used);

C_MAKE_DEF void *c_make_allocate(size_t size);
C_MAKE_DEF size_t c_make_memory_save(void);
C_MAKE_DEF void c_make_memory_restore(size_t saved);

C_MAKE_DEF void c_make_command_append_va(CMakeCommand *command, size_t count, ...);
C_MAKE_DEF void c_make_command_append_slice(CMakeCommand *command, size_t count, const char **items);
C_MAKE_DEF void c_make_command_append_command_line(CMakeCommand *command, const char *str);
C_MAKE_DEF void c_make_command_append_output_object(CMakeCommand *command, const char *output_path, CMakePlatform platform);
C_MAKE_DEF void c_make_command_append_output_executable(CMakeCommand *command, const char *output_path, CMakePlatform platform);
C_MAKE_DEF void c_make_command_append_input_static_library(CMakeCommand *command, const char *input_path, CMakePlatform platform);
C_MAKE_DEF void c_make_command_append_default_compiler_flags(CMakeCommand *command, CMakeBuildType build_type);
C_MAKE_DEF void c_make_command_append_default_linker_flags(CMakeCommand *command, CMakeArchitecture architecture);
C_MAKE_DEF CMakeString c_make_command_to_string(CMakeCommand command);

C_MAKE_DEF bool c_make_strings_are_equal(CMakeString a, CMakeString b);
C_MAKE_DEF CMakeString c_make_copy_string(CMakeMemory *memory, CMakeString str);
C_MAKE_DEF CMakeString c_make_string_split_left(CMakeString *str, char c);
C_MAKE_DEF CMakeString c_make_string_split_right(CMakeString *str, char c);
C_MAKE_DEF CMakeString c_make_string_split_right_path_separator(CMakeString *str);
C_MAKE_DEF CMakeString c_make_string_trim(CMakeString str);
C_MAKE_DEF size_t c_make_string_find(CMakeString str, CMakeString pattern);
C_MAKE_DEF char *c_make_string_to_c_string(CMakeMemory *memory, CMakeString str);

C_MAKE_DEF CMakePlatform c_make_get_target_platform(void);
C_MAKE_DEF CMakeArchitecture c_make_get_target_architecture(void);
C_MAKE_DEF CMakeBuildType c_make_get_build_type(void);
C_MAKE_DEF const char *c_make_get_build_path(void);
C_MAKE_DEF const char *c_make_get_source_path(void);
C_MAKE_DEF const char *c_make_get_install_prefix(void);

C_MAKE_DEF const char *c_make_get_host_ar(void);
C_MAKE_DEF const char *c_make_get_target_ar(void);
C_MAKE_DEF const char *c_make_get_host_c_compiler(void);
C_MAKE_DEF const char *c_make_get_target_c_compiler(void);
C_MAKE_DEF const char *c_make_get_target_c_flags(void);
C_MAKE_DEF const char *c_make_get_host_cpp_compiler(void);
C_MAKE_DEF const char *c_make_get_target_cpp_compiler(void);
C_MAKE_DEF const char *c_make_get_target_cpp_flags(void);

C_MAKE_DEF bool c_make_find_visual_studio(CMakeWindowsSoftwarePackage *visual_studio_install);
C_MAKE_DEF bool c_make_find_windows_sdk(CMakeWindowsSoftwarePackage *windows_sdk);
C_MAKE_DEF bool c_make_get_visual_studio(CMakeWindowsSoftwarePackage *visual_studio_install);
C_MAKE_DEF bool c_make_get_windows_sdk(CMakeWindowsSoftwarePackage *windows_sdk);
C_MAKE_DEF const char *c_make_get_msvc_library_manager(CMakeArchitecture target_architecture);
C_MAKE_DEF const char *c_make_get_msvc_compiler(CMakeArchitecture target_architecture);
C_MAKE_DEF void c_make_command_append_msvc_compiler_flags(CMakeCommand *command);
C_MAKE_DEF void c_make_command_append_msvc_linker_flags(CMakeCommand *command, CMakeArchitecture target_architecture);

C_MAKE_DEF void c_make_config_set(const char *key, const char *value);
C_MAKE_DEF CMakeConfigValue c_make_config_get(const char *key);

C_MAKE_DEF bool c_make_store_config(const char *file_name);
C_MAKE_DEF bool c_make_load_config(const char *file_name);

C_MAKE_DEF bool c_make_needs_rebuild(const char *output_file, size_t input_file_count, const char **input_files);
C_MAKE_DEF bool c_make_needs_rebuild_single_source(const char *output_file, const char *input_file);

C_MAKE_DEF bool c_make_file_exists(const char *file_name);
C_MAKE_DEF bool c_make_directory_exists(const char *directory_name);
C_MAKE_DEF bool c_make_create_directory(const char *directory_name);
C_MAKE_DEF bool c_make_create_directory_recursively(const char *directory_name);
C_MAKE_DEF bool c_make_read_entire_file(const char *file_name, CMakeString *content);
C_MAKE_DEF bool c_make_write_entire_file(const char *file_name, CMakeString content);
C_MAKE_DEF bool c_make_copy_file(const char *src_file, const char *dst_file);
C_MAKE_DEF bool c_make_rename_file(const char *old_file_name, const char *new_file_name);
C_MAKE_DEF bool c_make_delete_file(const char *file_name);

C_MAKE_DEF bool c_make_has_slash_or_backslash(const char *path);
C_MAKE_DEF CMakeString c_make_get_environment_variable(CMakeMemory *memory, const char *variable_name);
C_MAKE_DEF const char *c_make_find_program(const char *program_name);

C_MAKE_DEF CMakeString c_make_string_concat_va(size_t count, ...);

C_MAKE_DEF char *c_make_c_string_concat_va(size_t count, ...);
C_MAKE_DEF char *c_make_c_string_path_concat_va(size_t count, ...);

C_MAKE_DEF CMakeProcessId c_make_command_run(CMakeCommand command);
C_MAKE_DEF CMakeProcessId c_make_command_run_and_reset(CMakeCommand *command);
C_MAKE_DEF bool c_make_process_wait(CMakeProcessId process_id);
C_MAKE_DEF bool c_make_command_run_and_reset_and_wait(CMakeCommand *command);
C_MAKE_DEF bool c_make_command_run_and_wait(CMakeCommand command);
C_MAKE_DEF bool c_make_process_wait_for_all(void);

static inline bool
c_make_is_msvc_library_manager(const char *cmd)
{
    if (cmd)
    {
        CMakeString cmd_path = CMakeCString(cmd);
        CMakeString cmd_name = c_make_string_split_right(&cmd_path, '\\');

        return c_make_strings_are_equal(cmd_name, CMakeStringLiteral("lib.exe"));
    }

    return false;
}

static inline bool
c_make_compiler_is_msvc(const char *compiler)
{
    if (compiler)
    {
        CMakeString compiler_path = CMakeCString(compiler);
        CMakeString compiler_name = c_make_string_split_right(&compiler_path, '\\');

        return c_make_strings_are_equal(compiler_name, CMakeStringLiteral("cl.exe"));
    }

    return false;
}

static inline void
c_make_config_set_if_not_exists(const char *key, const char *value)
{
    if (!c_make_config_get(key).is_valid)
    {
        c_make_config_set(key, value);
    }
}

static inline bool
c_make_config_is_enabled(const char *key, bool fallback)
{
    bool result = fallback;
    CMakeConfigValue config_value = c_make_config_get(key);

    if (config_value.is_valid)
    {
        const char *val = config_value.val;

        if (val && (val[0] == 'o') && (val[1] == 'n') && (val[2] == 0))
        {
            result = true;
        }
        else
        {
            result = false;
        }
    }

    return result;
}

#ifdef __cplusplus
}
#endif

#include <stdio.h>
#include <assert.h>
#include <stdarg.h>
#include <stdlib.h>

#if C_MAKE_PLATFORM_ANDROID || C_MAKE_PLATFORM_FREEBSD || C_MAKE_PLATFORM_LINUX || C_MAKE_PLATFORM_MACOS

#  include <time.h>
#  include <errno.h>
#  include <fcntl.h>
#  include <unistd.h>
#  include <sys/stat.h>

#endif

#endif // __C_MAKE_INCLUDE__

#if defined(C_MAKE_IMPLEMENTATION)

static CMakeContext _c_make_context;

#if !defined(c_make_strlen)
#  include <string.h>
#  define c_make_strcmp(a, b) strcmp(a, b)
#  define c_make_strdup(a) strdup(a)
#endif

#if !defined(c_make_malloc)
#  include <stdlib.h>
#  define c_make_malloc(a) malloc(a)
#endif

#if C_MAKE_PLATFORM_WINDOWS

#  if !defined(__MINGW32__)
#    pragma comment(lib, "ole32")
#    pragma comment(lib, "oleaut32")
#    pragma comment(lib, "advapi32")
#  endif

#  undef INTERFACE
#  define INTERFACE ISetupInstance

DECLARE_INTERFACE_(ISetupInstance, IUnknown)
{
    BEGIN_INTERFACE

    // IUnknown
    STDMETHOD  (QueryInterface)               (THIS_ REFIID, void **) PURE;
    STDMETHOD_ (ULONG, AddRef)                (THIS) PURE;
    STDMETHOD_ (ULONG, Release)               (THIS) PURE;

    // ISetupInstance
    STDMETHOD  (GetInstanceId)                (THIS_ BSTR *) PURE;
    STDMETHOD  (GetInstallDate)               (THIS_ LPFILETIME) PURE;
    STDMETHOD  (GetInstallationName)          (THIS_ BSTR *) PURE;
    STDMETHOD  (GetInstallationPath)          (THIS_ BSTR *) PURE;
    STDMETHOD  (GetInstallationVersion)       (THIS_ BSTR *) PURE;
    STDMETHOD  (GetDisplayName)               (THIS_ LCID, BSTR *) PURE;
    STDMETHOD  (GetDescription)               (THIS_ LCID, BSTR *) PURE;
    STDMETHOD  (ResolvePath)                  (THIS_ LPCOLESTR, BSTR *) PURE;

    END_INTERFACE
};

#  undef INTERFACE
#  define INTERFACE IEnumSetupInstances

DECLARE_INTERFACE_(IEnumSetupInstances, IUnknown)
{
    BEGIN_INTERFACE

    // IUnknown
    STDMETHOD  (QueryInterface)               (THIS_ REFIID, void **) PURE;
    STDMETHOD_ (ULONG, AddRef)                (THIS) PURE;
    STDMETHOD_ (ULONG, Release)               (THIS) PURE;

    // IEnumSetupInstances
    STDMETHOD  (Next)                         (THIS_ ULONG, ISetupInstance **, ULONG *) PURE;
    STDMETHOD  (Skip)                         (THIS_ ULONG) PURE;
    STDMETHOD  (Reset)                        (THIS) PURE;
    STDMETHOD  (Clone)                        (THIS_ IEnumSetupInstances **) PURE;

    END_INTERFACE
};

#  undef INTERFACE
#  define INTERFACE ISetupConfiguration

DECLARE_INTERFACE_(ISetupConfiguration, IUnknown)
{
    BEGIN_INTERFACE

    // IUnknown
    STDMETHOD  (QueryInterface)               (THIS_ REFIID, void **) PURE;
    STDMETHOD_ (ULONG, AddRef)                (THIS) PURE;
    STDMETHOD_ (ULONG, Release)               (THIS) PURE;

    // ISetupConfiguration
    STDMETHOD  (EnumInstances)                (THIS_ IEnumSetupInstances **) PURE;
    STDMETHOD  (GetInstanceForCurrentProcess) (THIS_ ISetupInstance **) PURE;
    STDMETHOD  (GetInstanceForPath)           (THIS_ LPCWSTR, ISetupInstance **) PURE;

    END_INTERFACE
};

static inline LPWSTR
c_make_c_string_utf8_to_utf16(CMakeMemory *memory, const char *utf8_str)
{
    size_t utf8_length = c_make_get_c_string_length(utf8_str);
    size_t utf16_size = 2 * (utf8_length + 1);
    LPWSTR utf16_str = (LPWSTR) c_make_memory_allocate(memory, utf16_size);
    MultiByteToWideChar(CP_UTF8, 0, utf8_str, -1, utf16_str, utf16_size);
    return utf16_str;
}

static inline const char *
c_make_c_string_utf16_to_utf8(CMakeMemory *memory, const wchar_t *utf16_string_data, DWORD utf16_string_char_count)
{
    char *utf8_string_data = 0;

    if (utf16_string_char_count > 0)
    {
        size_t utf8_string_count = 4 * utf16_string_char_count + 1;
        utf8_string_data = (char *) c_make_memory_allocate(memory, utf8_string_count);
        utf8_string_count = WideCharToMultiByte(CP_UTF8, 0, utf16_string_data, utf16_string_char_count,
                                                utf8_string_data, utf8_string_count, 0, 0);
        utf8_string_data[utf8_string_count] = 0;
    }

    return utf8_string_data;
}

static inline CMakeString
c_make_string_utf16_to_utf8(CMakeMemory *memory, const wchar_t *utf16_string_data, DWORD utf16_string_char_count)
{
    CMakeString utf8_string = { 0, 0 };

    if (utf16_string_char_count > 0)
    {
        utf8_string.count = 4 * utf16_string_char_count;
        utf8_string.data  = (char *) c_make_memory_allocate(memory, utf8_string.count);
        utf8_string.count = WideCharToMultiByte(CP_UTF8, 0, utf16_string_data, utf16_string_char_count,
                                                utf8_string.data, utf8_string.count, 0, 0);
    }

    return utf8_string;
}

#endif

C_MAKE_DEF void
c_make_set_failed(bool failed)
{
    _c_make_context.did_fail = failed;
}

C_MAKE_DEF bool
c_make_get_failed(void)
{
    return _c_make_context.did_fail;
}

C_MAKE_DEF void
c_make_log(CMakeLogLevel log_level, const char *format, ...)
{
    if (!_c_make_context.shell_initialized)
    {
#if C_MAKE_PLATFORM_WINDOWS
        HANDLE std_error = GetStdHandle(STD_ERROR_HANDLE);
        DWORD mode = 0;

        if (GetConsoleMode(std_error, &mode) && SetConsoleMode(std_error, mode | ENABLE_VIRTUAL_TERMINAL_PROCESSING ))
#else
#  if C_MAKE_PLATFORM_LINUX
        int fileno(FILE *);
#  endif

        if (isatty(fileno(stderr)))
#endif
        {
            _c_make_context.reset                = "\x1b[0m";
            _c_make_context.color_black          = "\x1b[30m";
            _c_make_context.color_red            = "\x1b[31m";
            _c_make_context.color_green          = "\x1b[32m";
            _c_make_context.color_yellow         = "\x1b[33m";
            _c_make_context.color_blue           = "\x1b[34m";
            _c_make_context.color_magenta        = "\x1b[35m";
            _c_make_context.color_cyan           = "\x1b[36m";
            _c_make_context.color_white          = "\x1b[37m";
            _c_make_context.color_bright_black   = "\x1b[1;30m";
            _c_make_context.color_bright_red     = "\x1b[1;31m";
            _c_make_context.color_bright_green   = "\x1b[1;32m";
            _c_make_context.color_bright_yellow  = "\x1b[1;33m";
            _c_make_context.color_bright_blue    = "\x1b[1;34m";
            _c_make_context.color_bright_magenta = "\x1b[1;35m";
            _c_make_context.color_bright_cyan    = "\x1b[1;36m";
            _c_make_context.color_bright_white   = "\x1b[1;37m";
        }
        else
        {
            _c_make_context.reset                = "";
            _c_make_context.color_black          = "";
            _c_make_context.color_red            = "";
            _c_make_context.color_green          = "";
            _c_make_context.color_yellow         = "";
            _c_make_context.color_blue           = "";
            _c_make_context.color_magenta        = "";
            _c_make_context.color_cyan           = "";
            _c_make_context.color_white          = "";
            _c_make_context.color_bright_black   = "";
            _c_make_context.color_bright_red     = "";
            _c_make_context.color_bright_green   = "";
            _c_make_context.color_bright_yellow  = "";
            _c_make_context.color_bright_blue    = "";
            _c_make_context.color_bright_magenta = "";
            _c_make_context.color_bright_cyan    = "";
            _c_make_context.color_bright_white   = "";
        }

        _c_make_context.shell_initialized = true;
    }

    switch (log_level)
    {
        case CMakeLogLevelRaw:
        {
        } break;

        case CMakeLogLevelInfo:
        {
            fprintf(stderr, "%s-- ", _c_make_context.color_bright_white);
        } break;

        case CMakeLogLevelWarning:
        {
            fprintf(stderr, "%s-- %swarning: ",
                    _c_make_context.color_bright_white,
                    _c_make_context.color_bright_yellow);
        } break;

        case CMakeLogLevelError:
        {
            fprintf(stderr, "%s-- %serror: ",
                    _c_make_context.color_bright_white,
                    _c_make_context.color_bright_red);
        } break;
    }

    va_list args;
    va_start(args, format);
    vfprintf(stderr, format, args);
    fprintf(stderr, "%s", _c_make_context.reset);
    fflush(stderr);
    va_end(args);
}

C_MAKE_DEF void *
c_make_memory_allocate(CMakeMemory *memory, size_t size)
{
    size = (size + 15) & ~15;

    if (memory->allocated == 0)
    {
        memory->used = 0;
        memory->allocated = 16 * 1024 * 1024;
        memory->base = c_make_malloc(memory->allocated);
    }

    void *result = 0;

    if ((memory->used + size) <= memory->allocated)
    {
        result = (unsigned char *) memory->base + memory->used;
        memory->used += size;
    }

    return result;
}

C_MAKE_DEF void *
c_make_memory_reallocate(CMakeMemory *memory, void *old_ptr, size_t old_size, size_t new_size)
{
    assert(new_size > old_size);

    old_size = (old_size + 15) & ~15;
    new_size = (new_size + 15) & ~15;

    void *end_ptr  = (unsigned char *) old_ptr + old_size;
    void *next_ptr = (unsigned char *) memory->base + memory->used;

    void *result = 0;

    if (!old_ptr || (end_ptr != next_ptr))
    {
        result = c_make_memory_allocate(memory, new_size);

        if (old_ptr && result)
        {
            // TODO: take advantage of the fact that old_size is a multiple of 16
            unsigned char *src = (unsigned char *) old_ptr;
            unsigned char *dst = (unsigned char *) result;

            while (old_size--)
            {
                *dst++ = *src++;
            }
        }
    }
    else
    {
        size_t size = new_size - old_size;

        if ((memory->used + size) <= memory->allocated)
        {
            result = old_ptr;
            memory->used += size;
        }
    }

    return result;
}

C_MAKE_DEF size_t
c_make_memory_get_used(CMakeMemory *memory)
{
    return memory->used;
}

C_MAKE_DEF void
c_make_memory_set_used(CMakeMemory *memory, size_t used)
{
    assert(used <= memory->used);
    assert(!(used & 15));

    memory->used = used;
}

C_MAKE_DEF void *
c_make_allocate(size_t size)
{
    return c_make_memory_allocate(&_c_make_context.public_memory, size);
}

C_MAKE_DEF size_t
c_make_memory_save(void)
{
    return c_make_memory_get_used(&_c_make_context.public_memory);
}

C_MAKE_DEF void
c_make_memory_restore(size_t saved)
{
    c_make_memory_set_used(&_c_make_context.public_memory, saved);
}

C_MAKE_DEF void
c_make_command_append_va(CMakeCommand *command, size_t count, ...)
{
    if ((command->count + count) > command->allocated)
    {
        size_t grow = 16;

        if (count > grow)
        {
            grow = count;
        }

        size_t old_count = command->allocated;
        command->allocated += grow;
        command->items = (const char **) c_make_memory_reallocate(&_c_make_context.public_memory,
                                                                  (void *) command->items,
                                                                  old_count * sizeof(const char *),
                                                                  command->allocated * sizeof(const char *));
    }

    va_list args;
    va_start(args, count);

    for (size_t i = 0; i < count; i += 1)
    {
        command->items[command->count] = va_arg(args, const char *);
        command->count += 1;
    }

    va_end(args);
}

C_MAKE_DEF void
c_make_command_append_slice(CMakeCommand *command, size_t count, const char **items)
{
    if ((command->count + count) > command->allocated)
    {
        size_t grow = 16;

        if (count > grow)
        {
            grow = count;
        }

        size_t old_count = command->allocated;
        command->allocated += grow;
        command->items = (const char **) c_make_memory_reallocate(&_c_make_context.public_memory,
                                                                  (void *) command->items,
                                                                  old_count * sizeof(const char *),
                                                                  command->allocated * sizeof(const char *));
    }

    for (size_t i = 0; i < count; i += 1)
    {
        command->items[command->count] = items[i];
        command->count += 1;
    }
}

C_MAKE_DEF void
c_make_command_append_command_line(CMakeCommand *command, const char *str)
{
    if (str)
    {
        while (*str)
        {
            while (*str == ' ') str += 1;

            const char *start = str;
            while (*str && (*str != ' ')) str += 1;
            size_t length = str - start;

            if (length)
            {
                char *c_str = (char *) c_make_memory_allocate(&_c_make_context.public_memory, length + 1);
                char *dst = c_str;
                const char *src = start;

                for (size_t i = 0; i < length; i += 1)
                {
                    *dst++ = *src++;
                }

                *dst = 0;

                c_make_command_append_slice(command, 1, (const char **) &c_str);
            }
        }
    }
}

C_MAKE_DEF void
c_make_command_append_output_object(CMakeCommand *command, const char *output_path, CMakePlatform platform)
{
    if ((command->count > 0) && command->items[0])
    {
        const char *compiler = command->items[0];

        if (c_make_compiler_is_msvc(compiler))
        {
            c_make_command_append(command, c_make_c_string_concat("-Fo", output_path, ".obj"));
        }
        else
        {
            if (platform == CMakePlatformWindows)
            {
                c_make_command_append(command, "-o", c_make_c_string_concat(output_path, ".obj"));
            }
            else
            {
                c_make_command_append(command, "-o", c_make_c_string_concat(output_path, ".o"));
            }
        }
    }
    else
    {
        c_make_log(CMakeLogLevelWarning, "%s: you need to append a c/c++ compiler command as the first argument\n", __func__);
    }
}

C_MAKE_DEF void
c_make_command_append_output_executable(CMakeCommand *command, const char *output_path, CMakePlatform platform)
{
    if ((command->count > 0) && command->items[0])
    {
        const char *compiler = command->items[0];

        const char *arguments[2];

        if (c_make_compiler_is_msvc(compiler))
        {
            arguments[0] = c_make_c_string_concat("-Fe", output_path, ".exe");
            arguments[1] = c_make_c_string_concat("-Fo", output_path, ".obj");
        }
        else
        {
            arguments[0] = "-o";
            arguments[1] = output_path;

            if (platform == CMakePlatformWindows)
            {
                arguments[1] = c_make_c_string_concat(output_path, ".exe");
            }
        }

        c_make_command_append_slice(command, CMakeArrayCount(arguments), arguments);
    }
    else
    {
        c_make_log(CMakeLogLevelWarning, "%s: you need to append a c/c++ compiler command as the first argument\n", __func__);
    }
}

C_MAKE_DEF void
c_make_command_append_input_static_library(CMakeCommand *command, const char *input_path, CMakePlatform platform)
{
    if ((command->count > 0) && command->items[0])
    {
        CMakeString path_str = CMakeCString(input_path);
        CMakeString name_str = c_make_string_split_right_path_separator(&path_str);

        size_t private_used = c_make_memory_get_used(&_c_make_context.private_memory);

        const char *path = c_make_string_to_c_string(&_c_make_context.private_memory, path_str);
        const char *name = c_make_string_to_c_string(&_c_make_context.private_memory, name_str);

        const char *full_path;

        if (platform == CMakePlatformWindows)
        {
            full_path = c_make_c_string_path_concat(path, c_make_c_string_concat("lib", name, ".lib"));
        }
        else
        {
            full_path = c_make_c_string_path_concat(path, c_make_c_string_concat("lib", name, ".a"));
        }

        c_make_memory_set_used(&_c_make_context.private_memory, private_used);

        c_make_command_append(command, full_path);
    }
    else
    {
        c_make_log(CMakeLogLevelWarning, "%s: you need to append a c/c++ compiler command as the first argument\n", __func__);
    }
}

C_MAKE_DEF void
c_make_command_append_default_compiler_flags(CMakeCommand *command, CMakeBuildType build_type)
{
    if ((command->count > 0) && command->items[0])
    {
        const char *compiler = command->items[0];

        if (c_make_compiler_is_msvc(compiler))
        {
            c_make_command_append_msvc_compiler_flags(command);
            c_make_command_append(command, "-nologo");

            switch (build_type)
            {
                case CMakeBuildTypeDebug:
                {
                    c_make_command_append(command, "-Od", "-Z7");
                } break;

                case CMakeBuildTypeRelDebug:
                {
                    c_make_command_append(command, "-O2", "-Z7");
                } break;

                case CMakeBuildTypeRelease:
                {
                    c_make_command_append(command, "-O2", "-DNDEBUG");
                } break;
            }
        }
        else
        {
            switch (build_type)
            {
                case CMakeBuildTypeDebug:
                {
                    c_make_command_append(command, "-g");
                } break;

                case CMakeBuildTypeRelDebug:
                {
                    c_make_command_append(command, "-O2", "-g");
                } break;

                case CMakeBuildTypeRelease:
                {
                    c_make_command_append(command, "-O2", "-DNDEBUG");
                } break;
            }
        }
    }
    else
    {
        c_make_log(CMakeLogLevelWarning, "%s: you need to append a c/c++ compiler command as the first argument\n", __func__);
    }
}

C_MAKE_DEF void
c_make_command_append_default_linker_flags(CMakeCommand *command, CMakeArchitecture architecture)
{
    if ((command->count > 0) && command->items[0])
    {
        const char *compiler = command->items[0];

        if (c_make_compiler_is_msvc(compiler))
        {
            c_make_command_append(command, "-link");
            c_make_command_append_msvc_linker_flags(command, architecture);
        }
    }
    else
    {
        c_make_log(CMakeLogLevelWarning, "%s: you need to append a c/c++ compiler command as the first argument\n", __func__);
    }
}

C_MAKE_DEF CMakeString
c_make_command_to_string(CMakeCommand command)
{
    CMakeString result = { 0, 0 };

    for (size_t i = 0; i < command.count; i += 1)
    {
        const char *str = command.items[i];
        result.count += c_make_get_c_string_length(str) + 3;
    }

    if (result.count > 0)
    {
        result.data = (char *) c_make_memory_allocate(&_c_make_context.public_memory, result.count);
        char *dst = result.data;

        for (size_t i = 0; i < command.count; i += 1)
        {
            bool needs_escaping = false;
            const char *str = command.items[i];

            while (*str)
            {
#if C_MAKE_PLATFORM_WINDOWS
                if ((*str == ',') || (*str == ';') || (*str == '=') || (*str == ' ') || (*str == '\t'))
#elif C_MAKE_PLATFORM_ANDROID || C_MAKE_PLATFORM_FREEBSD || C_MAKE_PLATFORM_LINUX || C_MAKE_PLATFORM_MACOS
                if ((*str == ' ') || (*str == '\t') || (*str == '\n'))
#endif
                {
                    needs_escaping = true;
                    break;
                }

                str += 1;
            }

            if (needs_escaping)
            {
                *dst++ = '"';

                const char *src = command.items[i];
                while (*src) *dst++ = *src++;

                *dst++ = '"';
            }
            else
            {
                const char *src = command.items[i];
                while (*src) *dst++ = *src++;

                result.count -= 2;
            }

            *dst++ = ' ';
        }

        result.count -= 1;
    }

    return result;
}

C_MAKE_DEF bool
c_make_strings_are_equal(CMakeString a, CMakeString b)
{
    if (a.count != b.count)
    {
        return false;
    }

    for (size_t i = 0; i < a.count; i += 1)
    {
        if (a.data[i] != b.data[i])
        {
            return false;
        }
    }

    return true;
}

C_MAKE_DEF CMakeString
c_make_copy_string(CMakeMemory *memory, CMakeString str)
{
    CMakeString result;
    result.count = str.count;
    result.data = (char *) c_make_memory_allocate(memory, str.count + 1);

    for (size_t i = 0; i < str.count; i += 1)
    {
        result.data[i] = str.data[i];
    }

    result.data[result.count] = 0;

    return result;
}

C_MAKE_DEF CMakeString
c_make_string_split_left(CMakeString *str, char c)
{
    CMakeString result = *str;

    while (str->count)
    {
        if (str->data[0] == c)
        {
            break;
        }

        str->count -= 1;
        str->data += 1;
    }

    if (str->count)
    {
        result.count = str->data - result.data;
        str->count -= 1;
        str->data += 1;
    }

    return result;
}

C_MAKE_DEF CMakeString
c_make_string_split_right(CMakeString *str, char c)
{
    CMakeString result = *str;

    while (str->count)
    {
        if (str->data[str->count - 1] == c)
        {
            break;
        }

        str->count -= 1;
    }

    if (str->count)
    {
        result.count -= str->count;
        result.data += str->count;
        str->count -= 1;
    }

    return result;
}

C_MAKE_DEF CMakeString
c_make_string_split_right_path_separator(CMakeString *str)
{
    CMakeString result = *str;

    while (str->count)
    {
        if ((str->data[str->count - 1] == '/') ||
            (str->data[str->count - 1] == '\\'))
        {
            break;
        }

        str->count -= 1;
    }

    if (str->count)
    {
        result.count -= str->count;
        result.data += str->count;
        str->count -= 1;
    }

    return result;
}

C_MAKE_DEF CMakeString
c_make_string_trim(CMakeString str)
{
    while (str.count && ((str.data[str.count - 1] == ' ') ||
                         (str.data[str.count - 1] == '\t') ||
                         (str.data[str.count - 1] == '\r') ||
                         (str.data[str.count - 1] == '\n')))
    {
        str.count -= 1;
    }

    while (str.count && ((str.data[0] == ' ') || (str.data[0] == '\t') ||
                         (str.data[0] == '\r') || (str.data[0] == '\n')))
    {
        str.count -= 1;
        str.data += 1;
    }

    return str;
}

C_MAKE_DEF size_t
c_make_string_find(CMakeString str, CMakeString pattern)
{
    size_t index = 0;

    for (; index < str.count; index += 1)
    {
        CMakeString slice;
        slice.count = pattern.count;
        slice.data  = str.data + index;

        size_t max_count = str.count - index;

        if (slice.count > max_count)
        {
            slice.count = max_count;
        }

        if (c_make_strings_are_equal(slice, pattern))
        {
            return index;
        }
    }

    return index;
}

C_MAKE_DEF char *
c_make_string_to_c_string(CMakeMemory *memory, CMakeString str)
{
    char *result = (char *) c_make_memory_allocate(memory, str.count + 1);

    for (size_t i = 0; i < str.count; i += 1)
    {
        result[i] = str.data[i];
    }

    result[str.count] = 0;

    return result;
}

C_MAKE_DEF CMakePlatform
c_make_get_target_platform(void)
{
    return _c_make_context.target_platform;
}

C_MAKE_DEF CMakeArchitecture
c_make_get_target_architecture(void)
{
    return _c_make_context.target_architecture;
}

C_MAKE_DEF CMakeBuildType
c_make_get_build_type(void)
{
    return _c_make_context.build_type;
}

C_MAKE_DEF const char *
c_make_get_build_path(void)
{
    return _c_make_context.build_path;
}

C_MAKE_DEF const char *
c_make_get_source_path(void)
{
    return _c_make_context.source_path;
}

C_MAKE_DEF const char *
c_make_get_install_prefix(void)
{
    const char *result = 0;

    CMakeConfigValue value = c_make_config_get("install_prefix");

    if (value.is_valid)
    {
        result = value.val;
    }
    else
    {
        // TODO: something else for windows
        result = "/usr/local";
    }

    return result;
}

C_MAKE_DEF const char *
c_make_get_host_ar(void)
{
    const char *result = 0;
    CMakeConfigValue value = c_make_config_get("host_ar");

    if (value.is_valid)
    {
        result = value.val;
    }
    else
    {
#if C_MAKE_PLATFORM_WINDOWS
#  ifdef __MINGW32__
        result = "x86_64-w64-mingw32-ar";
#  else
        result = "lib.exe";
#  endif
#elif C_MAKE_PLATFORM_ANDROID || C_MAKE_PLATFORM_LINUX
        result = "ar";
#elif C_MAKE_PLATFORM_FREEBSD || C_MAKE_PLATFORM_MACOS
        result = "ar";
#elif C_MAKE_PLATFORM_WEB
        result = "ar";
#endif
    }

    if (!c_make_has_slash_or_backslash(result))
    {
        result = c_make_find_program(result);
    }

#if C_MAKE_PLATFORM_WINDOWS
    if (!result)
    {
        result = c_make_get_msvc_library_manager(c_make_get_host_architecture());
    }
#endif

    return result;
}

C_MAKE_DEF const char *
c_make_get_target_ar(void)
{
    const char *result = 0;
    CMakeConfigValue value = c_make_config_get("target_ar");

    if (value.is_valid)
    {
        result = value.val;

        if (!c_make_has_slash_or_backslash(result))
        {
            result = c_make_find_program(result);
        }
    }
    else
    {
#if C_MAKE_PLATFORM_WINDOWS
        result = c_make_get_msvc_library_manager(c_make_get_target_architecture());
#else
        result = c_make_get_host_ar();
#endif
    }

    return result;
}

C_MAKE_DEF const char *
c_make_get_host_c_compiler(void)
{
    const char *result = 0;
    CMakeConfigValue value = c_make_config_get("host_c_compiler");

    if (value.is_valid)
    {
        result = value.val;
    }
    else
    {
#if C_MAKE_PLATFORM_WINDOWS
#  ifdef __MINGW32__
        result = "x86_64-w64-mingw32-gcc";
#  else
        result = "cl.exe";
#  endif
#elif C_MAKE_PLATFORM_ANDROID || C_MAKE_PLATFORM_LINUX
        result = "cc";
#elif C_MAKE_PLATFORM_FREEBSD || C_MAKE_PLATFORM_MACOS
        result = "clang";
#elif C_MAKE_PLATFORM_WEB
        result = "clang";
#endif
    }

    if (!c_make_has_slash_or_backslash(result))
    {
        result = c_make_find_program(result);
    }

#if C_MAKE_PLATFORM_WINDOWS
    if (!result)
    {
        result = c_make_get_msvc_compiler(c_make_get_host_architecture());
    }
#endif

    return result;
}

C_MAKE_DEF const char *
c_make_get_target_c_compiler(void)
{
    const char *result = 0;
    CMakeConfigValue value = c_make_config_get("target_c_compiler");

    if (value.is_valid)
    {
        result = value.val;

        if (!c_make_has_slash_or_backslash(result))
        {
            result = c_make_find_program(result);
        }
    }
    else
    {
#if C_MAKE_PLATFORM_WINDOWS
        result = c_make_get_msvc_compiler(c_make_get_target_architecture());
#else
        result = c_make_get_host_c_compiler();
#endif
    }

    return result;
}

C_MAKE_DEF const char *
c_make_get_target_c_flags(void)
{
    const char *result = 0;
    CMakeConfigValue value = c_make_config_get("target_c_flags");

    if (value.is_valid)
    {
        result = value.val;
    }

    return result;
}

C_MAKE_DEF const char *
c_make_get_host_cpp_compiler(void)
{
    const char *result = 0;
    CMakeConfigValue value = c_make_config_get("host_cpp_compiler");

    if (value.is_valid)
    {
        result = value.val;
    }
    else
    {
#if C_MAKE_PLATFORM_WINDOWS
#  ifdef __MINGW32__
        result = "x86_64-w64-mingw32-g++";
#  else
        result = "cl.exe";
#  endif
#elif C_MAKE_PLATFORM_ANDROID || C_MAKE_PLATFORM_LINUX
        result = "c++";
#elif C_MAKE_PLATFORM_FREEBSD || C_MAKE_PLATFORM_MACOS
        result = "clang++";
#elif C_MAKE_PLATFORM_WEB
        result = "clang++";
#endif
    }

    if (!c_make_has_slash_or_backslash(result))
    {
        result = c_make_find_program(result);
    }

#if C_MAKE_PLATFORM_WINDOWS
    if (!result)
    {
        result = c_make_get_msvc_compiler(c_make_get_host_architecture());
    }
#endif

    return result;
}

C_MAKE_DEF const char *
c_make_get_target_cpp_compiler(void)
{
    const char *result = 0;
    CMakeConfigValue value = c_make_config_get("target_cpp_compiler");

    if (value.is_valid)
    {
        result = value.val;

        if (!c_make_has_slash_or_backslash(result))
        {
            result = c_make_find_program(result);
        }
    }
    else
    {
#if C_MAKE_PLATFORM_WINDOWS
        result = c_make_get_msvc_compiler(c_make_get_target_architecture());
#else
        result = c_make_get_host_cpp_compiler();
#endif
    }

    return result;
}

C_MAKE_DEF const char *
c_make_get_target_cpp_flags(void)
{
    const char *result = 0;
    CMakeConfigValue value = c_make_config_get("target_cpp_flags");

    if (value.is_valid)
    {
        result = value.val;
    }

    return result;
}

C_MAKE_DEF bool
c_make_find_visual_studio(CMakeWindowsSoftwarePackage *visual_studio_install)
{
#if C_MAKE_PLATFORM_WINDOWS

#  ifdef __cplusplus
#    define CALL0(obj, method) obj->method()
#    define CALL1(obj, method, ...) obj->method(__VA_ARGS__)
#  else
#    define CALL0(obj, method) obj->lpVtbl->method(obj)
#    define CALL1(obj, method, ...) obj->lpVtbl->method(obj, __VA_ARGS__)
#  endif

    CoInitializeEx(0, COINIT_MULTITHREADED);

    const GUID CLSID_SetupConfiguration = { 0x177F0C4A, 0x1CD3, 0x4DE7, { 0xA3, 0x2C, 0x71, 0xDB, 0xBB, 0x9F, 0xA3, 0x6D } };
    const GUID IID_ISetupConfiguration  = { 0x42843719, 0xDB4C, 0x46C2, { 0x8E, 0x7C, 0x64, 0xF1, 0x81, 0x6E, 0xFD, 0x5B } };

    ISetupConfiguration *setup_configuration = 0;
#  ifdef __cplusplus
    HRESULT res = CoCreateInstance(CLSID_SetupConfiguration, 0, CLSCTX_INPROC_SERVER, IID_ISetupConfiguration, (void **) &setup_configuration);
#  else
    HRESULT res = CoCreateInstance(&CLSID_SetupConfiguration, 0, CLSCTX_INPROC_SERVER, &IID_ISetupConfiguration, (void **) &setup_configuration);
#  endif

    if (res != S_OK)
    {
        return false;
    }

    IEnumSetupInstances *enum_setup_instances = 0;
    res = CALL1(setup_configuration, EnumInstances, &enum_setup_instances);

    if ((res != S_OK) || !enum_setup_instances)
    {
        CALL0(setup_configuration, Release);
        return false;
    }

    bool result = false;
    size_t public_used = c_make_memory_get_used(&_c_make_context.public_memory);

    for (;;)
    {
        ULONG count = 0;
        ISetupInstance *setup_instance = 0;
        res = CALL1(enum_setup_instances, Next, 1, &setup_instance, &count);

        if (res != S_OK)
        {
            break;
        }

        BSTR installation_path_bstr;
        res = CALL1(setup_instance, GetInstallationPath, &installation_path_bstr);

        if (res != S_OK)
        {
            CALL0(setup_instance, Release);
            continue;
        }

        size_t installation_path_size = *((DWORD *) installation_path_bstr - 1);
        char *installation_path = (char *) c_make_memory_allocate(&_c_make_context.public_memory, 2 * (installation_path_size + 1));
        int ret = WideCharToMultiByte(CP_UTF8, 0, installation_path_bstr, installation_path_size, installation_path, 2 * (installation_path_size + 1), 0, 0);
        installation_path[ret] = 0;
        SysFreeString(installation_path_bstr);

        CALL0(setup_instance, Release);

        const char *version_file_path = c_make_c_string_path_concat(installation_path, "VC", "Auxiliary", "Build", "Microsoft.VCToolsVersion.default.txt");
        CMakeString version_file_content;
        c_make_read_entire_file(version_file_path, &version_file_content);

        CMakeString version_str = c_make_string_split_left(&version_file_content, '\n');
        char *version = c_make_string_to_c_string(&_c_make_context.public_memory, c_make_string_trim(version_str));

        visual_studio_install->version = version;
        visual_studio_install->root_path = installation_path;

        result = true;
        break;
    }

    if (!result)
    {
        c_make_memory_set_used(&_c_make_context.public_memory, public_used);
    }

    CALL0(enum_setup_instances, Release);
    CALL0(setup_configuration, Release);

    return result;

#  undef CALL0
#  undef CALL1

#elif C_MAKE_PLATFORM_ANDROID || C_MAKE_PLATFORM_FREEBSD || C_MAKE_PLATFORM_LINUX || C_MAKE_PLATFORM_MACOS
    (void) visual_studio_install;
    return false;
#endif
}

C_MAKE_DEF bool
c_make_find_windows_sdk(CMakeWindowsSoftwarePackage *windows_sdk)
{
#if C_MAKE_PLATFORM_WINDOWS
    HKEY installed_roots_key;

    LSTATUS res = RegOpenKeyEx(HKEY_LOCAL_MACHINE, L"SOFTWARE\\Microsoft\\Windows Kits\\Installed Roots", 0,
                               KEY_QUERY_VALUE | KEY_WOW64_32KEY | KEY_ENUMERATE_SUB_KEYS, &installed_roots_key);

    if (res != ERROR_SUCCESS)
    {
        return false;
    }

    DWORD root_length;
    res = RegGetValue(installed_roots_key, 0, L"KitsRoot10", RRF_RT_REG_SZ, 0, 0, &root_length);

    if (res != ERROR_SUCCESS)
    {
        RegCloseKey(installed_roots_key);
        return false;
    }

    size_t private_used = c_make_memory_get_used(&_c_make_context.private_memory);

    wchar_t *windows_sdk_root_path = (wchar_t *) c_make_memory_allocate(&_c_make_context.private_memory, root_length);

    res = RegGetValue(installed_roots_key, 0, L"KitsRoot10", RRF_RT_REG_SZ, 0, windows_sdk_root_path, &root_length);

    if (res != ERROR_SUCCESS)
    {
        c_make_memory_set_used(&_c_make_context.private_memory, private_used);
        RegCloseKey(installed_roots_key);
        return false;
    }

    size_t include_length = sizeof(L"Include\\*") - 2;
    wchar_t *include_path = (wchar_t *) c_make_memory_allocate(&_c_make_context.private_memory, root_length + include_length);

    char *dst = (char *) include_path;
    char *src = (char *) windows_sdk_root_path;

    for (size_t i = 0; i < (root_length - 2); i += 1)
    {
        *dst++ = *src++;
    }

    src = (char *) L"Include\\*";

    for (size_t i = 0; i < include_length; i += 1)
    {
        *dst++ = *src++;
    }

    *dst++ = 0;
    *dst++ = 0;

    size_t public_used = c_make_memory_get_used(&_c_make_context.public_memory);

    const char *best_version = 0;
    int best_v0 = 0, best_v1 = 0, best_v2 = 0, best_v3 = 0;

    WIN32_FIND_DATA entry = { 0 };
    HANDLE directory = FindFirstFile(include_path, &entry);

    if (directory != INVALID_HANDLE_VALUE)
    {
        for (;;)
        {
            if ((entry.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) && (entry.cFileName[0] != '.'))
            {
                int v0, v1, v2, v3;
                int ret = swscanf_s(entry.cFileName, L"%d.%d.%d.%d", &v0, &v1, &v2, &v3);

                if ((ret == 4) && ((v0 > best_v0) || ((v0 == best_v0) && ((v1 > best_v1) || ((v1 == best_v1) && ((v2 > best_v2) || ((v2 == best_v2) && (v3 > best_v3))))))))
                {
                    c_make_memory_set_used(&_c_make_context.public_memory, public_used);
                    best_v0 = v0;
                    best_v1 = v1;
                    best_v2 = v2;
                    best_v3 = v3;
                    best_version = c_make_c_string_utf16_to_utf8(&_c_make_context.public_memory, entry.cFileName, wcslen(entry.cFileName));
                }
            }

            if (!FindNextFile(directory, &entry))
            {
                FindClose(directory);
                break;
            }
        }
    }

    RegCloseKey(installed_roots_key);
    c_make_memory_set_used(&_c_make_context.private_memory, private_used);

    if (!best_version)
    {
        return false;
    }

    windows_sdk->root_path = c_make_c_string_utf16_to_utf8(&_c_make_context.public_memory, windows_sdk_root_path, (root_length - 1) / 2);
    windows_sdk->version = best_version;

    return true;
#elif C_MAKE_PLATFORM_ANDROID || C_MAKE_PLATFORM_FREEBSD || C_MAKE_PLATFORM_LINUX || C_MAKE_PLATFORM_MACOS
    (void) windows_sdk;
    return false;
#endif
}

C_MAKE_DEF bool
c_make_get_visual_studio(CMakeWindowsSoftwarePackage *visual_studio_install)
{
#if C_MAKE_PLATFORM_WINDOWS
    CMakeConfigValue visual_studio_version = c_make_config_get("visual_studio_version");
    CMakeConfigValue visual_studio_root_path = c_make_config_get("visual_studio_root_path");

    if (visual_studio_version.is_valid && visual_studio_root_path.is_valid)
    {
        visual_studio_install->version = visual_studio_version.val;
        visual_studio_install->root_path = visual_studio_root_path.val;
        return true;
    }
    else
    {
        return c_make_find_visual_studio(visual_studio_install);
    }
#elif C_MAKE_PLATFORM_ANDROID || C_MAKE_PLATFORM_FREEBSD || C_MAKE_PLATFORM_LINUX || C_MAKE_PLATFORM_MACOS
    (void) visual_studio_install;
    return false;
#endif
}

C_MAKE_DEF bool
c_make_get_windows_sdk(CMakeWindowsSoftwarePackage *windows_sdk)
{
#if C_MAKE_PLATFORM_WINDOWS
    CMakeConfigValue windows_sdk_version = c_make_config_get("windows_sdk_version");
    CMakeConfigValue windows_sdk_root_path = c_make_config_get("windows_sdk_root_path");

    if (windows_sdk_version.is_valid && windows_sdk_root_path.is_valid)
    {
        windows_sdk->version = windows_sdk_version.val;
        windows_sdk->root_path = windows_sdk_root_path.val;
        return true;
    }
    else
    {
        return c_make_find_windows_sdk(windows_sdk);
    }
#elif C_MAKE_PLATFORM_ANDROID || C_MAKE_PLATFORM_FREEBSD || C_MAKE_PLATFORM_LINUX || C_MAKE_PLATFORM_MACOS
    (void) windows_sdk;
    return false;
#endif
}

C_MAKE_DEF const char *
c_make_get_msvc_library_manager(CMakeArchitecture target_architecture)
{
    const char *result = 0;
    CMakeWindowsSoftwarePackage visual_studio_install;

    if (c_make_get_visual_studio(&visual_studio_install))
    {
        const char *arch = "x64";

        if (target_architecture == CMakeArchitectureAarch64)
        {
            arch = "arm64";
        }

        result = c_make_c_string_path_concat(visual_studio_install.root_path, "VC", "Tools", "MSVC",
                                             visual_studio_install.version, "bin", "Hostx64", arch, "lib.exe");
    }

    return result;
}

C_MAKE_DEF const char *
c_make_get_msvc_compiler(CMakeArchitecture target_architecture)
{
    const char *result = 0;
    CMakeWindowsSoftwarePackage visual_studio_install;

    if (c_make_get_visual_studio(&visual_studio_install))
    {
        const char *arch = "x64";

        if (target_architecture == CMakeArchitectureAarch64)
        {
            arch = "arm64";
        }

        result = c_make_c_string_path_concat(visual_studio_install.root_path, "VC", "Tools", "MSVC",
                                             visual_studio_install.version, "bin", "Hostx64", arch, "cl.exe");
    }

    return result;
}

C_MAKE_DEF void
c_make_command_append_msvc_compiler_flags(CMakeCommand *command)
{
    CMakeWindowsSoftwarePackage visual_studio_install;

    if (c_make_get_visual_studio(&visual_studio_install))
    {
        c_make_command_append(command,
            c_make_c_string_concat("-I",
                c_make_c_string_path_concat(visual_studio_install.root_path, "VC", "Tools", "MSVC",
                                            visual_studio_install.version, "include")));
    }

    CMakeWindowsSoftwarePackage windows_sdk;

    if (c_make_get_windows_sdk(&windows_sdk))
    {
        c_make_command_append(command,
            c_make_c_string_concat("-I",
                c_make_c_string_path_concat(windows_sdk.root_path, "Include", windows_sdk.version, "ucrt")));
        c_make_command_append(command,
            c_make_c_string_concat("-I",
                c_make_c_string_path_concat(windows_sdk.root_path, "Include", windows_sdk.version, "shared")));
        c_make_command_append(command,
            c_make_c_string_concat("-I",
                c_make_c_string_path_concat(windows_sdk.root_path, "Include", windows_sdk.version, "um")));
        c_make_command_append(command,
            c_make_c_string_concat("-I",
                c_make_c_string_path_concat(windows_sdk.root_path, "Include", windows_sdk.version, "winrt")));
        c_make_command_append(command,
            c_make_c_string_concat("-I",
                c_make_c_string_path_concat(windows_sdk.root_path, "Include", windows_sdk.version, "cppwinrt")));
    }
}

C_MAKE_DEF void
c_make_command_append_msvc_linker_flags(CMakeCommand *command, CMakeArchitecture target_architecture)
{
    const char *arch = "x64";

    if (target_architecture == CMakeArchitectureAarch64)
    {
        arch = "arm64";
    }

    CMakeWindowsSoftwarePackage visual_studio_install;

    if (c_make_get_visual_studio(&visual_studio_install))
    {
        c_make_command_append(command,
            c_make_c_string_concat("-libpath:",
                c_make_c_string_path_concat(visual_studio_install.root_path, "VC", "Tools", "MSVC",
                                            visual_studio_install.version, "lib", arch)));
    }

    CMakeWindowsSoftwarePackage windows_sdk;

    if (c_make_get_windows_sdk(&windows_sdk))
    {
        c_make_command_append(command,
            c_make_c_string_concat("-libpath:",
                c_make_c_string_path_concat(windows_sdk.root_path, "Lib", windows_sdk.version, "ucrt", arch)));
        c_make_command_append(command,
            c_make_c_string_concat("-libpath:",
                c_make_c_string_path_concat(windows_sdk.root_path, "Lib", windows_sdk.version, "um", arch)));
    }
}

C_MAKE_DEF void
c_make_config_set(const char *_key, const char *value)
{
    CMakeConfigEntry *entry = 0;
    CMakeString key = CMakeCString(_key);

    for (size_t i = 0; i < _c_make_context.config.count; i += 1)
    {
        CMakeConfigEntry *config_entry = _c_make_context.config.items + i;

        if (c_make_strings_are_equal(config_entry->key, key))
        {
            entry = config_entry;
            break;
        }
    }

    if (!entry)
    {
        if (_c_make_context.config.count == _c_make_context.config.allocated)
        {
            size_t old_count = _c_make_context.config.allocated;
            _c_make_context.config.allocated += 16;
            _c_make_context.config.items =
                (CMakeConfigEntry *) c_make_memory_reallocate(&_c_make_context.private_memory,
                                                              _c_make_context.config.items,
                                                              old_count * sizeof(*_c_make_context.config.items),
                                                              _c_make_context.config.allocated * sizeof(*_c_make_context.config.items));
        }

        entry = _c_make_context.config.items + _c_make_context.config.count;
        _c_make_context.config.count += 1;

        entry->key = c_make_copy_string(&_c_make_context.private_memory, key);
    }

    entry->value = c_make_copy_string(&_c_make_context.private_memory, CMakeCString(value));

    if (c_make_strings_are_equal(entry->key, CMakeStringLiteral("target_platform")))
    {
        if (c_make_strings_are_equal(entry->value, CMakeStringLiteral("android")))
        {
            _c_make_context.target_platform = CMakePlatformAndroid;
        }
        else if (c_make_strings_are_equal(entry->value, CMakeStringLiteral("freebsd")))
        {
            _c_make_context.target_platform = CMakePlatformFreeBsd;
        }
        else if (c_make_strings_are_equal(entry->value, CMakeStringLiteral("windows")))
        {
            _c_make_context.target_platform = CMakePlatformWindows;
        }
        else if (c_make_strings_are_equal(entry->value, CMakeStringLiteral("linux")))
        {
            _c_make_context.target_platform = CMakePlatformLinux;
        }
        else if (c_make_strings_are_equal(entry->value, CMakeStringLiteral("macos")))
        {
            _c_make_context.target_platform = CMakePlatformMacOs;
        }
        else if (c_make_strings_are_equal(entry->value, CMakeStringLiteral("web")))
        {
            _c_make_context.target_platform = CMakePlatformWeb;
        }
    }
    else if (c_make_strings_are_equal(entry->key, CMakeStringLiteral("target_architecture")))
    {
        if (c_make_strings_are_equal(entry->value, CMakeStringLiteral("amd64")))
        {
            _c_make_context.target_architecture = CMakeArchitectureAmd64;
        }
        else if (c_make_strings_are_equal(entry->value, CMakeStringLiteral("aarch64")))
        {
            _c_make_context.target_architecture = CMakeArchitectureAarch64;
        }
        else if (c_make_strings_are_equal(entry->value, CMakeStringLiteral("riscv64")))
        {
            _c_make_context.target_architecture = CMakeArchitectureRiscv64;
        }
        else if (c_make_strings_are_equal(entry->value, CMakeStringLiteral("wasm32")))
        {
            _c_make_context.target_architecture = CMakeArchitectureWasm32;
        }
        else if (c_make_strings_are_equal(entry->value, CMakeStringLiteral("wasm64")))
        {
            _c_make_context.target_architecture = CMakeArchitectureWasm64;
        }
        else
        {
            _c_make_context.target_architecture = CMakeArchitectureUnknown;
        }
    }
    else if (c_make_strings_are_equal(entry->key, CMakeStringLiteral("build_type")))
    {
        if (c_make_strings_are_equal(entry->value, CMakeStringLiteral("debug")))
        {
            _c_make_context.build_type = CMakeBuildTypeDebug;
        }
        else if (c_make_strings_are_equal(entry->value, CMakeStringLiteral("reldebug")))
        {
            _c_make_context.build_type = CMakeBuildTypeRelDebug;
        }
        else if (c_make_strings_are_equal(entry->value, CMakeStringLiteral("release")))
        {
            _c_make_context.build_type = CMakeBuildTypeRelease;
        }
    }
}

C_MAKE_DEF CMakeConfigValue
c_make_config_get(const char *_key)
{
    CMakeConfigValue result = { false, 0 };
    CMakeString key = CMakeCString(_key);

    for (size_t i = 0; i < _c_make_context.config.count; i += 1)
    {
        CMakeConfigEntry *config_entry = _c_make_context.config.items + i;

        if (c_make_strings_are_equal(config_entry->key, key))
        {
            result.is_valid = true;
            result.val = config_entry->value.data;
            break;
        }
    }

    return result;
}

C_MAKE_DEF void
c_make_print_config(void)
{
    for (size_t i = 0; i < _c_make_context.config.count; i += 1)
    {
        CMakeConfigEntry *entry = _c_make_context.config.items + i;
        c_make_log(CMakeLogLevelRaw, "  + %" CMakeStringFmt " = \"%" CMakeStringFmt "\"\n",
                   CMakeStringArg(entry->key), CMakeStringArg(entry->value));
    }
}

C_MAKE_DEF bool
c_make_store_config(const char *file_name)
{
    size_t public_used = c_make_memory_get_used(&_c_make_context.public_memory);

    CMakeString config_string = CMakeStringLiteral("");

    for (size_t i = 0; i < _c_make_context.config.count; i += 1)
    {
        CMakeConfigEntry *entry = _c_make_context.config.items + i;

        config_string = c_make_string_concat(config_string, entry->key,
                                             CMakeStringLiteral(" = \""),
                                             entry->value, CMakeStringLiteral("\"\n"));
    }

    if (!c_make_write_entire_file(file_name, config_string))
    {
        c_make_log(CMakeLogLevelError, "could not write config file '%s'\n", file_name);
        c_make_memory_set_used(&_c_make_context.public_memory, public_used);
        return false;
    }

    c_make_memory_set_used(&_c_make_context.public_memory, public_used);

    return true;
}

C_MAKE_DEF bool
c_make_load_config(const char *file_name)
{
    CMakeString config_string = { 0, 0 };

    if (!c_make_read_entire_file(file_name, &config_string))
    {
        c_make_log(CMakeLogLevelError, "could not read config file '%s'\n", file_name);
        return false;
    }

    while (config_string.count)
    {
        CMakeString line = c_make_string_split_left(&config_string, '\n');
        CMakeString key = c_make_string_trim(c_make_string_split_left(&line, '='));
        CMakeString value = c_make_string_trim(line);

        if (key.count && value.count)
        {
            if (value.data[0] == '"')
            {
                value.count -= 1;
                value.data += 1;

                if (value.count && (value.data[value.count - 1] == '"'))
                {
                    value.count -= 1;
                }

                size_t public_used = c_make_memory_get_used(&_c_make_context.public_memory);

                c_make_config_set(c_make_string_to_c_string(&_c_make_context.public_memory, key),
                                  c_make_string_to_c_string(&_c_make_context.public_memory, value));

                c_make_memory_set_used(&_c_make_context.public_memory, public_used);
            }
            else
            {
                // not a string
            }
        }
    }

    return true;
}

C_MAKE_DEF bool
c_make_needs_rebuild(const char *output_file, size_t input_file_count, const char **input_files)
{
#if C_MAKE_PLATFORM_WINDOWS
    size_t private_used = c_make_memory_get_used(&_c_make_context.private_memory);

    LPWSTR utf16_file_name = c_make_c_string_utf8_to_utf16(&_c_make_context.private_memory, output_file);
    HANDLE file = CreateFile(utf16_file_name, GENERIC_READ, FILE_SHARE_READ, 0, OPEN_EXISTING, 0, 0);

    c_make_memory_set_used(&_c_make_context.private_memory, private_used);

    if (file == INVALID_HANDLE_VALUE)
    {
        if (GetLastError() == ERROR_FILE_NOT_FOUND)
        {
            return true;
        }

        return false;
    }

    FILETIME output_file_last_write_time;

    if (!GetFileTime(file, 0, 0, &output_file_last_write_time))
    {
        CloseHandle(file);
        return true;
    }

    CloseHandle(file);

    for (size_t i = 0; i < input_file_count; i += 1)
    {
        size_t private_used = c_make_memory_get_used(&_c_make_context.private_memory);

        utf16_file_name = c_make_c_string_utf8_to_utf16(&_c_make_context.private_memory, input_files[i]);
        file = CreateFile(utf16_file_name, GENERIC_READ, FILE_SHARE_READ, 0, OPEN_EXISTING, 0, 0);

        c_make_memory_set_used(&_c_make_context.private_memory, private_used);

        if (file == INVALID_HANDLE_VALUE)
        {
            return false;
        }

        FILETIME input_file_last_write_time;

        if (!GetFileTime(file, 0, 0, &input_file_last_write_time))
        {
            CloseHandle(file);
            return true;
        }

        CloseHandle(file);

        if (CompareFileTime(&output_file_last_write_time, &input_file_last_write_time) < 0)
        {
            return true;
        }
    }

    return false;
#elif C_MAKE_PLATFORM_ANDROID || C_MAKE_PLATFORM_FREEBSD || C_MAKE_PLATFORM_LINUX || C_MAKE_PLATFORM_MACOS
    struct stat stats;

    if (stat(output_file, &stats))
    {
        if (errno == ENOENT)
        {
            return true;
        }

        return false;
    }

    time_t output_file_last_write_time = stats.st_mtime;

    for (size_t i = 0; i < input_file_count; i += 1)
    {
        const char *input_file = input_files[i];

        if (!stat(input_file, &stats))
        {
            time_t input_file_last_write_time = stats.st_mtime;

            if (input_file_last_write_time > output_file_last_write_time)
            {
                return true;
            }
        }
    }

    return false;
#endif
}

C_MAKE_DEF bool
c_make_needs_rebuild_single_source(const char *output_file, const char *input_file)
{
    return c_make_needs_rebuild(output_file, 1, &input_file);
}

C_MAKE_DEF bool
c_make_file_exists(const char *file_name)
{
#if C_MAKE_PLATFORM_WINDOWS
    size_t private_used = c_make_memory_get_used(&_c_make_context.private_memory);

    LPWSTR utf16_file_name = c_make_c_string_utf8_to_utf16(&_c_make_context.private_memory, file_name);
    DWORD file_attributes = GetFileAttributes(utf16_file_name);

    c_make_memory_set_used(&_c_make_context.private_memory, private_used);

    return ((file_attributes != INVALID_FILE_ATTRIBUTES) && !(file_attributes & FILE_ATTRIBUTE_DIRECTORY)) ? true : false;
#elif C_MAKE_PLATFORM_ANDROID || C_MAKE_PLATFORM_FREEBSD || C_MAKE_PLATFORM_LINUX || C_MAKE_PLATFORM_MACOS
    bool result = false;
    struct stat stats;

    if (!stat(file_name, &stats))
    {
        result = S_ISREG(stats.st_mode) ? true : false;
    }

    return result;
#endif
}

C_MAKE_DEF bool
c_make_directory_exists(const char *directory_name)
{
#if C_MAKE_PLATFORM_WINDOWS
    size_t private_used = c_make_memory_get_used(&_c_make_context.private_memory);

    LPWSTR utf16_directory_name = c_make_c_string_utf8_to_utf16(&_c_make_context.private_memory, directory_name);
    DWORD file_attributes = GetFileAttributes(utf16_directory_name);

    c_make_memory_set_used(&_c_make_context.private_memory, private_used);

    return ((file_attributes != INVALID_FILE_ATTRIBUTES) && (file_attributes & FILE_ATTRIBUTE_DIRECTORY)) ? true : false;
#elif C_MAKE_PLATFORM_ANDROID || C_MAKE_PLATFORM_FREEBSD || C_MAKE_PLATFORM_LINUX || C_MAKE_PLATFORM_MACOS
    bool result = false;
    struct stat stats;

    if (!stat(directory_name, &stats))
    {
        result = S_ISDIR(stats.st_mode) ? true : false;
    }

    return result;
#endif
}

C_MAKE_DEF bool
c_make_create_directory(const char *directory_name)
{
#if C_MAKE_PLATFORM_WINDOWS
    size_t private_used = c_make_memory_get_used(&_c_make_context.private_memory);

    LPWSTR utf16_directory_name = c_make_c_string_utf8_to_utf16(&_c_make_context.private_memory, directory_name);

    if (!CreateDirectory(utf16_directory_name, 0))
    {
        c_make_memory_set_used(&_c_make_context.private_memory, private_used);

        if (GetLastError() == ERROR_ALREADY_EXISTS)
        {
            return true;
        }

        return false;
    }

    c_make_memory_set_used(&_c_make_context.private_memory, private_used);

    return true;
#elif C_MAKE_PLATFORM_ANDROID || C_MAKE_PLATFORM_FREEBSD || C_MAKE_PLATFORM_LINUX || C_MAKE_PLATFORM_MACOS
    if (!mkdir(directory_name, 0775))
    {
        return true;
    }

    if (errno == EEXIST)
    {
        return true;
    }

    return false;
#endif
}

C_MAKE_DEF bool
c_make_create_directory_recursively(const char *directory_name)
{
#if C_MAKE_PLATFORM_WINDOWS
    size_t private_used = c_make_memory_get_used(&_c_make_context.private_memory);

    LPWSTR utf16_directory_name = c_make_c_string_utf8_to_utf16(&_c_make_context.private_memory, directory_name);
    DWORD file_attributes = GetFileAttributes(utf16_directory_name);

    if ((file_attributes == INVALID_FILE_ATTRIBUTES) || !(file_attributes & FILE_ATTRIBUTE_DIRECTORY))
    {
        CMakeString parent_string = CMakeCString(directory_name);
        c_make_string_split_right_path_separator(&parent_string);

        if (parent_string.count > 0)
        {
            const char *parent_directory = c_make_string_to_c_string(&_c_make_context.private_memory, parent_string);

            if (!c_make_create_directory_recursively(parent_directory))
            {
                c_make_memory_set_used(&_c_make_context.private_memory, private_used);
                return false;
            }
        }

        if (!CreateDirectory(utf16_directory_name, 0) && (GetLastError() != ERROR_ALREADY_EXISTS))
        {
            c_make_memory_set_used(&_c_make_context.private_memory, private_used);
            return false;
        }
    }

    c_make_memory_set_used(&_c_make_context.private_memory, private_used);
#elif C_MAKE_PLATFORM_ANDROID || C_MAKE_PLATFORM_FREEBSD || C_MAKE_PLATFORM_LINUX || C_MAKE_PLATFORM_MACOS
    struct stat stats;

    if (stat(directory_name, &stats) || S_ISDIR(stats.st_mode))
    {
        CMakeString parent_string = CMakeCString(directory_name);
        c_make_string_split_right_path_separator(&parent_string);

        if (parent_string.count > 0)
        {
            size_t private_used = c_make_memory_get_used(&_c_make_context.private_memory);

            const char *parent_directory = c_make_string_to_c_string(&_c_make_context.private_memory, parent_string);
            bool result = c_make_create_directory_recursively(parent_directory);

            c_make_memory_set_used(&_c_make_context.private_memory, private_used);

            if (!result)
            {
                return false;
            }
        }

        if (mkdir(directory_name, 0775) && (errno != EEXIST))
        {
            return false;
        }
    }
#endif

    return true;
}

C_MAKE_DEF bool
c_make_read_entire_file(const char *file_name, CMakeString *content)
{
#if C_MAKE_PLATFORM_WINDOWS
    size_t private_used = c_make_memory_get_used(&_c_make_context.private_memory);

    LPWSTR utf16_file_name = c_make_c_string_utf8_to_utf16(&_c_make_context.private_memory, file_name);
    HANDLE file = CreateFile(utf16_file_name, GENERIC_READ, FILE_SHARE_READ, 0, OPEN_EXISTING, 0, 0);

    c_make_memory_set_used(&_c_make_context.private_memory, private_used);

    if (file == INVALID_HANDLE_VALUE)
    {
        return false;
    }

    LARGE_INTEGER file_size;

    if (!GetFileSizeEx(file, &file_size))
    {
        CloseHandle(file);
        return false;
    }

    size_t public_used = c_make_memory_get_used(&_c_make_context.public_memory);

    content->count = file_size.QuadPart;
    content->data = (char *) c_make_memory_allocate(&_c_make_context.public_memory, content->count + 1);

    size_t index = 0;

    while (index < content->count)
    {
        DWORD bytes_read = 0;
        if (!ReadFile(file, content->data + index, content->count - index, &bytes_read, 0))
        {
            c_make_memory_set_used(&_c_make_context.public_memory, public_used);
            CloseHandle(file);
            return false;
        }

        index += bytes_read;
    }

    content->data[content->count] = 0;

    CloseHandle(file);
    return true;
#elif C_MAKE_PLATFORM_ANDROID || C_MAKE_PLATFORM_FREEBSD || C_MAKE_PLATFORM_LINUX || C_MAKE_PLATFORM_MACOS
    int fd = open(file_name, O_RDONLY);

    if (fd >= 0)
    {
        struct stat stats;

        if (fstat(fd, &stats) < 0)
        {
            close(fd);
            return false;
        }

        size_t public_used = c_make_memory_get_used(&_c_make_context.public_memory);

        content->count = stats.st_size;
        content->data = (char *) c_make_memory_allocate(&_c_make_context.public_memory, content->count + 1);

        size_t index = 0;

        while (index < content->count)
        {
            ssize_t read_bytes = read(fd, content->data + index, content->count - index);

            if (read_bytes < 0)
            {
                c_make_memory_set_used(&_c_make_context.public_memory, public_used);
                close(fd);
                return false;
            }

            index += read_bytes;
        }

        content->data[content->count] = 0;

        close(fd);
        return true;
    }
    else
    {
        return false;
    }
#endif
}

C_MAKE_DEF bool
c_make_write_entire_file(const char *file_name, CMakeString content)
{
#if C_MAKE_PLATFORM_WINDOWS
    size_t private_used = c_make_memory_get_used(&_c_make_context.private_memory);

    LPWSTR utf16_file_name = c_make_c_string_utf8_to_utf16(&_c_make_context.private_memory, file_name);
    HANDLE file = CreateFile(utf16_file_name, GENERIC_WRITE, 0, 0,
                             CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, 0);

    c_make_memory_set_used(&_c_make_context.private_memory, private_used);

    if (file == INVALID_HANDLE_VALUE)
    {
        return false;
    }

    while (content.count)
    {
        DWORD bytes_written = 0;
        if (!WriteFile(file, content.data, content.count, &bytes_written, 0))
        {
            CloseHandle(file);
            return false;
        }

        content.data += bytes_written;
        content.count -= bytes_written;
    }

    CloseHandle(file);
    return true;
#elif C_MAKE_PLATFORM_ANDROID || C_MAKE_PLATFORM_FREEBSD || C_MAKE_PLATFORM_LINUX || C_MAKE_PLATFORM_MACOS
    int fd = open(file_name, O_WRONLY | O_TRUNC | O_CREAT, 0664);

    if (fd >= 0)
    {
        while (content.count)
        {
            ssize_t written_bytes = write(fd, content.data, content.count);

            if (written_bytes < 0)
            {
                close(fd);
                return false;
            }

            content.data += written_bytes;
            content.count -= written_bytes;
        }

        close(fd);
        return true;
    }
    else
    {
        return false;
    }
#endif
}

C_MAKE_DEF bool
c_make_copy_file(const char *src_file_name, const char *dst_file_name)
{
#if C_MAKE_PLATFORM_WINDOWS
    size_t private_used = c_make_memory_get_used(&_c_make_context.private_memory);

    LPWSTR utf16_src_file_name = c_make_c_string_utf8_to_utf16(&_c_make_context.private_memory, src_file_name);
    LPWSTR utf16_dst_file_name = c_make_c_string_utf8_to_utf16(&_c_make_context.private_memory, dst_file_name);

    if (!CopyFile(utf16_src_file_name, utf16_dst_file_name, FALSE))
    {
        c_make_memory_set_used(&_c_make_context.private_memory, private_used);
        return false;
    }

    c_make_memory_set_used(&_c_make_context.private_memory, private_used);

    return true;
#elif C_MAKE_PLATFORM_ANDROID || C_MAKE_PLATFORM_FREEBSD || C_MAKE_PLATFORM_LINUX || C_MAKE_PLATFORM_MACOS
    int src_fd = open(src_file_name, O_RDONLY);

    if (src_fd < 0)
    {
        c_make_log(CMakeLogLevelError, "could not open file '%s': %s\n", src_file_name, strerror(errno));
        return false;
    }

    struct stat stats;

    if (fstat(src_fd, &stats) < 0)
    {
        c_make_log(CMakeLogLevelError, "could not get file stats on '%s': %s\n", src_file_name, strerror(errno));
        close(src_fd);
        return false;
    }

    size_t src_file_size = stats.st_size;

    int dst_fd = open(dst_file_name, O_WRONLY | O_TRUNC | O_CREAT, stats.st_mode);

    if (dst_fd < 0)
    {
        c_make_log(CMakeLogLevelError, "could not create file '%s': %s\n", dst_file_name, strerror(errno));
        close(src_fd);
        return false;
    }

    size_t private_used = c_make_memory_get_used(&_c_make_context.private_memory);

    void *copy_buffer = c_make_memory_allocate(&_c_make_context.private_memory, 4096);

    size_t index = 0;

    while (index < src_file_size)
    {
        size_t remaining_size = src_file_size - index;
        size_t size_to_read = 4096;

        if (remaining_size < size_to_read)
        {
            size_to_read = remaining_size;
        }

        ssize_t read_bytes = read(src_fd, copy_buffer, size_to_read);

        if ((read_bytes < 0) || ((size_t) read_bytes != size_to_read))
        {
            c_make_memory_set_used(&_c_make_context.private_memory, private_used);
            close(src_fd);
            close(dst_fd);
            return false;
        }

        write(dst_fd, copy_buffer, size_to_read);
        index += size_to_read;
    }

    c_make_memory_set_used(&_c_make_context.private_memory, private_used);

    close(dst_fd);
    close(src_fd);
    return true;
#endif
}

C_MAKE_DEF bool
c_make_rename_file(const char *old_file_name, const char *new_file_name)
{
#if C_MAKE_PLATFORM_WINDOWS
    size_t private_used = c_make_memory_get_used(&_c_make_context.private_memory);

    LPWSTR utf16_old_file_name = c_make_c_string_utf8_to_utf16(&_c_make_context.private_memory, old_file_name);
    LPWSTR utf16_new_file_name = c_make_c_string_utf8_to_utf16(&_c_make_context.private_memory, new_file_name);

    bool result =
        MoveFileEx(utf16_old_file_name, utf16_new_file_name, MOVEFILE_REPLACE_EXISTING) ? true : false;

    c_make_memory_set_used(&_c_make_context.private_memory, private_used);

    return result;
#elif C_MAKE_PLATFORM_ANDROID || C_MAKE_PLATFORM_FREEBSD || C_MAKE_PLATFORM_LINUX || C_MAKE_PLATFORM_MACOS
    if (rename(old_file_name, new_file_name))
    {
        return false;
    }

    return true;
#endif
}

C_MAKE_DEF bool
c_make_delete_file(const char *file_name)
{
#if C_MAKE_PLATFORM_WINDOWS
    size_t private_used = c_make_memory_get_used(&_c_make_context.private_memory);

    LPWSTR utf16_file_name = c_make_c_string_utf8_to_utf16(&_c_make_context.private_memory, file_name);

    bool result = DeleteFile(utf16_file_name) ? true : false;

    if (!result && (GetLastError() == ERROR_FILE_NOT_FOUND))
    {
        result = true;
    }

    c_make_memory_set_used(&_c_make_context.private_memory, private_used);

    return result;
#elif C_MAKE_PLATFORM_ANDROID || C_MAKE_PLATFORM_FREEBSD || C_MAKE_PLATFORM_LINUX || C_MAKE_PLATFORM_MACOS
    if (unlink(file_name))
    {
        if (errno == ENOENT)
        {
            return true;
        }

        return false;
    }

    return true;
#endif
}

C_MAKE_DEF bool
c_make_has_slash_or_backslash(const char *path)
{
    if (path)
    {
        while (*path)
        {
            if ((*path == '/') || (*path == '\\'))
            {
                return true;
            }

            path += 1;
        }

        return false;
    }
    else
    {
        return false;
    }
}

C_MAKE_DEF CMakeString
c_make_get_environment_variable(CMakeMemory *memory, const char *variable_name)
{
    CMakeString result = { 0, 0 };

#if C_MAKE_PLATFORM_WINDOWS
    wchar_t *utf16_variable_name = c_make_c_string_utf8_to_utf16(memory, variable_name);
    DWORD variable_size = GetEnvironmentVariable(utf16_variable_name, 0, 0);

    if (variable_size > 0)
    {
        size_t memory_used = c_make_memory_get_used(memory);
        wchar_t *variable = (wchar_t *) c_make_memory_allocate(memory, sizeof(wchar_t) * variable_size);
        DWORD ret = GetEnvironmentVariable(utf16_variable_name, variable, variable_size);

        if ((ret > 0) && (ret < variable_size))
        {
            result = c_make_string_utf16_to_utf8(memory, variable, ret);
        }
        else
        {
            c_make_memory_set_used(memory, memory_used);
        }
    }
#elif C_MAKE_PLATFORM_ANDROID || C_MAKE_PLATFORM_FREEBSD || C_MAKE_PLATFORM_LINUX || C_MAKE_PLATFORM_MACOS
    (void) memory;

    char *variable = getenv(variable_name);

    if (variable)
    {
        result = CMakeCString(variable);
    }
#endif

    return result;
}

C_MAKE_DEF const char *
c_make_find_program(const char *program_name)
{
    size_t private_used = c_make_memory_get_used(&_c_make_context.private_memory);
    CMakeString paths = c_make_get_environment_variable(&_c_make_context.private_memory, "PATH");

    size_t public_used = c_make_memory_get_used(&_c_make_context.public_memory);
    size_t inner_private_used = c_make_memory_get_used(&_c_make_context.private_memory);

    while (paths.count)
    {
#if C_MAKE_PLATFORM_WINDOWS
        char *path = c_make_string_to_c_string(&_c_make_context.private_memory, c_make_string_split_left(&paths, ';'));
#elif C_MAKE_PLATFORM_ANDROID || C_MAKE_PLATFORM_FREEBSD || C_MAKE_PLATFORM_LINUX || C_MAKE_PLATFORM_MACOS
        char *path = c_make_string_to_c_string(&_c_make_context.private_memory, c_make_string_split_left(&paths, ':'));
#endif

        char *full_path = c_make_c_string_path_concat(path, program_name);

        c_make_memory_set_used(&_c_make_context.private_memory, inner_private_used);

        // TODO: is executable?
        if (c_make_file_exists(full_path))
        {
            c_make_memory_set_used(&_c_make_context.private_memory, private_used);
            return full_path;
        }

        c_make_memory_set_used(&_c_make_context.public_memory, public_used);
    }

    c_make_memory_set_used(&_c_make_context.private_memory, private_used);

    return 0;
}

C_MAKE_DEF CMakeString
c_make_string_concat_va(size_t count, ...)
{
    CMakeString result = { 0, 0 };

    va_list args;
    va_start(args, count);

    for (size_t i = 0; i < count; i += 1)
    {
        CMakeString str = va_arg(args, CMakeString);
        result.count += str.count;
    }

    va_end(args);

    result.data = (char *) c_make_memory_allocate(&_c_make_context.public_memory, result.count);
    char *dst = result.data;

    va_start(args, count);

    for (size_t i = 0; i < count; i += 1)
    {
        CMakeString str = va_arg(args, CMakeString);

        for (size_t j = 0; j < str.count; j += 1)
        {
            *dst++ = str.data[j];
        }
    }

    va_end(args);

    return result;
}

C_MAKE_DEF char *
c_make_c_string_concat_va(size_t count, ...)
{
    size_t length = 1;

    va_list args;
    va_start(args, count);

    for (size_t i = 0; i < count; i += 1)
    {
        const char *str = va_arg(args, const char *);
        length += c_make_get_c_string_length(str);
    }

    va_end(args);

    char *result = (char *) c_make_memory_allocate(&_c_make_context.public_memory, length);
    char *dst = result;

    va_start(args, count);

    for (size_t i = 0; i < count; i += 1)
    {
        const char *str = va_arg(args, const char *);
        while (*str) *dst++ = *str++;
    }

    va_end(args);

    *dst = 0;

    return result;
}

C_MAKE_DEF char *
c_make_c_string_path_concat_va(size_t count, ...)
{
    size_t length = 0;

    va_list args;
    va_start(args, count);

    for (size_t i = 0; i < count; i += 1)
    {
        const char *str = va_arg(args, const char *);
        length += c_make_get_c_string_length(str) + 1;
    }

    va_end(args);

    char *result = (char *) c_make_memory_allocate(&_c_make_context.public_memory, length);
    char *dst = result;

    va_start(args, count);

    for (size_t i = 0; i < count; i += 1)
    {
        const char *str = va_arg(args, const char *);
        while (*str) *dst++ = *str++;

#if C_MAKE_PLATFORM_WINDOWS
        *dst++ = '\\';
#else
        *dst++ = '/';
#endif
    }

    va_end(args);

    *--dst = 0;

    return result;
}

static size_t
__c_make_process_wait(CMakeProcessId process_id)
{
    if (process_id == CMakeInvalidProcessId)
    {
        _c_make_context.did_fail = true;
        return -1;
    }

    size_t index = _c_make_context.process_group.count;

    for (size_t i = 0; i < _c_make_context.process_group.count; i += 1)
    {
        if (_c_make_context.process_group.items[i].id == process_id)
        {
            index = i;
            break;
        }
    }

    if (index < _c_make_context.process_group.count)
    {
        CMakeProcess *process = _c_make_context.process_group.items + index;

        if (!process->exited)
        {
            process->exited = true;

#if C_MAKE_PLATFORM_WINDOWS
            DWORD wait_result = WaitForSingleObject(process_id, INFINITE);

            if (wait_result == WAIT_FAILED)
            {
                _c_make_context.did_fail = true;
                process->succeeded = false;
                // TODO: log error
            }
            else
            {
                DWORD exit_code = 0;

                if (!GetExitCodeProcess(process_id, &exit_code))
                {
                    _c_make_context.did_fail = true;
                    process->succeeded = false;
                    // TODO: log error
                }

                if (exit_code != 0)
                {
                    _c_make_context.did_fail = true;
                    process->succeeded = false;
                    // TODO: log that the process has exited with an error code
                    // TODO: What to return here? false or true?
                }

                CloseHandle(process_id);
            }
#else
            for (;;)
            {
                int status;

                if (waitpid(process_id, &status, 0) < 0)
                {
                    _c_make_context.did_fail = true;
                    process->succeeded = false;
                    // TODO: log error
                    break;
                }

                if (WIFEXITED(status))
                {
                    int exit_code = WEXITSTATUS(status);

                    if (exit_code != 0)
                    {
                        _c_make_context.did_fail = true;
                        process->succeeded = false;
                        // TODO: log that the process has exited with an error code
                        // TODO: What to return here? false or true?
                    }

                    break;
                }

                if (WIFSIGNALED(status))
                {
                    _c_make_context.did_fail = true;
                    process->succeeded = false;
                    // TODO: log the signal that terminated the child
                    break;
                }
            }
#endif
        }
    }

    return index;
}

C_MAKE_DEF CMakeProcessId
c_make_command_run(CMakeCommand command)
{
    if (command.count == 0)
    {
        return CMakeInvalidProcessId;
    }

    if (_c_make_context.verbose)
    {
        size_t public_used = c_make_memory_get_used(&_c_make_context.public_memory);

        CMakeString command_string = c_make_command_to_string(command);
        c_make_log(CMakeLogLevelRaw, "%" CMakeStringFmt "\n", CMakeStringArg(command_string));

        c_make_memory_set_used(&_c_make_context.public_memory, public_used);
    }

    CMakeProcessId process_id;

#if C_MAKE_PLATFORM_WINDOWS
    STARTUPINFO start_info = { 0 };
    start_info.cb = sizeof(start_info);
    start_info.hStdError  = GetStdHandle(STD_ERROR_HANDLE);
    start_info.hStdOutput = GetStdHandle(STD_OUTPUT_HANDLE);
    start_info.hStdInput  = GetStdHandle(STD_INPUT_HANDLE);
    start_info.dwFlags    = STARTF_USESTDHANDLES;

    PROCESS_INFORMATION process_info = { 0 };

    size_t public_used = c_make_memory_get_used(&_c_make_context.public_memory);
    size_t private_used = c_make_memory_get_used(&_c_make_context.private_memory);

    CMakeString command_line = c_make_command_to_string(command);

    int wide_command_line_size = 2 * command_line.count;
    LPWSTR wide_command_line = (LPWSTR) c_make_memory_allocate(&_c_make_context.private_memory, wide_command_line_size);

    int wide_count = MultiByteToWideChar(CP_UTF8, 0, command_line.data, command_line.count, wide_command_line, wide_command_line_size);
    wide_command_line[wide_count] = 0;

    BOOL result = CreateProcess(0, wide_command_line, 0, 0, TRUE, 0, 0, 0, &start_info, &process_info);

    c_make_memory_set_used(&_c_make_context.private_memory, private_used);
    c_make_memory_set_used(&_c_make_context.public_memory, public_used);

    if (!result)
    {
        size_t public_used = c_make_memory_get_used(&_c_make_context.public_memory);

        CMakeString command_string = c_make_command_to_string(command);
        c_make_log(CMakeLogLevelError, "could not run command (GetLastError = %lu): %" CMakeStringFmt "\n", GetLastError(), CMakeStringArg(command_string));

        c_make_memory_set_used(&_c_make_context.public_memory, public_used);
        return CMakeInvalidProcessId;
    }

    CloseHandle(process_info.hThread);

    process_id = process_info.hProcess;
#else
    char **command_line = (char **) c_make_memory_allocate(&_c_make_context.private_memory, (command.count + 1) * sizeof(char *));

    for (size_t i = 0; i < command.count; i += 1)
    {
        command_line[i] = (char *) command.items[i];
    }

    command_line[command.count] = 0;

    pid_t pid = fork();

    if (pid < 0)
    {
        // TODO: log error
        fprintf(stderr, "Could not fork\n");
        return CMakeInvalidProcessId;
    }

    if (pid == 0)
    {
        int ret = execvp(command_line[0], command_line);

        if (ret < 0)
        {
            // TODO: log error
            fprintf(stderr, "Could not execvp: %s\n", strerror(errno));
            exit(1);
        }

        /* unreachable */
        return CMakeInvalidProcessId;
    }

    process_id = pid;
#endif

    if (_c_make_context.process_group.count == _c_make_context.process_group.allocated)
    {
        size_t old_count = _c_make_context.process_group.allocated;
        _c_make_context.process_group.allocated += 16;
        _c_make_context.process_group.items =
            (CMakeProcess *) c_make_memory_reallocate(&_c_make_context.private_memory,
                                                      _c_make_context.process_group.items,
                                                      old_count * sizeof(*_c_make_context.process_group.items),
                                                      _c_make_context.process_group.allocated * sizeof(*_c_make_context.process_group.items));
    }

    CMakeProcess *process = _c_make_context.process_group.items + _c_make_context.process_group.count;
    _c_make_context.process_group.count += 1;

    process->id = process_id;
    process->exited = false;
    process->succeeded = true;

    if (_c_make_context.sequential)
    {
        __c_make_process_wait(process_id);
    }

    return process_id;
}

C_MAKE_DEF CMakeProcessId
c_make_command_run_and_reset(CMakeCommand *command)
{
    CMakeProcessId process_id = c_make_command_run(*command);
    command->count = 0;
    return process_id;
}

C_MAKE_DEF bool
c_make_process_wait(CMakeProcessId process_id)
{
    size_t index = __c_make_process_wait(process_id);

    if (index < _c_make_context.process_group.count)
    {
        CMakeProcess *process = _c_make_context.process_group.items + index;

        assert(process->exited);
        bool succeeded = process->succeeded;

        _c_make_context.process_group.count -= 1;
        _c_make_context.process_group.items[index] = _c_make_context.process_group.items[_c_make_context.process_group.count];

        return succeeded;
    }

    return false;
}

C_MAKE_DEF bool
c_make_command_run_and_reset_and_wait(CMakeCommand *command)
{
    CMakeProcessId process_id = c_make_command_run(*command);
    command->count = 0;
    return c_make_process_wait(process_id);
}

C_MAKE_DEF bool
c_make_command_run_and_wait(CMakeCommand command)
{
    CMakeProcessId process_id = c_make_command_run(command);
    return c_make_process_wait(process_id);
}

C_MAKE_DEF bool
c_make_process_wait_for_all(void)
{
    bool result = true;

    while (_c_make_context.process_group.count)
    {
        result = result & c_make_process_wait(_c_make_context.process_group.items[0].id);
    }

    return result;
}

#if !defined(C_MAKE_NO_ENTRY_POINT)

static void
print_help(const char *program_name)
{
    fprintf(stderr, "usage: %s <command> <build-directory> [--verbose] [--sequential] [<key>=\"<value>\" ...]\n", program_name);
    fprintf(stderr, "\n");
    fprintf(stderr, "commands:\n");
    fprintf(stderr, "    setup                Create and configure a new build directory.\n");
    fprintf(stderr, "    build                Run the build target on the given build directory.\n");
    fprintf(stderr, "    install              Run the install target on the given build directory.\n");
    fprintf(stderr, "\n");
    fprintf(stderr, "options:\n");
    fprintf(stderr, "    --verbose            This will print out the configuration and all the\n");
    fprintf(stderr, "                         command lines that are executed.\n");
    fprintf(stderr, "    --sequential         This will make c_make_command_run wait for the command\n");
    fprintf(stderr, "                         to terminate. This effectively sequentializes the\n");
    fprintf(stderr, "                         build process.\n");
    fprintf(stderr, "\n");
    fprintf(stderr, "Every build directory has a configuration which is stored in 'c_make.txt'.\n");
    fprintf(stderr, "It consists of all the options that define a build. All options can be set\n");
    fprintf(stderr, "during the setup phase by passing one or multiple key+value pairs.\n");
    fprintf(stderr, "Or you can just edit the configuration file by hand. Any custom option you\n");
    fprintf(stderr, "want can be stored in there, but there are some options that have special\n");
    fprintf(stderr, "meaning to c_make:\n");
    fprintf(stderr, "\n");
    fprintf(stderr, "    build_type               Build type. Either 'debug', 'reldebug' or 'release'.\n");
    fprintf(stderr, "                             Default: 'debug'\n");
    fprintf(stderr, "    host_ar                  Path to or name of the host archive/library program.\n");
    fprintf(stderr, "    host_c_compiler          Path to or name of the host c compiler.\n");
    fprintf(stderr, "    host_cpp_compiler        Path to or name of the host c++ compiler.\n");
    fprintf(stderr, "    install_prefix           Install prefix. Defaults to '/usr/local'.\n");
    fprintf(stderr, "    target_architecture      Architecture of the target. Either 'amd64', 'aarch64',\n");
    fprintf(stderr, "                             'riscv64', 'wasm32' or 'wasm64'. The default is the\n");
    fprintf(stderr, "                             host architecture.\n");
    fprintf(stderr, "    target_ar                Path to or name of the target archive/library program.\n");
    fprintf(stderr, "    target_c_compiler        Path to or name of the target c compiler.\n");
    fprintf(stderr, "    target_c_flags           Flags for the target c build.\n");
    fprintf(stderr, "    target_cpp_compiler      Path to or name of the target c++ compiler.\n");
    fprintf(stderr, "    target_cpp_flags         Flags for the target c++ build.\n");
    fprintf(stderr, "    target_platform          Platform of the target. Either 'android', 'freebsd',\n");
    fprintf(stderr, "                             'windows', 'linux', 'macos' or 'web'. The default is\n");
    fprintf(stderr, "                             the host platform.\n");
    fprintf(stderr, "    visual_studio_root_path  Path to the visual studio install. This should be the directory\n");
    fprintf(stderr, "                             in which you find 'VC\\Tools\\MSVC\\<version>'.\n");
    fprintf(stderr, "    visual_studio_version    The version of the visual studio install.\n");
    fprintf(stderr, "    windows_sdk_root_path    Path to the windows sdk. This should be the directory\n");
    fprintf(stderr, "                             in which you find 'bin', 'Include' and 'Lib'.\n");
    fprintf(stderr, "    windows_sdk_version      The version of the windows sdk.\n");
    fprintf(stderr, "\n");
    fprintf(stderr, "host platform: %s\n", c_make_get_platform_name(c_make_get_host_platform()));
    fprintf(stderr, "host architecture: %s\n", c_make_get_architecture_name(c_make_get_host_architecture()));
}

int main(int argument_count, char **arguments)
{
    if (argument_count < 3)
    {
        print_help(arguments[0]);
        return 2;
    }

    CMakeString command = CMakeCString(arguments[1]);
    const char *build_directory = arguments[2];
    const char *config_file_name = c_make_c_string_path_concat(build_directory, "c_make.txt");

    size_t private_used = c_make_memory_get_used(&_c_make_context.private_memory);

    CMakeString executable_path;
    executable_path.count = 4096;
    executable_path.data = (char *) c_make_memory_allocate(&_c_make_context.private_memory, executable_path.count);

#if C_MAKE_PLATFORM_ANDROID || C_MAKE_PLATFORM_LINUX
    ssize_t readlink(const char *, char *, size_t);
    ssize_t ret = readlink("/proc/self/exe", executable_path.data, executable_path.count);

    if (ret > 0)
    {
        executable_path.count = ret;
        c_make_string_split_right(&executable_path, '/');
    }
    else
    {
        executable_path = CMakeStringLiteral(".");
    }
#elif C_MAKE_PLATFORM_FREEBSD
    int sysctl_name[4] = { CTL_KERN, KERN_PROC, KERN_PROC_PATHNAME, -1 };
    size_t count = executable_path.count;
    int ret = sysctl(sysctl_name, 4, executable_path.data, &count, 0, 0);

    if (ret == 0)
    {
        executable_path.count = count;
        c_make_string_split_right(&executable_path, '/');
    }
    else
    {
        executable_path = CMakeStringLiteral(".");
    }
#elif C_MAKE_PLATFORM_WINDOWS
    c_make_memory_set_used(&_c_make_context.private_memory, private_used);

    wchar_t *module_file_name = (wchar_t *) c_make_memory_allocate(&_c_make_context.private_memory, 2 * executable_path.count);
    DWORD ret = GetModuleFileName(0, module_file_name, executable_path.count);

    if (ret > 0)
    {
        executable_path = CMakeCString(c_make_c_string_utf16_to_utf8(&_c_make_context.private_memory, module_file_name, ret));
        c_make_string_split_right(&executable_path, '\\');
    }
    else
    {
        executable_path = CMakeStringLiteral(".");
    }
#elif C_MAKE_PLATFORM_MACOS
    int ret = proc_pidpath(getpid(), executable_path.data, executable_path.count);

    if (ret > 0)
    {
        executable_path.count = ret;
        c_make_string_split_right(&executable_path, '/');
    }
    else
    {
        executable_path = CMakeStringLiteral(".");
    }
#endif

    const char *source_directory = c_make_string_to_c_string(&_c_make_context.public_memory, executable_path);

    c_make_memory_set_used(&_c_make_context.private_memory, private_used);

    _c_make_context.build_path = build_directory;
    _c_make_context.source_path = source_directory;

    for (int i = 3; i < argument_count; i += 1)
    {
        CMakeString argument = CMakeCString(arguments[i]);

        if (c_make_strings_are_equal(argument, CMakeStringLiteral("--verbose")))
        {
            _c_make_context.verbose = true;
        }
        else if (c_make_strings_are_equal(argument, CMakeStringLiteral("--sequential")))
        {
            _c_make_context.sequential = true;
        }
    }

#if C_MAKE_PLATFORM_WINDOWS
    const char *c_make_executable_file = "c_make.exe";
#else
    const char *c_make_executable_file = "c_make";
#endif
#ifdef __cplusplus
    const char *c_make_source_file = "c_make.cpp";
#else
    const char *c_make_source_file = "c_make.c";
#endif

    c_make_executable_file = c_make_c_string_path_concat(_c_make_context.source_path, c_make_executable_file);
    c_make_source_file = c_make_c_string_path_concat(_c_make_context.source_path, c_make_source_file);

    const char *c_make_source_files[] = { c_make_source_file, __FILE__ };

    if (c_make_needs_rebuild(c_make_executable_file, CMakeArrayCount(c_make_source_files), c_make_source_files))
    {
        size_t public_used = c_make_memory_get_used(&_c_make_context.public_memory);

        char *c_make_temp_file = c_make_c_string_concat(c_make_executable_file, ".orig");
        c_make_rename_file(c_make_executable_file, c_make_temp_file);

#ifdef __cplusplus
        const char *compiler = c_make_get_host_cpp_compiler();

        CMakeCommand command = { };
#else
        const char *compiler = c_make_get_host_c_compiler();

        CMakeCommand command = { 0 };
#endif

        c_make_command_append(&command, compiler);

        if (c_make_compiler_is_msvc(compiler))
        {
            c_make_command_append_msvc_compiler_flags(&command);
            c_make_command_append(&command, "-nologo");
        }

#ifdef C_MAKE_INCLUDE_PATH
        c_make_command_append(&command, "-I" CMakeStr(C_MAKE_INCLUDE_PATH));
        c_make_command_append(&command, "-DC_MAKE_INCLUDE_PATH=" CMakeStr(C_MAKE_INCLUDE_PATH));
#endif
#ifdef C_MAKE_COMPILER_FLAGS
        c_make_command_append_command_line(&command, CMakeStr(C_MAKE_COMPILER_FLAGS));
        c_make_command_append(&command, "-DC_MAKE_COMPILER_FLAGS=" CMakeStr(C_MAKE_COMPILER_FLAGS));
#endif
        c_make_command_append_output_executable(&command, "c_make", c_make_get_host_platform());
        c_make_command_append(&command, c_make_source_file);
        c_make_command_append_default_linker_flags(&command, c_make_get_host_architecture());

        c_make_log(CMakeLogLevelInfo, "rebuild c_make\n");

        if (!c_make_command_run_and_wait(command))
        {
            c_make_rename_file(c_make_temp_file, c_make_executable_file);
        }
        else
        {
            command.count = 0;
            c_make_command_append(&command, c_make_executable_file);
            c_make_command_append_slice(&command, argument_count - 1, (const char **) (arguments + 1));
            c_make_command_run_and_wait(command);
        }

        c_make_memory_set_used(&_c_make_context.public_memory, public_used);

        return _c_make_context.did_fail ? 1 : 0;
    }
    else
    {
        size_t public_used = c_make_memory_get_used(&_c_make_context.public_memory);

        c_make_delete_file(c_make_c_string_concat(c_make_executable_file, ".orig"));

        c_make_memory_set_used(&_c_make_context.public_memory, public_used);
    }

    _c_make_context.target_platform = c_make_get_host_platform();
    _c_make_context.target_architecture = c_make_get_host_architecture();
    _c_make_context.build_type = CMakeBuildTypeDebug;

    if (c_make_strings_are_equal(command, CMakeStringLiteral("setup")))
    {
        c_make_create_directory_recursively(build_directory);

        if (c_make_file_exists(config_file_name))
        {
            c_make_log(CMakeLogLevelError, "there is already a directory called '%s'\n", build_directory);
            return 2;
        }

        size_t public_used = c_make_memory_get_used(&_c_make_context.public_memory);

        {
            CMakeString ARCH = c_make_get_environment_variable(&_c_make_context.public_memory, "ARCH");

            if (c_make_strings_are_equal(ARCH, CMakeStringLiteral("arm64")))
            {
                c_make_config_set("target_architecture", "aarch64");
            }

            c_make_memory_set_used(&_c_make_context.public_memory, public_used);
        }

        {
            CMakeString AR = c_make_get_environment_variable(&_c_make_context.public_memory, "AR");

            if (AR.count)
            {
                const char *target_ar = c_make_string_to_c_string(&_c_make_context.public_memory, AR);

                if (!c_make_has_slash_or_backslash(target_ar))
                {
                    target_ar = c_make_find_program(target_ar);
                }

                if (target_ar)
                {
                    c_make_config_set("target_ar", target_ar);
                }
            }

            c_make_memory_set_used(&_c_make_context.public_memory, public_used);
        }

        {
            CMakeString CC = c_make_get_environment_variable(&_c_make_context.public_memory, "CC");

            if (CC.count)
            {
                const char *target_c_compiler = c_make_string_to_c_string(&_c_make_context.public_memory, c_make_string_split_left(&CC, ' '));

                if (!c_make_has_slash_or_backslash(target_c_compiler))
                {
                    target_c_compiler = c_make_find_program(target_c_compiler);
                }

                if (target_c_compiler)
                {
                    c_make_config_set("target_c_compiler", target_c_compiler);
                }

                CC = c_make_string_trim(CC);

                if (CC.count)
                {
                    c_make_config_set("target_c_flags", c_make_string_to_c_string(&_c_make_context.public_memory, CC));
                }
            }

            c_make_memory_set_used(&_c_make_context.public_memory, public_used);
        }

        {
            CMakeString CXX = c_make_get_environment_variable(&_c_make_context.public_memory, "CXX");

            if (CXX.count)
            {
                const char *target_cpp_compiler = c_make_string_to_c_string(&_c_make_context.public_memory, c_make_string_split_left(&CXX, ' '));

                if (!c_make_has_slash_or_backslash(target_cpp_compiler))
                {
                    target_cpp_compiler = c_make_find_program(target_cpp_compiler);
                }

                if (target_cpp_compiler)
                {
                    c_make_config_set("target_cpp_compiler", target_cpp_compiler);
                }

                CXX = c_make_string_trim(CXX);

                if (CXX.count)
                {
                    c_make_config_set("target_cpp_flags", c_make_string_to_c_string(&_c_make_context.public_memory, CXX));
                }
            }

            c_make_memory_set_used(&_c_make_context.public_memory, public_used);
        }

        for (int i = 3; i < argument_count; i += 1)
        {
            CMakeString argument = CMakeCString(arguments[i]);

            CMakeString key = c_make_string_trim(c_make_string_split_left(&argument, '='));
            CMakeString value = c_make_string_trim(argument);

            if (key.count && value.count)
            {
                if (value.data[0] == '"')
                {
                    value.count -= 1;
                    value.data += 1;
                }

                if (value.count && (value.data[value.count - 1] == '"'))
                {
                    value.count -= 1;
                }

                size_t public_used = c_make_memory_get_used(&_c_make_context.public_memory);

                c_make_config_set(c_make_string_to_c_string(&_c_make_context.public_memory, key),
                                  c_make_string_to_c_string(&_c_make_context.public_memory, value));

                c_make_memory_set_used(&_c_make_context.public_memory, public_used);
            }
        }

        _c_make_entry_(CMakeTargetSetup);

        c_make_process_wait_for_all();

        c_make_config_set_if_not_exists("target_platform", c_make_get_platform_name(c_make_get_host_platform()));
        c_make_config_set_if_not_exists("target_architecture", c_make_get_architecture_name(c_make_get_host_architecture()));
        c_make_config_set_if_not_exists("build_type", "debug");
        // TODO: set to something different for windows
        c_make_config_set_if_not_exists("install_prefix", "/usr/local");

#if C_MAKE_PLATFORM_WINDOWS
        CMakeConfigValue visual_studio_version = c_make_config_get("visual_studio_version");
        CMakeConfigValue visual_studio_root_path = c_make_config_get("visual_studio_root_path");

        if (!visual_studio_version.is_valid || !visual_studio_root_path.is_valid)
        {
            size_t public_used = c_make_memory_get_used(&_c_make_context.public_memory);

            CMakeString VCToolsVersion = c_make_string_trim(c_make_get_environment_variable(&_c_make_context.public_memory, "VCToolsVersion"));
            CMakeString VCToolsInstallDir = c_make_string_trim(c_make_get_environment_variable(&_c_make_context.public_memory, "VCToolsInstallDir"));

            if (VCToolsVersion.count && VCToolsInstallDir.count)
            {
                while (VCToolsInstallDir.count)
                {
                    CMakeString directory = c_make_string_split_right_path_separator(&VCToolsInstallDir);

                    if (c_make_strings_are_equal(directory, CMakeStringLiteral("VC")))
                    {
                        break;
                    }
                }

                c_make_config_set("visual_studio_version", c_make_string_to_c_string(&_c_make_context.public_memory, VCToolsVersion));
                c_make_config_set("visual_studio_root_path", c_make_string_to_c_string(&_c_make_context.public_memory, VCToolsInstallDir));
            }

            c_make_memory_set_used(&_c_make_context.public_memory, public_used);
        }

        CMakeConfigValue windows_sdk_version = c_make_config_get("windows_sdk_version");
        CMakeConfigValue windows_sdk_root_path = c_make_config_get("windows_sdk_root_path");

        if (!windows_sdk_version.is_valid || !windows_sdk_root_path.is_valid)
        {
            size_t public_used = c_make_memory_get_used(&_c_make_context.public_memory);

            CMakeString WindowsSDKVersion = c_make_string_trim(c_make_get_environment_variable(&_c_make_context.public_memory, "WindowsSDKVersion"));
            CMakeString WindowsSdkBinPath = c_make_string_trim(c_make_get_environment_variable(&_c_make_context.public_memory, "WindowsSdkBinPath"));

            if (WindowsSDKVersion.count && (WindowsSDKVersion.data[WindowsSDKVersion.count - 1] == '\\'))
            {
                WindowsSDKVersion.count -= 1;
            }

            if (WindowsSDKVersion.count && WindowsSdkBinPath.count)
            {
                while (WindowsSdkBinPath.count)
                {
                    CMakeString directory = c_make_string_split_right_path_separator(&WindowsSdkBinPath);

                    if (c_make_strings_are_equal(directory, CMakeStringLiteral("bin")))
                    {
                        break;
                    }
                }

                c_make_config_set("windows_sdk_version", c_make_string_to_c_string(&_c_make_context.public_memory, WindowsSDKVersion));
                c_make_config_set("windows_sdk_root_path", c_make_string_to_c_string(&_c_make_context.public_memory, WindowsSdkBinPath));
            }

            c_make_memory_set_used(&_c_make_context.public_memory, public_used);
        }
#endif

        if (c_make_get_host_platform() != c_make_get_target_platform())
        {
            if (c_make_get_target_platform() == CMakePlatformWindows)
            {
                CMakeConfigValue target_ar = c_make_config_get("target_ar");

                if (!target_ar.is_valid)
                {
                    size_t public_used = c_make_memory_get_used(&_c_make_context.public_memory);

                    const char *ar = c_make_find_program("x86_64-w64-mingw32-ar");

                    if (ar)
                    {
                        c_make_config_set("target_ar", ar);
                    }

                    c_make_memory_set_used(&_c_make_context.public_memory, public_used);
                }

                CMakeConfigValue target_c_compiler = c_make_config_get("target_c_compiler");

                if (!target_c_compiler.is_valid)
                {
                    size_t public_used = c_make_memory_get_used(&_c_make_context.public_memory);

                    const char *c_compiler = c_make_find_program("x86_64-w64-mingw32-gcc");

                    if (c_compiler)
                    {
                        c_make_config_set("target_c_compiler", c_compiler);
                    }

                    c_make_memory_set_used(&_c_make_context.public_memory, public_used);
                }

                CMakeConfigValue target_cpp_compiler = c_make_config_get("target_cpp_compiler");

                if (!target_cpp_compiler.is_valid)
                {
                    size_t public_used = c_make_memory_get_used(&_c_make_context.public_memory);

                    const char *cpp_compiler = c_make_find_program("x86_64-w64-mingw32-g++");

                    if (cpp_compiler)
                    {
                        c_make_config_set("target_cpp_compiler", cpp_compiler);
                    }

                    c_make_memory_set_used(&_c_make_context.public_memory, public_used);
                }
            }
        }

        c_make_log(CMakeLogLevelInfo, "store config:\n");
        c_make_print_config();

        if (!c_make_store_config(config_file_name))
        {
            return 2;
        }

        public_used = c_make_memory_get_used(&_c_make_context.public_memory);

        const char *gitignore_file_name = c_make_c_string_path_concat(build_directory, ".gitignore");
        c_make_write_entire_file(gitignore_file_name, CMakeStringLiteral("# This file was auto-generated by c_make\n*\n"));

        c_make_memory_set_used(&_c_make_context.public_memory, public_used);
    }
    else if (c_make_strings_are_equal(command, CMakeStringLiteral("build")) ||
             c_make_strings_are_equal(command, CMakeStringLiteral("install")))
    {
        if (!c_make_directory_exists(build_directory))
        {
            c_make_log(CMakeLogLevelError, "there is no build directory called '%s'\n", build_directory);
            return 2;
        }

        if (!c_make_file_exists(config_file_name))
        {
            c_make_log(CMakeLogLevelError, "the build directory '%s' was never setup\n", build_directory);
            return 2;
        }

        if (!c_make_load_config(config_file_name))
        {
            return 2;
        }

        if (_c_make_context.verbose)
        {
            c_make_log(CMakeLogLevelInfo, "load config:\n");
            c_make_print_config();
        }

        if (c_make_strings_are_equal(command, CMakeStringLiteral("build")))
        {
            _c_make_entry_(CMakeTargetBuild);
        }
        else
        {
            _c_make_entry_(CMakeTargetInstall);
        }

        c_make_process_wait_for_all();
    }

    return _c_make_context.did_fail ? 1 : 0;
}

#endif // !defined(C_MAKE_NO_ENTRY_POINT)

#endif // defined(C_MAKE_IMPLEMENTATION)

/*
MIT License

Copyright (c) 2024 Julius Range-Lüdemann

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
