#include <stdio.h>
#include "myfs.h"


int main()
{
	int i;
	int bn;
	int fd;
	char buf[520];
	for(i=0;i<512;i++)
	{
		buf[i]='0';
	}
	/* bn=dev_open(); */
	my_mkfs();
	fd=my_create("/testfile1");
	for(i=0;i<(/10);i++)
	{
		my_write(fd,buf,512);
	}
	my_close(fd);
	fd=my_create("/testfile2");
	for(i=0;i<(bn/10);i++)
	{
		my_write(fd,buf,512);
	}
	my_close(fd);
	fd=my_create("/testfile3");
	for(i=0;i<(bn/10);i++)
	{
		my_write(fd,buf,512);
	}
	my_close(fd);
	fd=my_create("/testfile4");
	for(i=0;i<(bn/10);i++)
	{
		my_write(fd,buf,512);
	}
	my_close(fd);
	fd=my_create("/testfile5");
	for(i=0;i<(bn/10);i++)
	{
		my_write(fd,buf,512);
	}
	my_close(fd);
	fd=my_create("/testfile6");
	for(i=0;i<(bn/10);i++)
	{
		my_write(fd,buf,512);
	}
	my_close(fd);
	fd=my_create("/testfile7");
	for(i=0;i<(bn/10);i++)
	{
		my_write(fd,buf,512);
	}
	my_close(fd);
	fd=my_create("/testfile8");
	for(i=0;i<(bn/10);i++)
	{
		my_write(fd,buf,512);
	}
	my_close(fd);
	fd=my_create("/testfile9");
	for(i=0;i<(bn/10);i++)
	{
		my_write(fd,buf,512);
	}
	my_close(fd);
	show_file_list();
	my_remove("/testfile1");
	my_remove("/testfile3");
	my_remove("/testfile5");
	my_remove("/testfile7");
	my_remove("/testfile9");
	show_file_list();
	fd=my_create("/testfile10");
	for(i=0;i<(bn/2);i++)
	{
		my_write(fd,buf,512);
	}
	my_close(fd);
	show_file_list();
	my_remove("/testfile10");
	show_file_list();
    write_back();
	return 0;
}
