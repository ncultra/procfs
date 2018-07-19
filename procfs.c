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
