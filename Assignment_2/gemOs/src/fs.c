#include<lib.h>
#include<types.h>
#include<fs.h>
#include<file.h>
#include<memory.h>
#include<context.h>

struct super_block* super_block; 

u64 get_contigous_pages(u32 region, int number_of_pages)
{
	int i;
	u32 startpfn, prevpfn, currentpfn;
	u64 first_page = os_pfn_alloc(region);
	prevpfn = first_page;

	for(i =1; i < number_of_pages; i++)
	{
		currentpfn = os_pfn_alloc(region);
		if(currentpfn != prevpfn + 1)
			dprintk("Error in get contiguous region %s\n", __func__);
		prevpfn = currentpfn;
	}
	first_page = first_page  << PAGE_SHIFT;
	bzero((char*)first_page, (PAGE_SIZE * number_of_pages));
	return first_page;
}

void init_file_inode(struct super_block * super_block)
{
	int index;
	u32 total_inode_page = ((sizeof(struct inode) * NUM_FILES)/PAGE_SIZE) + 3;
	u64 inode_pfn = get_contigous_pages(FILE_DS_REG, total_inode_page);

	for(index = 0; index < NUM_FILES; ++index)
	{
		super_block->inode[index] = (struct inode *) inode_pfn;
		inode_pfn = inode_pfn + sizeof(struct inode);
	}
	super_block->sb_op = (struct sb_operations*)os_page_alloc(FILE_DS_REG);
}
/*
 * start_reg is Starting address of the first usable page of FILE_STORE_REG 
 * file_size is maximum size of the file [in BYTES] we can allocate But for
 * simplicity we rounding the file_size to a PAGE_SIZE*num_pages;
 * Allocation of pages is taking too much time
 */
void alloc_inodes(struct super_block *sb)
{
	u32 i, num_pages;
	u64 start_reg, file_size, file_pfn;
	start_reg =  get_contigous_pages(FILE_STORE_REG, 1);
	file_size = (END - start_reg) / NUM_FILES;       
	num_pages = FILE_SIZE / PAGE_SIZE ;
	file_size = num_pages * PAGE_SIZE;
	for( i=0; i<NUM_FILES; i++)
	{
		struct inode *inode = sb->inode[i];
		if( i == 0 )
		{	// We allocated already 1 page to get start_reg
			file_pfn = get_contigous_pages(FILE_STORE_REG, num_pages-1);
			inode->s_pos = start_reg;
			inode->e_pos = start_reg + file_size;
		}
		else
		{
			file_pfn = get_contigous_pages(FILE_STORE_REG, num_pages);
                        inode->s_pos = file_pfn;
                        inode->e_pos = file_pfn + file_size;

		}
		inode->is_valid = 0;
                inode->filename[0] = '\0';
                inode->mode = 0;
		inode->type = REGULAR;

                inode->read = flat_read;
                inode->write = flat_write;
                inode->open = flat_open;
		inode->close = flat_close;

		inode->inode_no = i;
                inode->max_pos = inode->s_pos;
                inode->file_size = 0;
                inode->ref_count = 0;
                inode->sb = sb;
	}
}

void init_file_system()
{
	//struct super_block * super_block;
	super_block = (struct super_block*)os_page_alloc(FILE_DS_REG);

	init_file_inode(super_block);
	alloc_inodes(super_block);

	super_block->fs_name = "flat";
	super_block->num_files = 0;
	super_block->sb_op->remove_inode = flat_remove_inode;
	super_block->sb_op->create_inode = flat_create_inode;
	super_block->sb_op->lookup_inode = flat_lookup_inode;
	super_block->sb_op->get_inode_no = flat_get_inode_no;
	super_block->sb_op->get_num_files = flat_get_num_files;
	super_block->sb_op->list_all_files = flat_list_all_files;

#if 0 
	dprintk("Testing File System Functions......\n");    
	char *file_name = "siva";
	u32 inode_no;
	int i, num_files;
	struct inode *inode;
	for(i=0; i<=NUM_FILES; i++)
	{
	file_name[2] = 'a'+i;
	file_name[3] = '\0';
	super_block->sb_op->create_inode(super_block, file_name, 0x6);
	inode = super_block->sb_op->lookup_inode(super_block, file_name);	
	inode_no = super_block->sb_op->get_inode_no(super_block, file_name);
	num_files = super_block->sb_op->get_num_files(super_block);
	dprintk(" inode_no %x filename %s \t num_files %x\n", inode_no, inode->filename, num_files);

	}
	char *file = "sia", *buf;
	int *offp=0, count = 4096, ret;
	buf = get_contigous_pages(FILE_STORE_REG, 4);
	inode = super_block->sb_op->lookup_inode(super_block, file);
	if( inode != NULL )
	{

	inode->open(inode);
	inode->read(inode, buf, count, offp);
	for(i=0; i<count; i++)
	{
	// dprintk("%c", buf[i]);
	buf[i] = 'a';
	}
	count = 50;
	for(i=0; i<50; i++)
	{
	ret = inode->write(inode, buf+*offp, count, offp);
	if(ret == -1)
	continue;
	*offp += ret;
	dprintk("offp %d ret %d \t %d\n",*offp, ret, i);
	}
	count = 4096;
	for(i=0; i<count; i++)
	{
	buf[i] = 'b';
	}
	*offp = 0;
	count = 70;
	for(i=0; i<50; i++)
	{
	ret = inode->read(inode, buf+*offp, count, offp);
	if( ret == -1 )
	continue;
	*offp += ret;
	dprintk("offp %d ret %d \t %d\n",*offp, ret, i);
	}
	count = 4096;
	for(i=0; i<2501; i++)
	{
	dprintk("%c", buf[i]);
	}


	}

	for(i = NUM_FILES-1; i>=0 ; i--)
	{
	file_name[2] = 'a'+i;
	file_name[3] = '\0';
	dprintk("i%x \n", i); 
	inode_no = super_block->sb_op->get_inode_no(super_block, file_name);
	inode = super_block->sb_op->lookup_inode(super_block, file_name);
	super_block->sb_op->remove_inode(super_block, inode);
	inode = super_block->sb_op->lookup_inode(super_block, file_name);
	inode_no = super_block->sb_op->get_inode_no(super_block, file_name);
	num_files = super_block->sb_op->get_num_files(super_block);

	if( inode != NULL )
	dprintk(" %x  inode_no %x filename %s num_files \t \n", i, inode_no, inode->filename, num_files);

	}

	dprintk("Testing Complete ....\n");   
#endif
}

