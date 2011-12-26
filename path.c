#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include "dir.h"
#include "buf.h"
#include "file.h"
#include "inode.h"
#include "super.h"
#include "myfs.h"

/* char dot1[2] = ".";	[> used for search_dir to bypass the access <] */
/* char dot2[3] = "..";	[> permissions for . and ..		    <] */


void write_index1_block(struct buf * index, zone_t z, int pos){
    /* struct buf * bp; */
    /* bp = get_block(index); */
    /* bp->b_ind[pos] = z; */
    /* bp->b_dirt = DIRTY; */
    if(pos > NR_INDIRECT_BLOCK){
        printf("position in the indirect block is bigger than 128");
        exit(1);
    }
    /* put_block(bp); */
    index->b_ind[pos] = z;
    index->b_dirt = DIRTY;
}

void write_index2_block(struct buf * index, zone_t z, int pos){
    struct buf * bp;
    int block_pos, boff;
    block_pos = pos / NR_INDIRECT_BLOCK;
    boff = pos % NR_INDIRECT_BLOCK;
    if (index->b_ind[block_pos] == NO_ZONE){
        if(boff != 0){
            printf("this is not the first time to allocate a new index1 block in the index2 block, block_pos is %d, and boff is %d\n", block_pos, boff);
        }
        index->b_ind[block_pos] = (block_t)alloc_zone(z);
        index->b_dirt = DIRTY;
        bp = get_block(index->b_ind[block_pos]);
        zero_block(bp);
    }else{
        if(boff == 0){
            printf("not allocate a new index1 block in the index2 block\n");
        }
        bp = get_block(index->b_ind[block_pos]);
    }
    write_index1_block(bp, z, boff);
    put_block(bp);
}

void write_index3_block(struct buf * index, zone_t z, int pos){
    struct buf * bp;
    int block_pos, boff;
    block_pos = pos / NR_INDIRECT_BLOCK_2;
    boff = pos % NR_INDIRECT_BLOCK_2;
    if (index->b_ind[block_pos] == NO_ZONE){
        if(boff != 0){
            printf("this is not the first time to allocate a new index2 block in the index3 block, block_pos is %d, and boff is %d\n", block_pos, boff);
        }
        index->b_ind[block_pos] = (block_t)alloc_zone(z);
        index->b_dirt = DIRTY;
        bp = get_block(index->b_ind[block_pos]);
        zero_block(bp);
    }else{
        if(boff == 0){
            printf("not allocate a new index2 block in the index3 block\n");
        }
        bp = get_block(index->b_ind[block_pos]);
    }
    write_index2_block(bp, z, boff);
    put_block(bp);
}

struct buf* new_block(struct inode * rip, int position){
    struct buf * bp;
    int boff, block_pos, origin_pos;
    zone_t z;
    block_t b;
    block_pos = position / S_BLOCK_SIZE;
    if (rip->i_zone[0] == NO_ZONE){
        z = S_FIRSTDATAZONE;
    }else {
        z = rip->i_zone[0];
    }
    /* printf("the block_pos is %d\n", block_pos); */
    z = (block_t)alloc_zone(z);
    /* if (z != (S_FIRSTDATAZONE + 1)){ */
        /* printf("allocate zone is not as expected, allocate zone number %d\n", z); */
    /* } */
    /* printf("here\n");exit(0); */

