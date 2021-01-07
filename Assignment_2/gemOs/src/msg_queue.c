#include <msg_queue.h>
#include <context.h>
#include <memory.h>
#include <file.h>
#include <lib.h>
#include <entry.h>

/************************************************************************************/
/***************************Do Not Modify below Functions****************************/
/************************************************************************************/

struct msg_queue_info *alloc_msg_queue_info()
{
	struct msg_queue_info *info;
	info = (struct msg_queue_info *)os_page_alloc(OS_DS_REG);

	if (!info)
	{
		return NULL;
	}
	return info;
}

void free_msg_queue_info(struct msg_queue_info *q)
{
	os_page_free(OS_DS_REG, q);
}

struct message *alloc_buffer()
{
	struct message *buff;
	buff = (struct message *)os_page_alloc(OS_DS_REG);
	if (!buff)
		return NULL;
	return buff;
}

void free_msg_queue_buffer(struct message *b)
{
	os_page_free(OS_DS_REG, b);
}

/**********************************************************************************/
/**********************************************************************************/
/**********************************************************************************/
/**********************************************************************************/

int checkDummy(struct message m)
{
	if (m.from_pid == 0 && m.to_pid == 0)
		return 1;
	return 0;
}

int do_create_msg_queue(struct exec_context *ctx)
{
	/** 
	 * TODO Implement functionality to
	 * create a message queue
	 **/
	int fd = 3;
	while (ctx->files[fd])
		fd++;
	struct file *file = alloc_file();
	if (file == NULL)
		return -ENOMEM;
	file->fops = NULL;
	file->type = MSG_QUEUE;
	file->msg_queue = alloc_msg_queue_info();
	file->msg_queue->msg_buffer = alloc_buffer();
	for (int i = 0; i < MAX_MEMBERS; i++)
	{
		file->msg_queue->pids[i] = 0;
		for (int j = 0; j < MAX_MEMBERS; j++)
		{
			file->msg_queue->blocked[i][j] = 0;
		}
	}
	file->msg_queue->pids[0] = ctx->pid;
	struct message dummy;
	dummy.from_pid = 0;
	dummy.to_pid = 0;
	for (int i = 0; i < 128; i++)
	{
		file->msg_queue->read[i] = 0;
		file->msg_queue->msg_buffer[i] = dummy;
	}
	if (file->msg_queue == NULL)
		return -ENOMEM;
	file->pipe = NULL;
	ctx->files[fd] = file;
	return fd;
}

int do_msg_queue_rcv(struct exec_context *ctx, struct file *filep, struct message *msg)
{
	/** 
	 * TODO Implement functionality to
	 * recieve a message
	 **/
	if (filep->type != MSG_QUEUE)
		return -EINVAL;
	for (int i = 0; i < 128; i++)
	{
		if (checkDummy(filep->msg_queue->msg_buffer[i]))
			break;
		if ((filep->msg_queue->msg_buffer[i].to_pid == ctx->pid) && filep->msg_queue->read[i] == 0)
		{
			msg->from_pid = filep->msg_queue->msg_buffer[i].from_pid;
			msg->to_pid = filep->msg_queue->msg_buffer[i].to_pid;
			for (int j = 0; j < MAX_TXT_SIZE; j++)
			{
				msg->msg_txt[j] = filep->msg_queue->msg_buffer[i].msg_txt[j];
			}
			filep->msg_queue->read[i] == 1;
			return 1;
		}
	}
	return 0;
}

int do_msg_queue_send(struct exec_context *ctx, struct file *filep, struct message *msg)
{
	/** 
	 * TODO Implement functionality to
	 * send a message
	 **/
	if (filep->type != MSG_QUEUE)
		return -EINVAL;
	int free = 0;
	while (!checkDummy(filep->msg_queue->msg_buffer[free]))
		free++;
	int sent = 0;
	int sender_pid;
	for (int i = 0; i < MAX_MEMBERS; i++)
	{
		if (filep->msg_queue->pids[i] == msg->from_pid)
		{
			sender_pid = i;
			break;
		}
	}
	if (msg->to_pid == BROADCAST_PID)
	{
		for (int i = 0; i < MAX_MEMBERS; i++)
		{
			if (!filep->msg_queue->pids[i])
				break;
			if (filep->msg_queue->pids[i] != msg->from_pid)
			{
				if (filep->msg_queue->blocked[i][sender_pid])
					continue;
				struct message n;
				n.from_pid = msg->from_pid;
				n.to_pid = filep->msg_queue->pids[i];
				for (int j = 0; j < MAX_TXT_SIZE; j++)
				{
					n.msg_txt[j] = msg->msg_txt[j];
				}
				filep->msg_queue->msg_buffer[free] = n;
				free++;
				sent++;
			}
		}
	}
	else
	{
		int found = 0;
		int recevier;
		for (int i = 0; i < MAX_MEMBERS; i++)
		{
			if (filep->msg_queue->pids[i] == msg->to_pid)
			{
				found = 1;
				recevier = i;
				break;
			}
		}
		if (!found)
			return -EINVAL;
		if (filep->msg_queue->blocked[recevier][sender_pid])
			return -EINVAL;
		struct message n;
		n.from_pid = msg->from_pid;
		n.to_pid = msg->to_pid;
		for (int j = 0; j < MAX_TXT_SIZE; j++)
		{
			n.msg_txt[j] = msg->msg_txt[j];
		}
		filep->msg_queue->msg_buffer[free] = n;
		free++;
		sent++;
	}
	return sent;
}

