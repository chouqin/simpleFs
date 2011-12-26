/*
 * =====================================================================================
 *
 *       Filename:  fucttest.c
 *
 *    Description:
 *
 *        Version:  1.0
 *        Created:  2011年12月14日 18时41分10秒
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


#define MAX_FILE_SIZE 2000000

void dir_test(){
    struct inode* root_inode;
    struct inode *rip;
    char string[DIRSIZ];
    int number;
    int * inode_num = & number;
    int r;

    root_inode = get_inode(1);
    if(root_inode == NIL_INODE){
        printf("root_inode is a null inode\n");
        return;
    }
    /* printf("root_inode size is %d\n", root_inode->i_size); */
    /* delete_dir(root_inode, 0); */
    /* empty_inode_space(root_inode); */
    /* put_inode(root_inode); */
    /* return; */

    /* my_strcpy(string, "usr"); */
    /* if( r = search_dir(root_inode, string, inode_num, LOOK_UP) != OK){ */
        /* printf("LOOK_UP a usr error: %s\n", strerror(r)); */
        /* put_inode(root_inode); */
        /* return; */
    /* } */
    /* printf("this time i node size is %d\n", root_inode->i_size); */
    /* put_inode(root_inode); */
    /* return; */

    my_strcpy(string, "..");
    *inode_num = root_inode->i_num;
    if (r = search_dir(root_inode, string, inode_num, ENTER) != OK){
        printf("enter a item error: %s\n", strerror(r));
        put_inode(root_inode);
        return;
    }
    my_strcpy(string, ".");
    *inode_num = root_inode->i_num;
    if (r = search_dir(root_inode, string, inode_num, ENTER) != OK){
        printf("enter a item error: %s\n", strerror(r));
        put_inode(root_inode);
        return;
    }
    root_inode->i_dirt = DIRTY;
    put_inode(root_inode);
    show_file_list();
    /* return; */
    printf("\n");

    my_strcpy(string, ".");
    root_inode = get_inode(1);
    *inode_num = 0;
    if (r = search_dir(root_inode, string, inode_num, LOOK_UP) != OK){
        printf("find a item error: %s\n", strerror(r));
        put_inode(root_inode);
        return;
    }
    if(*inode_num != root_inode->i_num){
        printf("can't find . in the root directory!!!\n");
        put_inode(root_inode);
        return;
    }
    put_inode(root_inode);

    my_mkdir("./usr");
    my_mkdir("src");
    my_mkdir("../root");
    show_file_list();
    printf("\n");

    root_inode = last_dir("./../usr", string);
    if(root_inode == NIL_INODE){
        printf("last_dir error: can't find root dir\n");
        return;
    }
    if(root_inode->i_num != 1){
        printf("this time last dir is not root_inode, it's i_num is %d\n", root_inode->i_num);
        put_inode(root_inode);
        return;
    }
    printf("string remain is %s\n", string);
    show_file_list();
    printf("\n");
    rip = advance(root_inode, string);
    if(rip == NIL_INODE){
        printf("advance error: can't find usr dir\n");
        return;
    }
    show_file_list();
    printf("\n");
    put_inode(rip);
    put_inode(root_inode);

    if(my_rmdir("/src") != OK){
        printf("can't remove dir src\n");
        return;
    }
    show_file_list();
}


void file_test(){
    my_mkdir("usr/data");
    show_file_list();
    printf("\n");

    my_mkdir("src");
    my_mkdir("../usr/./data/data1");
    show_file_list();
    printf("\n");

    int fd;
    fd = my_create("usr/.././usr/data/.././data/data1/file1");
    show_file_list();
    my_close(fd);

    fd = my_open("usr/.././usr/data/.././data/data1/file1");
    my_close(fd);
    show_file_list();
}

void read_write_test(){
    int fd;
    fd = my_open("usr/data/data1/file1_1");

    if(fd < 0){
        printf("get fd error\n");
        return;
    }

    char string[MAX_FILE_SIZE];
    int i = 0;
    int count = 0;
    int j = 0;

    memset(string, 0, MAX_FILE_SIZE);
    /* exit(0); */
    for(; i < MAX_FILE_SIZE; ++i){
        string[i] = 'a';
    }
    for(i = 0; i < 25; ++i){
        if(my_write(fd, string, MAX_FILE_SIZE) != MAX_FILE_SIZE){
            printf("write error\n");
            exit(0);
            /* return; */
        }
    }
    /* my_close(fd); */
    /* return; */
    for(j = 0; j < 25; ++ j){
        memset(string, 0, MAX_FILE_SIZE);
        if(my_read(fd, string, MAX_FILE_SIZE) != MAX_FILE_SIZE){
            printf("read error\n");
            exit(0);
            /* return; */
        }
        for(i = 0; i < MAX_FILE_SIZE; ++i){
            if(string[i] == 'a'){
                count ++;
            }
        }
    }
    printf("number of a is %d\n", count);
    my_close(fd);
}


void rename_test(){
    int fd;
    show_file_list();
    char filename[40];
    char changename[40];
    printf("\n");
    if(my_rename("/usr/data/data1/file1", "/usr/data/data1/file1_1") != OK){
        printf("rename error!!\n");
        return;
    }
    show_file_list();
    printf("\n");

    if(my_mkdir("usr/data/manyfile") != OK){
        printf("mkdir error!!\n");
        return;
    }
    show_file_list();
    printf("\n");

    int i = 35;
    memset(filename, 0, 40);
    memset(changename, 0, 40);
    strcpy(filename, "usr/data/manyfile/file");
    strcpy(changename, "usr/data/manyfile/file");
    int length = strlen(filename);
    for(; i < 65; ++i){
        filename[length] = (char)i;
        if(fd = my_create(filename) < 0){
            my_close(fd);
            printf("create file error when i = %d\n", i);
            return;
        }
        my_close(fd);
    }
        show_file_list();
        printf("\n");

    /* int i = 30; */
    /* changename[length] = (char)70; */
    /* printf("changename is %s, length is %d\n",changename, length); */
    /* printf("filename is %s, length is %d\n",filename, length); */
    /* exit(0); */
    for( i = 35; i < 65; ++i){
        filename[length] = (char)i;
        changename[length] = (char)(100 - i);
        if(my_rename(filename, changename) != OK){
            printf("rename error when i = %d!!\n", i);
            return;
        }
    }
        show_file_list();
        printf("\n");
}

int main(){
    init();
    /* reset_disk(); */
    /* dir_test(); */
    /* file_test(); */
    show_file_list();
    /* read_write_test(); */
    /* rename_test(); */
    write_back();
}