    if (block_pos < DIRECT_ZONE){
        rip->i_zone[block_pos] = z;
        /* printf("allocate a new data block for root inode: %d\n", (int)z); */
        rip->i_dirt = DIRTY;
    }else if (block_pos < INDEX1_ZONE){
        if (rip->i_zone[10] == NO_ZONE){
            /*assert only the first index block alloct a new block*/
            if(block_pos != DIRECT_ZONE){
                printf("this time is not the first time to alloct a new index1 zone!!\n");
                return NIL_BUF;
            }
            rip->i_zone[10] = (block_t)alloc_zone(z);
            rip->i_dirt = DIRTY;
            bp = get_block(rip->i_zone[10]);
            zero_block(bp);
        }else{
            if(block_pos == DIRECT_ZONE){
                printf("this time not alloct a new index1 zone!!\n");
                return NIL_BUF;
            }
            bp = get_block(rip->i_zone[10]);
        }
        write_index1_block(bp, z, block_pos - DIRECT_ZONE);
        put_block(bp);
    }else if (block_pos < INDEX2_ZONE_1) {
        if (rip->i_zone[11] == NO_ZONE){
            /*assert only the first index block alloct a new block*/
            if(block_pos != INDEX1_ZONE){
                printf("this time is not the first time to alloct a new index2_1 zone!!\n");
                return NIL_BUF;
            }
            rip->i_zone[11] = (block_t)alloc_zone(z);
            rip->i_dirt = DIRTY;
            bp = get_block(rip->i_zone[11]);
            zero_block(bp);
        }else{
            if(block_pos == INDEX1_ZONE){
                printf("this time not alloct a new index1 zone!!\n");
                return NIL_BUF;
            }
            bp = get_block(rip->i_zone[11]);
        }
        write_index2_block(bp, z, block_pos - INDEX1_ZONE);
        put_block(bp);
    }else if (block_pos < INDEX2_ZONE_2) {
        if (rip->i_zone[12] == NO_ZONE){
            /*assert only the first index block alloct a new block*/
            if(block_pos != INDEX2_ZONE_1){
                printf("this time is not the first time to alloct a new index2_2 zone!!\n");
                return NIL_BUF;
            }
            rip->i_zone[12] = (block_t)alloc_zone(z);
            rip->i_dirt = DIRTY;
            bp = get_block(rip->i_zone[12]);
            zero_block(bp);
        }else{
            if(block_pos == INDEX2_ZONE_1){
                printf("this time not alloct a new index1 zone!!\n");
                return NIL_BUF;
            }
            bp = get_block(rip->i_zone[12]);
        }
        write_index2_block(bp, z, block_pos - INDEX2_ZONE_1);
        put_block(bp);
    }else if (block_pos < INDEX3_ZONE) {
        if (rip->i_zone[13] == NO_ZONE){
            /*assert only the first index block alloct a new block*/
            if(block_pos != INDEX2_ZONE_2){
                printf("this time is not the first time to alloct a new index2_3 zone!!\n");
                return NIL_BUF;
            }
            rip->i_zone[13] = (block_t)alloc_zone(z);
            rip->i_dirt = DIRTY;
            bp = get_block(rip->i_zone[13]);
            zero_block(bp);
        }else{
            if(block_pos == INDEX2_ZONE_2){
                printf("this time not alloct a new index1 zone!!\n");
                return NIL_BUF;
            }
            bp = get_block(rip->i_zone[13]);
        }
        write_index3_block(bp, z, block_pos - INDEX2_ZONE_2);
        put_block(bp);
    }

    b = (block_t)z;
    bp = get_block(b);
    /* exit(0); */
    zero_block(bp);
    return(bp);
}

block_t read_index1_block(block_t index, int pos){
    struct buf * bp;
    block_t b;
    bp = get_block(index);
    if(pos > NR_INDIRECT_BLOCK){
        printf("position in the indirect block is bigger than 128");
        exit(1);
    }
    b = (block_t)bp->b_ind[pos];
    put_block(bp);
    return b;
}

block_t read_index2_block(block_t index, int pos){
    int block_pos, boff;
    block_t b;
    struct buf * bp;

    block_pos = pos/NR_INDIRECT_BLOCK;
    boff = pos%NR_INDIRECT_BLOCK;
    bp = get_block(index);
    b = read_index1_block((block_t)bp->b_ind[block_pos],boff);
    put_block(bp);
    return b;
}

