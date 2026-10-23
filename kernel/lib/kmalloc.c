#include "kmalloc.h"

// AI !!! AI !!! AI !!!
// TODO: Remove this shit 

#define KMALLOC_STATE_FREE 0xA1A1A1A1
#define KMALLOC_STATE_USED 0xBFBFBFBF

#define KMALLOC_ALIGNMENT 16
#define KMALLOC_MIN_SPLIT 32

typedef struct kmalloc_chunk {
    uint64_t state;
    uint64_t length;              // Includes this header.
    struct kmalloc_chunk *next;
    struct kmalloc_chunk *prev;
} kmalloc_chunk_t;

static kmalloc_chunk_t *head = NULL;

static uint64_t heap_start = 0;
static uint64_t heap_end = 0;


static uint64_t align_up(uint64_t value, uint64_t alignment)
{
    return (value + alignment - 1) &
           ~(alignment - 1);
}


void kmalloc_init(void)
{
    head = NULL;
    heap_start = 0;
    heap_end = 0;
}


static void kmerge_next(kmalloc_chunk_t *chunk)
{
    if (!chunk)
        return;

    if (chunk->state != KMALLOC_STATE_FREE)
        return;

    kmalloc_chunk_t *next = chunk->next;

    if (!next)
        return;

    if (next->state != KMALLOC_STATE_FREE)
        return;

    chunk->length += next->length;
    chunk->next = next->next;

    if (chunk->next)
        chunk->next->prev = chunk;
}


static bool kmalloc_grow(uint64_t size)
{
    uint64_t bytes = align_up(size, PAGE_SIZE);
    uint64_t pages = bytes / PAGE_SIZE;

    uint64_t address = vallocs(true, pages);

    if (address == 0) {
        print("kmalloc: vallocs failed\n");
        return false;
    }

    /*
     * First heap allocation.
     */
    if (head == NULL) {

        heap_start = address;
        heap_end = address + bytes;

        head = (kmalloc_chunk_t *)address;

        head->state = KMALLOC_STATE_FREE;
        head->length = bytes;
        head->next = NULL;
        head->prev = NULL;

        return true;
    }


    /*
     * The VMM must give us the page range directly
     * after the current heap.
     */
    if (address != heap_end) {

        print("kmalloc: vallocs returned non-contiguous memory\n");
        print("requested: %lx\n", address);
        print("expected:  %lx\n", heap_end);

        vfrees(address, pages);

        return false;
    }


    /*
     * Find last chunk.
     */
    kmalloc_chunk_t *last = head;

    while (last->next)
        last = last->next;


    /*
     * New free chunk starts at the newly allocated pages.
     */
    kmalloc_chunk_t *new_chunk =
        (kmalloc_chunk_t *)address;

    new_chunk->state = KMALLOC_STATE_FREE;
    new_chunk->length = bytes;
    new_chunk->next = NULL;
    new_chunk->prev = last;

    last->next = new_chunk;

    heap_end += bytes;


    /*
     * If the old last chunk was free, merge.
     */
    if (last->state == KMALLOC_STATE_FREE)
        kmerge_next(last);

    return true;
}


static void ksplit(kmalloc_chunk_t *chunk,
                    uint64_t length)
{
    kmalloc_chunk_t *new_chunk =
        (kmalloc_chunk_t *)(
            (uint8_t *)chunk + length
        );

    new_chunk->state = KMALLOC_STATE_FREE;

    new_chunk->length =
        chunk->length - length;

    new_chunk->prev = chunk;
    new_chunk->next = chunk->next;

    if (chunk->next)
        chunk->next->prev = new_chunk;

    chunk->next = new_chunk;

    chunk->length = length;
}


void *kmalloc(uint64_t size)
{
    if (size == 0)
        return NULL;


    /*
     * Round requested data size up.
     */
    size = align_up(size, KMALLOC_ALIGNMENT);


    /*
     * Include allocator header.
     */
    uint64_t required =
        size + sizeof(kmalloc_chunk_t);


    /*
     * Search existing free chunks.
     */
    kmalloc_chunk_t *chunk = head;

    while (chunk) {

        if (chunk->state == KMALLOC_STATE_FREE &&
            chunk->length >= required)
            break;

        chunk = chunk->next;
    }


    /*
     * No suitable chunk.
     */
    if (!chunk) {

        uint64_t grow_size = required;

        /*
         * Grow by at least 16 pages.
         */
        if (grow_size < 16 * PAGE_SIZE)
            grow_size = 16 * PAGE_SIZE;

        if (!kmalloc_grow(grow_size))
            return NULL;


        /*
         * Search again.
         */
        chunk = head;

        while (chunk) {

            if (chunk->state == KMALLOC_STATE_FREE &&
                chunk->length >= required)
                break;

            chunk = chunk->next;
        }


        if (!chunk) {
            print("kmalloc: no chunk after grow\n");
            return NULL;
        }
    }


    /*
     * Split if enough space remains.
     */
    if (chunk->length >=
        required +
        sizeof(kmalloc_chunk_t) +
        KMALLOC_MIN_SPLIT) {

        ksplit(chunk, required);
    }


    chunk->state = KMALLOC_STATE_USED;


    /*
     * Return memory immediately following header.
     */
    return (void *)(chunk + 1);
}

void *kcalloc(unsigned int num_of_items, size_t len_of_items) {
    if (num_of_items == 0 || len_of_items == 0)
        return NULL;
    int size = num_of_items * len_of_items;
    void* ptr = kmalloc(size);
    memset(ptr, 0, size);
    return ptr;
}


void kfree(void *ptr)
{
    if (!ptr)
        return;


    kmalloc_chunk_t *chunk =
        ((kmalloc_chunk_t *)ptr) - 1;


    if (chunk->state != KMALLOC_STATE_USED) {

        print("kmalloc: invalid free: %lx\n",
              (uint64_t)ptr);

        return;
    }


    chunk->state = KMALLOC_STATE_FREE;


    /*
     * Merge with previous first.
     */
    if (chunk->prev)
        kmerge_next(chunk->prev);


    /*
     * Then merge with next.
     */
    kmerge_next(chunk);
}
