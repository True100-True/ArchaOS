#ifndef GRS_H
#define GRS_H

#include <stdint.h>

#define MAX_REGISTERS 500

typedef struct register_t {
    char* name; // its name
    uint64_t id;
    void* value; // pointer to its value
} register_t;

enum GRS_RETURN {
    GRS_SUCCESS,
    GRS_FAILED,
    GRS_REG_FILLED,
    GRS_NOT_EXISTS,
    GRS_INVALID,
};

// This implementation should be good enough for now.
// Fix this atleast before 2.0 update

void init_registry(void);
enum GRS_RETURN create_register(char* name, void* value);
enum GRS_RETURN read_register(char* name, void* read_buf);
enum GRS_RETURN write_register(char *name, void* write_buf);
enum GRS_RETURN destroy_register(char *name);

#endif 