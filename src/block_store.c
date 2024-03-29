#include <stdio.h>
#include <stdint.h>
#include <string.h>
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

/*
typedef struct block_store{
    char* blocks[BLOCK_STORE_NUM_BLOCKS][BLOCK_SIZE_BYTES];
    bitmap_t *fbm;
} block_store_t;*/

///
/// This creates a new BS device, ready to go
/// \return Pointer to a new block storage device, NULL on error
///
block_store_t *block_store_create()
{
    // block index where the fbm bitmap starts
    int fbmStartIndex = 127;
    // use calloc so the memory containing the fbm is initialized to all zeros
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
    return bs;
}

///
/// Destroys the provided block storage device
/// This is an idempotent operation, so there is no return value
/// \param bs BS device
///
void block_store_destroy(block_store_t *const bs)
{
    if(bs != NULL){
        // free the memory used by the block store
        free(bs);
    }
}

///
/// Searches for a free block, marks it as in use, and returns the block's id
/// \param bs BS device
/// \return Allocated block's id, SIZE_MAX on error
///
size_t block_store_allocate(block_store_t *const bs)
{
    if(bs == NULL){
        return SIZE_MAX;
    }
    // find the the first zero (unused block) in the fbm
    int id = bitmap_ffz(bs->fbm);
    // return SIZE_MAX if the end of the file is reached without a free block
    if(id > BLOCK_STORE_AVAIL_BLOCKS){
        return SIZE_MAX;
    }
    block_store_request(bs, id);
    
    return id;
}

///
/// Attempts to allocate the requested block id
/// \param bs the block store object
/// \block_id the requested block identifier
/// \return boolean indicating succes of operation
///
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

///
/// Frees the specified block
/// \param bs BS device
/// \param block_id The block to free
///
void block_store_release(block_store_t *const bs, const size_t block_id)
{
    if((bs != NULL) && (block_id <= BLOCK_STORE_AVAIL_BLOCKS)){
        // set the given bit in the bitmap to zero
        bitmap_reset(bs->fbm, block_id);
    }
}

///
/// Counts the number of blocks marked as in use
/// \param bs BS device
/// \return Total blocks in use, SIZE_MAX on error
///
size_t block_store_get_used_blocks(const block_store_t *const bs)
{
    if(bs != NULL){
        return bitmap_total_set(bs->fbm);
    }
    else { return SIZE_MAX; }
}

///
/// Counts the number of blocks marked free for use
/// \param bs BS device
/// \return Total blocks free, SIZE_MAX on error
///
size_t block_store_get_free_blocks(const block_store_t *const bs)
{
    if(bs != NULL){
        // calculate the total free blocks
        return (BLOCK_STORE_AVAIL_BLOCKS) - bitmap_total_set(bs->fbm);
    }
    else { return SIZE_MAX; }
}

///
/// Returns the total number of user-addressable blocks
///  (since this is constant, you don't even need the bs object)
/// \return Total blocks
///
size_t block_store_get_total_blocks()
{
    return BLOCK_STORE_AVAIL_BLOCKS;
}

///
/// Reads data from the specified block and writes it to the designated buffer
/// \param bs BS device
/// \param block_id Source block id
/// \param buffer Data buffer to write to
/// \return Number of bytes read, 0 on error
///
size_t block_store_read(const block_store_t *const bs, const size_t block_id, void *buffer)
{
    if((bs != NULL) && (block_id < BLOCK_STORE_NUM_BLOCKS) && (buffer != NULL)){
        // copy the block specified by the block_id to the given buffer
        memcpy(buffer, &bs->blocks[block_id], BLOCK_SIZE_BYTES);
        return BLOCK_SIZE_BYTES;
    }
    return 0;
}

///
/// Reads data from the specified buffer and writes it to the designated block
/// \param bs BS device
/// \param block_id Destination block id
/// \param buffer Data buffer to read from
/// \return Number of bytes written, 0 on error
///
size_t block_store_write(block_store_t *const bs, const size_t block_id, const void *buffer)
{
    if((bs != NULL) && (block_id < BLOCK_STORE_NUM_BLOCKS) && (buffer != NULL)){
        // write the data from the buffer to the block specified by the block_id
        memcpy(&bs->blocks[block_id], buffer, BLOCK_SIZE_BYTES);
        return BLOCK_SIZE_BYTES;
    }
    return 0;
}

///
/// Imports BS device from the given file - for grads/bonus
/// \param filename The file to load
/// \return Pointer to new BS device, NULL on error
///
block_store_t *block_store_deserialize(const char *const filename)
{
    if(filename == NULL){
        return NULL;
    }
    FILE * fp;
    fp = fopen (filename, "r");
    // return NULL if error opening file
    if(fp == NULL){
        return NULL;
    }
    
    block_store_t * bs = block_store_create();
    // read BLOCK_STORE_NUM_BLOCKS number of blocks, each of size BLOCK_SIZE_BYTES
    // from the file to the empty block store array
    int elementsRead = fread(bs->blocks, BLOCK_SIZE_BYTES, BLOCK_STORE_NUM_BLOCKS, fp);
    // an error occured if all of the blocks were not read from the file
    if(elementsRead != BLOCK_STORE_NUM_BLOCKS){
        printf("Error reading file\n");
    }
    return bs;
}

///
/// Writes the entirety of the BS device to file, overwriting it if it exists - for grads/bonus
/// \param bs BS device
/// \param filename The file to write to
/// \return Number of bytes written, 0 on error
///
size_t block_store_serialize(const block_store_t *const bs, const char *const filename)
{
    if(filename == NULL){
        return 0;
    }
    FILE * fp;
    fp = fopen (filename, "w");
    // return 0 if error opening file
    if(fp == NULL){
        return 0;
    }
    // write the enitre block store array the the file
    int elementsWritten = fwrite(bs->blocks, BLOCK_SIZE_BYTES, BLOCK_STORE_NUM_BLOCKS, fp);
    
    // elements written should be the same as BLOCK_STORE_NUM_BLOCKS
    // returns the total number of bytes written
    return elementsWritten*BLOCK_SIZE_BYTES;
}
