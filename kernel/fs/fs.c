#include "fs.h"

/*
File contains ID, name, its parent, its children and siblings aswell as pointer to data
and size of data and Flags
*/

#define VERBOSE 1

struct file root;

// Child manipulation

static struct file *find_child(struct file *dir, const char *name) {
    for (struct file *f = dir->children; f != NULL; f = f->next) {
        if (strcmp(f->name, name) == 0) {
            return f;
        }
    }
    return NULL;
}

static void link_child(struct file *dir, struct file *child) {
    child->parent = dir;
    child->next = dir->children;
    dir->children = child;
}

static int unlink_child(struct file *parent, struct file *target) {
    struct file **link = &parent->children;
    while (*link != NULL && *link != target) {
        link = &(*link)->next;
    }
    if (*link == NULL) {
        return 0;
    }
    *link = target->next;
    target->next = NULL;
    return 1;
}

// Parsing the path

enum FS_RETURN parse_path(const char *path, struct file **result) {
    struct file *current = &root;

    if (path == NULL || result == NULL) {
        return FS_INVALID_PATH;
    }
    while (*path == '/') {
        ++path;
    }
    while (*path != '\0') {
        const char *start = path;
        size_t length;
        struct file *child;

        while (*path != '\0' && *path != '/') {
            ++path;
        }
        length = (size_t)(path - start);

        while (*path == '/') {
            ++path;
        }

        if (length == 1 && start[0] == '.') {
            continue;
        }

        if (length == 2 && start[0] == '.' && start[1] == '.') {
            current = current->parent;
            continue;
        }

        if (!(current->flags & FS_IS_DIRECTORY)) {
            return FS_NOT_A_DIR;
        }

        child = current->children;
        while (child != NULL) {
            if (strlen(child->name) == length &&
                memcmp(child->name, start, length) == 0) {
                break;
            }
            child = child->next;
        }

        if (child == NULL) {
            return FS_NOT_FOUND;
        }

        current = child;
    }

    *result = current;
    return FS_SUCCESS;
}

// File read/write

enum FS_RETURN read_file_content(const char *path, void* buffer, size_t buf_size) {
    struct file *b;
    enum FS_RETURN a = parse_path(path, &b);
    if (!(a&FS_SUCCESS)) {
        return a;
    }
    // Ultimate security check
    if (b->flags & FS_IS_DIRECTORY) {
        return FS_IS_A_DIR;
    } else if (buf_size < b->size) {
        return FS_BUF_TOO_SMALL;
    } else if (b->size != 0 && b->data == NULL) {
        return FS_INVALID_FILE;
    }
    if (b->size != 0 && buffer == NULL)
        return FS_INV_ARG;
    memcpy(buffer, b->data, b->size);
    return FS_SUCCESS;
}
enum FS_RETURN write_file_content(const char *path, void* buffer, size_t buf_size) {
    struct file *b;
    enum FS_RETURN a = parse_path(path, &b);
    if (!(a&FS_SUCCESS)) {
        return a;
    }
    // Ultimate security check
    if (b->flags & FS_IS_DIRECTORY) {
        return FS_IS_A_DIR;
    } else if (buf_size > b->size) {
        return FS_FILE_TOO_SMALL;
    } else if (buf_size != 0 && b->data == NULL) {
        return FS_INVALID_FILE;
    }
    if (b->size != 0 && buffer == NULL)
        return FS_INV_ARG;
    memcpy(b->data, buffer, buf_size);
    return FS_SUCCESS;
}

// File manipulation

enum FS_RETURN delete_file(const char *path) {
    struct file *target;
    enum FS_RETURN status = parse_path(path, &target);

    if (status != FS_SUCCESS) {
        return status;
    } else if (target == &root) {
        return FS_PERMISSION_DENIED;
    } else if ((target->flags & FS_IS_DIRECTORY) && target->children != NULL) {
        return FS_DIR_NOT_EMPTY;
    } else if (target->parent == NULL || !unlink_child(target->parent, target)) {
        return FS_INVALID_FS;
    }

    kfree(target->name);
    kfree(target->data);
    kfree(target);
    return FS_SUCCESS;
}
enum FS_RETURN create_file(const char *path, const void *data, size_t size, enum FS_FLAGS flags) {
    char *parent_path;
    char *name;
    const char *slash;
    size_t parent_len;
    size_t name_len;
    struct file *parent;
    struct file *file;
    enum FS_RETURN status;
    bool is_dir = false;

    if (flags & FS_IS_DIRECTORY)
        is_dir = true;

    if (path == NULL || (data == NULL && size != 0 && is_dir == false)) {
        return FS_INV_ARG;
    }

    size_t path_len = strlen(path);
    if (path_len == 0 || path[path_len - 1] == '/') {
        return FS_INV_PATH;
    }

    slash = strrchr(path, '/');

