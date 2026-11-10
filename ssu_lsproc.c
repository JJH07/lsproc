#include "myheader.h"

/* Run within the option's child, so an omitted PID still means that child. */
static void execute_pid_option(int flag, string_list pids, string_list keywords)
{
	char default_pid[32];
	char *default_items[] = {default_pid};
	if(pids.count == 0)
	{
		snprintf(default_pid, sizeof(default_pid), "%d", (int)getpid());
		pids.items = default_items;
		pids.count = 1;
	}

	for(int i = 0; i < pids.count; ++i)
	{
		if(pids.count > 1)
			printf("([/proc/%s/fd])\n", pids.items[i]);
		if(i >= MAX_PID_ARGUMENTS)
		{
			printf("Maximum Number of Argument Exceeded. :: %s\n", pids.items[i]);
			continue;
		}

		int pid = parse_pid(pids.items[i]);
		switch(flag)
		{
			case FLAG_F: print_process_fds(pid); break;
			case FLAG_T: print_process_status(pid); break;
			case FLAG_C: print_process_cmdline(pid); break;
			case FLAG_N: print_process_io(pid); break;
			case FLAG_M: print_process_environ(pid, keywords); break;
		}
	}
}

void execute_options(program_options opts)
{
	pid_t child_pid;
	int output_fd = -1;
	int child_status;

	if(opts.flags & FLAG_O)
	{
		const char *output_path = opts.output_paths.count ? opts.output_paths.items[0] : "";
		if((output_fd = open(output_path, O_RDWR | O_CREAT | O_TRUNC, 0644)) == -1)
		{
			perror(output_path);
			return;
		}
		if(dup2(output_fd, STDOUT_FILENO) == -1)
		{
			perror("dup2() error : ");
			close(output_fd);
			return;
		}
	}

	printf(">: ssu_lsproc start.:<\n");
	fflush(stdout);
	/* Keep the original flag order and one fork/wait pair per selected flag,
	 * including the modifier flags K and R. O only redirects stdout. */
	for(int flag = FLAG_F; flag <= FLAG_S; flag <<= 1)
	{
		if((opts.flags & flag) == 0)
			continue;
		if((child_pid = fork()) < 0)
		{
			perror("fork() error : ");
			exit(0);
		}
		else if(child_pid == 0)
		{
			switch(flag)
			{
				case FLAG_F: execute_pid_option(flag, opts.fd_pids, opts.keywords); break;
				case FLAG_T: execute_pid_option(flag, opts.status_pids, opts.keywords); break;
				case FLAG_C: execute_pid_option(flag, opts.cmdline_pids, opts.keywords); break;
				case FLAG_N: execute_pid_option(flag, opts.io_pids, opts.keywords); break;
				case FLAG_M: execute_pid_option(flag, opts.environ_pids, opts.keywords); break;
				case FLAG_W: print_interrupts(); break;
				case FLAG_E: print_filesystems(); break;
				case FLAG_L: print_uptime(); break;
				case FLAG_V: print_kernel_version(); break;
				case FLAG_S: print_user_process_attributes(opts.attributes); break;
			}
			exit(0);
		}
		else
		{
			pid_t waited_pid;
			do {
				waited_pid = wait(&child_status);
			} while(waited_pid < 0 && errno == EINTR);
			if(waited_pid < 0)
				printf("wait error\n");
		}
	}
	printf(">: ssu_lsproc terminated.:<\n");

	if(output_fd > -1 && output_fd != STDOUT_FILENO)
		close(output_fd);
}
