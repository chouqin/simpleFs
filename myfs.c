/*
 * =====================================================================================
 *
 *       Filename:  myfs.c
 *
 *    Description:
 *
 *        Version:  1.0
 *        Created:  2011年12月10日 20时11分37秒
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  YOUR NAME (),
 *        Company:
 *
 * =====================================================================================
 */


#include <string.h>
#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include "myfs.h"



/* private functions of other file */
#include "buf.h"
#include "inode.h"
#include "super.h"
#include "file.h"
#include "dir.h"
#include "./socket/client.h"



/*private function to do strcpy */
void my_strcpy(char des[DIRSIZ], const char * src){
    memset(des, 0, DIRSIZ);
    int i = 0;
    strcpy(des, src);
    for (i = strlen(des); i < DIRSIZ; ++ i)
        des[i] = '\0';
}

void init()
{
    /* inode part, there is no inode in the memory now, set them all to 0*/
    struct inode * rip;
    for (rip = &inode[0]; rip < &inode[NR_INODES]; ++ rip){
        rip->i_count = 0;
        rip->i_mode = I_NOT_ALLOC;
        rip->i_dirt = CLEAN;
        rip->i_num = 0;
    }
    /* buf part, init the hash chain and free chain*/
    struct buf * bp;
    int i;
    for(bp = &buf[0]; bp < &buf[NR_BUFS]; ++ bp){
        /* bp->b_blocknr = 0; */
        if(bp != &buf[NR_BUFS - 1]){
            bp->b_next = bp + 1;
            bp->b_hash = bp + 1;
        }
        else {
            bp->b_next = NIL_BUF;
            bp->b_hash = NIL_BUF;
        }
        if (bp != &buf[0])
            bp->b_prev = bp - 1;
        else
            bp->b_prev = NIL_BUF;
        bp->b_blocknr = 0;
        bp->b_dirt = CLEAN;
        bp->b_count = 0;
    }
    for (i = 1; i < NR_BUFS; ++ i){
        buf_hash[i] = NIL_BUF;
    }
    buf_hash[0] = &buf[0];
    front = &buf[0];
    rear = &buf[NR_BUFS -1];
    bufs_in_use = 0;


    /*filp part ,same as inode*/
    struct filp * f;
    for (f = &filp[0]; f < &filp[NR_FILPS]; f++) {
        f->filp_count = 0;
        f->filp_ino = NIL_INODE;
        f->filp_type = FILP_CLOSED;
        f->filp_pos = 0;
    }
    /* superblock part*/
    super_block.s_isearch = 0;
    super_block.s_zsearch = 0;
}


void mkroot(){
    /* init(); */
    /*first, let the bit 0 don't be allocated as most fs do*/
    struct buf * bp = get_block(START_BLOCK);
    /* printf("here\n");exit(0); */
    int *wptr = &bp->b_bitmap[0];
    *wptr = (int)1;
    bp->b_dirt = DIRTY;
    put_block(bp);

    bp = get_block(START_BLOCK + S_IMAP_BLOCKS);
    /* printf("here\n");exit(0); */
    wptr = &bp->b_bitmap[0];
    *wptr = (int)1;
    bp->b_dirt = DIRTY;
    put_block(bp);

    /*allocate bit 1 for root inode  */
    /* bit_t root_bit = alloc_bit(IMAP, 1); */
    /* printf("root bit is %d\n", (int) root_bit); */
    int mode = 0x00000003;
    struct inode * root_inode = alloc_inode(mode);
    if (root_inode == NIL_INODE){
        printf("can't allocate root_inode\n");
        exit(1);
    }
    if (root_inode->i_num != 1){
        printf("root bit allocate error: can't set root to bit 1, the inode number is %d\n", (int)root_inode->i_num);
        exit(1);
    }
    if (!(root_inode->i_mode & I_DIRECTORY)){
        printf("root is not a directory!!\n");
        exit(1);
    }
    root_inode->i_dirt = DIRTY;
    /* put_inode(root_inode); */
    root_inode = get_inode(1);
    if (!(root_inode->i_mode & I_DIRECTORY)){
        printf("after put and get: root is not a directory!!\n");
        printf("this time the i number is %d and imode is %d\n", (int)root_inode->i_num, root_inode->i_mode);
        exit(1);
    }

    /* allocate disk for root inode */
    zone_t root_zone = alloc_zone(S_FIRSTDATAZONE);
    if (root_zone != (S_FIRSTDATAZONE + 1)){
        printf("root zone is not the first data zone!!\n");
        exit(1);
    }
    struct buf *rp = get_block(root_zone);
    struct direct *cdir = &rp->b_dir[0];
    struct direct *udir = &rp->b_dir[1];
    cdir->d_ino = (int) 1;
    strcpy(cdir->d_name, ".");
    udir->d_ino = (int)1;
    strcpy(udir->d_name, "..");
    rp->b_dirt = DIRTY;
    put_block(rp);
    root_inode->i_zone[0] = root_zone;
    root_inode->i_dirt = DIRTY;
    put_inode(root_inode);
}


