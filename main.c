#include "myheader.h"

void parse(int argc, char *argv[], options *opt);
int compareFuncPID(const void *a, const void *b);
int compareFuncKEY(const void *a, const void *b);
void ssu_lsproc(int argc, char *argv[], options opt); // ssu_lsproc 함수입니다.

void print_option(options *opt);

int main(int argc, char *argv[])
{
	options option;
	memset(&option, 0, sizeof(options));

	parse(argc, argv, &option);
	
	ssu_lsproc(argc, argv, option);

	return 0;
}

void parse(int argc, char *argv[], options *opt)
{
	int idx = 1;
	while(idx < argc)
	{
		namelist *list = NULL;
		switch(argv[idx++][1])
		{
			case 'f':
			{
				opt->bit |= FLAG_f;
				list = &opt->fPID;
				break;
			}
			case 't':
			{
				opt->bit |= FLAG_t;
				list = &opt->tPID;
				break;
			}
			case 'c':
			{
				opt->bit |= FLAG_c;
				list = &opt->cPID;
				break;
			}
			case 'n':
			{
				opt->bit |= FLAG_n;
				list = &opt->nPID;
				break;
			}
			case 'm':
			{
				opt->bit |= FLAG_m;
				list = &opt->mPID;
				break;
			}
			case 'k':
			{
				opt->bit |= FLAG_k;
				list = &opt->key;
				break;
			}
			case 'w':
			{
				opt->bit |= FLAG_w;
				break;
			}
			case 'e':
			{
				opt->bit |= FLAG_e;
				break;
			}
			case 'l':
			{
				opt->bit |= FLAG_l;
				break;
			}
			case 'v':
			{
				opt->bit |= FLAG_v;
				break;
			}
			case 'r':
			{
				opt->bit |= FLAG_r;
				break;
			}
			case 's':
			{
				opt->bit |= FLAG_s;
				list = &opt->Attributes;
				break;
			}
			case 'o':
			{
				opt->bit |= FLAG_o;
				list = &opt->Redirect;
				break;
			}
		}
		if(list != NULL)
		{
			list->count = 0;
			while(idx < argc)
			{
				if(argv[idx][0] == '-')
				{
					break;
				}
				strcpy(list->name[list->count++], argv[idx]);
				++idx;
			}
		}
	}
	if(opt->bit & FLAG_r)
	{
		qsort(opt->fPID.name, (opt->fPID.count < 16 ? opt->fPID.count : 16), sizeof(opt->fPID.name[0]), compareFuncPID);
		qsort(opt->tPID.name, (opt->tPID.count < 16 ? opt->tPID.count : 16), sizeof(opt->tPID.name[0]), compareFuncPID);
		qsort(opt->cPID.name, (opt->cPID.count < 16 ? opt->cPID.count : 16), sizeof(opt->cPID.name[0]), compareFuncPID);
		qsort(opt->nPID.name, (opt->nPID.count < 16 ? opt->nPID.count : 16), sizeof(opt->nPID.name[0]), compareFuncPID);
		qsort(opt->mPID.name, (opt->mPID.count < 16 ? opt->mPID.count : 16), sizeof(opt->mPID.name[0]), compareFuncPID);
		qsort(opt->key.name, (opt->key.count < 16 ? opt->key.count : 16), sizeof(opt->key.name[0]), compareFuncKEY);
	}
}

int compareFuncPID(const void *a, const void *b)
{
	return atoi((char *)a) > atoi((char *)b);
}
int compareFuncKEY(const void *a, const void *b)
{
	return strcmp((char *)a, (char *)b);
}

void print_option(options *opt)
{
	printf("%d %d\n", opt->bit & FLAG_f, opt->fPID.count);
	for(int i = 0; i < opt->fPID.count; i++)
	{
		printf("%s ", opt->fPID.name[i]);
	}
	printf("\n%d %d\n", opt->bit & FLAG_t, opt->tPID.count);;
	for(int i = 0; i < opt->tPID.count; i++)
	{
		printf("%s ", opt->tPID.name[i]);
	}
	printf("\n%d %d\n", opt->bit & FLAG_c, opt->cPID.count);
	for(int i = 0; i < opt->cPID.count; i++)
	{
		printf("%s ", opt->cPID.name[i]);
	}
	printf("\n%d %d\n", opt->bit & FLAG_n, opt->nPID.count);
	for(int i = 0; i < opt->nPID.count; i++)
	{
		printf("%s ", opt->nPID.name[i]);
	}
	printf("\n%d %d\n", opt->bit & FLAG_m, opt->mPID.count);
	for(int i = 0; i < opt->mPID.count; i++)
	{
		printf("%s ", opt->mPID.name[i]);
	}
	printf("\n%d\n", opt->bit & FLAG_k);
	printf("%d\n", opt->bit & FLAG_w);
	printf("%d\n", opt->bit & FLAG_e);
	printf("%d\n", opt->bit & FLAG_l);
	printf("%d\n", opt->bit & FLAG_v);
	printf("%d\n", opt->bit & FLAG_r);
	printf("%d\n", opt->bit & FLAG_s);
}
