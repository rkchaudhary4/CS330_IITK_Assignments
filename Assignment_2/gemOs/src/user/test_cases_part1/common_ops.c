#include <ulib.h>

int main(u64 arg1, u64 arg2, u64 arg3, u64 arg4, u64 arg5)
{
        int pid = 0;
        int fd = 0;
        int n = 0;
        char buf1[20];
        char *filename = "testFile";
        char *buf = "#include<stdio.h>";

        //open in write only mode. Create if file doesn't exist already.
        //Expected output: 3
        fd = open(filename, O_CREAT | O_WRONLY, O_READ | O_WRITE);
        printf("%d\n", fd);

        //perform write operation
        //Expected output: 17
        printf("%d\n", write(fd, buf, 17));

        //perform read operation
        //Expected output: -4
        printf("%d\n", read(fd, buf1, 17));

        //perform close operation
        //Expected output: 0
        printf("%d\n", close(fd));
        //Expected output: -1
        printf("%d\n", close(fd));

        //open in read only mode.
        //Expected output: 3
        fd = open(filename, O_RDONLY);
        printf("%d\n", fd);

        //perform read operation
        //Expected output:17
        printf("%d\n", read(fd, buf1, 17));
        buf1[17] = '\0';
        //Expected output: #include<stdio.h>
        printf("%s\n", buf1);

        //perform write operation
        //Expected output: -4
        printf("%d\n", write(fd, buf, 17));

        //Close file
        //Expected output: 0
        printf("%d\n", close(fd));
        close(fd);
        //Expected output: -1
        printf("%d\n", close(fd));
        close(fd);

        return 0;
}
