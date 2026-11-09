#include "myheader.h"

/* Preserve the existing access checks and diagnostics for /proc text files. */
static FILE *open_proc_file(const char *path)
{
	FILE *fp;
	if(access(path, F_OK) == -1 && errno == ENOENT)
	{
		printf("%s doesn't exist\n", path);
		return NULL;
	}
	if(access(path, R_OK) == -1 && errno == EACCES)
	{
		printf("%s can't be read.\n", path);
		return NULL;
	}
	if((fp = fopen(path, "r")) == NULL)
	{
		printf("fopen() error\n");
		return NULL;
	}
	return fp;
}

void print_process_fds(int pid) //f옵션 처리하는 함수입니다.
{
	struct dirent *entry;
	DIR *directory;
	char path[512];

	sprintf(path, "/proc/%d/fd", pid);
	char *ptr = path + strlen(path);
	*ptr++ = '/';
	*ptr = '\0';
	if(access(path, F_OK) == -1 && errno == ENOENT)
	{
		printf("%s doesn't exist\n", path);
		return ;
	}
	if(access(path, R_OK) == -1 && errno == EACCES)
	{
		printf("%s can't be read.\n", path);
		return ;
	}
	if((directory = opendir(path)) == NULL)
	{
		printf("opendir() error\n");
		return ;
	}
	while((entry = readdir(directory)) != NULL)
	{
		if(strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0)
		{
			continue;
		}
		strcpy(ptr, entry->d_name);
		size_t capacity = 256;
		char *link_target = NULL;
		ssize_t length;
		for(;;)
		{
			char *next = realloc(link_target, capacity + 1);
			if(next == NULL)
			{
				free(link_target);
				closedir(directory);
				perror("realloc() error : ");
				return;
			}
			link_target = next;
			length = readlink(path, link_target, capacity);
			if(length < 0 || (size_t)length < capacity)
				break;
			capacity *= 2;
		}
		if(length < 0)
		{
			free(link_target);
			continue;
		}
		link_target[length] = '\0';
		printf("File Descriptor number: %s, Opened File: %s\n", entry->d_name, link_target);
		free(link_target);
	}
	if(closedir(directory) < 0)
	{
		printf("closedir() error\n");
	}
}


void print_process_status(int pid) //t옵션을 처리하는 함수입니다.
{	
	FILE *fp;
	char path[512] = "/proc/";
	char *buf = NULL;
	size_t capacity = 0;

	sprintf(path, "/proc/%d/status", pid);
	fp = open_proc_file(path);
	if(fp == NULL)
		return;
	for(int c = 0; c < 9; c++)
	{
		if(getline(&buf, &capacity, fp) < 0)
			break;
		char *token = strtok(buf, " \t\n");
		if(token == NULL)
			continue;
		printf("%s ", token);
		if(strcmp(token, "State:") == 0)
		{
			token = strtok(NULL, " \t\n");
			if(token == NULL)
			{
				printf("\n");
				continue;
			}
			if(token[0] == 'R')
			{
				printf("Running\n");
			}
			else if(token[0] == 'S')
			{
				printf("Sleeping\n");
			}
			else if(token[0] == 'D')
			{
				printf("Disk sleep\n");
			}
			else if(token[0] == 'Z')
			{
				printf("Zombie\n");
			}
			else if(token[0] == 'X')
			{
				printf("Dead\n");
			}
			else if(token[0] == 'T')
			{
				printf("Stopped\n");
			}
			else
			{
				printf("%s\n", token);
			}
		}
		else
		{
			token = strtok(NULL, " \t\n");
			printf("%s\n", token ? token : "");
		}
	}
	free(buf);
	fclose(fp);
}


void print_process_cmdline(int pid)//c옵션 처리하는 함수입니다.
{
	FILE *fp;
	char path[512] = "/proc/";
	char *buf = NULL;
	size_t capacity = 0;

	sprintf(path, "/proc/%d/cmdline", pid);
	fp = open_proc_file(path);
	if(fp == NULL)
		return;
	int arg_index = 0;
	while(getdelim(&buf, &capacity, '\0', fp) != -1)
	{
		printf("argv[%d] : %s\n", arg_index++, buf);
	}
	free(buf);
	fclose(fp);
}


void print_process_io(int pid) //n옵션 처리하는 함수입니다.
{
	FILE *fp;
	char path[512] = "/proc/";
	char buf[256];

	sprintf(path, "/proc/%d/io", pid);
	fp = open_proc_file(path);
	if(fp == NULL)
		return;
	while(fgets(buf, 256, fp) != NULL)
	{
		char *token = strtok(buf, " \t\n");
		if(token == NULL)
			continue;
		if(strcmp(token, "rchar:") == 0)
		{
			printf("Characters read : ");
		}
		else if(strcmp(token, "wchar:") == 0)
		{
			printf("Characters written : ");
		}
		else if(strcmp(token, "syscr:") == 0)
		{
			printf("Read syscalls : ");
		}
		else if(strcmp(token, "syscw:") == 0)
		{
			printf("Write syscalls : ");
		}
		else if(strcmp(token, "read_bytes:") == 0)
		{
			printf("Bytes read : ");
		}
		else if(strcmp(token, "write_bytes:") == 0)
		{
			printf("Bytes written : ");
		}
		else
		{
			printf("Cancelled write bytes : ");
		}
		token = strtok(NULL, " \t\n");
		printf("%s\n", token ? token : "");
	}
	fclose(fp);
}


