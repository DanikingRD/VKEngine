#ifndef FILESYSTEM_H
#define FILESYSTEM_H

#include "types.h"

typedef struct File {
    // Handle to a platform-specific file object.
    void* handle;
} File;

typedef enum OpenFileMode {
    OPEN_FILE_MODE_READ,
    OPEN_FILE_MODE_READ_BINARY,
    OPEN_FILE_MODE_WRITE,
    OPEN_FILE_MODE_WRITE_BINARY,
} OpenFileMode;

bool fs_exists(const char* path);
bool fs_open(const char* path, OpenFileMode mode, File* file);
bool fs_write(File* file, u64 data_size, const void* data, u64* bytes_written);
void fs_close(File* file);
u8* fs_read_all(File* file, u64* bytes_read);

#endif
