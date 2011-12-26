#ifndef BUF_H
#define BUF_H


#include "myfs.h"



/*function buf module supply*/
struct buf *get_block(block_t block);
void put_block(struct buf *bp);
zone_t alloc_zone(zone_t z);
void free_zone(zone_t numb);


#endif /* BUF_H */
