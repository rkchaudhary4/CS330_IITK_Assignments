
#include<ulib.h>
int main(u64 arg1, u64 arg2, u64 arg3, u64 arg4, u64 arg5)
{
	int fd, pid, st;	 
	struct msg_queue_member_info info;
	struct message msg;
	
	// parent creates message queue
	fd = create_msg_queue();
	
	st = get_member_info(fd, &info);
	printf("Members before fork: %d\n", info.member_count);	
	pid = fork();
	if(pid == 0){
		while(get_msg_count(fd) == 0);
		st = get_member_info(fd, &info);
		
		// read message from parent
		st = msg_queue_rcv(fd, &msg);
		
		// send a ACK to child
		msg.to_pid = msg.from_pid; //get parents pid
		msg.from_pid = getpid();	
		msg.msg_txt[0] = 'A';
		msg.msg_txt[1] = 'C';
		msg.msg_txt[2] = 'K';
		msg.msg_txt[3] = '\0';
		st = msg_queue_send(fd, &msg);
	}
	else if(pid > 0){
		// child must be waiting for msg from parent
		st = get_member_info(fd, &info);
		printf("Members after fork: %d\n", info.member_count);	
		
		// send a message to child
		msg.from_pid = getpid();	
		msg.to_pid = pid;
		msg.msg_txt[0] = 'M';
		msg.msg_txt[1] = 'S';
		msg.msg_txt[2] = 'G';
		msg.msg_txt[3] = '\0';
		
		// send message to child
		st = msg_queue_send(fd, &msg);

		// wait for childs ACK
		while(get_msg_count(fd) == 0);
		st = msg_queue_rcv(fd, &msg);
	}
	else{
		printf("fork error\n");
	}
	return 0;
}
