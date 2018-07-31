/*******************************************************************
 * in-kernel file reader
 *
 * Copyright (C) 2018  Michael D. Day II
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA  02110-1301, USA.
 *
 *******************************************************************/

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/stat.h>
#include <linux/file.h>
#include <linux/printk.h>
#include <linux/slab.h>
#include <linux/moduleparam.h>

#define assert(s) do {													\
		if (unlikely(!(s))) printk(KERN_DEBUG "assertion failed: " #s " at %s:%d\n", \
								   __FILE__, __LINE__);					\
	} while(0)

#define _MODULE_LICENSE "GPL v2"
#define _MODULE_AUTHOR "Michael D. Day II <ncultra@gmail.com>"
#define _MODULE_INFO "in-kernel file reader"

char *name = "/proc/cpuinfo";
module_param(name, charp, 0644);
MODULE_PARM_DESC(name, "path of file to read");

int text_file = 1;
module_param(text_file, int, 0644);
MODULE_PARM_DESC(text_file, "file to be read is utf-8");

long chunk_size = 0x400;
module_param(chunk_size, long, 0644);
MODULE_PARM_DESC(chunk_size, "chunk size for virtual files");

long max_size = (~0UL >> 1);
module_param(max_size, long, 0644);
MODULE_PARM_DESC(max_size, "largest file buffer to allocate");


int file_getattr(struct file *f, struct kstat *k)
{
	int ccode = 0;
	memset(k, 0x00, sizeof(struct kstat));
	ccode = vfs_getattr(&f->f_path, k, 0x00000fffU, KSTAT_QUERY_FLAGS);
	return ccode;
}

ssize_t write_file(char *name, void *buf, size_t count, loff_t * pos)
{
	ssize_t ccode;
	struct file *f;
	f = filp_open(name, O_RDWR, 0);
	if (f) {
		ccode = __kernel_write(f, buf, count, pos);
		if (ccode < 0) {
			pr_err("Unable to write file: %s (%ld)", name, ccode);
		}
		filp_close(f, 0);
	} else {
		ccode = -EBADF;
		pr_err("Unable to open file: %s (%ld)", name, ccode);
	}
	return ccode;
}


void dump_buffer(uint8_t *buffer, ssize_t size)
{
	ssize_t characters = size;
	uint8_t *cursor = buffer;
	while (characters) {
		printk(KERN_CONT "%c", *cursor);
		cursor++;
		characters--;
	}
}


ssize_t vfs_read_file(char *name, void **buf, size_t max_count, loff_t *pos)
{
	ssize_t ccode = 0;
	struct file *f = NULL;

	assert(buf);
	*buf = NULL;
	assert(pos);
	*pos = 0LL;

	f = filp_open(name, O_RDONLY, 0);
	if (f) {
		ssize_t chunk = chunk_size, allocated = 0, cursor = 0;
		*buf = kzalloc(chunk, GFP_KERNEL);
		if (*buf) {
			allocated = chunk;
		} else {
			ccode =  -ENOMEM;
			goto out_err;
		}

		do {
			/**
			 * read one chunk at a time
			 **/
			cursor = *pos; /* initially zero, then positioned with further reads */
			ccode = kernel_read(f, *buf + cursor, chunk, pos);
			if (ccode < 0) {
				pr_err("Unable to read file chunk: %s (%ld)", name, ccode);
				goto out_err;
			}
			if (ccode > 0) {
				*buf = krealloc(*buf, allocated + chunk, GFP_KERNEL);
				if (! *buf) {
					ccode = -ENOMEM;
					goto out_err;
				}
				allocated += chunk;
			}
		} while (ccode && allocated <= max_count);
		filp_close(f, 0);
	} else {
		ccode = -EBADF;
		pr_err("Unable to open file: %s (%ld)", name, ccode);
	}
	return ccode;

out_err:
	if (f) {
		filp_close(f, 0);
	}
	if  (*buf) {
		kfree(*buf);
		*buf = NULL;
	}
	return ccode;
}


ssize_t module_main(void)
{
	void *buffer = NULL;
	int ccode = 0;
	loff_t size = 0x1000;

	ccode = kernel_read_file_from_path(name,
									   &buffer,
									   &size,
									   max_size,
									   READING_MODULE);
	if (ccode >= 0 && buffer != NULL && size >= 0) {
		if (text_file) {
			dump_buffer((uint8_t *)buffer, (ssize_t) size);
		}
		vfree(buffer);
	} else {
		size = 0LL;
		buffer = NULL;
		ccode = vfs_read_file(name,
							  &buffer,
							  max_size,
							  &size);
		if (text_file && buffer && size && ccode >= 0) {
			dump_buffer((uint8_t *)buffer, size);
		}
		if (buffer) {
			kfree(buffer);
		}
	}

	return ccode;
}

static int
__init procfs_init(void)
{
	module_main();
	return 0;
}

static void
__exit procfs_exit(void)
{
	return;
}

module_init(procfs_init);
module_exit(procfs_exit);

MODULE_LICENSE(_MODULE_LICENSE);
MODULE_AUTHOR(_MODULE_AUTHOR);
MODULE_DESCRIPTION(_MODULE_INFO);
