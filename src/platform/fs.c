#include "fs.h"
#include "core/mem.h"
#include "stdlib.h"
#include <stdio.h>
#include <sys/stat.h>

bool fs_exists(const char* path) {
    struct stat buf;
    return (stat(path, &buf) == 0);
}

bool fs_open(const char* path, OpenFileMode mode, File* file) {
    const char* mode_str;
    switch (mode) {
    case OPEN_FILE_MODE_READ:
        mode_str = "r";
        break;
    case OPEN_FILE_MODE_READ_BINARY:
        mode_str = "rb";
        break;
    case OPEN_FILE_MODE_WRITE:
        mode_str = "w";
        break;
    case OPEN_FILE_MODE_WRITE_BINARY:
        mode_str = "wb";
        break;
    }
    file->handle = fopen(path, mode_str);
    return (file->handle != NULL);
}

u8* fs_read_all(File* file, u64* bytes_read) {
    if (file != NULL && file->handle != NULL) {
        fseek(file->handle, 0, SEEK_END);
        *bytes_read = ftell(file->handle);
        fseek(file->handle, 0, SEEK_SET);
        u8* buf = mem_alloc(sizeof(u8) * (*bytes_read));
        fread(buf, 1, *bytes_read, file->handle);
        return buf;
    }
    return NULL;
}

bool fs_write(File* file, u64 data_size, const void* data, u64* bytes_written) {
    if (file->handle) {
        *bytes_written = fwrite(data, 1, data_size, file->handle);
        if (*bytes_written != data_size) {
            return false;
        }
        fflush(file->handle);
        return true;
    }
    return false;
}

void fs_close(File* file) {
    if (file && file->handle) {
        fclose(file->handle);
        file->handle = 0;
    }
}
