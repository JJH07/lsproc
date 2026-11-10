#include "myheader.h"

static int parse_options(int argc, char *argv[], program_options *opt);
static void free_options(program_options *opt);
static void sort_pid_arguments(string_list *pids);
static int compare_pid_strings(const void *a, const void *b);
static int compare_keywords(const void *a, const void *b);

void print_options_debug(program_options *opt);

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

static int parse_options(int argc, char *argv[], program_options *opt)
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
				opt->flags |= FLAG_F;
				list = &opt->fd_pids;
				break;
			}
			case 't':
			{
				opt->flags |= FLAG_T;
				list = &opt->status_pids;
				break;
			}
			case 'c':
			{
				opt->flags |= FLAG_C;
				list = &opt->cmdline_pids;
				break;
			}
			case 'n':
			{
				opt->flags |= FLAG_N;
				list = &opt->io_pids;
				break;
			}
			case 'm':
			{
				opt->flags |= FLAG_M;
				list = &opt->environ_pids;
				break;
			}
			case 'k':
			{
				opt->flags |= FLAG_K;
				list = &opt->keywords;
				break;
			}
			case 'w':
			{
				opt->flags |= FLAG_W;
				break;
			}
			case 'e':
			{
				opt->flags |= FLAG_E;
				break;
			}
			case 'l':
			{
				opt->flags |= FLAG_L;
				break;
			}
			case 'v':
			{
				opt->flags |= FLAG_V;
				break;
			}
			case 'r':
			{
				opt->flags |= FLAG_R;
				break;
			}
			case 's':
			{
				opt->flags |= FLAG_S;
				list = &opt->attributes;
				break;
			}
			case 'o':
			{
				opt->flags |= FLAG_O;
				list = &opt->output_paths;
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
	if(opt->flags & FLAG_R)
	{
		sort_pid_arguments(&opt->fd_pids);
		sort_pid_arguments(&opt->status_pids);
		sort_pid_arguments(&opt->cmdline_pids);
		sort_pid_arguments(&opt->io_pids);
		sort_pid_arguments(&opt->environ_pids);
		if(opt->keywords.count > 1)
		{
			int count = opt->keywords.count < MAX_SORT_ARGUMENTS ? opt->keywords.count : MAX_SORT_ARGUMENTS;
			qsort(opt->keywords.items, count, sizeof(opt->keywords.items[0]), compare_keywords);
		}
	}
	return 0;
}

/* The lists own their pointer arrays, but never the argv strings. */
static void free_options(program_options *opt)
{
	string_list *lists[] = {
		&opt->fd_pids, &opt->status_pids, &opt->cmdline_pids,
		&opt->io_pids, &opt->environ_pids, &opt->keywords,
		&opt->attributes, &opt->output_paths
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

void print_options_debug(program_options *opt)
{
	printf("%d %d\n", opt->flags & FLAG_F, opt->fd_pids.count);
	for(int i = 0; i < opt->fd_pids.count; i++)
	{
		printf("%s ", opt->fd_pids.items[i]);
	}
	printf("\n%d %d\n", opt->flags & FLAG_T, opt->status_pids.count);;
	for(int i = 0; i < opt->status_pids.count; i++)
	{
		printf("%s ", opt->status_pids.items[i]);
	}
	printf("\n%d %d\n", opt->flags & FLAG_C, opt->cmdline_pids.count);
	for(int i = 0; i < opt->cmdline_pids.count; i++)
	{
		printf("%s ", opt->cmdline_pids.items[i]);
	}
	printf("\n%d %d\n", opt->flags & FLAG_N, opt->io_pids.count);
	for(int i = 0; i < opt->io_pids.count; i++)
	{
		printf("%s ", opt->io_pids.items[i]);
	}
	printf("\n%d %d\n", opt->flags & FLAG_M, opt->environ_pids.count);
	for(int i = 0; i < opt->environ_pids.count; i++)
	{
		printf("%s ", opt->environ_pids.items[i]);
	}
	printf("\n%d\n", opt->flags & FLAG_K);
	printf("%d\n", opt->flags & FLAG_W);
	printf("%d\n", opt->flags & FLAG_E);
	printf("%d\n", opt->flags & FLAG_L);
	printf("%d\n", opt->flags & FLAG_V);
	printf("%d\n", opt->flags & FLAG_R);
	printf("%d\n", opt->flags & FLAG_S);
}
