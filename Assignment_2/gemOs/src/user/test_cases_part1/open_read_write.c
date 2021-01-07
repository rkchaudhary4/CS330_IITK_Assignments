
#include<ulib.h>


int main(u64 arg1, u64 arg2, u64 arg3, u64 arg4, u64 arg5)
{
    int pid;
    char buf1[20];
    char *filename="test.txt";
    int create_fd = open(filename, O_CREAT|O_RDWR, O_READ|O_WRITE);
    char *buf = "Hello, I am file!";
    printf("%d\n", write(create_fd, buf, 17));

    int read_fd = open(filename, O_RDONLY);
    printf("%d\n", read(read_fd, buf1, 17));
    buf1[17] = '\0';
    printf("%s\n",buf1);
    close(read_fd);
    close(create_fd);
    return 0;
}