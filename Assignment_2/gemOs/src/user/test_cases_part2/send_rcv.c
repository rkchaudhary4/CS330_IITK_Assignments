
// tests create_msg_queue
// msg_queue_snd
// msg_queue_rcv

#include<ulib.h>
int main(u64 arg1, u64 arg2, u64 arg3, u64 arg4, u64 arg5)
{
	int fd, pid, cnt;	 
	struct msg_queue_member_info info;
	struct message msg;
	
	// parent creates message queue
	fd = create_msg_queue();
	
	pid = fork();
	if(pid == 0){
		// child must wait for the parent to send a message to it
		while(get_msg_count(fd) == 0);
		
		// read message from parent
		cnt = msg_queue_rcv(fd, &msg);
		printf("Child recieved message: %s\n", msg.msg_txt);
		
		// send a ACK to paren 
		msg.to_pid = msg.from_pid; //get parents pid
		msg.from_pid = getpid();	
		msg.msg_txt[0] = 'A';
		msg.msg_txt[1] = 'C';
		msg.msg_txt[2] = 'K';
		msg.msg_txt[3] = '\0';
		cnt = msg_queue_send(fd, &msg);
	}
	else if(pid > 0){
		
		// send a message to child
		msg.from_pid = getpid();	
		msg.to_pid = pid;
		msg.msg_txt[0] = 'M';
		msg.msg_txt[1] = 'S';
		msg.msg_txt[2] = 'G';
		msg.msg_txt[3] = '\0';
		
		// send message to child
		cnt = msg_queue_send(fd, &msg);

		// wait for childs ACK
		while(get_msg_count(fd) == 0);
		cnt = msg_queue_rcv(fd, &msg);
		printf("Parent got ACK: %s\n", msg.msg_txt);

	}
	else{
		printf("fork error\n");
	}
	return 0;
}
