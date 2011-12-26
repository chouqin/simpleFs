#include <stdio.h>
#include <stdlib.h>
#include "buf.h"
#include "inode.h"
#include "super.h"


/*===========================================================================*
 *				alloc_bit				     *
 *===========================================================================*/
bit_t alloc_bit(int map,bit_t origin)
{
/* Allocate a bit from a bit map and return its bit number. */

  block_t start_block;		/* first bit block */
  bit_t map_bits;		/* how many bits are there in the bit map? */
  unsigned bit_blocks;		/* how many blocks are there in the bit map? */
  unsigned block, word, bcount;
  struct buf *bp;
  int *wptr, *wlim, k;
  bit_t i, b;

  struct super_block * sp = &super_block;

  if (map == IMAP) {
	start_block = START_BLOCK;
	map_bits = S_NINODES + 1; /*why + 1?*/
	bit_blocks = S_IMAP_BLOCKS;
  } else {
	start_block = START_BLOCK + S_IMAP_BLOCKS;
	map_bits = S_ZONES - S_FIRSTDATAZONE + 1;
	bit_blocks = S_ZMAP_BLOCKS;
  }

  /* Figure out where to start the bit search (depends on 'origin'). */
  if (origin >= map_bits) origin = 0;	/* for robustness */

  /* Locate the starting place. */
  block = origin / (512 * 8); /*bits per block: 512 * 8 */
  word = (origin % (512 * 8)) / 32;
  /* printf("start_block is %d and the block is %d\n", START_BLOCK, block); */

  /* Iterate over all blocks plus one, because we start in the middle. */
  bcount = bit_blocks + 1;
  do {
	bp = get_block(start_block + block);
	wlim = &bp->b_bitmap[NR_BIT_MAP];/*MAX_BLOCK_SIZE*/

	/* Iterate over the words in block. */
	for (wptr = &bp->b_bitmap[word]; wptr < wlim; wptr++) {

		/* Does this word contain a free bit? */
		if (*wptr == (int) ~0) continue;

		/* Find and allocate the free bit. */
		k = (unsigned) *wptr;
		for (i = 0; (k & (1 << i)) != 0; ++i) {}

		/* Bit number from the start of the bit map. */
		b = ((bit_t) block * 512 * 8)
		    + (wptr - &bp->b_bitmap[0]) * 32
		    + i;

		/* Don't allocate bits beyond the end of the map. */
		if (b >= map_bits) break;

		/* Allocate and return bit number. */
		k |= 1 << i;
		*wptr = (int) k;
		bp->b_dirt = DIRTY;
		put_block(bp);
		return(b);
	}
	put_block(bp);
	if (++block >= bit_blocks) block = 0;	/* last block, wrap around */
	word = 0;
  } while (--bcount > 0);
  return(NO_BIT);		/* no bit could be allocated */
}

/*===========================================================================*
 *				free_bit				     *
 *===========================================================================*/
void free_bit(int map, bit_t bit_returned)
{
/* Return a zone or inode by turning off its bitmap bit. */

  unsigned block, word, bit;
  struct buf *bp;
  int k, mask;
  block_t start_block;
  struct super_block * sp = &super_block;

  if (map == IMAP) {
	start_block = START_BLOCK;
  } else {
	start_block = START_BLOCK + S_IMAP_BLOCKS;
  }
  block = bit_returned / (512 * 8);
  word = (bit_returned % (512 * 8)) / 32;

  bit = bit_returned % 32;
  mask = 1 << bit;

  bp = get_block(start_block + block);

  k = (int) bp->b_bitmap[word];
  if (!(k & mask)) {
	printf("tried to free unused inode or zone\n");
    /* exit(0); */
  }

  k &= ~mask;
  bp->b_bitmap[word] = (int) k;
  bp->b_dirt = DIRTY;

  put_block(bp);
}

