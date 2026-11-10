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

#endif
