#include <types.h>
#include <mmap.h>
#include <page.h>

// Helper function to create a new vm_area
struct vm_area *create_vm_area(u64 start_addr, u64 end_addr, u32 flags, u32 mapping_type)
{
	struct vm_area *new_vm_area = alloc_vm_area();
	new_vm_area->vm_start = start_addr;
	new_vm_area->vm_end = end_addr;
	new_vm_area->access_flags = flags;
	new_vm_area->mapping_type = mapping_type;
	return new_vm_area;
}

/**
 * Function will invoked whenever there is page fault. (Lazy allocation)
 * 
 * For valid access. Map the physical page 
 * Return 0
 * 
 * For invalid access, i.e Access which is not matching the vm_area access rights (Writing on ReadOnly pages)
 * return -1. 
 */

void merger(struct vm_area *head)
{
	while (head->vm_next != NULL)
	{
		if ((head->vm_next->vm_start == head->vm_end) && (head->mapping_type == head->vm_next->mapping_type) && (head->access_flags == head->vm_next->access_flags))
		{
			head->vm_end = head->vm_next->vm_end;
			struct vm_area *toDelete = head->vm_next;
			head->vm_next = head->vm_next->vm_next;
			dealloc_vm_area(toDelete);
		}
		else
		{
			head = head->vm_next;
		}
	}
}

u64 *map_physical_area(struct exec_context *ctx, u64 addr, u32 flags, u32 page_mapping)
{
	u64 *base = (u64 *)osmap(ctx->pgd);
	u64 *entry;
	u64 pfn;
	u64 ac_flags = 0x5 | (flags & 0x2);

	entry = base + ((addr & PGD_MASK) >> PGD_SHIFT);
	if (*entry & 0x1)
	{
		pfn = (*entry >> PTE_SHIFT) & 0xFFFFFFFF;
		base = (u64 *)osmap(pfn);
	}
	else
	{
		pfn = os_pfn_alloc(OS_PT_REG);
		*entry = (pfn << PTE_SHIFT) | ac_flags;
		base = osmap(pfn);
	}

	entry = base + ((addr & PUD_MASK) >> PUD_SHIFT);
	if (*entry & 0x1)
	{
		pfn = (*entry >> PTE_SHIFT) & 0xFFFFFFFF;
		base = (u64 *)osmap(pfn);
	}
	else
	{
		pfn = os_pfn_alloc(OS_PT_REG);
		*entry = (pfn << PTE_SHIFT) | ac_flags;
		base = osmap(pfn);
	}

	entry = base + ((addr & PMD_MASK) >> PMD_SHIFT);
	if (page_mapping == HUGE_PAGE_MAPPING)
	{
		u64 *address = (u64 *)os_hugepage_alloc();
		pfn = get_hugepage_pfn((void *)address);
		*entry = (pfn << PMD_SHIFT) | ac_flags;
		*entry |= ((u64)1 << 7);
		return address;
	}
	else
	{
		if (*entry & 0x1)
		{
			pfn = (*entry >> PTE_SHIFT) & 0xFFFFFFFF;
			base = (u64 *)osmap(pfn);
		}
		else
		{
			pfn = os_pfn_alloc(OS_PT_REG);
			*entry = (pfn << PTE_SHIFT) | ac_flags;
			base = osmap(pfn);
		}

		entry = base + ((addr & PTE_MASK) >> PTE_SHIFT);
		pfn = os_pfn_alloc(USER_REG);
		*entry = (pfn << PTE_SHIFT) | ac_flags;
	}
	return 0;
}

u64 *get_pte(struct exec_context *ctx, u64 addr)
{
	u64 *base = (u64 *)osmap(ctx->pgd);
	u64 *entry;
	u64 pfn;

	entry = base + ((addr & PGD_MASK) >> PGD_SHIFT);
	if (*entry & 0x1)
	{
		pfn = (*entry >> PTE_SHIFT) & 0xFFFFFFFF;
		base = (u64 *)osmap(pfn);
	}
	else
	{
		return NULL;
	}

	entry = base + ((addr & PUD_MASK) >> PUD_SHIFT);
	if (*entry & 0x1)
	{
		pfn = (*entry >> PTE_SHIFT) & 0xFFFFFFFF;
		base = (u64 *)osmap(pfn);
	}
	else
	{
		return NULL;
	}

	entry = base + ((addr & PMD_MASK) >> PMD_SHIFT);
	if (*entry >> 7 & 0x1)
	{
		if (*entry & 0x1)
		{
			return NULL;
		}
		return entry;
	}
	if (*entry & 0x1)
	{
		pfn = (*entry >> PTE_SHIFT) & 0xFFFFFFFF;
		base = (u64 *)osmap(pfn);
	}
	else
	{
		return NULL;
	}
	if (*entry & ((u64)1 << 57))
	{
		return entry;
	}

	entry = base + ((addr & PTE_MASK) >> PTE_SHIFT);
	if (*entry & 0x1)
		return NULL;
	return entry;
}

