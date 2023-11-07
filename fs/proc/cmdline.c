// SPDX-License-Identifier: GPL-2.0
#include <linux/fs.h>
#include <linux/init.h>
#include <linux/proc_fs.h>
#include <linux/seq_file.h>

#ifdef CONFIG_PROC_MOD_CMDLINE
#include <asm/setup.h>

static char new_command_line[COMMAND_LINE_SIZE];
#endif

static int cmdline_proc_show(struct seq_file *m, void *v)
{
#ifdef CONFIG_PROC_MOD_CMDLINE
	seq_puts(m, new_command_line);
#else
	seq_puts(m, saved_command_line);
#endif
	seq_putc(m, '\n');
	return 0;
}

#ifdef CONFIG_PROC_PATCH_CMDLINE_FLAG
static void patch_cmdline_flag(char *cmd, const char *flag, const char *val)
{
	size_t flag_len, val_len;
	char *start, *end;

	start = strstr(cmd, flag);
	if (!start)
		return;

	flag_len = strlen(flag);
	val_len = strlen(val);
	end = start + flag_len + strcspn(start + flag_len, " ");
	memmove(start + flag_len + val_len, end, strlen(end) + 1);
	memcpy(start + flag_len, val, val_len);
}
#endif

#ifdef CONFIG_PROC_REPLACE_CMDLINE_FLAG
static char *padding = " ";
static void replace_cmdline_flag(char *cmd, const char *flag, const char *flag_new)
{
	char *start_addr, *end_addr;

	/* Ensure all instances of a flag are replaced */
	while ((start_addr = strstr(cmd, flag))) {
		end_addr = strchr(start_addr, ' ');
		if (end_addr) {
			if (strlen(flag)<strlen(flag_new)) {
				// xx yy=a zz
				//    ^   ^
				// xx yy=bb zz
				int length_to_copy = strlen(start_addr + (strlen(flag))) + 1; // +1 to copy trailing '/0'
				int length_diff = strlen(flag_new)-strlen(flag);
				memcpy(start_addr+(strlen(flag)+length_diff), start_addr+(strlen(flag)), length_to_copy);
				memcpy(start_addr+(strlen(flag)), padding, length_diff);
			}
			memcpy(start_addr, flag_new, strlen(flag_new));
		}
		else
			*(start_addr - 1) = '\0';
	}
}
#endif

#ifdef CONFIG_FORCE_LGE_DUAL_SIM
static void replace_sim_flag(char *cmd)
{
	/* WARNING: be aware that you can't replace shorter string 
	 * with longer ones in the function called here...*/

	replace_cmdline_flag(cmd, "androidboot.vendor.lge.sim_num=1",
				  "androidboot.vendor.lge.sim_num=2 ");
}
#endif

static int __init proc_cmdline_init(void)
{
#ifdef CONFIG_PROC_MOD_CMDLINE
	strcpy(new_command_line, saved_command_line);
#endif

#ifdef CONFIG_FORCE_LGE_DUAL_SIM
	replace_sim_flag(new_command_line);
#endif

	proc_create_single("cmdline", 0, NULL, cmdline_proc_show);
	return 0;
}
fs_initcall(proc_cmdline_init);
