#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "buf.h"
#include "super.h"
#include "./socket/client.h"


void rm_lru(struct buf *bp) ;


void zero_block(struct buf * bp){
    char * data = (char *)&bp->b;
    memset(data, 0, S_BLOCK_SIZE);
    bp->b_dirt = DIRTY;
}


/*===========================================================================*
 *				get_block				     *
 *===========================================================================*/
struct buf *get_block(block_t block)
{

  int b;
  struct buf *bp, *prev_ptr;

  /* Search the hash chain for (dev, block). Do_read() can use
   * get_block(NO_DEV ...) to get an unnamed block to fill with zeros when
   * someone wants to read from a hole in a file, in which case this search
   * is skipped
   */
  b = (int) block & HASH_MASK;
  bp = buf_hash[b];
  while (bp != NIL_BUF) {
      if (bp->b_blocknr == block ) {
          /* printf("here\n"); */
          /*bp->b_count = 0, means this block is in the lru chain*/
          if (bp->b_count == 0)
              rm_lru(bp);
          bp->b_count ++;
          return(bp);
      } else {
          bp = bp->b_hash;
      }
  }

  bp = front;
  if (bp == NIL_BUF){
    printf("Get_Block: error occurs: all buffers in use\n");
    exit(1);
  }
  /* if (block == 129) { */
    /* printf("front block number is %d\n", bp->b_blocknr); */
  /* } */
  rm_lru(bp);

  /* Remove the block that was just taken from its hash chain. */
  /* if (bp->b_blocknr == 1){ */
      /* printf("here"); */
  /* } */
  b = (int) bp->b_blocknr & HASH_MASK;
  prev_ptr = buf_hash[b];
  if (prev_ptr == bp) {
	buf_hash[b] = bp->b_hash;
  } else {
	/* The block just taken is not on the front of its hash chain. */
	while (prev_ptr->b_hash != NIL_BUF)
		if (prev_ptr->b_hash == bp) {
			prev_ptr->b_hash = bp->b_hash;	/* found it */
			break;
		} else {
			prev_ptr = prev_ptr->b_hash;	/* keep looking */
		}
  }

  /* If the block taken is dirty, make it clean by writing it to the disk.
   */
  if (bp->b_dirt == DIRTY)
      write_block(bp->b_blocknr, (char *)&bp->b);

  /* Fill in block's parameters and add it to the hash chain where it goes. */
  bp->b_blocknr = block;	/* fill in block number */
  bp->b_count = 1;	/* fill in block number */
  b = (int) bp->b_blocknr & HASH_MASK;
  bp->b_hash = buf_hash[b];
  buf_hash[b] = bp;		/* add to hash list */

  read_block(block, (char *)&bp->b);
  return(bp);			/* return the newly acquired block */
}

/*===========================================================================*
 *				put_block				     *
 *===========================================================================*/
void put_block(struct buf *bp)
{
  if (bp == NIL_BUF) return;	/* it is easier to check here than in caller */

  if (--bp->b_count == 0){

      bufs_in_use--;		/* one fewer block buffers in use */

      bp->b_prev = rear;
      bp->b_next = NIL_BUF;
      if (rear == NIL_BUF){
          printf("here\n");
          exit(0);
          front = bp;
      }
      else {
          rear->b_next = bp;
          /* if (bp->b_blocknr == 1) */
              /* printf("rear buf number is %d, and pre buf number is %d\n",rear->b_next->b_blocknr, rear->b_blocknr); */
      }
      rear = bp;
  }
}

/*===========================================================================*
 *				alloc_zone				     *
 *===========================================================================*/
zone_t alloc_zone(zone_t z)
{

  int major, minor;
  bit_t b, bit;
  struct super_block *sp;

  /* Note that the routine alloc_bit() returns 1 for the lowest possible
   * zone, which corresponds to S_FIRSTDATAZONE.  To convert a value
   * between the bit number, 'b', used by alloc_bit() and the zone number, 'z',
   * stored in the inode, use the formula:
   *     z = b + S_FIRSTDATAZONE - 1
   * Alloc_bit() never returns 0, since this is used for NO_BIT (failure).
   */
  sp = &super_block;

  /* If z is 0, skip initial part of the map known to be fully in use. */
  if (z == S_FIRSTDATAZONE) {
	bit = sp->s_zsearch;
  } else {
	bit = (bit_t) z - (S_FIRSTDATAZONE);
  }
  b = alloc_bit(ZMAP, bit);
  if (b == NO_BIT) {
	printf("No space to allocate zone\n");
    exit(1);
  }
  if (z == S_FIRSTDATAZONE) sp->s_zsearch = b;	/* for next time */
  return(S_FIRSTDATAZONE + (zone_t) b);
}

/*===========================================================================*
 *				free_zone				     *
 *===========================================================================*/
void free_zone(zone_t numb)
{
/* Return a zone. */

  struct super_block *sp;
  bit_t bit;

  /* Locate the appropriate super_block and return bit. */
  sp = &super_block;
  if (numb < S_FIRSTDATAZONE || numb >= S_ZONES) return;
  bit = (bit_t) (numb - (S_FIRSTDATAZONE));
  free_bit(ZMAP, bit);
  if (bit < sp->s_zsearch) sp->s_zsearch = bit;
}

/*===========================================================================*
 *				rm_lru					     *
 *===========================================================================*/
void rm_lru(struct buf *bp)
{
/* Remove a block from its LRU chain. */
  struct buf *next_ptr, *prev_ptr;

  /* printf("first buf block number is %d\n", bp->b_blocknr); */
  bufs_in_use++;
  next_ptr = bp->b_next;	/* successor on LRU chain */
  prev_ptr = bp->b_prev;	/* predecessor on LRU chain */
  if (prev_ptr != NIL_BUF)
	prev_ptr->b_next = next_ptr;
  else
	front = next_ptr;	/* this block was at front of chain */

  /* if ((bp->b_blocknr == 0) && (next_ptr == NIL_BUF)){ */
      /* printf("here"); */
  /* } */
  if (next_ptr != NIL_BUF)
	next_ptr->b_prev = prev_ptr;
  else
	rear = prev_ptr;	/* this block was at rear of chain */
}
