/*
 * =====================================================================================
 *
 *       Filename:  unittest.c
 *
 *    Description:
 *
 *        Version:  1.0
 *        Created:  2011年12月12日 21时00分53秒
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  YOUR NAME (),
 *        Company:
 *
 * =====================================================================================
 */
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include "myfs.h"



/* private functions of other file */
#include "buf.h"
#include "inode.h"
#include "super.h"
#include "file.h"
#include "dir.h"
#include "./socket/client.h"

void buf_test(){
    struct buf * bp = get_block(START_BLOCK);
    /* printf("here\n");exit(0); */
    int *wptr = &bp->b_bitmap[0];
    int i;
    *wptr = (int)1;
    bp->b_dirt = DIRTY;
    put_block(bp);
    bp = get_block(START_BLOCK);
    if (bp->b_dirt != DIRTY){
        printf("after put and get, buf is not the same\n");
        exit(0);
    }
    wptr = &bp->b_bitmap[0];
    if (*wptr != 1){
        printf("after put to get, the data changes!!\n");
        exit(0);
    }
    put_block(bp);
    for (i = 2; i < 500; ++ i){
        bp = get_block(i);
        put_block(bp);
    }
    bp = get_block(START_BLOCK);
    wptr = &bp->b_bitmap[0];
    if (*wptr != 1){
        printf("after write to disk, the data changes!!\n");
        exit(0);
    }
    put_block(bp);


    /* driver test part */
    /* char buff[512]; */
    /* int *p; */
    /* read_block(1, buff); */
    /* p = (int *)buff; */
    /* int i, count = 0; */
    /* for (i = 0; i < 128; ++i){ */
        /* if (*p == 0){ */
            /* count += 1; */
        /* } */
        /* p ++; */
    /* } */
    /* printf("number of 0 is %d\n", count); */
    /* p = (int *)buff; */
    /* *p = 0; */
    /* write_block(1, buff); */
    /* read_block(1, buff); */
    /* p = (int *)buff; */
    /* printf("* p is %d\n", *p); */
    /* write_block(bp->b_blocknr, &bp->b); */
}

void inode_test(){

    int i;
    /* for (i = 2; i < 1000; ++ i){ */
        /* free_inode(i); */
    /* } */

    /* free_inode(2); */
    /* free_inode(1); */

    int mode = 0x00000003;
    struct inode * root_inode = get_inode(1);
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
    put_inode(root_inode);
    root_inode = get_inode(1);
    if (!(root_inode->i_mode & I_DIRECTORY)){
        printf("after put and get: root is not a directory!!\n");
        printf("this time the i number is %d and imode is %d\n", (int)root_inode->i_num, root_inode->i_mode);
        exit(1);
    }

    struct inode *test_inode;
    for (i = 2; i < 1000; ++i){
        test_inode = alloc_inode(mode);
        if (test_inode == NIL_INODE){
            printf("can't allocate test_inode for inode number %d\n", i);
            exit(1);
        }
        if (test_inode->i_num != i){
            printf("alloc_inode is not as expected!!\n, expect: %d, result: %d", i, test_inode->i_num);
            exit(1);
        }
        if (!(test_inode->i_mode & I_DIRECTORY)){
            printf("allocate error: test_inode is not a directory!!\n");
            printf("this time the i number is %d and imode is %d\n", (int)test_inode->i_num, test_inode->i_mode);
            exit(1);
        }
        /* test_inode->i_dirt = DIRTY; */
        put_inode(test_inode);
        test_inode = get_inode(i);
        if (!(test_inode->i_mode & I_DIRECTORY)){
            printf("after put and get: test_inode is not a directory!!\n");
            printf("this time the i number is %d and imode is %d\n", (int)test_inode->i_num, test_inode->i_mode);
            exit(1);
        }
        put_inode(test_inode);
    }

    for (i = 1; i < 1000; ++i){
        /* test_inode = get_inode(i); */
        /* if (!(test_inode->i_mode & I_DIRECTORY)){ */
            /* printf("after write to disk: test_inode is not a directory!!\n"); */
            /* printf("this time the i number is %d and imode is %d\n", (int)test_inode->i_num, test_inode->i_mode); */
            /* exit(1); */
        /* } */
        free_inode(i);
    }

    root_inode = alloc_inode(mode);
    if (root_inode == NIL_INODE){
        printf("after allocate and free: can't allocate root_inode\n");
        exit(1);
    }
    if (root_inode->i_num != 1){
        printf("after allocate and free:root bit allocate error: can't set root to bit 1, the inode number is %d\n", (int)root_inode->i_num);
        exit(1);
    }
}