/* Quote one complete shell argument without changing grep's regex semantics. */
static char *quote_shell_argument(const char *text)
{
	size_t length = strlen(text);
	if(length > ((size_t)-1 - 3) / 4)
	{
		errno = ENOMEM;
		return NULL;
	}
	char *quoted = malloc(length * 4 + 3);
	if(quoted == NULL)
		return NULL;
	char *out = quoted;
	*out++ = '\'';
	for(; *text; ++text)
	{
		if(*text == '\'')
		{
			memcpy(out, "'\\''", 4);
			out += 4;
		}
		else
			*out++ = *text;
	}
	*out++ = '\'';
	*out = '\0';
	return quoted;
}

void print_process_environ(int pid, string_list keywords) //m옵션 처리하는 함수입니다.
{
	char path[512] = "/proc/";
	char shell_command[1024];

	sprintf(path, "/proc/%d/environ", pid);
	if(access(path, F_OK) == -1 && errno == ENOENT)
	{
		printf("%s doesn't exist\n", path);
		return ;
	}
	if(access(path, R_OK) == -1 && errno == EACCES)
	{
		printf("%s can't be read.\n", path);
		return ;
	}
	fflush(stdout);
	if(keywords.count == 0)
	{
		sprintf(shell_command, "cat %s | tr '\\000' '\n'", path);
		system(shell_command);
	}
	else
	{
		for(int keyword_index = 0; keyword_index < keywords.count; keyword_index++)
		{
			char *quoted = quote_shell_argument(keywords.items[keyword_index]);
			if(quoted == NULL)
			{
				perror("malloc() error : ");
				return;
			}
			size_t size = strlen(path) + strlen(quoted) + 64;
			char *command = malloc(size);
			if(command == NULL)
			{
				free(quoted);
				perror("malloc() error : ");
				return;
			}
			snprintf(command, size, "cat %s | tr '\\000' '\n' | grep -- %s", path, quoted);
			system(command);
			free(command);
			free(quoted);
		}
	}
}


void print_interrupts(void) //w옵션 처리하는 함수입니다.
{
	FILE *fp;
	char path[] = "/proc/interrupts"; // 경로 설정하고
	char *buf = NULL, *token;
	size_t capacity = 0;

	int cpu_count = 0;
	
	fp = open_proc_file(path);
	if(fp == NULL)
		return;

	if(getline(&buf, &capacity, fp) < 0)
	{
		free(buf);
		fclose(fp);
		return;
	}
	token = strtok(buf, " \t\n");
	while(token != NULL)
	{
		cpu_count++;
		token = strtok(NULL, " \t\n");
	}
	printf("---Number of CPUs : %d---\n      [Average] : [Description]\n", cpu_count);

	while(cpu_count > 0 && getline(&buf, &capacity, fp) >= 0)
	{
		char *name;
		long double sum = 0;

		token = strtok(buf, " :\t\n");
		if(token == NULL || (token[0] >= '0' && token[0] <= '9'))
		{
			continue;
		}
		name = token;
		for(int i = 0; i < cpu_count; i++)
		{
			token = strtok(NULL, " \t\n");
			if(token == NULL)
			{
				break;
			}
			char *end;
			errno = 0;
			unsigned long long count = strtoull(token, &end, 10);
			if(errno == ERANGE || end == token || *end != '\0')
				break;
			sum += count;
		}
		printf("%15.3Lf : <%s> ", sum / cpu_count, name);
		token = strtok(NULL, "\t\n");
		if(token != NULL)
		{
			printf("%s\n", token);
		}
		else
		{
			printf("\n");
		}
	}
	free(buf);
	fclose(fp);
}


void print_filesystems(void) //e옵션 처리하는 함수입니다.
{
	FILE *fp;
	char path[] = "/proc/filesystems";
	char buf[256];
	int items_on_line = 0;

	fp = open_proc_file(path);
	if(fp == NULL)
		return;
	printf("<<Supported Filesystems>>\n");
	while(fgets(buf, 256, fp) != NULL)
	{
		char *token = strtok(buf, " \t\n");
		if(token == NULL || strcmp(token, "nodev") == 0)
		{
			continue;
		}
		if(items_on_line == 5)
		{
			items_on_line = 0;
			printf("\n");
		}
		if(items_on_line > 0)
		{
			printf(", ");
		}
		printf("%s", token);
		items_on_line++;
	}
	printf("\n");
	fclose(fp);
}