block_t read_index3_block(block_t index, int pos){
    int block_pos, boff;
    block_t b;
    struct buf * bp;

    block_pos = pos/NR_INDIRECT_BLOCK_2;
    boff = pos%NR_INDIRECT_BLOCK_2;
    bp = get_block(index);
    b = read_index2_block((block_t)bp->b_ind[block_pos],boff);
    put_block(bp);
    return b;
}

block_t read_map(struct inode * rip, int position){
    block_t b;
    int block_pos;

    block_pos = position/S_BLOCK_SIZE;	/* relative blk # in file */

    /* printf("the block_pos is %d\n", block_pos); */
    /* if (block_pos == 0) */
        /* return NO_BLOCK; */
    if (block_pos < DIRECT_ZONE){
        b = (block_t)rip->i_zone[block_pos];
    }else if (block_pos < INDEX1_ZONE){
        b = read_index1_block(rip->i_zone[10], block_pos - DIRECT_ZONE);
    }else if (block_pos < INDEX2_ZONE_1) {
        b = read_index2_block(rip->i_zone[11], block_pos - INDEX1_ZONE);
    }else if (block_pos < INDEX2_ZONE_2) {
        b = read_index2_block(rip->i_zone[12], block_pos - INDEX2_ZONE_1);
    }else if (block_pos < INDEX3_ZONE) {
        b = read_index3_block(rip->i_zone[13], block_pos - INDEX2_ZONE_2);
    }else{
        printf("the position is bigger than max file size");
        return NO_BLOCK;
    }
    return(b);
}

struct inode * eat_path(char *path)
{
/* Parse the path 'path' and put its inode in the inode table. If not possible,
 * return NIL_INODE as function value and an error code in 'err_code'.
 */

  struct inode *ldip, *rip;
  char string[DIRSIZ];	/* hold 1 path component name here */

  /* First open the path down to the final directory. */
  if ( (ldip = last_dir(path, string)) == NIL_INODE) {
      return(NIL_INODE);	/* we couldn't open final directory */
  }

  /* The path consisting only of "/" is a special case, check for it. */
  if (string[0] == '\0') return(ldip);

  /* Get final component of the path. */
  rip = advance(ldip, string);
  put_inode(ldip);
  return(rip);
}

struct inode *last_dir(char *path, char string[DIRSIZ])
{
/* Given a path, 'path', located in the fs address space, parse it as
 * far as the last directory, fetch the inode for the last directory into
 * the inode table, and return a pointer to the inode.  In
 * addition, return the final component of the path in 'string'.
 * If the last directory can't be opened, return NIL_INODE and
 * the reason for failure in 'err_code'.
 */

  struct inode *rip;
  char *new_name;
  struct inode *new_ip;

  /* Is the path absolute or relative?  Initialize 'rip' accordingly. */
  rip = get_inode(1);

  /* If dir has been removed or path is empty, return ENOENT. */
  if (*path == '\0') {
	errno = ENOENT;
	return(NIL_INODE);
  }

  dup_inode(rip);		/* inode will be returned with put_inode */

  /* Scan the path component by component. */
  while (TRUE) {
	/* Extract one component. */
	if ( (new_name = get_name(path, string)) == (char*) 0) {
		put_inode(rip);	/* bad path in user space */
		return(NIL_INODE);
	}
    if (*new_name == '\0') {
        if ( (rip->i_mode & I_DIRECTORY) ) {
            return(rip);	/* normal exit */
        } else {
            /* last file of path prefix is not a directory */
            put_inode(rip);
            errno = ENOTDIR;
            return(NIL_INODE);
        }
    }

	/* There is more path.  Keep parsing. */
	new_ip = advance(rip, string);
	put_inode(rip);		/* rip either obsolete or irrelevant */
	if (new_ip == NIL_INODE) return(NIL_INODE);

	/* The call to advance() succeeded.  Fetch next component. */
	path = new_name;
	rip = new_ip;
  }
}

/*===========================================================================*
 *				get_name				     *
 *===========================================================================*/
