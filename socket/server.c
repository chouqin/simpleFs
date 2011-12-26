/*
 * =====================================================================================
 *
 *       Filename:  server.c
 *
 *    Description:
 *
 *        Version:  1.0
 *        Created:  2011年12月09日 16时07分33秒
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  YOUR NAME (),
 *        Company:
 *
 * =====================================================================================
 */

#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<errno.h>
#include<sys/types.h>
#include<sys/socket.h>
#include<netinet/in.h>
#include <fcntl.h>

#define MAXLINE 4096
#define READ_BLOCK 1
#define WRITE_BLOCK 2
#define OPEN 3

#define OK 0
#define ERR 1

/* void read_block() */
/* { */

/* } */

int main(int argc, char** argv)
{
    int    listenfd, connfd;
    struct sockaddr_in     servaddr;
    char    buff[4096];
    char  tmp[512];
    /* char * tmp; */
    int     n;
    int request_number, block_num;
    int return_msg;
    int fd;
    int position;

    if( (listenfd = socket(AF_INET, SOCK_STREAM, 0)) == -1 ){
        printf("create socket error: %s(errno: %d)\n",strerror(errno),errno);
        exit(0);
    }

    memset(&servaddr, 0, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    servaddr.sin_port = htons(6666);

    if( bind(listenfd, (struct sockaddr*)&servaddr, sizeof(servaddr)) == -1){
        printf("bind socket error: %s(errno: %d)\n",strerror(errno),errno);
        exit(0);
    }

    if( listen(listenfd, 10) == -1){
        printf("listen socket error: %s(errno: %d)\n",strerror(errno),errno);
        exit(0);
    }

    printf("======waiting for client's request======\n");
    fd = open("bigfile", O_RDWR);
    /* char c; */
    /* read(fd, &c, 1); */
    /* printf("the char is %c\n", c); */
    /* exit(0); */
    /* if (read(fd, tmp, 512) != 512) */
    /* { */
        /* printf("read block from file error: %s(errno: %d)\n", strerror(errno), errno); */
        /* exit(0); */
    /* } */
                /* printf("string length of tmp is %d\n", strlen(tmp)); */
    /* int *p = (int *)tmp; */
    /* int i, count = 0; */
    /* for(i = 0; i < 128; ++ i){ */
        /* if(*p == 0) count += 1; */
        /* ++ p; */
    /* } */
    /* printf("count is %d\n", count); */
    /* exit(0); */
    while(1){
        if( (connfd = accept(listenfd, (struct sockaddr*)NULL, NULL)) == -1){
            printf("accept socket error: %s(errno: %d)",strerror(errno),errno);
            continue;
        }
        /* printf("here"); */
        recv(connfd, &request_number, sizeof(int), 0);
        switch(request_number){
            case READ_BLOCK:
                /*read_block */
                return_msg = OK;
                if( send(connfd, &return_msg, sizeof(int), 0) < 0)
                {
                    printf("send msg error: %s(errno: %d)\n", strerror(errno), errno);
                    exit(0);
                }


                if(recv(connfd, &block_num, sizeof(int), 0) < 0){
                    printf("receive block number from client error: %s(errno: %d)\n", strerror(errno), errno);
                    exit(0);
                }

                printf("read block: block_num is %d\n", block_num);
                position = block_num * 512;

                if(lseek(fd, position, SEEK_SET) == -1){
                    printf("lseek error: %s(errno: %d)\n", strerror(errno), errno);
                    exit(0);
                }
                /* exit(0); */

                if (read(fd, tmp, 512) < 0)
                {
                    printf("read block from file error: %s(errno: %d)\n", strerror(errno), errno);
                    exit(0);
                }
                /* printf("string length of tmp is %d\n", strlen(tmp)); */
                /* tmp = "this is the message used to represent the block being read"; */
                /* printf("strlen of tmp is: %d\n", strlen(tmp)); */
                if( send(connfd, tmp, 512, 0) < 0)
                {
                    printf("send msg error: %s(errno: %d)\n", strerror(errno), errno);
                    exit(0);
                }
                break;
            case WRITE_BLOCK:
                /* write_block(); */
                return_msg = OK;
                if( send(connfd, &return_msg, sizeof(int), 0) < 0)
                {
                    printf("send msg error: %s(errno: %d)\n", strerror(errno), errno);
                    exit(0);
                }
                recv(connfd, &block_num, sizeof(int), 0);
                printf("write block: block_num is %d\n", block_num);
                return_msg = OK;
                if( send(connfd, &return_msg, sizeof(int), 0) < 0)
                {
                    printf("send msg error: %s(errno: %d)\n", strerror(errno), errno);
                    exit(0);
                }

                recv(connfd, buff, 512, 0);
                position = block_num * 512;
                if(lseek(fd, position, SEEK_SET) == -1){
                    printf("lseek error: %s(errno: %d)\n", strerror(errno), errno);
                    exit(0);
                }
                if (write(fd, buff, 512) < 0)
                {
                    printf("write block to file error: %s(errno: %d)\n", strerror(errno), errno);
                    exit(0);
                }
                if( send(connfd, &return_msg, sizeof(int), 0) < 0)
                {
                    printf("send msg error: %s(errno: %d)\n", strerror(errno), errno);
                    exit(0);
                }
               /* printf("write block %d into disk, the block is: %s\n", block_num ,buff); */
                break;
            case OPEN:
                /* open(); */
                break;
            default:
                printf("accept bad request_number: %d\n", request_number);
                return_msg = ERR;
                if( send(connfd, &return_msg, MAXLINE, 0) < 0)
                {
                    printf("send msg error: %s(errno: %d)\n", strerror(errno), errno);
                    exit(0);
                }
                exit(0);
        }
        /* printf("recv msg from client: %s\n", buff); */
        close(connfd);
    }

    close(listenfd);
}