void write_back(){
    /*buf part*/
    struct buf * bp;
    for(bp = &buf[0]; bp < &buf[NR_BUFS]; ++ bp){
        if(bp->b_dirt == DIRTY && bp->b_blocknr != 0){
            write_block(bp->b_blocknr, (char *)&bp->b);
        }
    }
}


int my_open(char * path){
    struct inode * rip;
    struct inode * ip;
    char string[DIRSIZ];
    struct filp * fp;
    int fd;
    int r;

    rip = last_dir(path, string);
    if (rip == NIL_INODE){
        printf("the path name is not correct: incorrect dir!\n");
        return errno;
    }
    ip = advance(rip, string);
    put_inode(rip);
    if (ip == NIL_INODE){
        printf("the path name is not correct: incorrect filename!\n");
        return errno;
    }
    if (r = get_fd(&fd, &fp) != OK)
        return r;
    fp->filp_ino = ip;
    fp->filp_count = 1;
    fp->filp_type = RDWR;
    fp->filp_pos = 0;
    return fd;
}

int my_create(char * path){
    struct inode * rip;
    struct inode * ip;
    char string[DIRSIZ];
    struct filp * fp;
    int fd;
    int r;
    int *inode_num, number;
    inode_num = &number;


    rip = last_dir(path, string);
    if (rip == NIL_INODE){
        printf("the path name is not correct!\n");
        return errno;
    }
    *inode_num = 0;
    if(r = search_dir(rip, string, inode_num, LOOK_UP) != OK){
        printf("error occured when trying to search diretory");
        put_inode(rip);
        return;
    }
    if(*inode_num != 0){
        printf("item with the same name exits, please delete that item first\n ");
        put_inode(rip);
        return;
    }
    ip = alloc_inode(1);
    printf("allocate a new inode for %s, inode_num is %d\n", string, ip->i_num);
    *inode_num = ip->i_num;
    if (r = search_dir(rip, string, inode_num, ENTER) != OK){
        printf("enter a item error: %s\n", strerror(r));
        put_inode(rip);
        return r;
    }
    rip->i_dirt = DIRTY;
    put_inode(rip);
    if (r = get_fd(&fd, &fp) != OK)
        return r;
    fp->filp_ino = ip;
    fp->filp_count = 1;
    fp->filp_type = RDWR;
    fp->filp_pos = 0;
    return fd;
}

int my_read(int fd, char * buf, int count){
    struct inode *rip;
    struct buf *bp;
    char * data;
    int position,b, buf_pos, pos;
    int tmp;
    int left;
    int i;

    rip = filp[fd].filp_ino;
    if(rip == NIL_INODE){
        printf("incorrect fd!!\n");
        return -1;
    }
    printf("size of this file is %d\n", rip->i_size);

    if(filp[fd].filp_type == READ){
        position = filp[fd].filp_pos;
    }else{
        position = 0;
    }


    if ((position + count) > rip->i_size) {
        printf("count is bigger than file size\n");
        return -1;
    }

    tmp = count + position;

    buf_pos = 0;
    /*if the last block still left some size*/
    if(position % S_BLOCK_SIZE != 0){
        left = S_BLOCK_SIZE - position % S_BLOCK_SIZE;
        pos = position % S_BLOCK_SIZE;
        b = read_map(rip, position);
        bp = get_block(b);
        for(i = 0; i < left; ++ i){
            buf[buf_pos++] = bp->b_data[pos+i];
        }
        put_block(bp);
        position += left;
    }

    for(; position < tmp; position += S_BLOCK_SIZE){
        b = read_map(rip, position);
        if(b <= 0){
            printf("read map error\n");
            return -1;
        }
        /* printf("read the inode block, block number is %d\n", b); */
        bp = get_block(b);
        left =  tmp - position;
        if (left >= S_BLOCK_SIZE){
            for(i = 0; i < S_BLOCK_SIZE; ++i)
                buf[buf_pos++] = bp->b_data[i];
        }else{
            for(i = 0; i < left; ++i)
                buf[buf_pos++] = bp->b_data[i];
        }
        put_block(bp);
    }
    filp[fd].filp_type = READ;
    filp[fd].filp_pos = tmp;
    return buf_pos;
}

