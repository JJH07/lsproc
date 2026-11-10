#include "myheader.h"

static int parse_options(int argc, char *argv[], program_options *opts);
static void free_options(program_options *opts);
static void sort_pid_arguments(string_list *pids);
static int compare_pid_strings(const void *a, const void *b);
static int compare_keywords(const void *a, const void *b);

void print_options_debug(program_options *opts);

int main(int argc, char *argv[])
{
	program_options opts;
	memset(&opts, 0, sizeof(program_options));

	int result = EXIT_FAILURE;
	int parse_result = parse_options(argc, argv, &opts);
	
	if(parse_result == 0)
		result = execute_options(opts);
	free_options(&opts);

	return result;
}

static int parse_options(int argc, char *argv[], program_options *opts)
{
	int arg_index = 1;
	while(arg_index < argc)
	{
		string_list *list = NULL;
		const char *argument = argv[arg_index];
		if(argument[0] != '-' || argument[1] == '\0' || argument[2] != '\0' ||
		   strchr("ftcnmkwelvrso", argument[1]) == NULL)
		{
			fprintf(stderr, "Invalid option: %s\n", argument);
			return -1;
		}
		switch(argv[arg_index++][1])
		{
			case 'f':
			{
				opts->flags |= FLAG_F;
				list = &opts->fd_pids;
				break;
			}
			case 't':
			{
				opts->flags |= FLAG_T;
				list = &opts->status_pids;
				break;
			}
			case 'c':
			{
				opts->flags |= FLAG_C;
				list = &opts->cmdline_pids;
				break;
			}
			case 'n':
			{
				opts->flags |= FLAG_N;
				list = &opts->io_pids;
				break;
			}
			case 'm':
			{
				opts->flags |= FLAG_M;
				list = &opts->environ_pids;
				break;
			}
			case 'k':
			{
				opts->flags |= FLAG_K;
				list = &opts->keywords;
				break;
			}
			case 'w':
			{
				opts->flags |= FLAG_W;
				break;
			}
			case 'e':
			{
				opts->flags |= FLAG_E;
				break;
			}
			case 'l':
			{
				opts->flags |= FLAG_L;
				break;
			}
			case 'v':
			{
				opts->flags |= FLAG_V;
				break;
			}
			case 'r':
			{
				opts->flags |= FLAG_R;
				break;
			}
			case 's':
			{
				opts->flags |= FLAG_S;
				list = &opts->attributes;
				break;
			}
			case 'o':
			{
				opts->flags |= FLAG_O;
				list = &opts->output_paths;
				break;
			}
		}
		if(list != NULL)
		{
			list->count = 0;
			while(arg_index < argc)
			{
				if(argv[arg_index][0] == '-')
				{
					break;
				}
				char **items = realloc(list->items, ((size_t)list->count + 1) * sizeof(*items));
				if(items == NULL)
				{
					perror("realloc() error : ");
					return -1;
				}
				list->items = items;
				list->items[list->count++] = argv[arg_index];
				++arg_index;
			}
		}
		if(argument[1] == 'o' && (list->count != 1 || list->items[0][0] == '\0'))
		{
			fprintf(stderr, "Option -o requires exactly one output file.\n");
			return -1;
		}
		if((argument[1] == 'k' || argument[1] == 's') && list->count == 0)
		{
			fprintf(stderr, "Option -%c requires an argument.\n", argument[1]);
			return -1;
		}

	}
	if(opts->flags & FLAG_R)
	{
		sort_pid_arguments(&opts->fd_pids);
		sort_pid_arguments(&opts->status_pids);
		sort_pid_arguments(&opts->cmdline_pids);
		sort_pid_arguments(&opts->io_pids);
		sort_pid_arguments(&opts->environ_pids);
		if(opts->keywords.count > 1)
		{
			int count = opts->keywords.count < MAX_SORT_ARGUMENTS ? opts->keywords.count : MAX_SORT_ARGUMENTS;
			qsort(opts->keywords.items, count, sizeof(opts->keywords.items[0]), compare_keywords);
		}
	}
	return 0;
}

/* The lists own their pointer arrays, but never the argv strings. */
static void free_options(program_options *opts)
{
	string_list *lists[] = {
		&opts->fd_pids, &opts->status_pids, &opts->cmdline_pids,
		&opts->io_pids, &opts->environ_pids, &opts->keywords,
		&opts->attributes, &opts->output_paths
	};
	for(size_t i = 0; i < sizeof(lists) / sizeof(lists[0]); ++i)
	{
		free(lists[i]->items);
		lists[i]->items = NULL;
		lists[i]->count = 0;
	}
}

static void sort_pid_arguments(string_list *pids)
{
	if(pids->count > 1)
	{
		int count = pids->count < MAX_SORT_ARGUMENTS ? pids->count : MAX_SORT_ARGUMENTS;
		qsort(pids->items, count, sizeof(pids->items[0]), compare_pid_strings);
	}
}

int parse_pid(const char *text)
{
	char *end;
	errno = 0;
	long value = strtol(text, &end, 10);
	if(errno == ERANGE || end == text || *end != '\0' || value < 0 || value > INT_MAX)
		return 0;
	return (int)value;
}

static int compare_pid_strings(const void *a, const void *b)
{
	int left = parse_pid(*(char *const *)a);
	int right = parse_pid(*(char *const *)b);
	return (left > right) - (left < right);
}
static int compare_keywords(const void *a, const void *b)
{
	return strcmp(*(char *const *)a, *(char *const *)b);
}

void print_options_debug(program_options *opts)
{
	printf("%d %d\n", opts->flags & FLAG_F, opts->fd_pids.count);
	for(int i = 0; i < opts->fd_pids.count; i++)
	{
		printf("%s ", opts->fd_pids.items[i]);
	}
	printf("\n%d %d\n", opts->flags & FLAG_T, opts->status_pids.count);;
	for(int i = 0; i < opts->status_pids.count; i++)
	{
		printf("%s ", opts->status_pids.items[i]);
	}
	printf("\n%d %d\n", opts->flags & FLAG_C, opts->cmdline_pids.count);
	for(int i = 0; i < opts->cmdline_pids.count; i++)
	{
		printf("%s ", opts->cmdline_pids.items[i]);
	}
	printf("\n%d %d\n", opts->flags & FLAG_N, opts->io_pids.count);
	for(int i = 0; i < opts->io_pids.count; i++)
	{
		printf("%s ", opts->io_pids.items[i]);
	}
	printf("\n%d %d\n", opts->flags & FLAG_M, opts->environ_pids.count);
	for(int i = 0; i < opts->environ_pids.count; i++)
	{
		printf("%s ", opts->environ_pids.items[i]);
	}
	printf("\n%d\n", opts->flags & FLAG_K);
	printf("%d\n", opts->flags & FLAG_W);
	printf("%d\n", opts->flags & FLAG_E);
	printf("%d\n", opts->flags & FLAG_L);
	printf("%d\n", opts->flags & FLAG_V);
	printf("%d\n", opts->flags & FLAG_R);
	printf("%d\n", opts->flags & FLAG_S);
}
