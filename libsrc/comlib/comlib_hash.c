#include "gendef.h"
#include "comlib.h"
#include "comlib.x"

/*
 * Block read - if your platform needs to do endian-swapping or can only
 * handle aligned reads, do the conversion here
 */
#ifdef __GNUC__
#define FORCE_INLINE __attribute__((always_inline)) INLINE
#else
#define FORCE_INLINE INLINE 
#endif

FT_PRIVATE FORCE_INLINE U_32      hashTbl_rotl32      (U_32 x, U_8 r);
FT_PRIVATE FORCE_INLINE U_64      hashTbl_rotl64      (U_64 x, U_8 r);
FT_PRIVATE FORCE_INLINE U_32      hashTbl_fmix32      (U_32 h);

FT_PRIVATE FORCE_INLINE U_32 hashTbl_rotl32 (U_32 x, U_8 r)
{
      return (x << r) | (x >> (32 - r));
}

FT_PRIVATE FORCE_INLINE U_64 hashTbl_rotl64 (U_64 x, U_8 r)
{
      return (x << r) | (x >> (64 - r));
}

/*
 * Finalization mix - force all bits of a hash block to avalanche
 */
FT_PRIVATE FORCE_INLINE U_32 hashTbl_fmix32 (U_32 h)
{
    h ^= h >> 16;
    h *= 0x85ebca6b;
    h ^= h >> 13;
    h *= 0xc2b2ae35;
    h ^= h >> 16;

    return h;
}

FT_PUBLIC RT_RESULT comlib_hashMurMur(CONST VOID *key, CONST UINT keyLen, CONST U_32 seed, 
                                      REGISTER UINT *rt_hashKey)
{
    CONST U_8* data = (CONST U_8*)key;
    CONST SINT nblocks = keyLen / 4;
    SINT i = 0;

    U_32 h1 = seed;

    U_32 c1 = 0xcc9e2d51;
    U_32 c2 = 0x1b873593;

    /* body */
    CONST U_32 *blocks = (CONST U_32*)(data + nblocks*4);

    for(i = -nblocks; i; i++){
        U_32 k1 = blocks[i];

        k1 *= c1;
        k1 = hashTbl_rotl32(k1,15);
        k1 *= c2;

        h1 ^= k1;
        h1 = hashTbl_rotl32(h1,13);
        h1 = h1*5+0xe6546b64;
    }

    /* tail */
    CONST U_8* tail = (CONST U_8*)(data + nblocks*4);

    U_32 k1 = 0;

    switch(keyLen & 3){
        case 3: k1 ^= tail[2] << 16;
        case 2: k1 ^= tail[1] << 8;
        case 1: k1 ^= tail[0];
                k1 *= c1; k1 = hashTbl_rotl32(k1,15); k1 *= c2; h1 ^= k1;
    };

    /* finalization */
    h1 ^= keyLen;

    h1 = hashTbl_fmix32(h1);

    (*rt_hashKey) = (UINT)h1;

    return RC_OK;
}

FT_PUBLIC RT_RESULT comlib_hashStr(CONST VOID *key, CONST UINT keyLen, REGISTER UINT *rt_hashKey)
{
    REGISTER UINT strIdx = 0;

    for(strIdx = 0; strIdx < keyLen;strIdx++){
        (*rt_hashKey) = (UINT)((31 * (*rt_hashKey)) + ((UCHAR*)key)[strIdx]);
        /* (*rt_hashKey) = (UINT)((((*rt_hashKey)<<5)-(*rt_hashKey))+ ((UCHAR*)key)[strIdx]); */
    }

    return RC_OK;
}

