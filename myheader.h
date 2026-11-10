#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <dirent.h>
#include <fcntl.h>
#include <limits.h>

#define MAX_PID_ARGUMENTS 16
#define MAX_SORT_ARGUMENTS 16

#define FLAG_F 1
#define FLAG_T 2
#define FLAG_C 4
#define FLAG_N 8
#define FLAG_M 16
#define FLAG_K 32
#define FLAG_W 64
#define FLAG_E 128
#define FLAG_L 256
#define FLAG_V 512
#define FLAG_R 1024
#define FLAG_S 2048
#define FLAG_O 4096

typedef struct string_list{
	int count;
	/* Parsed lists own this array, but borrow the strings from argv. */
	char **items;
}string_list;

typedef struct program_options{
	int flags;
	string_list fd_pids;
	string_list status_pids;
	string_list cmdline_pids;
	string_list io_pids;
	string_list environ_pids;
	string_list keywords;
	string_list attributes;
	string_list output_paths;
}program_options;


void print_process_fds(int pid);
void print_process_status(int pid);
void print_process_cmdline(int pid);
void print_process_io(int pid);
void print_process_environ(int pid, string_list keywords);
void print_interrupts(void);
void print_filesystems(void);
void print_uptime(void);
void print_kernel_version(void);
void print_user_process_attributes(string_list attributes);
int parse_pid(const char *text);

void execute_options(program_options opts);