void super_test(){
    struct buf * bp;
    int * wptr;
    int i;
    bp = get_block(START_BLOCK + S_IMAP_BLOCKS);
    /* printf("here\n");exit(0); */
    wptr = &bp->b_bitmap[0];
    *wptr = (int)1;
    bp->b_dirt = DIRTY;
    put_block(bp);

    for (i = 2; i < 500; ++ i){
        bp = get_block(i);
        put_block(bp);
    }

    bp = get_block(START_BLOCK + S_IMAP_BLOCKS);
    wptr = &bp->b_bitmap[0];
    if (*wptr != 1){
        printf("after write to disk, the data changes!!\n");
        exit(0);
    }
    put_block(bp);


    /* allocate disk for root inode */
    zone_t root_zone = alloc_zone(S_FIRSTDATAZONE);
    if (root_zone != (S_FIRSTDATAZONE + 1)){
        printf("root zone is not the first data zone!!\n");
        exit(1);
    }
    free_zone(root_zone);
    root_zone = alloc_zone(S_FIRSTDATAZONE);
    if (root_zone != (S_FIRSTDATAZONE + 1)){
        printf("root zone is not the first data zone after free_zone!!\n");
        exit(1);
    }
}


void search_dir_test(){
    struct inode * root_inode;
    struct inode * new_inode;
    root_inode = get_inode(1);
    /* exit(0); */
    if (root_inode == NIL_INODE){
        printf("root_inode is NULL\n");
        exit(1);
    }
    char  name[DIRSIZ];
    int *inode_num;
    int r;
    int num = root_inode->i_num;
    zone_t z;
    int i;
    inode_num = &num;
    /* exit(0); */
    memset(name, 0, DIRSIZ);
    name[0] = 'u';name[1]='s';name[2]='r';
    strcpy(name, "usr");
    for( i = strlen(name); i < DIRSIZ; ++i){
        name[i] = '\0';
    }
    /* new_inode = alloc_inode(3); */
    /* printf("alloc a new inode %d\n", new_inode->i_num); */
    /* *inode_num = new_inode->i_num; */
    /* if (r = search_dir(root_inode, name, inode_num, ENTER) != OK){ */
        /* printf("enter a item error: %s\n", strerror(r)); */
        /* exit(1); */
    /* } */
    *inode_num = 0;
    if (r = search_dir(root_inode, name, inode_num, LOOK_UP) != OK){
        printf("enter a item error: %s\n", strerror(r));
        exit(1);
    }
    if (*inode_num != 2){
        printf("direct .. is not set as expected, it turns out to be: %d\n", *inode_num);
        exit(1);
    }

    memset(name, 0, DIRSIZ);
    strcpy(name, "root");
    for( i = strlen(name); i < DIRSIZ; ++i){
        name[i] = '\0';
    }
    /* new_inode = alloc_inode(3); */
    /* printf("alloc a new inode %d\n", new_inode->i_num); */
    /* *inode_num = new_inode->i_num; */
    /* if (r = search_dir(root_inode, name, inode_num, ENTER) != OK){ */
        /* printf("enter a item error: %s\n", strerror(r)); */
        /* exit(1); */
    /* } */
    *inode_num = 0;
    if (r = search_dir(root_inode, name, inode_num, LOOK_UP) != OK){
        printf("enter a item error: %s\n", strerror(r));
        exit(1);
    }
    if (*inode_num != 3){
        printf("direct .. is not set as expected, it turns out to be: %d\n", *inode_num);
        exit(1);
    }

    memset(name, 0, DIRSIZ);
    strcpy(name, "etc");
    for( i = strlen(name); i < DIRSIZ; ++i){
        name[i] = '\0';
    }
    /* new_inode = alloc_inode(3); */
    /* printf("alloc a new inode %d\n", new_inode->i_num); */
    /* *inode_num = new_inode->i_num; */
    /* if (r = search_dir(root_inode, name, inode_num, ENTER) != OK){ */
        /* printf("enter a item error: %s\n", strerror(r)); */
        /* exit(1); */
    /* } */
    *inode_num = 0;
    if (r = search_dir(root_inode, name, inode_num, LOOK_UP) != OK){
        printf("enter a item error: %s\n", strerror(r));
        exit(1);
    }
    if (*inode_num != 4){
        printf("direct .. is not set as expected, it turns out to be: %d\n", *inode_num);
        exit(1);
    }

    /* name[0] = '.'; */
    /* [> if (r = search_dir(root_inode, name, inode_num, ENTER) != OK){ <] */
        /* [> printf("enter a item error: %s\n", strerror(r)); <] */
        /* [> exit(1); <] */
    /* [> } <] */
    /* [> name[1] = '.'; <] */
    /* [> if (r = search_dir(root_inode, name, inode_num, ENTER) != OK){ <] */
        /* [> printf("enter a item error: %s\n", strerror(r)); <] */
        /* [> exit(1); <] */
    /* [> } <] */
    /* *inode_num = 0; */
    /* if (r = search_dir(root_inode, name, inode_num, LOOK_UP) != OK){ */
        /* printf("enter a item error: %s\n", strerror(r)); */
        /* exit(1); */
    /* } */
    /* if (*inode_num != root_inode->i_num){ */
        /* printf("direct . is not set as expected, it turns out to be: %d\n", *inode_num); */
        /* exit(1); */
    /* } */
    /* *inode_num = 0; */
    /* [> exit(0); <] */
    /* name[1] = '.'; */
    /* if (r = search_dir(root_inode, name, inode_num, LOOK_UP) != OK){ */
        /* printf("enter a item error: %s\n", strerror(r)); */
        /* exit(1); */
    /* } */
    /* if (*inode_num != root_inode->i_num){ */
        /* printf("direct .. is not set as expected, it turns out to be: %d\n", *inode_num); */
        /* exit(1); */
    /* } */
    put_inode(root_inode);
}

