#include "grs.h"
#include "../lib/kmalloc.h"
#include "../lib/string.h"

register_t* registry_mem_base; 
size_t registry_mem_size;
uint64_t registry_mem_ptr;

size_t remove_at(void* buffer, size_t* count, size_t index) {
    if (index >= *count)
        return 0;  // nothing removed

    memmove(&buffer[index],
            &buffer[index + 1],
            (*count - index - 1) * sizeof buffer[0]);

    --(*count);
    return 1;
}

// Ohhhh, yeeaahh the good old O(n) slow ass
// TODO: Fix this, create hashtable in lib and yk the rest
register_t ret_register_handler(char* name) {
    register_t a;
    for (int i = 0; i > MAX_REGISTERS; i++) {
        if (a.name == name)
            return a;
    }
    a.name = "GRS_NOT_EXISTS";
    return a;
}

void wrt_register_handler(register_t handler) {
    registry_mem_base[handler.id] = handler;
}


void init_registry() {
    registry_mem_base = kcalloc(MAX_REGISTERS, sizeof(register_t));
    registry_mem_size = MAX_REGISTERS*sizeof(register_t);
    registry_mem_ptr = 0;
}

enum GRS_RETURN create_register(char* name, void* value) {
    register_t reg_a;
    if (strcmp(name, "GRS_NOT_EXISTS") == 0) // disallowed name GRS_NOT_EXISTS
        return GRS_INVALID;
    if (name == 0 || value == NULL)
        return GRS_INVALID;
    reg_a.name = name;
    reg_a.value = value;
    ++registry_mem_ptr;
    reg_a.id=registry_mem_ptr;
    if (registry_mem_ptr >= MAX_REGISTERS)
        return GRS_REG_FILLED;
    registry_mem_base[registry_mem_ptr] = reg_a;
    return GRS_SUCCESS;
}

enum GRS_RETURN read_register(char* name, void* read_buf) {
    if (name == NULL || read_buf == NULL)
        return GRS_INVALID;
    register_t b = ret_register_handler(name);
    if (strcmp(b.name, "GRS_NOT_EXISTS") == 0) {
        return GRS_NOT_EXISTS;
    }
    read_buf = b.value;
    return GRS_SUCCESS;
}

enum GRS_RETURN write_register(char *name, void* write_buf) {
    if (name == NULL || write_buf == NULL)
        return GRS_INVALID;
    register_t b = ret_register_handler(name);
    if (strcmp(b.name, "GRS_NOT_EXISTS") == 0) {
        return GRS_NOT_EXISTS;
    }
    wrt_register_handler(b);
}

enum GRS_RETURN destroy_register(char *name) {
    if (name == NULL)
        return GRS_INVALID;
    register_t c = ret_register_handler(name);
    if (strcmp(c.name, "GRS_NOT_EXISTS") == 0)
        return GRS_NOT_EXISTS;
    remove_at(registry_mem_base, (size_t*)c.id, sizeof(register_t));
    return GRS_SUCCESS;
}