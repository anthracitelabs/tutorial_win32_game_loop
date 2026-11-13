#if !defined(GAME_PLATFORM_H)
#define GAME_PLATFORM_H

typedef unsigned char      uint8;
typedef unsigned short     uint16;
typedef unsigned int       uint32;
typedef unsigned long long uint64;

typedef signed char        int8;
typedef signed short       int16;
typedef signed int         int32;
typedef signed long long   int64;

typedef int32                b32; // Used explicitly for boolean values (padded to 32 bits for speed)


#define ArrayCount(Array) (sizeof(Array) / sizeof((Array)[0]))

struct game_memory
{
    bool IsInitialized;

    uint64 PermanentStorageSize;
    void *PermanentStorage; // Must be cleared to zero at startup

    uint64 TransientStorageSize;
    void *TransientStorage; // Must be cleared to zero at startup
};

#define PushStruct(Arena, type) (type *)PushSize_(Arena, sizeof(type))
#define PushArray(Arena, Count, type) (type *)PushSize_(Arena, (Count)*sizeof(type))

struct memory_arena
{
    size_t Size;
    uint8_t *Base;
    size_t Used;
};

void InitializeArena(memory_arena *Arena, size_t Size, void *Base)
{
    Arena->Size = Size;
    Arena->Base = (uint8_t *)Base;
    Arena->Used = 0;
}

void *PushSize_(memory_arena *Arena, size_t Size)
{
    // Ensure you don't run out of memory
    //Assert((Arena->used + Size) <= Arena->size);
    
    // Grab the pointer to the start of this specific allocation
    void *Result = Arena->Base + Arena->Used;
    
    // Move the pointer forward for the next allocation
    Arena->Used += Size;
    
    return(Result);
}

// may be a different header file?
typedef enum {
	CMD_CLEAR = 1,
	CMD_DRAW_RECTANGLE
} CmdType;

typedef struct {
    uint16_t type;
    uint16_t size;
} CmdHeader;

typedef struct {
    float r, g, b, a;
} CmdClear;

typedef struct {
    float x, y, w, h;
    float r, g, b, a;
} CmdDrawRectangle;

typedef struct {
    uint8_t* buffer;
    size_t capacity;
    size_t offset;
} CommandBuffer;

void cmd_buffer_reset(CommandBuffer* cb) {
    cb->offset = 0;
}

// Core allocation function: writes header and returns pointer to payload
void* cmd_buffer_push_raw(CommandBuffer* cb, uint16_t type, size_t payload_size) {
    // Align current offset to an 8-byte boundary
    size_t aligned_offset = (cb->offset + 7) & ~((size_t)7);
    size_t total_size = sizeof(CmdHeader) + payload_size;

    //assert(aligned_offset + total_size <= cb->capacity && "CommandBuffer out of memory!");

    CmdHeader* header = (CmdHeader*)(cb->buffer + aligned_offset);
    header->type = type;
    header->size = (uint16_t)total_size;

    cb->offset = aligned_offset + total_size;

    // Return memory address immediately following the header
    return (void*)(header + 1);
}

// Type-safe helper macro for pushing structs
#define CMD_PUSH(cb, type_enum, struct_type) \
    ((struct_type*)cmd_buffer_push_raw((cb), (type_enum), sizeof(struct_type)))

#endif
