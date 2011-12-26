DEBUG=-g

all: test1 test2

test1:	test1.c path.c inode.c super.c cache.c filedes.c myfs.c ./socket/client.c
	gcc  $(DEBUG) test1.c path.c inode.c super.c cache.c filedes.c myfs.c ./socket/client.c -o test1
test2:	test2.c path.c inode.c super.c cache.c filedes.c myfs.c ./socket/client.c
	gcc  $(DEBUG) test2.c path.c inode.c super.c cache.c filedes.c myfs.c ./socket/client.c -o test2
clean:
	rm -f test1 test2
