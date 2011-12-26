/* This file contains the procedures that manipulate file descriptors.
 *
 * The entry points into this file are
 *   get_fd:	look for free file descriptor and free filp slots
 *   get_filp:	look up the filp entry for a given file descriptor
 *   find_filp:	find a filp slot that points to a given inode
 */

#include "file.h"
#include "inode.h"
#include "errno.h"
#define OK 0

/*===========================================================================*
 *				get_fd					     *
 *===========================================================================*/
int get_fd(int *k, struct filp **fpt)
{
/* Look for a free file descriptor and a free filp slot.  Fill in the mode word
 * in the latter, but don't claim either one yet, since the open() or creat()
 * may yet fail.
 */

  struct filp *f;
  int i = 0;

  /* Now that a file descriptor has been found, look for a free filp slot. */
  for (f = &filp[0]; f < &filp[NR_FILPS]; f++) {
	if (f->filp_count == 0) {
        *fpt = f;
        *k = i;
		return(OK);
	}
    i ++;
  }

  /* If control passes here, the filp table must be full.  Report that back. */
  return(ENFILE);
}

/*===========================================================================*
 *				get_filp				     *
 *===========================================================================*/
struct filp *get_filp(int fild)
{
/* See if 'fild' refers to a valid file descr.  If so, return its filp ptr. */

  errno = EBADF;
  if (fild < 0 || fild >= OPEN_MAX ) return(NIL_FILP);
  return(&filp[fild]);	/* may also be NIL_FILP */
}

/*===========================================================================*
 *				find_filp				     *
 *===========================================================================*/
struct filp *find_filp(struct inode *rip)
{
/* Find a filp slot that refers to the inode 'rip' in a way as described
 * by the mode bit 'bits'. Used for determining whether somebody is still
 * interested in either end of a pipe.  Also used when opening a FIFO to
 * find partners to share a filp field with (to shared the file position).
 * Like 'get_fd' it performs its job by linear search through the filp table.
 */

  struct filp *f;

  for (f = &filp[0]; f < &filp[NR_FILPS]; f++) {
	if (f->filp_count != 0 && f->filp_ino == rip){
		return(f);
	}
  }

  /* If control passes here, the filp wasn't there.  Report that back. */
  return(NIL_FILP);
}
