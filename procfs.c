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

#include <linux/compiler.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/spinlock.h>
#include <linux/semaphore.h>
#include <linux/fs.h>
#include <linux/namei.h>
#include <linux/stat.h>
#include <linux/file.h>
#include <linux/fdtable.h>
#include <linux/printk.h>
#include <linux/spinlock.h>
#include <linux/rculist.h>
#include <linux/flex_array.h>
#include <linux/slab.h>
#include <linux/delay.h>
#include <linux/moduleparam.h>
#include <uapi/linux/stat.h>
#include <asm/atomic64_64.h>
#define assert(s) do { \
		if (unlikely(!(s))) printk(KERN_DEBUG "assertion failed: " #s " at %s:%d\n", \
						  __FILE__, __LINE__);							\
  } while(0)

#define _MODULE_LICENSE "GPL v2"
#define _MODULE_AUTHOR "Michael D. Day II <ncultra@gmail.com>"
#define _MODULE_INFO "in-kernel file reader"

/**
 * Note on /sys and /proc files:
 * on Linux 4.x they stat as having no blocks and zero size,
 * but they do have a blocksize of 0x400. So, by default, we
 * will allocate a buffer the size of one block
 **/

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

int module_main(void)
{
	do {
		;
	} while (0);
	return 0;
}

static int
__init procfs_init(void)
{
	int ccode = 0;

	return ccode;
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
