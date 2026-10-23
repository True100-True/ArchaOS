#include "../fs.h"

#define MAX_NAME 200
#define MAX_PATH 1024*4

static int hex8(const char *s, uint32_t *out) {
    uint32_t value = 0;
    for (unsigned i = 0; i < 8; ++i) {
        unsigned char c = (unsigned char)s[i];
        unsigned digit;
        if (c >= '0' && c <= '9') digit = c - '0';
        else if (c >= 'a' && c <= 'f') digit = c - 'a' + 10;
        else if (c >= 'A' && c <= 'F') digit = c - 'A' + 10;
        else return 0;
        value = (value << 4) | digit;
    }
    *out = value;
    return 1;
}

static int ensure_parent_directories(char *path) {
    for (char *p = path; *p; ++p) {
        if (*p != '/')
            continue;
        if (p == path)
            return 0;
        *p = '\0';
        enum FS_RETURN result = create_folder(path);
        *p = '/';
        if (result != FS_SUCCESS && result != FS_ALR_EXIST)
            return 0;
    }
    return 1;
}

/*
-6 - failed memory creation
-5 - malformed header
-4 - corrupted data
-3 - invalid name
-2 - not supported format
-1 - invalid hexadecimal field
 0 - end
 1 - general fault
*/
int parse_cpio_disk(const void *disk, size_t disk_size) {
    if (disk == NULL)
        return -7;

    const unsigned char *bytes = (const unsigned char *)disk;
    size_t offset = 0;

    for (;;) {
        uint32_t fields[13];
        if (offset > disk_size || disk_size - offset < 110)
            return -5;
        const unsigned char *header = bytes + offset;
        if (memcmp(header, "070701", 6) != 0)
            return -2;
        for (unsigned i = 0; i < 13; ++i) {
            if (!hex8((const char *)header + 6 + 8 * i, &fields[i]))
                return -1;
        }
        offset += 110;

        uint32_t filesize = fields[6];
        uint32_t namesize = fields[11];
        uint32_t mode = fields[1];
        if (namesize == 0 || namesize > MAX_PATH + 1)
            return -3;
        if (offset > disk_size || disk_size - offset < namesize)
            return -3;

        char name[MAX_PATH + 1];
        memcpy(name, bytes + offset, namesize);
        offset += namesize;
        if (name[namesize - 1] != '\0' || strlen(name) != namesize - 1)
            return -3;

        size_t padding = (4 - (offset & 3)) & 3;
        if (disk_size - offset < padding)
            return -5;
        offset += padding;

        if (strcmp(name, "TRAILER!!!") == 0)
            return 0;
        while (name[0] == '.' && name[1] == '/')
            memmove(name, name + 2, strlen(name + 2) + 1);
        if (name[0] == '/')
            return -3;

        if (disk_size - offset < filesize)
            return -4;

        /* Ignore CPIO's root entry, commonly named ".". */
        if (name[0] == '\0' || strcmp(name, ".") == 0) {
            offset += filesize;
            padding = (4 - (offset & 3)) & 3;
            if (disk_size - offset < padding)
                return -5;
            offset += padding;
            continue;
        }
        uint32_t type = mode & 0170000;
        size_t name_len = strlen(name);
        if (type == 0040000) {
            while (name_len > 0 && name[name_len - 1] == '/')
                name[--name_len] = '\0';
            if (name_len == 0)
                return -3;
            if (!ensure_parent_directories(name))
                return -6;
            enum FS_RETURN result = create_folder(name);
            if (result != FS_SUCCESS && result != FS_ALR_EXIST)
                return -6;
        } else if (type == 0100000 || type == 0120000 || type == 0) {
            if (name_len != 0 && name[name_len - 1] == '/')
                name[--name_len] = '\0';
            if (name_len == 0 || !ensure_parent_directories(name))
                return -3;
            enum FS_RETURN result = create_file(name, bytes + offset, filesize, 0);
            if (result != FS_SUCCESS && result != FS_ALR_EXIST)
                return -6;
        }

        offset += filesize;
        padding = (4 - (offset & 3)) & 3;
        if (disk_size - offset < padding)
            return -5;
        offset += padding;
    }
}