char *get_name(char *old_name, char string[DIRSIZ])
{
/* Given a pointer to a path name in fs space, 'old_name', copy the next
 * component to 'string' and pad with zeros.  A pointer to that part of
 * the name as yet unparsed is returned.  Roughly speaking,
 * 'get_name' = 'old_name' - 'string'.
 *
 * This routine follows the standard convention that /usr/ast, /usr//ast,
 * //usr///ast and /usr/ast/ are all equivalent.
 */

  int c;
  char *np, *rnp;

  np = string;			/* 'np' points to current position */
  rnp = old_name;		/* 'rnp' points to unparsed string */
  while ( (c = *rnp) == '/') rnp++;	/* skip leading slashes */

  /* Copy the unparsed path, 'old_name', to the array, 'string'. */
  while ( rnp < &old_name[PATH_MAX]  &&  c != '/'   &&  c != '\0') {
	if (np < &string[DIRSIZ]) *np++ = c;
	c = *++rnp;		/* advance to next character */
  }

  /* To make /usr/ast/ equivalent to /usr/ast, skip trailing slashes. */
  while (c == '/' && rnp < &old_name[PATH_MAX]) c = *++rnp;

  if (np < &string[DIRSIZ]) *np = '\0';	/* Terminate string */

  if (rnp >= &old_name[PATH_MAX]) {
	errno = ENAMETOOLONG;
    printf("path name reach to the max size!!!\n");
	return((char *) 0);
  }
  return(rnp);
}

/*===========================================================================*
 *				advance					     *
 *===========================================================================*/
struct inode *advance(struct inode *dirp, char string[DIRSIZ])
{
/* Given a directory and a component of a path, look up the component in
 * the directory, find the inode, open it, and return a pointer to its inode
 * slot.  If it can't be done, return NIL_INODE.
 */

  struct inode *rip;
  struct inode *rip2;
  struct super_block *sp;
  int r, inumb;
  int numb = 0;

  /* If 'string' is empty, yield same inode straight away. */
  if (string[0] == '\0') { return(get_inode((int) dirp->i_num)); }

  /* Check for NIL_INODE. */
  if (dirp == NIL_INODE) { return(NIL_INODE); }

  /* If 'string' is not present in the directory, signal error. */
  if ( (r = search_dir(dirp, string, &numb, LOOK_UP)) != OK) {
	errno = r;
	return(NIL_INODE);
  }
  if (numb == 0) return (NIL_INODE);

  return(get_inode((int) numb));

}

/*===========================================================================*
 *				search_dir				     *
 *===========================================================================*/
