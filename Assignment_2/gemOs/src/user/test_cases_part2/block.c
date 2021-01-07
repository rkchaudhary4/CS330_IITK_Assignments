
#include <ulib.h>
int main(u64 arg1, u64 arg2, u64 arg3, u64 arg4, u64 arg5)
{
	int fd, pid, st;
	struct msg_queue_member_info info;
	struct message msg;

	// parent creates message queue
	fd = create_msg_queue();

	pid = fork();
	if (pid == 0)
	{
		// wait for parents msg
		while (get_msg_count(fd) == 0)
			;

		// read message from parent
		st = msg_queue_rcv(fd, &msg);

		// send a ACK to parent
		// this call must fail
		msg.to_pid = msg.from_pid; //get parents pid
		msg.from_pid = getpid();
		msg.msg_txt[0] = 'A';
		msg.msg_txt[1] = 'C';
		msg.msg_txt[2] = 'K';
		msg.msg_txt[3] = '\0';
		st = msg_queue_send(fd, &msg);

		// the above call must fail
		if (st == -EINVAL)
			printf("Child blocked by parent\n");
		else
			printf("Child not blocked by parent\n");
	}
	else if (pid > 0)
	{

		// block the child
		st = msg_queue_block(fd, pid);

		// send a message to child
		// so that child can continue
		msg.from_pid = getpid();
		msg.to_pid = pid;
		msg.msg_txt[0] = 'M';
		msg.msg_txt[1] = 'S';
		msg.msg_txt[2] = 'G';
		msg.msg_txt[3] = '\0';

		// send message to child
		st = msg_queue_send(fd, &msg);
		// sleep till the child checks if it is blocked
		sleep(20);
	}
	else
	{
		printf("fork error\n");
	}
	return 0;
}