void do_add_child_to_msg_queue(struct exec_context *child_ctx)
{
	/** 
	 * TODO Implementation of fork handler 
	 **/
	for (int i = 3; i < MAX_OPEN_FILES; i++)
	{
		if (child_ctx->files[i])
		{
			if (child_ctx->files[i]->type == MSG_QUEUE)
			{
				for (int j = 0; j < MAX_MEMBERS; j++)
				{
					if (!child_ctx->files[i]->msg_queue->pids[j])
					{
						child_ctx->files[i]->msg_queue->pids[j] = child_ctx->pid;
						break;
					}
				}
			}
		}
		else
			break;
	}
}

void do_msg_queue_cleanup(struct exec_context *ctx)
{
	/** 
	 * TODO Implementation of exit handler 
	 **/
	for (int i = 3; i < MAX_OPEN_FILES; i++)
	{
		if (!ctx->files[i])
			break;
		if (ctx->files[i]->type == MSG_QUEUE)
		{
			for (int j = 0; j < MAX_MEMBERS; j++)
			{
				if (ctx->files[i]->msg_queue->pids[j] == ctx->pid)
				{
					do_msg_queue_close(ctx, i);
					break;
				}
			}
		}
	}
}

int do_msg_queue_get_member_info(struct exec_context *ctx, struct file *filep, struct msg_queue_member_info *info)
{
	/** 
	 * TODO Implementation of exit handler 
	 **/
	if (filep->type != MSG_QUEUE)
		return -EINVAL;
	info->member_count = 0;
	for (int i = 0; i < MAX_MEMBERS; i++)
	{
		if (filep->msg_queue->pids[i])
		{
			info->member_count++;
			info->member_pid[info->member_count - 1] = filep->msg_queue->pids[i];
		}
	}
	return 0;
}

int do_get_msg_count(struct exec_context *ctx, struct file *filep)
{
	/** 
	 * TODO Implement functionality to
	 * return pending message count to calling process
	 **/
	if (filep->type != MSG_QUEUE)
		return -EINVAL;
	int pend = 0;
	for (int i = 0; i < 128; i++)
	{
		if (checkDummy(filep->msg_queue->msg_buffer[i]))
			break;
		if ((filep->msg_queue->msg_buffer[i].to_pid == ctx->pid) && (!filep->msg_queue->read[i]))
			pend++;
	}
	return pend;
}

int do_msg_queue_block(struct exec_context *ctx, struct file *filep, int pid)
{
	/** 
	 * TODO Implement functionality to
	 * block messages from another process 
	 **/
	if (filep->type != MSG_QUEUE)
		return -EINVAL;
	int receiver = -1, sender = -1;
	for (int i = 0; i < MAX_MEMBERS; i++)
	{
		if (filep->msg_queue->pids[i] == ctx->pid)
			receiver = i;
		if (filep->msg_queue->pids[i] == pid)
			sender = i;
	}
	if ((receiver == -1) || (sender == -1))
		return -EINVAL;
	filep->msg_queue->blocked[receiver][sender] = 1;
	return 0;
}

int do_msg_queue_close(struct exec_context *ctx, int fd)
{
	/** 
	 * TODO Implement functionality to
	 * remove the calling process from the message queue 
	 **/
	if (ctx->files[fd]->type != MSG_QUEUE)
		return -EINVAL;
	int indice = -1;
	struct msg_queue_info *msg = ctx->files[fd]->msg_queue;
	for (int i = 0; i < MAX_MEMBERS; i++)
	{
		if (msg->pids[i] == ctx->pid)
		{
			indice = 1;
			break;
		}
	}
	if (indice == 0)
	{
		free_msg_queue_buffer(msg->msg_buffer);
		free_msg_queue_info(msg);
		return 0;
	}
	for (int i = indice; i < MAX_MEMBERS - 1; i++)
	{
		msg->pids[i] = msg->pids[i + 1];
	}
	msg->pids[MAX_MEMBERS - 1] = 0;
	return 0;
}
