#include <types.h>
#include <context.h>
#include <file.h>
#include <lib.h>
#include <serial.h>
#include <entry.h>
#include <memory.h>
#include <fs.h>
#include <kbd.h>

/************************************************************************************/
/***************************Do Not Modify below Functions****************************/
/************************************************************************************/

void free_file_object(struct file *filep)
{
	if (filep)
	{
		os_page_free(OS_DS_REG, filep);
		stats->file_objects--;
	}
}

struct file *alloc_file()
{
	struct file *file = (struct file *)os_page_alloc(OS_DS_REG);
	file->fops = (struct fileops *)(file + sizeof(struct file));
	bzero((char *)file->fops, sizeof(struct fileops));
	file->ref_count = 1;
	file->offp = 0;
	stats->file_objects++;
	return file;
}

void *alloc_memory_buffer()
{
	return os_page_alloc(OS_DS_REG);
}

void free_memory_buffer(void *ptr)
{
	os_page_free(OS_DS_REG, ptr);
}

/* STDIN,STDOUT and STDERR Handlers */

/* read call corresponding to stdin */

static int do_read_kbd(struct file *filep, char *buff, u32 count)
{
	kbd_read(buff);
	return 1;
}

/* write call corresponding to stdout */

static int do_write_console(struct file *filep, char *buff, u32 count)
{
	struct exec_context *current = get_current_ctx();
	return do_write(current, (u64)buff, (u64)count);
}

long std_close(struct file *filep)
{
	filep->ref_count--;
	if (!filep->ref_count)
		free_file_object(filep);
	return 0;
}
struct file *create_standard_IO(int type)
{
	struct file *filep = alloc_file();
	filep->type = type;
	if (type == STDIN)
		filep->mode = O_READ;
	else
		filep->mode = O_WRITE;
	if (type == STDIN)
	{
		filep->fops->read = do_read_kbd;
	}
	else
	{
		filep->fops->write = do_write_console;
	}
	filep->fops->close = std_close;
	return filep;
}

int open_standard_IO(struct exec_context *ctx, int type)
{
	int fd = type;
	struct file *filep = ctx->files[type];
	if (!filep)
	{
		filep = create_standard_IO(type);
	}
	else
	{
		filep->ref_count++;
		fd = 3;
		while (ctx->files[fd])
			fd++;
	}
	ctx->files[fd] = filep;
	return fd;
}
/**********************************************************************************/
/**********************************************************************************/
/**********************************************************************************/
/**********************************************************************************/

/* File exit handler */
void do_file_exit(struct exec_context *ctx)
{
	/*TODO the process is exiting. Adjust the refcount
	of files*/
	for (int i = 0; i < MAX_OPEN_FILES; i++)
	{
		if (ctx->files[i] != NULL)
		{
			do_file_close(ctx->files[i]);
		}
	}
}

/*Regular file handlers to be written as part of the assignmemnt*/

static int do_read_regular(struct file *filep, char *buff, u32 count)
{
	/** 
	*  TODO Implementation of File Read, 
	*  You should be reading the content from File using file system read function call and fill the buf
	*  Validate the permission, file existence, Max length etc
	*  Incase of Error return valid Error code 
	**/
	if (filep == NULL)
		return -EINVAL;
	if ((filep->mode & O_READ) != O_READ)
		return -EACCES;
	if (filep->inode->s_pos + filep->offp + count > filep->inode->e_pos)
		return -EINVAL;

	int ret = flat_read(filep->inode, buff, count, &filep->offp);
	if (ret > 0)
	{
		filep->offp += ret;
	}
	return ret;
}

/*write call corresponding to regular file */

static int do_write_regular(struct file *filep, char *buff, u32 count)
{
	/** 
	*   TODO Implementation of File write, 
	*   You should be writing the content from buff to File by using File system write function
	*   Validate the permission, file existence, Max length etc
	*   Incase of Error return valid Error code 
	* */
	if (filep == NULL)
		return -EINVAL;
	if ((filep->mode & O_WRITE) != O_WRITE)
		return -EACCES;
	if (filep->inode->s_pos + filep->offp + count > filep->inode->e_pos)
		return -EINVAL;
	int ret = flat_write(filep->inode, buff, count, &filep->offp);
	if (ret > 0)
	{
		filep->offp += ret;
	}
	return ret;
}

long do_file_close(struct file *filep)
{
	/** TODO Implementation of file close  
	*   Adjust the ref_count, free file object if needed
	*   Incase of Error return valid Error code 
	*/
	if (filep == NULL)
		return -1;
	filep->ref_count--;
	if (filep->ref_count == 0)
	{
		free_file_object(filep);
	}
	return 0;
}