struct super_block * get_superblock(){
        return super_block;
}

static int is_valid_inode( struct inode *inode )
{
        return inode->is_valid;
}


struct inode* flat_lookup_inode(struct super_block *sb, char *filename)
{
	u32 i;
        for( i=0; i< NUM_FILES; i++)
        {
                struct inode *inode = sb->inode[i];
  	   	if( is_valid_inode(inode) && strcmp(inode->filename, filename) == 0 )
                        return inode;
        }
        return NULL;
}

int flat_get_inode_no( struct super_block *sb, char *name)
{
	u32 i;
        for( i=0; i< NUM_FILES; i++)
        {
                struct inode *inode = sb->inode[i];
                if( is_valid_inode(inode) && strcmp(inode->filename, name) == 0 )
                        return i;
        }
        return -1;
}

static int get_free_inode( struct super_block *sb)
{
        u32 i;
        for(i=0; i<NUM_FILES; i++)
        {
                if( !is_valid_inode(sb->inode[i]) )
                {
                        return i;
                }
        }
        return -1;
}


int flat_create_inode(struct super_block *sb, char *file_name, u32 mode)
{
   	u32 inode_no, i=0;
        inode_no = get_free_inode(sb);
	// check if the filename is exits or not [ Upper layers ]
	if( sb->sb_op->lookup_inode(sb, file_name) != NULL )
		return -1;

        if( inode_no >= 0 && inode_no < NUM_FILES )
        {
                struct inode *inode = sb->inode[inode_no];
                
		inode->is_valid = 1;
                inode->mode = mode;
		// file_name length should not be 0, check before file is there are not
                while( file_name[i] != '\0' )
                {
                        inode->filename[i] = file_name[i];
                        i++;
                }
		inode->filename[i] = '\0';

                inode->max_pos = inode->s_pos;
                inode->file_size = 0;
                inode->ref_count = 0;
                
		sb->num_files += 1;
        }
	else
		return -1;
        return inode_no;
}

int flat_remove_inode(struct super_block *sb, struct inode *inode)
{
	if( !inode && inode->ref_count != 0 )
		return -1;
	// file->ref_count should be 0 [ Make sure Upper layers ] 
        inode->is_valid = 0;
        inode->filename[0] ='\0';
        inode->max_pos = inode->s_pos;
        inode->file_size = 0;
	inode->mode = 0;

        sb->num_files -= 1;

        return 0;
}

int flat_get_num_files( struct super_block *sb)
{
	return sb->num_files;
}

void flat_list_all_files(struct super_block *sb, void *buf)
{
	

}
int flat_read(struct inode *inode, char *buf, int count, int *offp)
{
	// *offp is  start byte to read from the inode->s_pos
	/*
	 * s_pos is inclusive It will read data from s_pos
	 * max_pos is exclusive means next byte will be written in max_pos
	 * e_pos is exclusive meeans max_pos can go upto e_pos
	 * file_size is max_pos - s_pos ; 
	 */

	u64 i, size = 0, s_pos;
    	
        long int remain_len;
	remain_len = (long int)inode->file_size - *offp;
	if( remain_len <= 0 )
		return 0;
	size = ( count > remain_len ? remain_len : count );
        s_pos = inode->s_pos;
	for( i=0 ; i<size ; i++)
	{
		buf[i] = *(char *)(s_pos+ *offp + i);
	}

	return size;
}

int flat_write(struct inode *inode, char *buf, int count, int *offp)
{
	u64 i, size, s_pos;

	long int max_len;
        max_len = (long int)inode->e_pos - *offp - (long int)inode->s_pos;

	if( count > max_len )
	{
		return -1; // file_size exceeded
	}
        s_pos = inode->s_pos;
        for( i=0 ; i<count ; i++)
        {
		*(char *)(s_pos + *offp + i) = buf[i];
        }

	if( inode->s_pos + *offp + count > inode->max_pos )
		inode->max_pos = inode->s_pos + *offp + count;
	inode->file_size = inode->max_pos - inode->s_pos;
        return count;
}

static int get_inode(struct inode *inode)
{
         inode->ref_count ++;
         return inode->ref_count;
}

static int put_inode(struct inode *inode)
{
         inode->ref_count --;
         return inode->ref_count;
}
int flat_open(struct inode *inode)
{
	get_inode(inode);
	return 0;
}

int flat_close(struct inode *inode)
{
        put_inode(inode);
	return 0;
}

struct inode* lookup_inode(char *filename)
{
	return super_block->sb_op->lookup_inode(super_block, filename); 
}

struct inode *create_inode (char *filename, u64 mode)
{
	int inode_num = super_block->sb_op->create_inode(super_block, filename, mode);
	if(inode_num < 0)
		return NULL; 
	return super_block->inode[inode_num]; 
}