int do_unmap(struct exec_context *ctx, u64 addr)
{
	u64 *entry = get_pte(ctx, addr);
	if (!entry)
		return -1;
	os_pfn_free(USER_REG, (*entry >> PTE_SHIFT) & 0xFFFFFFFF);
	*entry = 0;
	return 1;
}

int vm_area_pagefault(struct exec_context *current, u64 addr, int error_code)
{
	struct vm_area *head = current->vm_area;
	while (head != NULL)
	{
		if (head->vm_start <= addr && addr < head->vm_end)
		{
			if ((error_code & 0x2) && !(head->access_flags & MM_WR))
				return -1;
			else
			{
				map_physical_area(current, addr, head->access_flags, head->mapping_type);
				return 1;
			}
		}
		head = head->vm_next;
	}
	return -1;
}

/**
 * mmap system call implementation.
 */
long vm_area_map(struct exec_context *current, u64 addr, int length, int prot, int flags)
{
	u64 ret_addr = -1;
	if (current->vm_area == NULL)
	{
		current->vm_area = create_vm_area(MMAP_AREA_START, MMAP_AREA_START + 4096, 0x4, NORMAL_PAGE_MAPPING);
	}
	if (current->vm_area == NULL)
		return -EINVAL;
	if (addr && (addr < MMAP_AREA_START) || (addr + length > MMAP_AREA_END))
		return -EINVAL;
	int page = 4096;
	if (addr && (addr % page != 0))
	{
		if (flags & MAP_FIXED)
			return -EINVAL;
		addr = (addr / page + 1) * page;
	}
	if (length % page != 0)
	{
		length = (length / page + 1) * page;
	}
	struct vm_area *head = current->vm_area;
	if (addr)
	{
		if ((flags & MAP_FIXED) && (addr + length) > MMAP_AREA_END)
			return -EINVAL;
		while (head != NULL)
		{
			if (addr + length <= head->vm_start || addr >= head->vm_end)
			{
				if (head->vm_next != NULL && head->vm_next->vm_start < addr + length)
				{
					head = head->vm_next;
					continue;
				}
				if (head->vm_next && (head->vm_next->vm_start == addr + length) && (head->vm_next->access_flags == prot) && (head->mapping_type == NORMAL_PAGE_MAPPING) && (head->vm_next->mapping_type == NORMAL_PAGE_MAPPING))
				{
					if ((head->vm_end == addr) && (head->access_flags == prot))
					{
						head->vm_end = head->vm_next->vm_end;
						struct vm_area *toDelete = head->vm_next;
						head->vm_next = head->vm_next->vm_next;
						dealloc_vm_area(toDelete);
					}
					else
					{
						head->vm_next->vm_start = addr;
					}
					return addr;
				}
				else if ((head->vm_end == addr) && (head->access_flags == prot) && (head->mapping_type == NORMAL_PAGE_MAPPING))
				{
					head->vm_end = addr + length;
					return addr;
				}
				else
				{
					struct vm_area *new = alloc_vm_area();
					new->vm_start = addr;
					new->vm_end = addr + length;
					new->access_flags = prot;
					new->mapping_type = NORMAL_PAGE_MAPPING;
					new->vm_next = head->vm_next;
					head->vm_next = new;
					return addr;
				}
			}
			else
			{
				if (flags & MAP_FIXED)
					return -EINVAL;
				break;
			}
		}
	}
	head = current->vm_area;
	while ((head->vm_next != NULL) && ((head->vm_next->vm_start - head->vm_end) < length))
		head = head->vm_next;
	if (head->access_flags == prot && head->mapping_type == NORMAL_PAGE_MAPPING)
	{
		if (head->vm_next && (head->vm_next->access_flags == prot) && (head->vm_end + length == head->vm_next->vm_start))
		{
			head->vm_end = head->vm_next->vm_end;
			struct vm_area *toDelete = head->vm_next;
			head->vm_next = head->vm_next->vm_next;
			dealloc_vm_area(toDelete);
		}
		else
		{
			head->vm_end += length;
		}
		return head->vm_end;
	}
	else
	{
		if (head->vm_next && (head->vm_next->access_flags == prot) && (head->vm_end + length == head->vm_next->vm_start) && (head->vm_next->mapping_type == NORMAL_PAGE_MAPPING))
		{
			head->vm_next->vm_start -= length;
			return head->vm_next->vm_start;
		}
		else
		{
			struct vm_area *new = alloc_vm_area();
			new->access_flags = prot;
			new->mapping_type = NORMAL_PAGE_MAPPING;
			new->vm_end = head->vm_end + length;
			new->vm_start = head->vm_end;
			new->vm_next = head->vm_next;
			head->vm_next = new;
			return new->vm_start;
		}
	}
	return -1;
}

