#include <stdio.h>
#include <stdint.h>
#include "bitmap.h"
#include "block_store.h"
// include more if you need

// You might find this handy.  I put it around unused parameters, but you should
// remove it before you submit. Just allows things to compile initially.
#define UNUSED(x) (void)(x)

typedef struct{
    //unsigned char block[BLOCK_SIZE_BYTES];
    unsigned char block[BLOCK_SIZE_BYTES];
}block_t;

//struct block_store block_store_t;
typedef struct block_store{
    block_t blocks[BLOCK_STORE_NUM_BLOCKS];
    bitmap_t *fbm;
}block_store_t;


block_store_t *block_store_create()
{
    // block index where the fbm bitmap starts
    int fbmStartIndex = 127;
    block_store_t *bs = (block_store_t *)calloc(sizeof(block_store_t),1);;
    // number of bits in the bitmap equals the number of blocks
    bs->fbm = bitmap_overlay(BLOCK_STORE_NUM_BLOCKS, &bs->blocks[fbmStartIndex]);
    
    // calculate number of blocks required to store the bitmap
    int bitmapNumBlocks = BITMAP_SIZE_BYTES/BLOCK_SIZE_BYTES;
    if((BITMAP_SIZE_BYTES%BLOCK_SIZE_BYTES) > 0){
        bitmapNumBlocks += 1;
    }
    // mark each block used by the bitmap as used in the fbm
    // block id = block index
    for(int i = fbmStartIndex; i < fbmStartIndex+bitmapNumBlocks; i++){
        block_store_request(bs, i);
    }
    
    //bs->fbm = (bitmap_t*)bs->blocks[fbmStartIndex];
    
    // block id = block index
    //block_store_request(bs, fmbStartIndex);
    
    return bs;
}

void block_store_destroy(block_store_t *const bs)
{
    if(bs != NULL){
        free(bs);
    }
}
size_t block_store_allocate(block_store_t *const bs)
{
    if(bs == NULL){
        return SIZE_MAX;
    }
    int id = bitmap_ffz(bs->fbm);
    if(id > BLOCK_STORE_AVAIL_BLOCKS){
        return SIZE_MAX;
    }
    block_store_request(bs, id);
    
    return id;
}

bool block_store_request(block_store_t *const bs, const size_t block_id)
{
    //check for bad parameters, block id is equal to the block index
    if((bs == NULL) || (block_id > BLOCK_STORE_AVAIL_BLOCKS)){
        return false;
    }   
    // return false if the requested block is in use
    if(bitmap_test(bs->fbm, block_id)){
        return false;
    }
    // set the block corresponding to the block id to used, block 127 is the fbm
    bitmap_set(bs->fbm, block_id);
    return true;
}

void block_store_release(block_store_t *const bs, const size_t block_id)
{
    if((bs != NULL) && (block_id <= BLOCK_STORE_AVAIL_BLOCKS)){
        // set the given bit in the bitmap to zero
        bitmap_reset(bs->fbm, block_id);
    }
}

size_t block_store_get_used_blocks(const block_store_t *const bs)
{
    UNUSED(bs);
    return 0;
}

size_t block_store_get_free_blocks(const block_store_t *const bs)
{
    UNUSED(bs);
    return 0;
}

size_t block_store_get_total_blocks()
{
    return 0;
}

size_t block_store_read(const block_store_t *const bs, const size_t block_id, void *buffer)
{
    UNUSED(bs);
    UNUSED(block_id);
    UNUSED(buffer);
    return 0;
}

size_t block_store_write(block_store_t *const bs, const size_t block_id, const void *buffer)
{
    UNUSED(bs);
    UNUSED(block_id);
    UNUSED(buffer);
    return 0;
}

block_store_t *block_store_deserialize(const char *const filename)
{
    UNUSED(filename);
    return NULL;
}

size_t block_store_serialize(const block_store_t *const bs, const char *const filename)
{
    UNUSED(bs);
    UNUSED(filename);
    return 0;
}