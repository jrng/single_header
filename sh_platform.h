// sh_platform.h - MIT License
// See end of file for full license

#ifndef __SH_PLATFORM_INCLUDE__
#define __SH_PLATFORM_INCLUDE__

#  ifndef __SH_BASE_INCLUDE__
#    error "sh_platform.h requires sh_base.h to be included first"
#  endif

#  ifndef __SH_STRING_BUILDER_INCLUDE__
#    error "sh_platform.h requires sh_string_builder.h to be included first"
#  endif

#  if SH_PLATFORM_WINDOWS

#    define NOMINMAX
#    define WIN32_LEAN_AND_MEAN

#    include <windows.h>

#  elif SH_PLATFORM_UNIX

#    include <errno.h>
#    include <fcntl.h>
#    include <stdlib.h>
#    include <unistd.h>
#    include <sys/stat.h>

#  endif

#  if defined(SH_STATIC) || defined(SH_PLATFORM_STATIC)
#    define SH_PLATFORM_DEF static
#  else
#    define SH_PLATFORM_DEF extern
#  endif

typedef enum
{
    SH_FILE_TYPE_DIRECTORY = 0,
    SH_FILE_TYPE_REGULAR   = 1,
    SH_FILE_TYPE_OTHER     = 2,
} ShFileType;

typedef struct
{
    ShFileType type;
    uint64_t size;
    uint64_t modification_time_ms;
} ShFileInformation;

SH_PLATFORM_DEF ShString sh_get_environment_variable(ShThreadContext *thread_context, ShAllocator allocator, ShString variable_name, ShString fallback);

SH_PLATFORM_DEF bool sh_get_file_information(ShThreadContext *thread_context, ShString filename, ShFileInformation *information);

SH_PLATFORM_DEF bool sh_file_exists(ShThreadContext *thread_context, ShString file_name);
SH_PLATFORM_DEF bool sh_directory_exists(ShThreadContext *thread_context, ShString directory_name);

SH_PLATFORM_DEF bool sh_create_directory(ShThreadContext *thread_context, ShString directory_name, bool create_parents);

SH_PLATFORM_DEF bool sh_read_entire_file(ShThreadContext *thread_context, ShAllocator allocator, ShString filename, ShString *content);
SH_PLATFORM_DEF bool sh_write_entire_file(ShThreadContext *thread_context, ShString filename, ShStringBuilder *content);

#endif // __SH_PLATFORM_INCLUDE__

#ifdef SH_PLATFORM_IMPLEMENTATION

SH_PLATFORM_DEF ShString
sh_get_environment_variable(ShThreadContext *thread_context, ShAllocator allocator, ShString variable_name, ShString fallback)
{
    ShString result = fallback;

#  if SH_PLATFORM_WINDOWS
    ShTemporaryMemory temp_memory = sh_begin_temporary_memory(thread_context, 1, &allocator);

    LPCWSTR utf16_variable_name = (LPCWSTR) sh_string_to_c_string(temp_memory.allocator, variable_name);
    DWORD count = GetEnvironmentVariableW(utf16_variable_name, NULL, 0);

    if (count > 0)
    {
        LPWSTR variable = sh_alloc_array(allocator, WCHAR, count);
        DWORD ret = GetEnvironmentVariableW(utf16_variable_name, variable, count);

        if ((ret > 0) && (ret < count))
        {
            result = sh_string_utf16le_to_utf8(allocator, ShMakeString(ret, variable));
        }
    }

    sh_end_temporary_memory(temp_memory);
#  elif SH_PLATFORM_UNIX
    (void) allocator;

    ShTemporaryMemory temp_memory = sh_begin_temporary_memory(thread_context, 0, NULL);

    char *variable = getenv(sh_string_to_c_string(temp_memory.allocator, variable_name));

    if (variable)
    {
        result = ShCString(variable);
    }

    sh_end_temporary_memory(temp_memory);
#  endif

    return result;
}

