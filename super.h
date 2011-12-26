#ifndef SUPER_H
#define SUPER_H


#include "myfs.h"



/*function super_block module supply*/
bit_t alloc_bit(int map, bit_t origin);
void free_bit(int map, bit_t bit_returned);

#endif /* SUPER_H */