    if (slash == NULL) {
        parent_path = kmalloc(2);
        if (parent_path == NULL) {
            return FS_FAIL;
        }
        strcpy(parent_path, ".");
        name_len = path_len;
        slash = path;
    } else {
        parent_len = (size_t)(slash - path);
        parent_path = kmalloc(parent_len + 2);
        if (parent_path == NULL) {
            return FS_FAIL;
        }

        if (parent_len == 0) {
            strcpy(parent_path, "/");
        } else {
            memcpy(parent_path, path, parent_len);
            parent_path[parent_len] = '\0';
        }

        name_len = path_len - (size_t)(slash + 1 - path);
        ++slash;
    }

    name = kmalloc(name_len + 1);
    if (name == NULL) {
        kfree(parent_path);
        return FS_FAIL;
    }
    memcpy(name, slash, name_len);
    name[name_len] = '\0';

    if (strcmp(name, ".") == 0 || strcmp(name, "..") == 0) {
        kfree(name);
        kfree(parent_path);
        return FS_FAIL;
    }

    status = parse_path(parent_path, &parent);
    kfree(parent_path);

    if (status != FS_SUCCESS) {
        kfree(name);
        return status;
    }

    if (!(parent->flags & FS_IS_DIRECTORY)) {
        kfree(name);
        return FS_NOT_A_DIR;
    }

    if (find_child(parent, name) != NULL) {
        kfree(name);
        return FS_ALR_EXIST;
    }

    file = kcalloc(1, sizeof(*file));
    if (file == NULL) {
        kfree(name);
        return FS_FAIL;
    }

    if (size != 0) {
        file->data = kmalloc(size);
        if (file->data == NULL) {
            kfree(name);
            kfree(file);
            return FS_FAIL;
        }
        memcpy(file->data, data, size);
    }

    file->name = name;
    file->size = size;
    file->flags = flags;

    link_child(parent, file);
    return FS_SUCCESS;
}

// Folder manipulation

enum FS_RETURN create_folder(const char *path) {
    enum FS_RETURN folder_ret = create_file(path, 0, 0, FS_IS_DIRECTORY);
    if (!(folder_ret & FS_SUCCESS))
        return folder_ret;
    return FS_SUCCESS;
}
enum FS_RETURN delete_folder(const char *path) {
    enum FS_RETURN folder_ret = delete_file(path);
    if (!(folder_ret & FS_SUCCESS))
        return folder_ret;
    return FS_SUCCESS;
}

// Listing and discovery
enum FS_RETURN list_files(const char *path, struct file** list) {
    struct file *target_folder;
    if (list == NULL)
        return FS_INV_ARG;
    enum FS_RETURN a = parse_path(path, &target_folder);
    if (!(a & FS_SUCCESS))
        return a;
    // Return list of files in dir
    *list = target_folder->children;
    return FS_SUCCESS; // Finish
}

// Useful (DO NOT EXPOSE TO PLAYGROUND)
enum FS_RETURN create_file_force(const char *path, const void *data, size_t size, enum FS_FLAGS flags) {
    // Forcefully create file, if path doesnt exists, create all folders until it fits
    struct file *current = &root;
    const char* p = path;

    if (path == NULL || (data == NULL && size != 0)) {
        return FS_INV_ARG;
    }

    while (*p == '/') {
        ++p;
    }

    while (*p != '\0') {
        const char *name = p;
        size_t name_length;
        while (*p != '\0' && *p != '/') ++p;
        name_length = (size_t)(p - name);
        while (*p == '/') ++p;
        bool is_last = (*p == '\0');
        
        // test if exists
        struct file *b;
        enum FS_RETURN c;
        if (!(parse_path(p, &b) & FS_SUCCESS)) {
            if (VERBOSE) print("[+] creating item: %s \n", name);
            // Doesnt exists
            if (is_last) { // If is last = means its file
                c = create_file(p, data, size, flags);
            } else { // is folder
                c = create_folder(p);
            }

            if (!(c&FS_SUCCESS)) {
                if (VERBOSE) print(
                    "[-] Failed to create file: path=%s, name=%s \n", p, name);
                return FS_FAIL;
            }
            continue;
        } else {
            // Does exists
            continue;
        }
    }
    if (VERBOSE) print("[+] finished creating path: %s", path);
    return FS_SUCCESS;
}


/*
About: This is a simple ram file system. Used for initialal ram disk (initramdisk) passed from
bootloader. Its pourpuse is really simple serve at start and later be useless. 
There isn't much use of it when you enter main stage (playground) the playground will 
prefer to be on disk, but maximal configurabilty doesn't allow us to just throw it away later
matters by config. 
*/
void init_fs(BootInfo *boot) {
    print("Generating virutal ramsys...\n");
    (void)boot;
    root.id = 0;
    root.name = "root";
    root.parent = &root; // Chained to itself
    root.children = 0; // not children YET
    root.next = 0; // No siblings
    root.flags = FS_IS_DIRECTORY;

    print("Root generated...\n");
    print("Extracting ramdisk...");
    int ret_code = parse_cpio_disk(
        (void*)boot->initramfs_address, 
        boot->initramfs_size
    );
    if (ret_code == 0)
        print("OK (0)");
    else {
        print("ERR (%d)", ret_code);
        // Panic, because we have no way of getting drivers, config tables, etc.
        kernel_panic("Failed to retrive initial ramdisk.");
    }
    // Done, everything should we accessible thru FS.h lib
}