/**
 * munmap system call implemenations
 */
int vm_area_unmap(struct exec_context *current, u64 addr, int length)
{
	int page = 4096;
	if ((addr < MMAP_AREA_START) || (addr + length > MMAP_AREA_END) || length <= 0)
		return -EINVAL;
	if (length % page != 0)
	{
		length = (length / page + 1) * page;
	}
	if (addr & page != 0)
	{
		return -1;
	}
	struct vm_area *head = current->vm_area;
	if (head == NULL)
		return -1;
	struct vm_area *prev = head;
	while (head != NULL)
	{
		if (head->vm_start >= addr && head->vm_end <= addr + length)
		{
			prev->vm_next = head->vm_next;
			dealloc_vm_area(head);
			head = prev;
		}
		else if ((addr <= head->vm_start) && (addr + length > head->vm_start) && (addr + length < head->vm_end))
		{
			head->vm_start = addr + length;
			break;
		}
		else if (addr > head->vm_start && addr < head->vm_end && addr + length >= head->vm_end)
		{
			head->vm_end = addr;
		}
		else if (addr > head->vm_start && addr + length < head->vm_end)
		{
			struct vm_area *new = alloc_vm_area();
			if (new == NULL)
				return -1;
			new->vm_start = addr + length;
			new->vm_end = head->vm_end;
			new->access_flags = head->access_flags;
			new->vm_next = head->vm_next;

			head->vm_end = addr;
			head->vm_next = new;
		}
		prev = head;
		head = head->vm_next;
	}
	for (int i = 0; i * page < length; i++)
	{
		do_unmap(current, addr + i * page);
	}
	return 0;
}

/**
 * make_hugepage system call implemenation
 */
