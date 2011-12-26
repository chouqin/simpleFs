/* This file manages the inode table.  There are procedures to allocate and
 * deallocate inodes, acquire, erase, and release them, and read and write
 * them from the disk.
 *
 * The entry points into this file are
 *   get_inode:	   search inode table for a given inode; if not there,
 *                 read it
 *   put_inode:	   indicate that an inode is no longer needed in memory
 *   alloc_inode:  allocate a new, unused inode
 *   wipe_inode:   erase some fields of a newly allocated inode
 *   free_inode:   mark an inode as available for a new file
 *   update_times: update atime, ctime, and mtime
 *   rw_inode:	   read a disk block and extract an inode, or corresp. write
 *   old_icopy:	   copy to/from in-core inode struct and disk inode (V1.x)
 *   new_icopy:	   copy to/from in-core inode struct and disk inode (V2.x)
 *   dup_inode:	   indicate that someone else is using an inode table entry
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "myfs.h"
#include "inode.h"
#include "buf.h"
#include "super.h"
#include "file.h"
#include "errno.h"






void rw_inode(struct inode *rip, int rw_flag);



/* void free_index1_zone(zone_t z, int pos){ */
    /* struct buf * bp; */
    /* int i = 0; */

    /* bp = get_block(z); */
    /* for (i = 0; i <= pos; ++i){ */
        /* [> printf("free zone %d\n", (int)bp->b_ind[i]); <] */
        /* free_zone(bp->b_ind[i]); */
    /* } */
    /* put_block(bp); */
/* } */

/* void free_index2_zone(zone_t z, int pos){ */
    /* struct buf * bp; */
    /* int i = 0; */
    /* int block_pos, boff; */

    /* bp = get_block(z); */
    /* block_pos = pos / NR_INDIRECT_BLOCK; */
    /* boff = pos % NR_INDIRECT_BLOCK; */

    /* for (i = 0; i < block_pos; ++ i){ */
        /* free_index1_zone(bp->b_ind[i], NR_INDIRECT_BLOCK - 1); */
    /* } */
    /* if (boff != 0){ */
        /* [>assert that this zone is not NO_ZONE<] */
        /* if (bp->b_ind[i] == NO_ZONE){ */
            /* printf("error: this indirect block can't be NO_ZONE\n"); */
            /* return; */
        /* } */
        /* free_index1_zone(bp->b_ind[i], boff - 1); */
    /* } */
    /* put_block(bp); */
/* } */


void empty_inode_space(struct inode *rip){
    int block_pos;
    int pos = 0;
    int i = 0;
    int b;

    if (rip->i_size == 0) {
        printf("try to empty a empty inode's space\n");
        return;
    }

    printf("try to empty a inode's space, size: %d\n", rip->i_size);


    for(; pos < rip->i_size; pos += S_BLOCK_SIZE){
        b = read_map(rip, pos);
        free_zone(b);
    }

    wipe_inode(rip);
    rip->i_size = 0;
    rip->i_dirt = DIRTY;

    /* block_pos = (rip->i_size - 1)/ S_BLOCK_SIZE; */

    /* if (block_pos < DIRECT_ZONE){ */
        /* for (i = 0; i <= block_pos; ++i){ */
            /* [> printf("free zone %d\n", (int)rip->i_zone[i]); <] */
            /* free_zone(rip->i_zone[i]); */
        /* } */
    /* }else if (block_pos < INDEX1_ZONE){ */
        /* for (i = 0; i < 10; ++i){ */
            /* free_zone(rip->i_zone[i]); */
            /* [> printf("free zone %d\n", (int)rip->i_zone[i]); <] */
        /* } */
        /* free_index1_zone(rip->i_zone[10], block_pos - DIRECT_ZONE); */
        /* free_zone(rip->i_zone[10]); */
    /* }else if (block_pos < INDEX2_ZONE_1) { */
        /* for (i = 0; i < 10; ++i){ */
            /* free_zone(rip->i_zone[i]); */
            /* [> printf("free zone %d\n", (int)rip->i_zone[i]); <] */
        /* } */
        /* free_index1_zone(rip->i_zone[10], INDEX1_ZONE - DIRECT_ZONE - 1); */
        /* free_zone(rip->i_zone[10]); */
        /* free_index2_zone(rip->i_zone[11], block_pos - INDEX1_ZONE); */
        /* free_zone(rip->i_zone[11]); */
    /* }else if (block_pos < INDEX2_ZONE_2) { */
        /* for (i = 0; i < 10; ++i){ */
            /* free_zone(rip->i_zone[i]); */
            /* [> printf("free zone %d\n", (int)rip->i_zone[i]); <] */
        /* } */
        /* free_index1_zone(rip->i_zone[10], INDEX1_ZONE - DIRECT_ZONE - 1); */
        /* free_zone(rip->i_zone[10]); */
        /* free_index2_zone(rip->i_zone[11], INDEX2_ZONE_1 - INDEX1_ZONE - 1); */
        /* free_zone(rip->i_zone[11]); */
        /* free_index2_zone(rip->i_zone[12], block_pos - INDEX2_ZONE_1); */
        /* free_zone(rip->i_zone[12]); */
    /* }else if (block_pos < INDEX2_ZONE_3) { */
        /* for (i = 0; i < 10; ++i){ */
            /* [> printf("free zone %d\n", (int)rip->i_zone[i]); <] */
            /* free_zone(rip->i_zone[i]); */
        /* } */
        /* free_index1_zone(rip->i_zone[10], INDEX1_ZONE - DIRECT_ZONE - 1); */
        /* free_zone(rip->i_zone[10]); */
        /* free_index2_zone(rip->i_zone[11], INDEX2_ZONE_1 - INDEX1_ZONE - 1); */
        /* free_zone(rip->i_zone[11]); */
        /* free_index2_zone(rip->i_zone[12], INDEX2_ZONE_2 - INDEX2_ZONE_1 - 1); */
        /* free_zone(rip->i_zone[12]); */
        /* free_index2_zone(rip->i_zone[13], block_pos - INDEX2_ZONE_2); */
        /* free_zone(rip->i_zone[13]); */
    /* } */
    /* rip->i_size = 0; */
    /* rip->i_dirt = DIRTY; */
}


