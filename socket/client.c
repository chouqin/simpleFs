/*
 * =====================================================================================
 *
 *       Filename:  client.c
 *
 *    Description:
 *
 *        Version:  1.0
 *        Created:  2011年12月09日 16时07分21秒
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

#define MAXLINE 4096
#define READ_BLOCK 1
#define WRITE_BLOCK 2
#define OPEN 3


#define OK 0
#define ERR 1

void read_block(int block_num, char * block){
    /* printf("here\n"); */
    int    sockfd, n, read = READ_BLOCK;
    char    recvline[4096];
    struct sockaddr_in    servaddr;
    int return_msg;
    if( (sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0){
        printf("create socket error: %s(errno: %d)\n", strerror(errno),errno);
        exit(0);
    }
    memset(&servaddr, 0, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(6666);
    /* servaddr.sin_addr = inet_addr("120.0.0.1"); */
    char * tmp = "127.0.0.1";
    if( inet_pton(AF_INET, tmp, &servaddr.sin_addr) <= 0){
        printf("inet_pton error for %s\n", tmp);
        exit(0);
    }

    /* printf("here\n"); */
    if( connect(sockfd, (struct sockaddr*)&servaddr, sizeof(servaddr)) < 0){
        printf("connect error: %s(errno: %d)\n",strerror(errno),errno);
        exit(0);
    }

    /* printf("send read block msg to server: \n"); */
    if( send(sockfd, &read, sizeof(int), 0) < 0)
    {
        printf("send msg error: %s(errno: %d)\n", strerror(errno), errno);
        exit(0);
    }
    if(recv(sockfd, &return_msg, sizeof(int), 0) < 0){
        printf("receive back message error: %s(errno: %d)\n", strerror(errno), errno);
        exit(0);
    }
    /* printf("server receive message status is %d\n", return_msg); */
    if (return_msg != OK){
        printf("don't receive OK message from server when send read_block\n");
        exit(0);
    }
    if( send(sockfd, &block_num, sizeof(int), 0) < 0)
    {
        printf("send msg error: %s(errno: %d)\n", strerror(errno), errno);
        exit(0);
    }
    if(recv(sockfd, block, MAXLINE, 0) < 0){
        printf("receive back message error: %s(errno: %d)\n", strerror(errno), errno);
        exit(0);
    }
    /* printf("server return block is: %s\n", block); */
    /* printf("block size is : %d\n", strlen(block)); */
    close(sockfd);
}


void write_block(int block_num, char * block){
    int    sockfd, n, write = WRITE_BLOCK;
    char    sendline[4096];
    struct sockaddr_in    servaddr;
    int return_msg;
    if( (sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0){
        printf("create socket error: %s(errno: %d)\n", strerror(errno),errno);
        exit(0);
    }
    memset(&servaddr, 0, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(6666);
    /* servaddr.sin_addr = inet_addr("120.0.0.1"); */
    char * tmp = "127.0.0.1";
    if( inet_pton(AF_INET, tmp, &servaddr.sin_addr) <= 0){
        printf("inet_pton error for %s\n", tmp);
        exit(0);
    }

    /* printf("here\n"); */
    if( connect(sockfd, (struct sockaddr*)&servaddr, sizeof(servaddr)) < 0){
        printf("connect error: %s(errno: %d)\n",strerror(errno),errno);
        exit(0);
    }

    /* printf("send write block msg to server: \n"); */
    if( send(sockfd, &write, sizeof(int), 0) < 0)
    {
        printf("send msg error: %s(errno: %d)\n", strerror(errno), errno);
        exit(0);
    }
    if(recv(sockfd, &return_msg, sizeof(int), 0) < 0){
        printf("receive back message error: %s(errno: %d)\n", strerror(errno), errno);
        exit(0);
    }
    /* printf("server receive message status is %d\n", return_msg); */
    if (return_msg != OK){
        printf("don't receive OK message from server when send write_block\n");
        exit(0);
    }
    if( send(sockfd, &block_num, sizeof(int), 0) < 0)
    {
        printf("send msg error: %s(errno: %d)\n", strerror(errno), errno);
        exit(0);
    }
    if(recv(sockfd, &return_msg, sizeof(int), 0) < 0){
        printf("receive back message error: %s(errno: %d)\n", strerror(errno), errno);
        exit(0);
    }
    /* printf("server receive message status is %d\n", return_msg); */
    if (return_msg != OK){
        printf("don't receive OK message from server when send block_num\n");
        exit(0);
    }
    /* printf("strlen of recvline is %d\n", strlen(recvline)); */
    if( send(sockfd, block, 512, 0) < 0)
    {
        printf("send msg error: %s(errno: %d)\n", strerror(errno), errno);
        exit(0);
    }
    if(recv(sockfd, &return_msg, sizeof(int), 0) < 0){
        printf("receive back message error: %s(errno: %d)\n", strerror(errno), errno);
        exit(0);
    }
    /* printf("server receive message status is %d\n", return_msg); */
    if (return_msg != OK){
        printf("don't receive OK message from server when write block\n");
        exit(0);
    }
    /* printf("strlen of recvline is %d\n", strlen(recvline)); */
    /* if(recv(sockfd, recvline, MAXLINE, 0) < 0){ */
        /* printf("receive back message error: %s(errno: %d)\n", strerror(errno), errno); */
        /* exit(0); */
    /* } */
 /* [> printf("strlen of recvline is %d\n", strlen(recvline)); <] */
    /* printf("server return block is: %s\n", recvline); */
    close(sockfd);

}

/* int main(int argc, char** argv) */
/* { */
    /* char block[512]; */
    /* memset(block, 0, 512); */
    /* int i = 0; */
    /* for(; i < 10; ++i){ */
        /* [> memset(block, 0, 512); <] */
        /* [> block[0]='h'; <] */
        /* [> block[1]='i'; <] */
        /* write_block(i, block); */
        /* [> memset(block, 0, 512); <] */
        /* [> read_block(i, block); <] */
        /* [> printf("the block read is %s\n", block); <] */
    /* } */
    /* [> int i, j; <] */
    /* [> int *p; <] */
    /* [> int count; <] */
    /* [> char * test = "test"; <] */
    /* [> printf("string test length is %d\n", strlen(test)); <] */
    /* [> for ( i = 0; i < 1; ++ i){ <] */
        /* [> count = 0; <] */
        /* [> read_block(i, block); <] */
        /* [> printf("the block size is %d\n", strlen(block)); <] */
        /* [> p = (int *)block; <] */
        /* [> for(j = 0; j < 128; ++ j){ <] */
            /* [> if (*p == 0) <] */
                /* [> count += 1; <] */
            /* [> printf("%d ", *p); <] */
            /* [> p ++; <] */
        /* [> } <] */
        /* [> printf("\n"); <] */
        /* [> printf("number of 0 is: %d\n", count); <] */
    /* [> } <] */
    /* [> read_block(5, block); <] */
    /* [> write_block(5, block); <] */
    /* [> int block_num = 50; <] */
    /* [> int    sockfd, n, read = READ_BLOCK; <] */
    /* [> char    recvline[4096], sendline[4096]; <] */
    /* [> struct sockaddr_in    servaddr; <] */
    /* [> if( (sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0){ <] */
        /* [> printf("create socket error: %s(errno: %d)\n", strerror(errno),errno); <] */
        /* [> exit(0); <] */
    /* [> } <] */
    /* [> memset(&servaddr, 0, sizeof(servaddr)); <] */
    /* [> servaddr.sin_family = AF_INET; <] */
    /* [> servaddr.sin_port = htons(6666); <] */
    /* [> [> servaddr.sin_addr = inet_addr("120.0.0.1"); <] <] */
    /* [> [> char * tmp = "120.0.0.1"; <] <] */
    /* [> if( inet_pton(AF_INET, argv[1], &servaddr.sin_addr) <= 0){ <] */
        /* [> printf("inet_pton error for %s\n", argv[1]); <] */
        /* [> exit(0); <] */
    /* [> } <] */

    /* [> [> printf("here\n"); <] <] */
    /* [> if( connect(sockfd, (struct sockaddr*)&servaddr, sizeof(servaddr)) < 0){ <] */
        /* [> printf("connect error: %s(errno: %d)\n",strerror(errno),errno); <] */
        /* [> exit(0); <] */
    /* [> } <] */

    /* [> printf("send read block msg to server: \n"); <] */
    /* [> if( send(sockfd, &read, strlen(sendline), 0) < 0) <] */
    /* [> { <] */
        /* [> printf("send msg error: %s(errno: %d)\n", strerror(errno), errno); <] */
        /* [> exit(0); <] */
    /* [> } <] */
    /* [> if(recv(sockfd, recvline, strlen(recvline), 0) < 0){ <] */
        /* [> printf("receive back message error: %s(errno: %d)\n", strerror(errno), errno); <] */
        /* [> exit(0); <] */
    /* [> } <] */
    /* [> printf("server receive message status is %s\n", recvline); <] */
    /* [> if (strcmp(recvline, "OK") != 0){ <] */
        /* [> printf("don't receive OK message from server when send %s\n", sendline); <] */
        /* [> exit(0); <] */
    /* [> } <] */
    /* [> if( send(sockfd, &block_num, strlen(sendline), 0) < 0) <] */
    /* [> { <] */
        /* [> printf("send msg error: %s(errno: %d)\n", strerror(errno), errno); <] */
        /* [> exit(0); <] */
    /* [> } <] */
    /* [> if(recv(sockfd, recvline, strlen(recvline), 0) < 0){ <] */
        /* [> printf("receive back message error: %s(errno: %d)\n", strerror(errno), errno); <] */
        /* [> exit(0); <] */
    /* [> } <] */
    /* [> printf("server return block is %s\n", recvline); <] */
    /* [> close(sockfd); <] */
    /* exit(0); */
/* } */