int search_dir(struct inode *ldir_ptr, char string[DIRSIZ], int * numb, int  flag)
{
/* This function searches the directory whose inode is pointed to by 'ldip':
 * if (flag == ENTER)  enter 'string' in the directory with inode # '*numb';
 * if (flag == DELETE) delete 'string' from the directory;
 * if (flag == LOOK_UP) search for 'string' and return inode # in 'numb';
 * if (flag == IS_EMPTY) return OK if only . and .. in dir else ENOTEMPTY;
 *
 *    if 'string' is dot1 or dot2, no access permissions are checked.
 */

  struct direct *dp = NULL;
  struct buf *bp = NULL;
  int i, r, e_hit, t, match;
  int bits;
  int pos;
  unsigned new_slots, old_slots;
  block_t b;
  struct super_block *sp;
  int extended = 0;

  if ( !(ldir_ptr->i_mode & I_DIRECTORY)) return(ENOTDIR);

  r = OK;


  /* Step through the directory one block at a time. */
  old_slots = (unsigned) (ldir_ptr->i_size/DIR_ENTRY_SIZE);/* how many directory slots in this inode */
  new_slots = 0;
  e_hit = FALSE;
  match = 0;			/* set when a string match occurs */

  /* printf("inode size is %d\n", ldir_ptr->i_size); */

  for (pos = 0; pos < ldir_ptr->i_size; pos += S_BLOCK_SIZE) {
	b = read_map(ldir_ptr, pos);	/* get block number */

	/* Since directories don't have holes, 'b' cannot be NO_BLOCK. */
	bp = get_block(b);	/* get a dir block */

	if (bp == NIL_BUF){
        printf("Search Dir: error occurs: can't get a block\n");exit(1);
    }

	/* Search a directory block. */
	for (dp = &bp->b_dir[0];
		dp < &bp->b_dir[NR_DIR_BLOCK];
		dp++) {
        /* read every slot , add new_slot by 1 */
		if (++new_slots > old_slots) { /* not found, but room left */
			if (flag == ENTER) e_hit = TRUE;
			break;
		}
        /* printf("the string read is %s\n", dp->d_name); */

		/* Match occurs if string found. */
		if (flag != ENTER && dp->d_ino != 0) {
			if (flag == IS_EMPTY) {
				/* If this test succeeds, dir is not empty. */
				if (strcmp(dp->d_name, "." ) != 0 &&
				    strcmp(dp->d_name, "..") != 0) match = 1;
			} else {
				if (strncmp(dp->d_name, string, DIRSIZ) == 0) {
                    /* printf("here\n"); */
					match = 1;
				}
			}
		}

		if (match) {
			/* LOOK_UP or DELETE found what it wanted. */
			r = OK;
			if (flag == DELETE) {
                *numb = (int)dp->d_ino;
                dp->d_ino = 0;
				bp->b_dirt = DIRTY;
				ldir_ptr->i_dirt = DIRTY;
			} else {
				*numb = (int) dp->d_ino;
			}
			put_block(bp);
			return(r);
		}

		/* Check for free slot for the benefit of ENTER. */
		if (flag == ENTER && dp->d_ino == 0) {
            printf("here reset a new slot\n");
			e_hit = TRUE;	/* we found a free slot */
			break;
		}
	}

	/* The whole block has been searched or ENTER has a free slot. */
	if (e_hit) break;	/* e_hit set if ENTER can be performed now */
	put_block(bp);	/* otherwise, continue searching dir */
  }

  if(flag != ENTER){
      return OK;
  }

    /* printf("it should not reached here!\n"); */
  /* This call is for ENTER.  If no free slot has been found so far, try to
   * extend directory.
   */
  if (e_hit == FALSE) { /* directory is full and no room left in last block */
	new_slots++;		/* increase directory size by 1 entry */
	if (new_slots == 0) {
        printf("here new slot is 0\n");
        return(EFBIG); /* dir size limited by slot count */
    }
    /* exit(0); */
	if ( (bp = new_block(ldir_ptr, ldir_ptr->i_size)) == NIL_BUF)
		return(errno);
	dp = &bp->b_dir[0];
	extended = 1;
  }

  /* printf("here, the string to search is %s\n", string);exit(0); */
  /* 'bp' now points to a directory block with space. 'dp' points to slot. */
  /* exit(0); */
  /* printf("new slot is %d\n", new_slots); */
  memset(dp->d_name, 0, (size_t) DIRSIZ); /* clear entry */
  /* exit(0); */
  for(i = 0; i < DIRSIZ && string[i]; ++i)
      dp->d_name[i] = string[i];
  /* strcpy(dp->d_name, string); */
  dp->d_ino = (int) *numb;
  bp->b_dirt = DIRTY;
  put_block(bp);
  ldir_ptr->i_dirt = DIRTY;
  if (new_slots > old_slots) {
	ldir_ptr->i_size = (int) new_slots * DIR_ENTRY_SIZE;
	/* Send the change to disk if the directory is extended. */
	if (extended) rw_inode(ldir_ptr, WRITING);
  }
  return(OK);
}

void print_tab(int num){
    int i = 0;
    for(; i < num; ++ i){
        printf("\t");
    }
}