/*===========================================================================*
 *				get_inode				     *
 *===========================================================================*/
struct inode *get_inode(int numb)
{
/* Find a slot in the inode table, load the specified inode into it, and
 * return a pointer to the slot.  If 'dev' == NO_DEV, just return a free slot.
 */

  struct inode *rip, *xp;

  /* Search the inode table both for (dev, numb) and a free slot. */
  xp = NIL_INODE;
  for (rip = &inode[0]; rip < &inode[NR_INODES]; rip++) {
	if (rip->i_count > 0) { /* only check used slots for (numb) */
		if (rip->i_num == numb) {
			/* This is the inode that we are looking for. */
			rip->i_count++;
			return(rip);	/* (dev, numb) found */
		}
	} else {
		xp = rip;	/* remember this free slot for later */
	}
  }

  /* Inode we want is not currently in use.  Did we find a free slot? */
  if (xp == NIL_INODE) {	/* inode table completely full */
	errno = ENFILE;
	return(NIL_INODE);
  }

  /* A free inode slot has been located.  Load the inode into it. */
  xp->i_num = numb;
  xp->i_count = 1;
  rw_inode(xp, READING);	/* get inode from disk */

  return(xp);
}

/*===========================================================================*
 *				put_inode				     *
 *===========================================================================*/
void put_inode(struct inode *rip)
{
/* The caller is no longer using this inode.  If no one else is using it either
 * write it back to the disk immediately.  If it has no links, truncate it and
 * return it to the pool of available inodes.
 */

  if (rip == NIL_INODE) return;	/* checking here is easier than in caller */
  if (--rip->i_count == 0) {	/* i_count == 0 means no one is using it now */
      /* rip->i_dirt = DIRTY; */
      if (rip->i_dirt == DIRTY) rw_inode(rip, WRITING);
      rip->i_mode = I_NOT_ALLOC;	/* clear I_TYPE field */
  }
}

/*===========================================================================*
 *				alloc_inode				     *
 *===========================================================================*/
struct inode *alloc_inode(int bits)
{
/* Allocate a free inode on 'dev', and return a pointer to it. */

  struct inode *rip;
  struct super_block *sp;
  int inumb;
  bit_t b;

  sp = &super_block;	/* get pointer to super_block */

  /* Acquire an inode from the bit map. */
  b = alloc_bit(IMAP, sp->s_isearch);
  if (b == NO_BIT) {
	errno = ENFILE;
	printf("Out of i-nodes on device \n");
	return(NIL_INODE);
  }
  sp->s_isearch = b;		/* next time start here */
  inumb = (int) b;		/* be careful not to pass unshort as param */
  /* printf("allocate bit for root is %d\n", inumb); */

