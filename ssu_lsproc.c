#include "myheader.h"

static int flush_output(void)
{
	int failed = ferror(stdout);
	errno = 0;
	if(fflush(stdout) == EOF || failed)
	{
		if(errno == 0)
			errno = EIO;
		perror("stdout");
		return EXIT_FAILURE;
	}
	return EXIT_SUCCESS;
}

/* Run within the option's child, so an omitted PID still means that child. */
static int execute_pid_option(int flag, string_list pids, string_list keywords)
{
	int result = EXIT_SUCCESS;
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

		int query_result = EXIT_SUCCESS;
		int pid = parse_pid(pids.items[i]);
		switch(flag)
		{
			case FLAG_F: query_result = print_process_fds(pid); break;
			case FLAG_T: query_result = print_process_status(pid); break;
			case FLAG_C: query_result = print_process_cmdline(pid); break;
			case FLAG_N: query_result = print_process_io(pid); break;
			case FLAG_M: query_result = print_process_environ(pid, keywords); break;
		}
		if(query_result != EXIT_SUCCESS)
			result = EXIT_FAILURE;
	}
	return result;
}

int execute_options(program_options opts)
{
	int result = EXIT_SUCCESS;
	pid_t child_pid;
	int output_fd = -1;
	int child_status;

	if(opts.flags & FLAG_O)
	{
		const char *output_path = opts.output_paths.count ? opts.output_paths.items[0] : "";
		if((output_fd = open(output_path, O_RDWR | O_CREAT | O_TRUNC, 0644)) == -1)
		{
			perror(output_path);
			return EXIT_FAILURE;
		}
		if(dup2(output_fd, STDOUT_FILENO) == -1)
		{
			perror("dup2() error : ");
			close(output_fd);
			return EXIT_FAILURE;
		}
	}

	printf(">: ssu_lsproc start.:<\n");
	if(flush_output() != EXIT_SUCCESS)
	{
		if(output_fd >= 0 && output_fd != STDOUT_FILENO)
			close(output_fd);
		return EXIT_FAILURE;
	}
	/* Keep the original flag order and one fork/wait pair per selected flag,
	 * including the modifier flags K and R. O only redirects stdout. */
	for(int flag = FLAG_F; flag <= FLAG_S; flag <<= 1)
	{
		if((opts.flags & flag) == 0)
			continue;
		if((child_pid = fork()) < 0)
		{
			perror("fork() error : ");
			result = EXIT_FAILURE;
			break;
		}
		else if(child_pid == 0)
		{
			int query_result = EXIT_SUCCESS;
			switch(flag)
			{
				case FLAG_F: query_result = execute_pid_option(flag, opts.fd_pids, opts.keywords); break;
				case FLAG_T: query_result = execute_pid_option(flag, opts.status_pids, opts.keywords); break;
				case FLAG_C: query_result = execute_pid_option(flag, opts.cmdline_pids, opts.keywords); break;
				case FLAG_N: query_result = execute_pid_option(flag, opts.io_pids, opts.keywords); break;
				case FLAG_M: query_result = execute_pid_option(flag, opts.environ_pids, opts.keywords); break;
				case FLAG_W: query_result = print_interrupts(); break;
				case FLAG_E: query_result = print_filesystems(); break;
				case FLAG_L: query_result = print_uptime(); break;
				case FLAG_V: query_result = print_kernel_version(); break;
				case FLAG_S: query_result = print_user_process_attributes(opts.attributes); break;
			}
			if(flush_output() != EXIT_SUCCESS)
				query_result = EXIT_FAILURE;
			exit(query_result);
		}
		else
		{
			pid_t waited_pid;
			do {
				waited_pid = wait(&child_status);
			} while(waited_pid < 0 && errno == EINTR);
			if(waited_pid < 0)
			{
				printf("wait error\n");
				result = EXIT_FAILURE;
			}
			else if(WIFSIGNALED(child_status))
			{
				fprintf(stderr, "Child terminated by signal %d.\n", WTERMSIG(child_status));
				result = EXIT_FAILURE;
			}
			else if(!WIFEXITED(child_status) || WEXITSTATUS(child_status) != EXIT_SUCCESS)
				result = EXIT_FAILURE;
		}
	}
	printf(">: ssu_lsproc terminated.:<\n");

	if(flush_output() != EXIT_SUCCESS)
		result = EXIT_FAILURE;
	if(output_fd > -1 && output_fd != STDOUT_FILENO && close(output_fd) == -1)
	{
		perror("close() error : ");
		result = EXIT_FAILURE;
	}
	return result;
}
