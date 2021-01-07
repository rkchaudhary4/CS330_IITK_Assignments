#include<ulib.h>

int main(u64 arg1, u64 arg2, u64 arg3, u64 arg4, u64 arg5)
{
    int pid;
    char buf1[20];
    char buf2[20];
    char *filename1="test1.txt";
    char *filename2="test2.txt";
    int create_fd1 = open(filename1, O_CREAT|O_RDWR, O_READ|O_WRITE);
    int create_fd2 = open(filename2, O_CREAT|O_RDWR, O_READ|O_WRITE);

    char *buf = "Hello, I am file!";
    write(create_fd1, buf, 17);
    lseek(create_fd1, 0, SEEK_SET);

    sendfile(create_fd2, create_fd1, NULL, 17);
    lseek(create_fd1, 0, SEEK_SET);

    int read_fd = open(filename2, O_RDONLY);
    
    read(read_fd, buf2, 17);
    read(create_fd1, buf1, 17);
    buf2[17] = '\0';
    buf1[17] = '\0';
    
    printf("buf1 = %s\n",buf1);
    printf("buf2 = %s\n",buf2);

    close(read_fd);
    close(create_fd1);
    close(create_fd2);
    
    return 0;
}