  /* Try to acquire a slot in the inode table. */
  if ((rip = get_inode(inumb)) == NIL_INODE) {
	/* No inode table slots available.  Free the inode just allocated. */
	free_bit(IMAP, b);
  } else {
	/* An inode slot is available. Put the inode just allocated into it. */

	/* Fields not cleared already are cleared in wipe_inode().  They have
	 * been put there because truncate() needs to clear the same fields if
	 * the file happens to be open while being truncated.  It saves space
	 * not to repeat the code twice.
	 */
	rip->i_mode = bits;
	wipe_inode(rip);
  }

  /* printf("after all this, the inumb is %d\n", (int)rip->i_num); */
  return(rip);
}

/*===========================================================================*
 *				wipe_inode				     *
 *===========================================================================*/
void wipe_inode(struct inode *rip)
{
/* Erase some fields in the inode.  This function is called from alloc_inode()
 * when a new inode is to be allocated, and from truncate(), when an existing
 * inode is to be truncated.
 */

  int i;

  rip->i_size = 0;
  rip->i_dirt = DIRTY;
  for (i = 0; i < NR_TZONES; i++) rip->i_zone[i] = NO_ZONE;
}

/*===========================================================================*
 *				free_inode				     *
 *===========================================================================*/
void free_inode(int inumb)
{
/* Return an inode to the pool of unallocated inodes. */

  struct super_block *sp;
  bit_t b;

  /* Locate the appropriate super_block. */
  sp = &super_block;
  if (inumb <= 0 || inumb > S_NINODES) return;
  b = inumb;
  free_bit(IMAP, b);
  if (b < sp->s_isearch) sp->s_isearch = b;
}


/*===========================================================================*
 *				rw_inode				     *
 *===========================================================================*/
void rw_inode(struct inode *rip, int rw_flag)
{
/* An entry in the inode table is to be copied to or from the disk. */

  struct buf *bp;
  d_inode *dip;
  block_t b, offset;

  /* Get the block where the inode resides. */
  offset = S_IMAP_BLOCKS + S_ZMAP_BLOCKS + 1;/*origin is 2, but I think it should be 1*/
  b = (block_t) (rip->i_num - 1)/INODE_PER_BLOCK + offset;
  /* printf("i number is %d and block number is %d\n", (int)rip->i_num, (int)b); */
  bp = get_block(b);
  dip  = bp->b_ino + (rip->i_num - 1) % INODE_PER_BLOCK;

  /* Do the read or write. */
  if (rw_flag == WRITING) {
	bp->b_dirt = DIRTY;
  }

  /* Copy the inode from the disk block to the in-core table or vice versa.
   * If the fourth parameter below is FALSE, the bytes are swapped.
   */
  icopy(rip, dip,  rw_flag);

  put_block(bp);
  rip->i_dirt = CLEAN;
}

/*===========================================================================*
 *				icopy				     *
 *===========================================================================*/
icopy(struct inode *rip, d_inode * dip, int direction)
{
/* The V1.x IBM disk, the V1.x 68000 disk, and the V2 disk (same for IBM and
 * 68000) all have different inode layouts.  When an inode is read or written
 * this routine handles the conversions so that the information in the inode
 * table is independent of the disk structure from which the inode came.
 * The old_icopy routine copies to and from V1 disks.
 */

  int i;

  if (direction == READING) {
	/* Copy V1.x inode to the in-core table, swapping bytes if need be. */
	rip->i_size    = dip->d_size;
    rip->i_mode    = (int) dip->d_mode;
	for (i = 0; i < NR_TZONES; i++)
		rip->i_zone[i] = (int) dip->d_zone[i];
  } else {
	/* Copying V1.x inode to disk from the in-core table. */
	dip->d_mode   = (int) rip->i_mode;
	dip->d_size   = rip->i_size;
	for (i = 0; i < NR_TZONES; i++)
		dip->d_zone[i] = (int) rip->i_zone[i];
  }
}


/*===========================================================================*
 *				dup_inode				     *
 *===========================================================================*/
void dup_inode(struct inode * ip)
{
/* This routine is a simplified form of get_inode() for the case where
 * the inode pointer is already known.
 */

  ip->i_count++;
}