void print_uptime(void) //l옵션 처리하는 함수입니다.
{
	FILE *fp;
	char path[] = "/proc/uptime"; //경로 설정합니다.
	char buf[256], *token;

	fp = open_proc_file(path);
	if(fp == NULL)
		return;
	if(fgets(buf, sizeof(buf), fp) == NULL)
	{
		fclose(fp);
		return;
	}

	token = strtok(buf, " \t");
	printf("Process worked time : %s(sec)\n", token ? token : "");
	token = strtok(NULL, " \t\n");
	printf("Process idle time : %s(sec)\n", token ? token : "");
	fclose(fp);
}


void print_kernel_version(void) //v옵션 처리하는 함수입니다.
{
	fflush(stdout);
	system("cat /proc/version");
}

void print_user_process_attributes(string_list attributes)
{
	uid_t uid = getuid();

	DIR *directory;
	FILE *fp;

	struct dirent *entry;
	struct stat statbuf;

	char path[512];
	char buf[256];

	int *pids = NULL;
	size_t pid_count = 0, pid_capacity = 0;
	int show_fds = 0, show_cmdline = 0, show_io = 0, show_status = 0, show_environ = 0;

	for(int i = 0; i < attributes.count; i++)
	{
		if(strcmp(attributes.items[i], "FILEDES") == 0)
		{
			show_fds = 1;
		}
		if(strcmp(attributes.items[i], "CMDLINE") == 0)
		{
			show_cmdline = 1;
		}
		if(strcmp(attributes.items[i], "IO") == 0)
		{
			show_io = 1;
		}
		if(strcmp(attributes.items[i], "STAT") == 0)
		{
			show_status = 1;
		}
		if(strcmp(attributes.items[i], "ENVIRON") == 0)
		{
			show_environ = 1;
		}
	}

	if((directory = opendir("/proc")) == NULL)
	{
		printf("opendir() error\n");
		return;
	}
	while((entry = readdir(directory)) != NULL)
	{
		if(strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0)
		{
			continue;
		}		
		int candidate_pid = parse_pid(entry->d_name);
		if(candidate_pid <= 0)
			continue;
		sprintf(path, "/proc/%s", entry->d_name);
		if(lstat(path, &statbuf) < 0)
		{
			continue;
		}
		if(!S_ISDIR(statbuf.st_mode))
		{
			continue;
		}
		strcat(path, "/status");
		if(access(path, F_OK) == -1 && errno == ENOENT)
		{
			continue;
		}
		if(access(path, R_OK) == -1 && errno == EACCES)
		{
			continue;
		}
		if((fp = fopen(path, "r")) != NULL) // 둘 다 가능하면
		{
			while(fgets(buf, 256, fp) != NULL)
			{
				char *token = strtok(buf, " \t\n");
				if(token != NULL && strcmp(token, "Uid:") == 0) // Uid 데이터를 찾고 찾았으면,
				{
					token = strtok(NULL, " \t\n");
					if(token != NULL && strtoul(token, NULL, 10) == (unsigned long)uid)
					{
						if(pid_count == pid_capacity)
						{
							size_t next_capacity = pid_capacity ? pid_capacity * 2 : 64;
							int *next = realloc(pids, next_capacity * sizeof(*next));
							if(next == NULL)
							{
								perror("realloc() error : ");
								free(pids);
								fclose(fp);
								closedir(directory);
								return;
							}
							pids = next;
							pid_capacity = next_capacity;
						}
						pids[pid_count++] = candidate_pid;
					}
					break;
				}
			}
			fclose(fp);
		}
	}
	closedir(directory);
	for(size_t pid_index = 0; pid_index < pid_count; pid_index++)
	{
		if(show_fds)
		{
			printf("## Attribute : FILEDES, Target Process ID : %d ##\n", pids[pid_index]);
			print_process_fds(pids[pid_index]);
		}
		if(show_cmdline)
		{
			printf("## Attribute : CMDLINE, Target Process ID : %d ##\n", pids[pid_index]);
			print_process_cmdline(pids[pid_index]);
		}
		if(show_io)
		{
			printf("## Attribute : IO, Target Process ID : %d ##\n", pids[pid_index]);
			print_process_io(pids[pid_index]);
		}
		if(show_status)
		{
			printf("## Attribute : STAT, Target Process ID : %d ##\n", pids[pid_index]);
			print_process_status(pids[pid_index]);
		}
		if(show_environ)
		{
			printf("## Attribute : _ENVIRON, Target Process ID : %d ##\n", pids[pid_index]);
			string_list empty_keywords = {0};
			print_process_environ(pids[pid_index], empty_keywords);
		}
	}
	free(pids);
}