/* void my_strcpy(char des[DIRSIZ], const char * src){ */
    /* memset(des, 0, DIRSIZ); */
    /* int i = 0; */
    /* strcpy(des, src); */
    /* for (i = strlen(des); i < DIRSIZ; ++ i) */
        /* des[i] = '\0'; */
/* } */


void reset_directory(){
    /* free_zone(1355); */
    /* struct inode * root_inode; */
    /* root_inode = get_inode(1); */
    /* [> exit(0); <] */
    /* if (root_inode == NIL_INODE){ */
        /* printf("root_inode is NULL\n"); */
        /* exit(1); */
    /* } */
    /* root_inode->i_size = 0; */
    /* root_inode->i_zone[0] = NO_ZONE; */
    /* root_inode->i_dirt = DIRTY; */
    /* put_inode(root_inode); */
    /* int i; */
    struct inode * rip;
    /* for (i = 9; i > 4; -- i) { */
        /* rip = get_inode(i); */
        /* empty_inode_space(rip); */
        /* put_inode(rip); */
        /* free_inode(i); */
    /* } */
    rip = get_inode(1);
    /* empty_inode_space(rip); */
    /* put_inode(rip); */
    /* free_inode(11); */
    /* rip = get_inode(10); */
    /* empty_inode_space(rip); */
    /* put_inode(rip); */
    /* free_inode(10); */
    delete_dir(rip, 0);
    empty_inode_space(rip);
    put_inode(rip);
}