void list_dir(struct inode *rip, int level){
    struct inode * item;
    struct direct *dp = NULL;
    struct buf *bp = NULL;
    int pos;
    unsigned slots, count;
    block_t b;

    slots = (unsigned) (rip->i_size/DIR_ENTRY_SIZE);/* how many directory slots in this inode */
    count = 0;

    for(pos = 0; pos < rip->i_size; pos += S_BLOCK_SIZE){
        b = read_map(rip, pos);
        bp = get_block(b);
        if(bp == NIL_BUF){
            printf("get a null buf when list dir\n");
            return;
        }
        for(dp = &bp->b_dir[0]; dp < &bp->b_dir[NR_DIR_BLOCK]; ++ dp){
            if(++ count > slots){
                break;
            }
            if(dp->d_ino == 0){
                printf("meet a empty inode\n");
                break;
            }
            print_tab(level);
            printf("filename: %s ",dp->d_name);
            item = get_inode(dp->d_ino);
            printf("size: %d ", item->i_size);
            if(item->i_mode & I_DIRECTORY){
                printf("Directory\n");
                if (strcmp(dp->d_name, ".") != 0 && strcmp(dp->d_name, "..") != 0){
                    list_dir(item, level + 1);
                }
            }else{
                printf("Normal\n");
            }
            put_inode(item);
        }
        put_block(bp);
    }
}


int dir_rename(struct inode* rip, char old_name[NAME_MAX], char new_name[NAME_MAX]){
    struct direct *dp = NULL;
    struct buf *bp = NULL;
    int pos;
    unsigned slots, count;
    block_t b;

    slots = (unsigned) (rip->i_size/DIR_ENTRY_SIZE);/* how many directory slots in this inode */
    count = 0;
    /* printf("inode's size is %d, number of slots is%d\n", rip->i_size, slots); */

    for(pos = 0; pos < rip->i_size; pos += S_BLOCK_SIZE){
        b = read_map(rip, pos);
        bp = get_block(b);
        if(bp == NIL_BUF){
            printf("get a null buf when list dir\n");
            return;
        }
        for(dp = &bp->b_dir[0]; dp < &bp->b_dir[NR_DIR_BLOCK]; ++ dp){
            if(++ count > slots){
                break;
            }
            if(dp->d_ino == 0){
                /* printf("meet a empty inode\n"); */
                break;
            }
            if(strcmp(dp->d_name, old_name) == 0){
                my_strcpy(dp->d_name, new_name);
                bp->b_dirt = DIRTY;
                put_block(bp);
                return OK;
            }
        put_block(bp);
        }
    }
    return -1;
}


void delete_dir(struct inode *rip, int level){
    struct inode * item;
    struct direct *dp = NULL;
    struct buf *bp = NULL;
    int pos;
    int number;
    unsigned slots, count;
    block_t b;

    print_tab(level);
    slots = (unsigned) (rip->i_size/DIR_ENTRY_SIZE);/* how many directory slots in this inode */
    count = 0;
    /* printf("inode's size is %d, number of slots is%d\n", rip->i_size, slots); */

    for(pos = 0; pos < rip->i_size; pos += S_BLOCK_SIZE){
        b = read_map(rip, pos);
        bp = get_block(b);
        if(bp == NIL_BUF){
            printf("get a null buf when list dir\n");
            return;
        }
        for(dp = &bp->b_dir[0]; dp < &bp->b_dir[NR_DIR_BLOCK]; ++ dp){
            if(++ count > slots){
                break;
            }
            if(dp->d_ino == 0){
                printf("meet a empty inode\n");
                break;
            }
            if (strcmp(dp->d_name, ".") != 0 && strcmp(dp->d_name, "..") != 0){
                print_tab(level);
                printf("delete filename: %s ",dp->d_name);
                item = get_inode(dp->d_ino);
                if(item->i_mode & I_DIRECTORY){
                    printf("Directory\n");
                    delete_dir(item, level + 1);
                }else{
                    printf("Normal\n");
                }
                empty_inode_space(item);
                number = item->i_num;
                put_inode(item);
                printf("free_inode %d\n", number);
                free_inode(number);
            }
        }
        put_block(bp);
    }
}