static long do_lseek_regular(struct file *filep, long offset, int whence)
{
	/** 
	*   TODO Implementation of lseek 
	*   Set, Adjust the ofset based on the whence
	*   Incase of Error return valid Error code 
	* */
	if (filep == NULL)
		return -EINVAL;
	if (whence == SEEK_SET)
	{
		if (filep->inode->s_pos + offset > filep->inode->e_pos)
			return -EINVAL;
		filep->offp = offset;
	}
	else if (whence == SEEK_CUR)
	{
		if (filep->inode->s_pos + filep->offp + offset > filep->inode->e_pos)
			return -EINVAL;
		filep->offp += offset;
	}
	else if (whence == SEEK_END)
	{
		if (filep->inode->max_pos + offset > filep->inode->e_pos)
			return -EINVAL;
		filep->offp = filep->inode->max_pos + offset;
	}
	return filep->offp;
}

extern int do_regular_file_open(struct exec_context *ctx, char *filename, u64 flags, u64 mode)
{

	/**  
	*  TODO Implementation of file open, 
	*  You should be creating file(use the alloc_file function to creat file), 
	*  To create or Get inode use File system function calls, 
	*  Handle mode and flags 
	*  Validate file existence, Max File count is 16, Max Size is 4KB, etc
	*  Incase of Error return valid Error code 
	* */
	struct inode *file;
	file = lookup_inode(filename);
	if (file == NULL)
	{
		if ((flags & O_CREAT) == O_CREAT)
		{
			file = create_inode(filename, mode);
			if (file == NULL)
			{
				return -ENOMEM;
			}
		}
		else
		{
			return -EINVAL;
		}
	}
	struct file *fp = alloc_file();
	if (fp == NULL)
		return -ENOMEM;
	if (((flags & O_READ) == O_READ) && ((file->mode & O_READ) != O_READ))
		return -EACCES;
	if (((flags & O_WRITE) == O_WRITE) && ((file->mode & O_WRITE) != O_WRITE))
		return -EACCES;
	if (((flags & O_RDWR) == O_RDWR) && ((file->mode & O_RDWR) != O_RDWR))
		return -EACCES;
	int free = 3;
	while (ctx->files[free])
		free++;
	fp->pipe = NULL;
	fp->type = REGULAR;
	fp->offp = 0;
	fp->mode = flags;
	fp->inode = file;
	fp->fops->read = do_read_regular;
	fp->fops->write = do_write_regular;
	fp->fops->lseek = do_lseek_regular;
	fp->fops->close = do_file_close;
	fp->ref_count = 1;
	fp->msg_queue = NULL;
	ctx->files[free] = fp;
	return free;
}

/**
 * Implementation dup 2 system call;
 */
int fd_dup2(struct exec_context *current, int oldfd, int newfd)
{
	/** 
	*  TODO Implementation of the dup2 
	*  Incase of Error return valid Error code 
	**/
	if (oldfd < 0 || oldfd >= MAX_OPEN_FILES)
		return -EINVAL;
	if (newfd < 0 || newfd >= MAX_OPEN_FILES)
		return -EINVAL;
	struct file *f = current->files[oldfd];
	if (f == NULL)
		return -EINVAL;
	current->files[newfd] = NULL;
	current->files[newfd] = f;
	return newfd;
}

int do_sendfile(struct exec_context *ctx, int outfd, int infd, long *offset, int count)
{
	/** 
	*  TODO Implementation of the sendfile 
	*  Incase of Error return valid Error code 
	**/
	if (ctx->files[outfd] == NULL || ctx->files[infd] == NULL)
		return -EINVAL;
	struct file *out = ctx->files[outfd];
	struct file *in = ctx->files[infd];
	if ((in->mode & O_READ )!= O_READ)
		return -EACCES;
	if ((out->mode & O_WRITE) != O_WRITE)
		return -EACCES;
	if (offset != NULL)
	{
		int curr = in->offp;
		in->offp = *offset;
		char *buff = alloc_memory_buffer();
		int ret = do_read_regular(in, buff, count);
		if (ret < 0)
			return ret;
		*offset = in->offp;
		in->offp = curr;
		int wret = do_write_regular(out, buff, ret);
		free_memory_buffer(buff);
		return wret;
	}
	else
	{
		char *buff = alloc_memory_buffer();
		int ret = do_read_regular(in, buff, count);
		if (ret < 0)
			return ret;
		int wret = do_write_regular(out, buff, ret);
		free_memory_buffer(buff);
		return wret;
	}
	return -EINVAL;
}