void advance_test(){
    struct inode* root_inode;
    struct inode* a_inode;
    char name[DIRSIZ];
    root_inode = get_inode(1);

    my_strcpy(name, "usr");
    a_inode = advance(root_inode, name);
    if (a_inode == NIL_INODE){
        printf("can't find file in the directory\n");
        exit(1);
    }
    printf("the inode number of %s is %d, mode: %d, size: %d\n", name, a_inode->i_num, a_inode->i_mode, a_inode->i_size);
    my_strcpy(name, "root");
    a_inode = advance(root_inode, name);
    if (a_inode == NIL_INODE){
        printf("can't find file in the directory\n");
        exit(1);
    }
    printf("the inode number of %s is %d, mode: %d, size: %d\n", name, a_inode->i_num, a_inode->i_mode, a_inode->i_size);
    a_inode = advance(root_inode, name);
    if (a_inode == NIL_INODE){
        printf("can't find file in the directory\n");
        exit(1);
    }
    printf("the inode number of %s is %d, mode: %d, size: %d\n", name, a_inode->i_num, a_inode->i_mode, a_inode->i_size);
}


void list_file_test(){
    struct inode * root_inode = get_inode(1);
    list_dir(root_inode, 0);
    put_inode(root_inode);
}

void new_file_test(){
    char filename[DIRSIZ];
    struct inode * root_inode;
    struct inode * usrnode;
    struct inode * new_inode;
    int inode_n;
    int * inode_num = &inode_n;
    int r;


    root_inode = get_inode(1);

    my_strcpy(filename, "usr");
    new_inode = alloc_inode(3);
    printf("allocate a new inode for %s, inode_num is %d\n", filename, new_inode->i_num);
    *inode_num = new_inode->i_num;
    if (r = search_dir(root_inode, filename, inode_num, ENTER) != OK){
        printf("enter a item error: %s\n", strerror(r));
        exit(1);
    }
    my_strcpy(filename, ".");
    *inode_num = new_inode->i_num;
    if (r = search_dir(new_inode, filename, inode_num, ENTER) != OK){
        printf("enter a item error: %s\n", strerror(r));
        exit(1);
    }
    my_strcpy(filename, "..");
    *inode_num = root_inode->i_num;
    if (r = search_dir(new_inode, filename, inode_num, ENTER) != OK){
        printf("enter a item error: %s\n", strerror(r));
        exit(1);
    }
    put_inode(new_inode);

    my_strcpy(filename, "src");
    new_inode = alloc_inode(3);
    printf("allocate a new inode for %s, inode_num is %d\n", filename, new_inode->i_num);
    *inode_num = new_inode->i_num;
    if (r = search_dir(root_inode, filename, inode_num, ENTER) != OK){
        printf("enter a item error: %s\n", strerror(r));
        exit(1);
    }
    my_strcpy(filename, ".");
    *inode_num = new_inode->i_num;
    if (r = search_dir(new_inode, filename, inode_num, ENTER) != OK){
        printf("enter a item error: %s\n", strerror(r));
        exit(1);
    }
    my_strcpy(filename, "..");
    *inode_num = root_inode->i_num;
    if (r = search_dir(new_inode, filename, inode_num, ENTER) != OK){
        printf("enter a item error: %s\n", strerror(r));
        exit(1);
    }
    put_inode(new_inode);

    my_strcpy(filename, "include");
    new_inode = alloc_inode(3);
    printf("allocate a new inode for %s, inode_num is %d\n", filename, new_inode->i_num);
    *inode_num = new_inode->i_num;
    if (r = search_dir(root_inode, filename, inode_num, ENTER) != OK){
        printf("enter a item error: %s\n", strerror(r));
        exit(1);
    }
    my_strcpy(filename, ".");
    *inode_num = new_inode->i_num;
    if (r = search_dir(new_inode, filename, inode_num, ENTER) != OK){
        printf("enter a item error: %s\n", strerror(r));
        exit(1);
    }
    my_strcpy(filename, "..");
    *inode_num = root_inode->i_num;
    if (r = search_dir(new_inode, filename, inode_num, ENTER) != OK){
        printf("enter a item error: %s\n", strerror(r));
        exit(1);
    }
    put_inode(new_inode);

    usrnode = get_inode(2);

    my_strcpy(filename, ".");
    *inode_num = usrnode->i_num;
    if (r = search_dir(usrnode, filename, inode_num, ENTER) != OK){
        printf("enter a item error: %s\n", strerror(r));
        exit(1);
    }

    my_strcpy(filename, "..");
    *inode_num = 1;
    if (r = search_dir(usrnode, filename, inode_num, ENTER) != OK){
        printf("enter a item error: %s\n", strerror(r));
        exit(1);
    }

    my_strcpy(filename, "data");
    new_inode = alloc_inode(3);
    printf("allocate a new inode for %s, inode_num is %d\n", filename, new_inode->i_num);
    *inode_num = new_inode->i_num;
    if (r = search_dir(usrnode, filename, inode_num, ENTER) != OK){
        printf("enter a item error: %s\n", strerror(r));
        exit(1);
    }
    my_strcpy(filename, ".");
    *inode_num = new_inode->i_num;
    if (r = search_dir(new_inode, filename, inode_num, ENTER) != OK){
        printf("enter a item error: %s\n", strerror(r));
        exit(1);
    }
    my_strcpy(filename, "..");
    *inode_num = usrnode->i_num;
    if (r = search_dir(new_inode, filename, inode_num, ENTER) != OK){
        printf("enter a item error: %s\n", strerror(r));
        exit(1);
    }
    put_inode(new_inode);

    my_strcpy(filename, "src");
    new_inode = alloc_inode(3);
    printf("allocate a new inode for %s, inode_num is %d\n", filename, new_inode->i_num);
    *inode_num = new_inode->i_num;
    if (r = search_dir(usrnode, filename, inode_num, ENTER) != OK){
        printf("enter a item error: %s\n", strerror(r));
        exit(1);
    }
    my_strcpy(filename, ".");
    *inode_num = new_inode->i_num;
    if (r = search_dir(new_inode, filename, inode_num, ENTER) != OK){
        printf("enter a item error: %s\n", strerror(r));
        exit(1);
    }
    my_strcpy(filename, "..");
    *inode_num = usrnode->i_num;
    if (r = search_dir(new_inode, filename, inode_num, ENTER) != OK){
        printf("enter a item error: %s\n", strerror(r));
        exit(1);
    }
    put_inode(new_inode);

    my_strcpy(filename, "include");
    new_inode = alloc_inode(3);
    printf("allocate a new inode for %s, inode_num is %d\n", filename, new_inode->i_num);
    *inode_num = new_inode->i_num;
    if (r = search_dir(usrnode, filename, inode_num, ENTER) != OK){
        printf("enter a item error: %s\n", strerror(r));
        exit(1);
    }
    my_strcpy(filename, ".");
    *inode_num = new_inode->i_num;
    if (r = search_dir(new_inode, filename, inode_num, ENTER) != OK){
        printf("enter a item error: %s\n", strerror(r));
        exit(1);
    }
    my_strcpy(filename, "..");
    *inode_num = usrnode->i_num;
    if (r = search_dir(new_inode, filename, inode_num, ENTER) != OK){
        printf("enter a item error: %s\n", strerror(r));
        exit(1);
    }
    put_inode(new_inode);



    new_inode = advance(usrnode, "data");
    put_inode(usrnode);
    usrnode = new_inode;

    my_strcpy(filename, "data1");
    new_inode = alloc_inode(3);
    printf("allocate a new inode for %s, inode_num is %d\n", filename, new_inode->i_num);
    *inode_num = new_inode->i_num;
    if (r = search_dir(usrnode, filename, inode_num, ENTER) != OK){
        printf("enter a item error: %s\n", strerror(r));
        exit(1);
    }
    my_strcpy(filename, ".");
    *inode_num = new_inode->i_num;
    if (r = search_dir(new_inode, filename, inode_num, ENTER) != OK){
        printf("enter a item error: %s\n", strerror(r));
        exit(1);
    }
    my_strcpy(filename, "..");
    *inode_num = usrnode->i_num;
    if (r = search_dir(new_inode, filename, inode_num, ENTER) != OK){
        printf("enter a item error: %s\n", strerror(r));
        exit(1);
    }
    put_inode(new_inode);

    my_strcpy(filename, "data2");
    new_inode = alloc_inode(3);
    printf("allocate a new inode for %s, inode_num is %d\n", filename, new_inode->i_num);
    *inode_num = new_inode->i_num;
    if (r = search_dir(usrnode, filename, inode_num, ENTER) != OK){
        printf("enter a item error: %s\n", strerror(r));
        exit(1);
    }
    my_strcpy(filename, ".");
    *inode_num = new_inode->i_num;
    if (r = search_dir(new_inode, filename, inode_num, ENTER) != OK){
        printf("enter a item error: %s\n", strerror(r));
        exit(1);
    }
    my_strcpy(filename, "..");
    *inode_num = usrnode->i_num;
    if (r = search_dir(new_inode, filename, inode_num, ENTER) != OK){
        printf("enter a item error: %s\n", strerror(r));
        exit(1);
    }
    put_inode(new_inode);

    my_strcpy(filename, "data3");
    new_inode = alloc_inode(3);
    printf("allocate a new inode for %s, inode_num is %d\n", filename, new_inode->i_num);
    *inode_num = new_inode->i_num;
    if (r = search_dir(usrnode, filename, inode_num, ENTER) != OK){
        printf("enter a item error: %s\n", strerror(r));
        exit(1);
    }
    my_strcpy(filename, ".");
    *inode_num = new_inode->i_num;
    if (r = search_dir(new_inode, filename, inode_num, ENTER) != OK){
        printf("enter a item error: %s\n", strerror(r));
        exit(1);
    }
    my_strcpy(filename, "..");
    *inode_num = usrnode->i_num;
    if (r = search_dir(new_inode, filename, inode_num, ENTER) != OK){
        printf("enter a item error: %s\n", strerror(r));
        exit(1);
    }
    put_inode(new_inode);

    my_strcpy(filename, "data4.txt");
    new_inode = alloc_inode(1);
    printf("allocate a new inode for %s, inode_num is %d\n", filename, new_inode->i_num);
    *inode_num = new_inode->i_num;
    if (r = search_dir(usrnode, filename, inode_num, ENTER) != OK){
        printf("enter a item error: %s\n", strerror(r));
        exit(1);
    }
    put_inode(new_inode);
}

