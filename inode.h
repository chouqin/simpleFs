#ifndef INODE_H
#define INODE_H
/* Inode table.  This table holds inodes that are currently in use.  In some
 * cases they have been opened by an open() or creat() system call, in other
 * cases the file system itself needs the inode for one reason or another,
 * such as to search a directory for a path name.
 * The first part of the struct holds fields that are present on the
 * disk; the second part holds fields not present on the disk.
 * The disk inode part is also declared in "type.h" as 'd1_inode' for V1
 * file systems and 'd2_inode' for V2 file systems.
 */

#include "myfs.h"


/*function inode module supply*/
struct inode *get_inode(int numb);
void put_inode(struct inode *rip);
struct inode *alloc_inode(int bits);
void wipe_inode(struct inode *rip);
void free_inode(int inumb);
void dup_inode(struct inode *ip);
void empty_inode_space(struct inode* ip);

#endif /* INODE_H */
