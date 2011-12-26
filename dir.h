/* The <dir.h> header gives the layout of a directory. */

#ifndef _DIR_H
#define _DIR_H

#include "myfs.h"
#define NAME_MAX 252
#ifndef OK
#define OK 0
#endif
#define LOOK_UP 1
#define ENTER 2
#define DELETE 3
#define IS_EMPTY 4
#define TRUE 1
#define FALSE 0
#ifndef WRITING
#define READING 0
#define WRITING 1
#endif

#define PATH_MAX 252

#define NAME_MAX 252
struct inode *eat_path(char *path);
struct inode *last_dir(char *path, char string[NAME_MAX]);
char *get_name(char *old_name, char string [NAME_MAX]);
struct inode *advance(struct inode *dirp, char string[NAME_MAX]);
int search_dir(struct inode *ldir_ptr, char string[NAME_MAX], int *numb, int flag);
struct buf* new_block(struct inode * rip, int size);
block_t read_map(struct inode * rip, int pos);
int write_map(struct inode *rip, int position, zone_t new_zone);
void list_dir(struct inode *rip, int level);
int dir_rename(struct inode* rip, char old_name[NAME_MAX], char new_name[NAME_MAX]);
void delete_dir(struct inode *rip, int level);


#endif /* _DIR_H */