int my_write(int fd, char *buf, int count){
    struct inode *rip;
    struct buf *bp;
    char * data;
    int position,b, pos, buf_pos;
    int left;
    int i;
    int tmp;

    rip = filp[fd].filp_ino;
    if(rip == NIL_INODE){
        printf("incorrect fd!!\n");
        return -1;
    }

    /* printf("size of this file is %d\n", rip->i_size); */
    if(filp[fd].filp_type == WRITE){
        position = filp[fd].filp_pos;
        printf("current position is %d\n", position);
    }else{
        position = 0;
        if(rip->i_size > 0){
            empty_inode_space(rip);
            wipe_inode(rip);
            /* return; */
        }
    }

    tmp = count + position;

    buf_pos = 0;
    /*if the last block still left some size*/
    if((position % S_BLOCK_SIZE) != 0){
        left = S_BLOCK_SIZE - position % S_BLOCK_SIZE;
        pos = position % S_BLOCK_SIZE;
        b = read_map(rip, position);
        bp = get_block(b);
        for(i = 0; i < left; ++ i){
            bp->b_data[pos + i] = buf[buf_pos];
            buf_pos ++;
        }
        bp->b_dirt = DIRTY;
        put_block(bp);
        position += left;
    printf("current position is %d and tmp is %d and buf_pos is %d\n", position, tmp, buf_pos);
    /* exit(0); */
    }


    for(; position < tmp; position += S_BLOCK_SIZE){
        bp = new_block(rip, position);
        if(bp == NIL_BUF){
            printf("create a new block error\n");
            return -1;
        }
        /* printf("write a new inode block, block number is %d\n", bp->b_blocknr); */
        left = tmp - position;
        if (left >= S_BLOCK_SIZE){
            for(i = 0; i < S_BLOCK_SIZE; ++i){
                if(buf_pos >= count){
                    printf("buf_pos bigger than count, this time left is %d, and position is%d\n", left, position);
                    exit(1);
                }
                bp->b_data[i] = buf[buf_pos++];
            }
        }else{
            /* if(left != 128){ */
                /* printf("here, current left is %d buf_pos is %d\n",left,  buf_pos); */
                /* exit(0); */
            /* } */
            for(i = 0; i < left; ++i){
                if(buf_pos > count){
                    printf("buf_pos bigger than count\n");
                    exit(1);
                }
                bp->b_data[i] = buf[buf_pos++];
            }
        }
        bp->b_dirt = DIRTY;
        put_block(bp);
    }
    rip->i_size = tmp;
    rip->i_dirt = DIRTY;
    filp[fd].filp_type = WRITE;
    filp[fd].filp_pos = tmp;
    return buf_pos;
}

int my_close(int fd){
    struct inode * rip;
    filp[fd].filp_count = 0;
    rip = filp[fd].filp_ino;
    filp[fd].filp_ino = NIL_INODE;
    filp[fd].filp_type = FILP_CLOSED;
    filp[fd].filp_pos = 0;
    if(rip == NIL_INODE){
        printf("incorrect fd!!\n");
        return -1;
    }
    put_inode(rip);
    return OK;
}

int my_remove(char * name){
    struct inode *rip;
    struct inode *ip;
    char string[DIRSIZ];
    int r;
    int *inode_num;
    int number;

    inode_num = &number;

    rip = last_dir(name, string);

    if(rip == NIL_INODE){
        printf("search directory error: please enter the correct path\n");
        return -1;
    }

    if(r = search_dir(rip, string, inode_num, DELETE) != OK){
        printf("delete file error: %s", strerror(r));
    }

    put_inode(rip);
    ip = get_inode(*inode_num);
    empty_inode_space(ip);
    put_inode(ip);
    free_inode(*inode_num);
    return r;
}