SH_PLATFORM_DEF bool
sh_get_file_information(ShThreadContext *thread_context, ShString filename, ShFileInformation *information)
{
#  if SH_PLATFORM_WINDOWS
    ShTemporaryMemory temp_memory = sh_begin_temporary_memory(thread_context, 0, NULL);

    LPWSTR utf16_filename = (LPWSTR) sh_string_to_c_string(temp_memory.allocator, sh_string_utf8_to_utf16le(temp_memory.allocator, filename));
    HANDLE file = CreateFileW(utf16_filename, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, 0, 0);

    sh_end_temporary_memory(temp_memory);

    if (file == INVALID_HANDLE_VALUE)
    {
        return false;
    }

    BY_HANDLE_FILE_INFORMATION file_info;

    if (!GetFileInformationByHandle(file, &file_info))
    {
        CloseHandle(file);
        return false;
    }

    if (file_info.dwFileAttributes == INVALID_FILE_ATTRIBUTES)
    {
        information->type = SH_FILE_TYPE_OTHER;
    }
    else if (file_info.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
    {
        information->type = SH_FILE_TYPE_DIRECTORY;
    }
    else
    {
        information->type = SH_FILE_TYPE_REGULAR;
    }

    information->size = ((uint64_t) file_info.nFileSizeHigh << 32) | (uint64_t) file_info.nFileSizeLow;
    information->modification_time_ms = (((uint64_t) file_info.ftLastWriteTime.dwHighDateTime << 32) | (uint64_t) file_info.ftLastWriteTime.dwLowDateTime) / 10000;

    CloseHandle(file);

    return true;
#  elif SH_PLATFORM_UNIX
    ShTemporaryMemory temp_memory = sh_begin_temporary_memory(thread_context, 0, NULL);

    struct stat stats;
    int ret = lstat(sh_string_to_c_string(temp_memory.allocator, filename), &stats);

    sh_end_temporary_memory(temp_memory);

    if (ret)
    {
        return false;
    }

#    if SH_PLATFORM_ANDROID || SH_PLATFORM_FREEBSD || SH_PLATFORM_LINUX
    struct timespec mod_time = stats.st_mtim;
#    elif SH_PLATFORM_MACOS
    struct timespec mod_time = stats.st_mtimespec;
#    endif

    if (S_ISREG(stats.st_mode))
    {
        information->type = SH_FILE_TYPE_REGULAR;
    }
    else if (S_ISDIR(stats.st_mode))
    {
        information->type = SH_FILE_TYPE_DIRECTORY;
    }
    else
    {
        information->type = SH_FILE_TYPE_OTHER;
    }

    information->size = stats.st_size;
    information->modification_time_ms = ((uint64_t) mod_time.tv_sec * 1000) + ((uint64_t) mod_time.tv_nsec / 1000000);

    return true;
#  else
    return false;
#  endif
}

SH_PLATFORM_DEF bool
sh_file_exists(ShThreadContext *thread_context, ShString file_name)
{
#  if SH_PLATFORM_WINDOWS
    ShTemporaryMemory temp_memory = sh_begin_temporary_memory(thread_context, 0, NULL);

    LPWSTR utf16_file_name = (LPWSTR) sh_string_to_c_string(temp_memory.allocator, sh_string_utf8_to_utf16le(temp_memory.allocator, file_name));
    DWORD file_attributes = GetFileAttributesW(utf16_file_name);

    sh_end_temporary_memory(temp_memory);

    return ((file_attributes != INVALID_FILE_ATTRIBUTES) || !(file_attributes & FILE_ATTRIBUTE_DIRECTORY)) ? true : false;
#  elif SH_PLATFORM_UNIX
    bool result = false;
    struct stat stats;

    ShTemporaryMemory temp_memory = sh_begin_temporary_memory(thread_context, 0, NULL);

    if (!stat(sh_string_to_c_string(temp_memory.allocator, file_name), &stats))
    {
        result = S_ISREG(stats.st_mode) ? true : false;
    }

    sh_end_temporary_memory(temp_memory);

    return result;
#  else
    return false;
#  endif
}

SH_PLATFORM_DEF bool
sh_directory_exists(ShThreadContext *thread_context, ShString directory_name)
{
#  if SH_PLATFORM_WINDOWS
    ShTemporaryMemory temp_memory = sh_begin_temporary_memory(thread_context, 0, NULL);

    LPWSTR utf16_directory_name = (LPWSTR) sh_string_to_c_string(temp_memory.allocator, sh_string_utf8_to_utf16le(temp_memory.allocator, directory_name));
    DWORD file_attributes = GetFileAttributesW(utf16_directory_name);

    sh_end_temporary_memory(temp_memory);

    return ((file_attributes != INVALID_FILE_ATTRIBUTES) || (file_attributes & FILE_ATTRIBUTE_DIRECTORY)) ? true : false;
#  elif SH_PLATFORM_UNIX
    bool result = false;
    struct stat stats;

    ShTemporaryMemory temp_memory = sh_begin_temporary_memory(thread_context, 0, NULL);

    if (!stat(sh_string_to_c_string(temp_memory.allocator, directory_name), &stats))
    {
        result = S_ISDIR(stats.st_mode) ? true : false;
    }

    sh_end_temporary_memory(temp_memory);

    return result;
#  else
    return false;
#  endif
}

SH_PLATFORM_DEF bool
sh_create_directory(ShThreadContext *thread_context, ShString directory_name, bool create_parents)
{
#  if SH_PLATFORM_WINDOWS
    ShTemporaryMemory temp_memory = sh_begin_temporary_memory(thread_context, 0, NULL);

    LPWSTR *directories = NULL;
    sh_array_init(directories, 4, temp_memory.allocator);

    do {
        LPWSTR utf16_directory_name = (LPWSTR) sh_string_to_c_string(temp_memory.allocator, sh_string_utf8_to_utf16le(temp_memory.allocator, directory_name));
        DWORD file_attributes = GetFileAttributesW(utf16_directory_name);

        if ((file_attributes != INVALID_FILE_ATTRIBUTES) && (file_attributes & FILE_ATTRIBUTE_DIRECTORY))
        {
            break;
        }

        *sh_array_append(directories) = utf16_directory_name;
        sh_string_split_right_on_path_separator(&directory_name);
    } while (create_parents && (directory_name.count > 0));

    bool result = true;
    usize count = sh_array_count(directories);

    while (count > 0)
    {
        count -= 1;

        if (!CreateDirectoryW(directories[count], NULL) && (GetLastError() != ERROR_ALREADY_EXISTS))
        {
            result = false;
            break;
        }
    }

    sh_end_temporary_memory(temp_memory);

    return result;
#  elif SH_PLATFORM_UNIX
    ShTemporaryMemory temp_memory = sh_begin_temporary_memory(thread_context, 0, NULL);

    const char **directories = NULL;
    sh_array_init(directories, 4, temp_memory.allocator);

    do {
        const char *c_directory_name = sh_string_to_c_string(temp_memory.allocator, directory_name);

        struct stat stats;

        if (!stat(c_directory_name, &stats) && S_ISDIR(stats.st_mode))
        {
            break;
        }

        *sh_array_append(directories) = c_directory_name;
        sh_string_split_right_on_path_separator(&directory_name);
    } while (create_parents && (directory_name.count > 0));

    bool result = true;
    usize count = sh_array_count(directories);

    while (count > 0)
    {
        count -= 1;

        if (mkdir(directories[count], 0775) && (errno != EEXIST))
        {
            result = false;
            break;
        }
    }

    sh_end_temporary_memory(temp_memory);

    return result;
#  else
    return false;
#  endif
}

SH_PLATFORM_DEF bool
sh_read_entire_file(ShThreadContext *thread_context, ShAllocator allocator, ShString filename, ShString *content)
{
#  if SH_PLATFORM_WINDOWS
    ShTemporaryMemory temp_memory = sh_begin_temporary_memory(thread_context, 1, &allocator);

    LPWSTR utf16_filename = (LPWSTR) sh_string_to_c_string(temp_memory.allocator, sh_string_utf8_to_utf16le(temp_memory.allocator, filename));
    HANDLE file = CreateFileW(utf16_filename, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, 0, 0);

    if (file == INVALID_HANDLE_VALUE)
    {
        sh_end_temporary_memory(temp_memory);
        return false;
    }

    LARGE_INTEGER file_size;

    if (!GetFileSizeEx(file, &file_size))
    {
        sh_end_temporary_memory(temp_memory);
        CloseHandle(file);
        return false;
    }

    content->count = file_size.QuadPart;
    content->data  = sh_alloc_array(allocator, uint8_t, content->count);

    usize index = 0;

    while (index < content->count)
    {
        DWORD bytes_read = 0;

        if (!ReadFile(file, content->data + index, content->count - index, &bytes_read, NULL))
        {
            sh_free(allocator, content->data);
            sh_end_temporary_memory(temp_memory);
            CloseHandle(file);
            return false;
        }

        index += bytes_read;
    }

    sh_end_temporary_memory(temp_memory);
    CloseHandle(file);

    return true;
#  elif SH_PLATFORM_UNIX
    ShTemporaryMemory temp_memory = sh_begin_temporary_memory(thread_context, 1, &allocator);

    int fd = open(sh_string_to_c_string(temp_memory.allocator, filename), O_RDONLY);

    if (fd < 0)
    {
        sh_end_temporary_memory(temp_memory);
        return false;
    }

    struct stat stats;

    if (fstat(fd, &stats) < 0)
    {
        sh_end_temporary_memory(temp_memory);
        close(fd);
        return false;
    }

    content->count = stats.st_size;
    content->data  = sh_alloc_array(allocator, uint8_t, content->count);

    usize index = 0;

    while (index < content->count)
    {
        ssize_t read_bytes = read(fd, content->data + index, content->count - index);

        if (read_bytes < 0)
        {
            sh_free(allocator, content->data);
            sh_end_temporary_memory(temp_memory);
            close(fd);
            return false;
        }

        index += read_bytes;
    }

    sh_end_temporary_memory(temp_memory);
    close(fd);

    return true;
#  else
    return false;
#  endif
}

SH_PLATFORM_DEF bool
sh_write_entire_file(ShThreadContext *thread_context, ShString filename, ShStringBuilder *content)
{
#  if SH_PLATFORM_WINDOWS
    ShTemporaryMemory temp_memory = sh_begin_temporary_memory(thread_context, 0, NULL);

    LPWSTR utf16_filename = (LPWSTR) sh_string_to_c_string(temp_memory.allocator, sh_string_utf8_to_utf16le(temp_memory.allocator, filename));
    HANDLE file = CreateFileW(utf16_filename, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, 0);

    sh_end_temporary_memory(temp_memory);

    if (file == INVALID_HANDLE_VALUE)
    {
        return false;
    }

    ShStringBuffer *buffer = content->first_buffer;

    while (buffer)
    {
        uint8_t *src = buffer->data;
        usize count = buffer->occupied;

        while (count)
        {
            DWORD bytes_written = 0;

            if (!WriteFile(file, src, count, &bytes_written, 0))
            {
                CloseHandle(file);
                return false;
            }

            src += bytes_written;
            count -= bytes_written;
        }

        buffer = buffer->next;
    }

    CloseHandle(file);

    return true;
#  elif SH_PLATFORM_UNIX
    ShTemporaryMemory temp_memory = sh_begin_temporary_memory(thread_context, 0, NULL);

    int fd = open(sh_string_to_c_string(temp_memory.allocator, filename),  O_WRONLY | O_TRUNC | O_CREAT, 0664);

    sh_end_temporary_memory(temp_memory);

    if (fd < 0)
    {
        return false;
    }

    ShStringBuffer *buffer = content->first_buffer;

    while (buffer)
    {
        uint8_t *src = buffer->data;
        usize count = buffer->occupied;

        while (count)
        {
            ssize bytes_written = write(fd, src, count);

            if (bytes_written < 0)
            {
                close(fd);
                return false;
            }

            src += bytes_written;
            count -= bytes_written;
        }

        buffer = buffer->next;
    }

    close(fd);

    return true;
#  else
    return false;
#  endif
}

#endif // SH_PLATFORM_IMPLEMENTATION

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
