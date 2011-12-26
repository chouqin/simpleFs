#include <stdio.h>
#include "myfs.h"

int main()
{
	int fd;
	char buf[16];
	/* dev_open(); */
    my_mkfs();
    reset_disk();
	show_file_list();
	my_mkdir("/dir1");
	show_file_list();
	fd=my_create("/dir1/file1");
	my_write(fd,"123",3);
	my_close(fd);
	show_file_list();
	fd=my_open("/dir1/file1");
	my_read(fd,buf,3);
	if(!((buf[0]=='1')&&(buf[1]=='2')&&(buf[2]=='3')))
	{
		printf("error\n");
	}
	my_close(fd);
	show_file_list();
	my_rename("/dir1/file1","/dir1/file2");
	show_file_list();
	my_remove("/dir1/file2");
	show_file_list();
	my_rmdir("/dir1");
	show_file_list();
    write_back();
	return 0;
}
