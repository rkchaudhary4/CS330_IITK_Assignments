
#include<ulib.h>

void do_child_stuff(int fd)
{
	int st;
	struct message msg;
	
	// wait for parent to send msg
	while(get_msg_count(fd) == 0);
	
	// read message from parent
	st = msg_queue_rcv(fd, &msg);
	
	// send a ACK to parent
	msg.to_pid = msg.from_pid; //get parents pid
	msg.from_pid = getpid();
	msg.msg_txt[0] = 'A';
	msg.msg_txt[1] = 'C';
	msg.msg_txt[2] = 'K';
	msg.msg_txt[3] = '\0';
	st = msg_queue_send(fd, &msg);
}

int main(u64 arg1, u64 arg2, u64 arg3, u64 arg4, u64 arg5)
{
	// test broadcast
	int fd, pid, ch, cnt, st, ok;	 
	struct message msg;
	
	// parent creates message queue
	fd = create_msg_queue();
	
	// get_member_info(fd, &info);
	// printf("Member count in parent before fork:%d\n", info.member_count);
	
	for(ch = 0; ch < 2; ++ch){
		pid = fork();
		if(pid == 0){
			do_child_stuff(fd);
			break;
		}
		else if(pid > 0){
		}
		else{
			printf("Error in fork\n");
		}
	}

	if(ch == 2){

		// only parent reaches here
		// send a broadcast message to children 
		msg.from_pid = getpid();
		msg.to_pid = BROADCAST_PID;
		msg.msg_txt[0] = 'M';
		msg.msg_txt[1] = 'S';
		msg.msg_txt[2] = 'G';
		msg.msg_txt[3] = '\0';
			
		st = msg_queue_send(fd, &msg);
		if(st == 2){
			// now parent must recieve ACK from children
			cnt = 0;	
			for(ch = 0; ch < 2; ++ch){
				while(get_msg_count(fd) == 0);
				st = msg_queue_rcv(fd, &msg);
				if(st > 0)
					++cnt;
			}
			if(cnt == 2)
				printf("Broadcast OK\n");
			else
				printf("Broadcast FAILED\n");
		}
		else{
			printf("Broadcast FAILED\n");
		}
	}
	return 0;
}