long vm_area_make_hugepage(struct exec_context *current, void *addr, u32 length, u32 prot, u32 force_prot)
{
	u32 mb = 1 << 20;
	struct vm_area *head = current->vm_area;
	if (length <= 0)
		return -EINVAL;
	if (((u64)addr < MMAP_AREA_START) || (u64)addr + length > MMAP_AREA_END)
		return -EINVAL;
	if (length % (2 * mb) != 0)
	{
		length = length / (2 * mb);
	}
	if ((u64)addr % (2 * mb) != 0)
	{
		addr = (((u64)addr) / (2 * mb) + 1) * (2 * mb);
	}
	int end = 0;
	int toAlloc = 0;
	while (head != NULL)
	{
		if ((u64)addr >= head->vm_start && (u64)addr <= head->vm_end)
		{
			if (head->mapping_type == HUGE_PAGE_MAPPING && head->vm_end == (u64)addr)
			{
				if (head->vm_next)
				{
					if (head->vm_next->vm_start == (u64)addr)
					{
						struct vm_area *next = head->vm_next;
						unsigned long prev = head->vm_end;
						head->vm_end = (u64)addr + length;
						while (next->vm_start >= (u64)addr && next->vm_end <= (u64)addr + length)
						{
							if (force_prot)
							{
								next->access_flags = prot;
							}
							else if (next->access_flags != prot)
								return -EDIFFPROT;
							if (next->mapping_type == HUGE_PAGE_MAPPING)
								return -EVMAOCCUPIED;
							if (next->vm_start != prev)
								return -ENOMAPPING;
							prev = next->vm_end;
							if (get_pte(current, next->vm_start) != NULL)
								toAlloc = 1;
							if (next->vm_end == (u64)addr + length)
							{
								end = 1;
								head->vm_next = next->vm_next;
								dealloc_vm_area(next);
								break;
							}
							else
							{
								struct vm_area *toDelete = next;
								prev = next->vm_end;
								next = next->vm_next;
								dealloc_vm_area(toDelete);
							}
						}
						if (!end)
						{
							struct vm_area *new = alloc_vm_area();
							new->vm_start = (u64)addr + length;
							new->vm_end = next->vm_end;
							new->access_flags = next->access_flags;
							new->vm_next = next;
							new->mapping_type = NORMAL_PAGE_MAPPING;
							if (force_prot)
							{
								next->access_flags = prot;
							}
							else if (next->access_flags != prot)
								return -EDIFFPROT;
							dealloc_vm_area(next);
							head->vm_next = new;
							end = 1;
						}
					}
					else
					{
						return -ENOMAPPING;
					}
				}
				else
				{
					return -ENOMAPPING;
				}
			}
			else if (head->vm_start == (u64)addr)
			{
				if (head->mapping_type == HUGE_PAGE_MAPPING)
					return -EVMAOCCUPIED;
				struct vm_area *next = head->vm_next;
				if (head->vm_end == (u64)addr + length)
				{
					head->mapping_type = HUGE_PAGE_MAPPING;
					end = 1;
					break;
				}
				unsigned long prev = head->vm_end;
				while (next && next->vm_start >= (u64)addr && next->vm_end <= (u64)addr + length)
				{
					if (force_prot)
					{
						next->access_flags = prot;
					}
					else if (next->access_flags != prot)
						return -EDIFFPROT;
					if (next->mapping_type == HUGE_PAGE_MAPPING)
						return -EVMAOCCUPIED;
					if (next->vm_start != prev)
						return -ENOMAPPING;
					prev = next->vm_end;
					if (get_pte(current, next->vm_start) != NULL)
						toAlloc = 1;
					if (next->vm_end == (u64)addr + length)
					{
						end = 1;
						head->vm_next = next->vm_next;
						dealloc_vm_area(next);
						break;
					}
					else
					{
						struct vm_area *toDelete = next;
						prev = next->vm_end;
						next = next->vm_next;
						dealloc_vm_area(toDelete);
					}
				}
				head->mapping_type = HUGE_PAGE_MAPPING;
				head->vm_end = (u64)addr + length;
				if (!end)
				{
					struct vm_area *new = alloc_vm_area();
					new->vm_start = (u64)addr + length;
					new->vm_end = next->vm_end;
					new->access_flags = next->access_flags;
					new->vm_next = next;
					new->mapping_type = NORMAL_PAGE_MAPPING;
					if (force_prot)
					{
						next->access_flags = prot;
					}
					else if (next->access_flags != prot)
						return -EDIFFPROT;
					dealloc_vm_area(next);
					head->vm_next = new;
					end = 1;
				}
			}
			else if (head->vm_start < (u64)addr && head->vm_end > (u64)addr)
			{
				if (head->mapping_type == HUGE_PAGE_MAPPING)
				{
					return -EVMAOCCUPIED;
				}
				if (!force_prot && head->access_flags != prot)
					return -EDIFFPROT;
				struct vm_area *next = head->vm_next;
				unsigned long prev = head->vm_end;
				u64 last = head->vm_end;
				while (next->vm_start >= (u64)addr && next->vm_end <= (u64)addr + length)
				{
					if (force_prot)
					{
						next->access_flags = prot;
					}
					else if (next->access_flags != prot)
						return -EDIFFPROT;
					if (next->mapping_type == HUGE_PAGE_MAPPING)
						return -EVMAOCCUPIED;
					if (next->vm_start != prev)
						return -ENOMAPPING;
					prev = next->vm_end;
					if (get_pte(current, next->vm_start) != NULL)
						toAlloc = 1;
					if (next->vm_end == (u64)addr + length)
					{
						last = next->vm_end;
						end = 1;
						head->vm_next = next->vm_next;
						dealloc_vm_area(next);
						break;
					}
					else
					{
						struct vm_area *toDelete = next;
						prev = next->vm_end;
						next = next->vm_next;
						last = next->vm_end;
						dealloc_vm_area(toDelete);
					}
				}
				struct vm_area *new = alloc_vm_area();
				new->vm_start = (u64)addr;
				new->mapping_type = HUGE_PAGE_MAPPING;
				new->vm_end = (u64)addr + length;
				while (new->vm_end > last)
				{
					new->vm_end -= 2 * mb;
				}
				new->access_flags = prot;
				new->vm_next = head->vm_next;
				if (new->vm_end != (u64)addr + length)
				{
					struct vm_area *new2 = alloc_vm_area();
					new2->vm_start = new->vm_end;
					new2->mapping_type = NORMAL_PAGE_MAPPING;
					new2->vm_end = last;
					new2->access_flags = head->access_flags;
					new2->vm_next = new->vm_next;
					new->vm_next = new2;
				}
				head->vm_next = new;
				head->vm_end = (u64)addr;
				head = new;
				if (!end)
				{
					struct vm_area *new = alloc_vm_area();
					new->vm_start = (u64)addr + length;
					new->vm_end = next->vm_end;
					new->access_flags = next->access_flags;
					new->vm_next = next;
					new->mapping_type = NORMAL_PAGE_MAPPING;
					if (force_prot)
					{
						next->access_flags = prot;
					}
					else if (next->access_flags != prot)
						return -EDIFFPROT;
					dealloc_vm_area(next);
					head->vm_next = new;
					end = 1;
				}
			}
		}
		if (end)
			break;
		head = head->vm_next;
	}
	while (head->vm_next && (head->vm_next->mapping_type == HUGE_PAGE_MAPPING) && (head->vm_next->access_flags == head->access_flags))
	{
		head->vm_end = head->vm_next->vm_end;
		head->vm_next = head->vm_next->vm_next;
		dealloc_vm_area(head->vm_next);
	}
	if (end)
	{
		if (toAlloc)
		{
			u64 *address = map_physical_area(current, (u64)addr, prot, HUGE_PAGE_MAPPING);
			u64 src = ((*get_pte(current, (u64)addr) >> PTE_SHIFT) & 0xFFFFFFFF) * 4096;
			memcpy((char *)address, (char *)src, length);
		}
		return (u64)addr;
	}
	return -ENOMAPPING;
}