int my_rename(char * oldname, char * newname){
    struct inode *rip;
    struct inode *ip;
    char string[DIRSIZ];
    char newstring[DIRSIZ];
    int r;

    rip = last_dir(oldname, string);

    if(rip == NIL_INODE){
        printf("search directory error: please enter the correct oldname path\n");
        return -1;
    }

    ip = last_dir(newname, newstring);

    if((ip == NIL_INODE) || (ip->i_num != rip->i_num)){
        printf("search directory error: please enter the correct newname path\n");
        return -1;
    }
    put_inode(ip);

    /* my_strcpy(newstring, newname); */
    if (r = dir_rename(rip, string, newstring) != OK){
        printf("rename file error: please enter the correct name\n");
    }
    /* rip->i_dirt = DIRTY; */
    put_inode(rip);
    return r;
}

int my_mkdir(char * name){
    struct inode * rip;
    struct inode * ip;
    char string[DIRSIZ];
    int r;
    int *inode_num, number;
    inode_num = &number;


    rip = last_dir(name, string);
    if (rip == NIL_INODE){
        printf("the path name is not correct!\n");
        return errno;
    }
    *inode_num = 0;
    if(r = search_dir(rip, string, inode_num, LOOK_UP) != OK){
        printf("error occured when trying to search diretory");
        put_inode(rip);
        return;
    }
    if(*inode_num != 0){
        printf("item with the same name exits, please delete that item first\n ");
        put_inode(rip);
        return;
    }
    ip = alloc_inode(3);
    printf("allocate a new inode for %s, inode_num is %d\n", string, ip->i_num);
    *inode_num = ip->i_num;
    if (r = search_dir(rip, string, inode_num, ENTER) != OK){
        printf("enter a item %s error: %s\n", string, strerror(r));
    }

    my_strcpy(string, "..");
    *inode_num = rip->i_num;
    if (r = search_dir(ip, string, inode_num, ENTER) != OK){
        printf("enter a item %s error: %s\n", string, strerror(r));
    }

    my_strcpy(string, ".");
    *inode_num = ip->i_num;
    if (r = search_dir(ip, string, inode_num, ENTER) != OK){
        printf("enter a item %s error: %s\n", string, strerror(r));
    }
    ip->i_dirt = DIRTY;
    put_inode(ip);
    rip->i_dirt = DIRTY;
    put_inode(rip);

    return r;
}




int my_rmdir(char * name){
    struct inode *rip;
    struct inode *ip;
    char string[DIRSIZ];
    int r;
    int *inode_num;
    int number = 0;

    inode_num = &number;

    rip = last_dir(name, string);

    if(rip == NIL_INODE){
        printf("search directory error: please enter the correct path\n");
        return -1;
    }

    if(r = search_dir(rip, string, inode_num, DELETE) != OK){
        printf("delete file error: %s", strerror(r));
        put_inode(rip);
        return -1;
    }
    put_inode(rip);

    ip = get_inode(*inode_num);
    if(ip == NIL_INODE || !(ip->i_mode & I_DIRECTORY)){
        printf("search directory error: please enter the correct path\n");
        put_inode(ip);
        return -1;
    }

    delete_dir(ip, 0);
    empty_inode_space(ip);
    put_inode(ip);
    free_inode(*inode_num);
    /* return delete_dir(ip); */
    return OK;
}

void show_file_list(){
    printf("files in the disk:\n");
    struct inode * root_inode = get_inode(1);
    list_dir(root_inode, 0);
    put_inode(root_inode);
}


void reset_disk(){
    struct buf *bp;
    struct inode * root;
    int * bit;
    int i;
    char string[S_BLOCK_SIZE];
    int number;
    memset(string, 0, S_BLOCK_SIZE);

    /* for(i = 0; i < S_ZONES; ++i){ */
        /* write_block(i, string); */
    /* } */

    /*no_block*/
    /* 1~3 imap block */
    bp = get_block(1);
    bit = &bp->b_bitmap[0];
    *bit = 1;
    bp->b_dirt = DIRTY;
    put_block(bp);
    /* 4~103 zmap block*/
    bp = get_block(4);
    bit = &bp->b_bitmap[0];
    *bit = 1;
    bp->b_dirt = DIRTY;
    put_block(bp);
    /* 104~1353 inode block */

    root = alloc_inode(3);
    if(root->i_num != 1){
        printf("root is not allocate as 1!! it is %d\n", root->i_num);
        number = root->i_num;
        put_inode(root);
        free_inode(number);
        return;
    }
    put_inode(root);
}

void my_mkfs(){
    init();
}
