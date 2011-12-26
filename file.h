#ifndef FILE_H
#define FILE_H

#include "myfs.h"

int get_fd(int *k, struct filp **fpt);
struct filp *get_filp(int fild);
struct filp *find_filp(struct inode * rip);
#endif /* FILE_H */