/**
 * break_system call implemenation
 */
int vm_area_break_hugepage(struct exec_context *current, void *addr, u32 length)
{
	u32 mb = 1 << 20;
	if (!addr)
		return -EINVAL;
	if (length % (2 * mb) != 0)
		return -EINVAL;
	if ((u64)addr % (2 * mb) != 0)
		return -EINVAL;
	struct vm_area *head = current->vm_area;
	while (head != NULL)
	{
		if ((u64)addr >= head->vm_start && ((u64)addr + length) >= head->vm_end)
		{
			if (head->mapping_type == NORMAL_PAGE_MAPPING)
			{
				head = head->vm_next;
				continue;
			}
			if (head->vm_start == (u64)addr)
			{
				head->mapping_type = NORMAL_PAGE_MAPPING;
			}
			else
			{
				struct vm_area *new = alloc_vm_area();
				new->vm_start = (u64)addr;
				new->vm_end = head->vm_end;
				new->access_flags = head->access_flags;
				new->vm_next = head->vm_next;
				new->mapping_type = NORMAL_PAGE_MAPPING;
				head->vm_end = (u64)addr;
				head->vm_next = new;
			}
		}
		else if ((u64)addr <= head->vm_start && ((u64)addr + length) >= head->vm_end)
		{
			head->mapping_type = NORMAL_PAGE_MAPPING;
		}
		else if ((u64)addr <= head->vm_start && ((u64)addr + length) <= head->vm_end)
		{
			if ((u64)addr + length == head->vm_end)
			{
				head->mapping_type = NORMAL_PAGE_MAPPING;
			}
			else
			{
				struct vm_area *new = alloc_vm_area();
				new->access_flags = head->access_flags;
				new->vm_end = head->vm_end;
				new->vm_start = (u64)addr + length;
				new->mapping_type = head->mapping_type;
				new->vm_next = head->vm_next;
				head->vm_next = new;
				head->mapping_type = NORMAL_PAGE_MAPPING;
				head->vm_end = (u64)addr + length;
			}
		}
		head = head->vm_next;
	}
	merger(current->vm_area);
	return 0;
}