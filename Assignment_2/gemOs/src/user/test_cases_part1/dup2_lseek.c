
#include<ulib.h>


int main(u64 arg1, u64 arg2, u64 arg3, u64 arg4, u64 arg5)
{
	int pid;
	char buf1[20];
	char *filename="test.txt";
	int create_fd = open(filename, O_CREAT|O_RDWR, O_READ|O_WRITE);
	char *buf = "Hello, I am file!";
	write(create_fd, buf, 17);

	int dup_fd2 = dup2(create_fd, 15);

	printf("%d\n", dup_fd2);

	printf("%d\n", lseek(create_fd, 0, SEEK_CUR));
	return 0;
}