void lastdir_test(){
    struct inode * rip, *ip;
    char string[DIRSIZ];

    /* new_file_test(); */
    rip = last_dir("/usr/data/data4.txt", string);
    printf("the inode get: inode number: %d, size: %d, mode: %d and string left: %s\n", rip->i_num, rip->i_size, rip->i_mode, string);
    ip = advance(rip, "..");
    printf("the advance inode get: inode number: %d, size: %d, mode: %d and string left: %s\n", ip->i_num, ip->i_size, ip->i_mode, string);
    put_inode(rip);

    rip = last_dir("//usr/./data/../data4.txt", string);
    printf("the inode get: inode number: %d, size: %d, mode: %d and string left: %s\n", rip->i_num, rip->i_size, rip->i_mode, string);
    put_inode(rip);

    rip = last_dir("../usr/.././usr", string);
    printf("the inode get: inode number: %d, size: %d, mode: %d and string left: %s\n", rip->i_num, rip->i_size, rip->i_mode, string);
    put_inode(rip);
}

void eat_path_test(){
    /* advance_test(); */

}





int main(){
    init();
    /* buf_test(); */
    /* super_test(); */
    /* inode_test(); */
    /* search_dir_test(); */
    /* reset_directory(); */
    /* eat_path_test(); */
    /* lastdir_test(); */
    /* list_file_test(); */
    write_back();
}
