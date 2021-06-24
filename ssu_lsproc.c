#include "myheader.h"

void ssu_lsproc(int argc, char *argv[], options opt)
{
	pid_t pid;
	int fd = -1;
	int status;

	if(opt.bit & FLAG_o)
	{
		if((fd = open(opt.Redirect.name[0], O_RDWR | O_CREAT | O_TRUNC, 0644)) == -1)
		{
			perror(opt.Redirect.name[0]);
			return ;
		}
		dup2(fd, 1);
	}

	printf(">: ssu_lsproc start.:<\n");
	fflush(stdout);
	for(int i = 0; i <= 11; i++)
	{
		int bit = 1 << i;
		if((opt.bit & bit) == 0)
		{
			continue;
		}
		if((pid = fork()) < 0)
		{ 
			perror("fork() error : ");
			exit(0);
		}
		else if(pid == 0)
		{
			switch(bit)
			{
				case FLAG_f :
				{
					if(opt.fPID.count == 0)
					{
						sprintf(opt.fPID.name[opt.fPID.count++], "%d", getpid());
					}
					for(int fidx = 0; fidx < opt.fPID.count; fidx++)
					{
						if(opt.fPID.count > 1)
						{
							printf("([/proc/%s/fd])\n", opt.fPID.name[fidx]);
						}
						if(fidx < 16)
						{
							option_f(atoi(opt.fPID.name[fidx]));
						}
						else
						{
							printf("Maximum Number of Argument Exceeded. :: %s\n", opt.fPID.name[fidx]);
						}
					}
					break;
				}
				case FLAG_t :
				{
					if(opt.tPID.count == 0)
					{
						sprintf(opt.tPID.name[opt.tPID.count++], "%d", getpid());
					}
					for(int tidx = 0; tidx < opt.tPID.count; tidx++)
					{
						if(opt.tPID.count > 1)
						{
							printf("([/proc/%s/fd])\n", opt.tPID.name[tidx]);
						}
						if(tidx < 16)
						{
							option_t(atoi(opt.tPID.name[tidx]));
						}
						else
						{
							printf("Maximum Number of Argument Exceeded. :: %s\n", opt.tPID.name[tidx]);
						}
					}
					break;
				}
				case FLAG_c :
				{
					if(opt.cPID.count == 0)
					{
						sprintf(opt.cPID.name[opt.cPID.count++], "%d", getpid());
					}
					for(int cidx = 0; cidx < opt.cPID.count; cidx++)
					{
						if(opt.cPID.count > 1)
						{
							printf("([/proc/%s/fd])\n", opt.cPID.name[cidx]);
						}
						if(cidx < 16)
						{
							option_c(atoi(opt.cPID.name[cidx]));
						}
						else
						{
							printf("Maximum Number of Argument Exceeded. :: %s\n", opt.cPID.name[cidx]);
						}
					}
					break;
				}
				case FLAG_n :
				{
					if(opt.nPID.count == 0)
					{
						sprintf(opt.nPID.name[opt.nPID.count++], "%d", getpid());
					}
					for(int nidx = 0; nidx < opt.nPID.count; nidx++)
					{
						if(opt.nPID.count > 1)
						{
							printf("([/proc/%s/fd])\n", opt.nPID.name[nidx]);
						}
						if(nidx < 16)
						{
							option_n(atoi(opt.nPID.name[nidx]));
						}
						else
						{
							printf("Maximum Number of Argument Exceeded. :: %s\n", opt.nPID.name[nidx]);
						}
					}
					break;
				}
				case FLAG_m :
				{
					if(opt.mPID.count == 0)
					{
						sprintf(opt.mPID.name[opt.mPID.count++], "%d", getpid());
					}
					for(int midx = 0; midx < opt.mPID.count; midx++)
					{
						if(opt.mPID.count > 1)
						{
							printf("([/proc/%s/fd])\n", opt.mPID.name[midx]);
						}
						if(midx < 16)
						{
							option_m(atoi(opt.mPID.name[midx]), opt.key);
						}
						else
						{
							printf("Maximum Number of Argument Exceeded. :: %s\n", opt.mPID.name[midx]);
						}
					}
					break;
				}
				case FLAG_w :
				{
					option_w();
					break;
				}
				case FLAG_e :
				{
					option_e();
					break;
				}
				case FLAG_l :
				{
					option_l();
					break;
				}
				case FLAG_v :
				{
					option_v();
					break;
				}
				case FLAG_s :
				{
					option_s(opt.Attributes);
					break;
				}
			}
			exit(0);
		}
		else
		{
			if(wait(&status) < 0) // 자식프로세스 종료 기다립니다.
			{
				printf("wait error\n");
			}
		}
	}
	printf(">: ssu_lsproc terminated.:<\n");

	if(fd > -1)
	{
		close(fd);
	}
}