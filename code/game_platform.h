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

struct game_memory
{
	int red;
	int green;
	int blue;
};

#define ArrayCount(Array) (sizeof(Array) / sizeof((Array)[0]))

#endif
