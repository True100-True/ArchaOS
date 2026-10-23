#ifndef FILESYSTEM_H
#define FILESYSTEM_H


#include <stddef.h>
#include <stdint.h>
#include <stdint.h>
#include "../lib/string.h"
#include "../lib/kmalloc.h"
#include "../lib/stdio.h"
#include "../../common/fromboot.h"
#include "../lib/kmalloc.h"
#include "../errs/pnc.h"

// Each file is its own node
// each folder is collection

enum FS_FLAGS {
    FS_IS_DIRECTORY = (1u << 0),
};

// Reliezed just now, it would make sense to not include its bit position because it
// isn't a flag, and doesn't need support for uint conversion..
enum FS_RETURN {
    FS_SUCCESS       = (1u << 0),
    FS_FAIL          = (1u << 1),
    FS_NOT_FOUND     = (1u << 2),
    FS_INVALID_PATH  = (1u << 3),
    FS_NOT_A_DIR     = (1u << 4),
    FS_IS_A_DIR      = (1u << 5),
    FS_BUF_TOO_SMALL = (1u << 6),
    FS_INVALID_FILE  = (1u << 7), 
    FS_FILE_TOO_SMALL = (1u << 8),
    FS_PERMISSION_DENIED = (1u << 9),
    FS_DIR_NOT_EMPTY = (1u << 10),
    FS_INVALID_FS    = (1u << 11),
    FS_ALR_EXIST     = (1u << 12),
    FS_INV_ARG       = (1u << 13),
    FS_INV_PATH      = (1u << 14),
};

struct file {
    uint64_t id;
    char *name;
    struct file *parent;
    struct file *children;
    struct file *next;
    void *data;                // files only
    size_t size;
    uint16_t flags;
};


enum FS_RETURN read_file_content(const char *path, void* buffer, size_t buf_size);
enum FS_RETURN write_file_content(const char *path, void* buffer, size_t buf_size);

// File manipulation

enum FS_RETURN delete_file(const char *path);
enum FS_RETURN create_file(const char *path, const void *data, size_t size, enum FS_FLAGS flags);

// Folder manipulation

enum FS_RETURN create_folder(const char *path);
enum FS_RETURN delete_folder(const char *path);
void init_fs(BootInfo *boot);

// Discovery

enum FS_RETURN list_files(const char *path, struct file** list);

// Helpers
// Now not used, but I will need it later
enum FS_RETURN create_file_force(const char *path, const void *data, size_t size, enum FS_FLAGS flags);

// Initramfs.c
int parse_cpio_disk(const void *disk, size_t disk_size); // CPIO parser

#endif 
