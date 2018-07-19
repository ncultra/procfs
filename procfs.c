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


/**
 * Note on /sys and /proc files:
 * on Linux 4.x they stat as having no blocks and zero size,
 * but they do have a blocksize of 0x400. So, by default, we
 * will allocate a buffer the size of one block.
 *
 * This is certainly a gross hack and may change with future
 * kernel versions.
 **/

static inline size_t
calc_file_size(struct kstat *kstat)
{
	if (kstat->size) {
		return kstat->size;
	}
	if (kstat->blocks) {
		return kstat->blocks * kstat->blksize;
	}
	return kstat->blksize > 0 ? kstat->blksize: 0x100;
}

int file_getattr(struct file *f, struct kstat *k)
{
	int ccode = 0;
	memset(k, 0x00, sizeof(struct kstat));
	ccode = vfs_getattr(&f->f_path, k, 0x00000fffU, KSTAT_QUERY_FLAGS);
	return ccode;
}

ssize_t
write_file_struct(struct file * f, void *buf, size_t count, loff_t * pos)
{
	ssize_t ccode;
	ccode = kernel_write(f, buf, count, pos);
	if (ccode < 0) {
		pr_err("Unable to write file: %p (%ld)", f, ccode);
	}
	return ccode;
}

ssize_t read_file_struct(struct file * f, void *buf, size_t count, loff_t * pos)
{
	ssize_t ccode;

	ccode = kernel_read(f, buf, count, pos);
	if (ccode < 0) {
		pr_err("Unable to read file: %p (%ld)", f, ccode);
	}

	return ccode;
}

ssize_t write_file(char *name, void *buf, size_t count, loff_t * pos)
{
	ssize_t ccode;
	struct file *f;
	f = filp_open(name, O_WRONLY, 0);
	if (f) {
		ccode = kernel_write(f, buf, count, pos);
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

ssize_t read_file(char *name, void *buf, size_t count, loff_t * pos)
{
	ssize_t ccode;
	struct file *f;

	f = filp_open(name, O_RDONLY, 0);
	if (f) {
		ccode = kernel_read(f, buf, count, pos);
		if (ccode < 0) {
			pr_err("Unable to read file: %s (%ld)", name, ccode);
		}
		filp_close(f, 0);
	} else {
		ccode = -EBADF;
		pr_err("Unable to open file: %s (%ld)", name, ccode);
	}
	return ccode;
}

ssize_t
vfs_read_data(char *path, void **data)
{
	struct file *f = NULL;
	ssize_t ccode = 0;

	if (!data || !path) {
		return -EINVAL;
	}
	*data = NULL;

	/* open the file, get the file (or block) size */

	f = filp_open(path, O_RDONLY, 0);
	if (f) {
		struct kstat stat = {0};
		size_t max_size = 0x100000;
		loff_t pos = 0;
		ccode = file_getattr(f, &stat);
		if (ccode) {
			printk(KERN_INFO "error getting file attributes %zx\n", ccode);
			goto err_exit;
		}
		/**
		 * get the size, or default size if /proc or /sys
		 * set an arbitrary limit of 1 MB for read buffer
		 **/
		ccode = min(calc_file_size(&stat), max_size);
		*data = kzalloc(ccode, GFP_KERNEL);
		if (*data == NULL) {
			ccode = -ENOMEM;
			goto err_exit;
		}

		ccode = read_file_struct(f, *data, ccode, &pos);
		if (ccode < 0) {
			goto err_exit;
		}
	} else {
		ccode = -EBADF;
		pr_err("sysfs-probe Unable to get a file handle: %s (%zx)\n", path, ccode);
		goto err_exit;
	}
	return ccode;

err_exit:
	if (f) {
		filp_close(f, 0);
	}
	if (*data != NULL) {
		kfree(*data);
		*data = NULL;
	}
	return ccode;
}


ssize_t module_main(void)
{
	void *buffer = NULL;
	ssize_t bytes = vfs_read_data(name, &buffer);
	printk(KERN_DEBUG "PROCFS bytes read: %ld\n", bytes);

	if (bytes > 0) {
		if (text_file) {
			printk(KERN_INFO "%s", (char *)buffer);
		}
		kfree(buffer);
	}
	return bytes;
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
