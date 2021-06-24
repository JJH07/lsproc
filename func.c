#include "myheader.h"

void option_f(int fPID) //f옵션 처리하는 함수입니다.
{
	struct dirent *dirp; 
	DIR *dp;
	char path[512];

	sprintf(path, "/proc/%d/fd", fPID);
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
	if((dp = opendir(path)) == NULL)
	{
		printf("opendir() error\n");
		return ;
	}
	while((dirp = readdir(dp)) != NULL)
	{
		char linkname[256];
		if(strcmp(dirp->d_name, ".") == 0 || strcmp(dirp->d_name, "..") == 0)
		{
			continue;
		}
		strcpy(ptr, dirp->d_name);
		readlink(path, linkname, 256);
		printf("File Descriptor number: %s, Opened File: %s\n", dirp->d_name, linkname);
	}
	if(closedir(dp) < 0)
	{
		printf("closedir() error\n");
	}
}


void option_t(int tPID) //t옵션을 처리하는 함수입니다.
{	
	FILE *fp;
	char path[512] = "/proc/";
	char buf[256];

	sprintf(path, "/proc/%d/status", tPID);
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
	if((fp = fopen(path, "r")) == NULL)
	{
		printf("fopen() error\n");
		return ;
	}
	for(int c = 0; c < 9; c++)
	{
		fgets(buf, 256, fp);
		char *token = strtok(buf, " \t\n");
		printf("%s ", token);
		if(strcmp(token, "State:") == 0)
		{
			token = strtok(NULL, " \t\n");
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
		}
		else
		{
			token = strtok(NULL, " \t\n");
			printf("%s\n", token);
		}
	}
	fclose(fp);
}


void option_c(int cPID)//c옵션 처리하는 함수입니다.
{
	FILE *fp;
	char path[512] = "/proc/";
	char buf[256];

	sprintf(path, "/proc/%d/cmdline", cPID);
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
	if((fp = fopen(path, "r")) == NULL)
	{
		printf("fopen() error\n");
		return ;
	}
	int cnt = 0;
	while(fscanf(fp, "%s", buf) != EOF)
	{
		printf("argv[%d] : %s\n", cnt++, buf);
	}
	fclose(fp);
}


void option_n(int nPID) //n옵션 처리하는 함수입니다.
{
	FILE *fp;
	char path[512] = "/proc/";
	char buf[256];

	sprintf(path, "/proc/%d/io", nPID);
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
	if((fp = fopen(path, "r")) == NULL)
	{
		printf("fopen() error\n");
		return ;
	}
	while(fgets(buf, 256, fp) != NULL)
	{
		char *token = strtok(buf, " \t\n");
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
		printf("%s\n", token);
	}
	fclose(fp);
}


void option_m(int mPID, namelist key) //m옵션 처리하는 함수입니다.
{
	char path[512] = "/proc/";
	char sys_arg[1024];

	sprintf(path, "/proc/%d/environ", mPID);
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
	if(key.count == 0)
	{
		sprintf(sys_arg, "cat %s | tr '\\000' '\n'", path);
		system(sys_arg);
	}
	else
	{
		for(int kidx = 0; kidx < key.count; kidx++)
		{
			sprintf(sys_arg, "cat %s | tr '\\000' '\n' | grep %s", path, key.name[kidx]);
			system(sys_arg);
		}
	}
}


void option_w() //w옵션 처리하는 함수입니다.
{
	FILE *fp;
	char path[] = "/proc/interrupts"; // 경로 설정하고
	char buf[256], *token;

	int cpu_count = 0;
	
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
	if((fp = fopen(path, "r")) == NULL)
	{
		printf("fopen() error\n");
		return ;
	}

	fgets(buf, 256, fp);
	token = strtok(buf, " \t\n");
	while(token != NULL)
	{
		cpu_count++;
		token = strtok(NULL, " \t\n");
	}
	printf("---Number of CPUs : %d---\n      [Average] : [Description]\n", cpu_count);

	while(fgets(buf, 256, fp) != NULL)
	{
		char name[64];
		long long sum = 0;

		token = strtok(buf, " :\t\n");
		if(token[0] >= '0' && token[0] <= '9')
		{
			continue;
		}
		strcpy(name, token);
		for(int i = 0; i < cpu_count; i++)
		{
			token = strtok(NULL, " \t\n");
			if(token == NULL)
			{
				break;
			}
			sum += atoi(token);
		}
		printf("%15.3lf : <%s> ", 1.0 * sum / cpu_count, name);
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
	fclose(fp);
}


void option_e() //e옵션 처리하는 함수입니다.
{
	FILE *fp;
	char path[] = "/proc/filesystems";
	char buf[256];
	int counting = 0;

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
	if((fp = fopen(path, "r")) == NULL)
	{
		printf("fopen() error\n");
		return ;
	}
	printf("<<Supported Filesystems>>\n");
	while(fgets(buf, 256, fp) != NULL)
	{
		char *Token = strtok(buf, " \t\n");
		if(strcmp(Token, "nodev") == 0)
		{
			continue;
		}
		if(counting == 5)
		{
			counting = 0;
			printf("\n");
		}
		if(counting > 0)
		{
			printf(", ");
		}
		printf("%s", Token);
		counting++;
	}
	printf("\n");
	fclose(fp);
}


void option_l() //l옵션 처리하는 함수입니다.
{
	FILE *fp;
	char path[] = "/proc/uptime"; //경로 설정합니다.
	char buf[256], *Token;

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
	if((fp = fopen(path, "r")) == NULL)
	{
		printf("fopen() error\n");
		return ;
	}
	fgets(buf, 256, fp);

	Token = strtok(buf, " \t");
	printf("Process worked time : %s(sec)\n", Token);
	Token = strtok(NULL, " \t\n");
	printf("Process idle time : %s(sec)\n", Token);
	fclose(fp);
}


void option_v() //v옵션 처리하는 함수입니다.
{
	system("cat /proc/version");
}

void option_s(namelist Attributes)
{
	uid_t uid = getuid();

	DIR *dp;
	FILE *fp;

	struct dirent *dirp;
	struct stat statbuf;

	char path[512];
	char buf[256];

	int pid[1024], p_count = 0;
	int _FILEDES = 0, _CMDLINE = 0, _IO = 0, _STAT = 0, _ENVIRON = 0;

	for(int i = 0; i < Attributes.count; i++)
	{
		if(strcmp(Attributes.name[i], "FILEDES") == 0)
		{
			_FILEDES = 1;
		}
		if(strcmp(Attributes.name[i], "CMDLINE") == 0)
		{
			_CMDLINE = 1;
		}
		if(strcmp(Attributes.name[i], "IO") == 0)
		{
			_IO = 1;
		}
		if(strcmp(Attributes.name[i], "STAT") == 0)
		{
			_STAT = 1;
		}
		if(strcmp(Attributes.name[i], "ENVIRON") == 0)
		{
			_ENVIRON = 1;
		}
	}

	if((dp = opendir("/proc")) == NULL)
	{
		printf("opendir() error\n");
	}
	while((dirp = readdir(dp)) != NULL)
	{
		if(strcmp(dirp->d_name, ".") == 0 || strcmp(dirp->d_name, "..") == 0)
		{
			continue;
		}		
		sprintf(path, "/proc/%s", dirp->d_name);
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
				char *Token = strtok(buf, " \t\n");
				if((strcmp(Token, "Uid:") == 0)) // Uid 데이터를 찾고 찾았으면,
				{
					Token = strtok(NULL, " \t\n");
					if(atoi(Token) == uid)
					{
						pid[p_count++] = atoi(dirp->d_name);
					}
					break;
				}
			}
			fclose(fp);
		}
	}
	closedir(dp);
	for(int pc = 0; pc < p_count; pc++)
	{
		if(_FILEDES)
		{
			printf("## Attribute : FILEDES, Target Process ID : %d ##\n", pid[pc]);
			option_f(pid[pc]);
		}
		if(_CMDLINE)
		{
			printf("## Attribute : CMDLINE, Target Process ID : %d ##\n", pid[pc]);
			option_c(pid[pc]);
		}
		if(_IO)
		{
			printf("## Attribute : IO, Target Process ID : %d ##\n", pid[pc]);
			option_n(pid[pc]);
		}
		if(_STAT)
		{
			printf("## Attribute : STAT, Target Process ID : %d ##\n", pid[pc]);
			option_t(pid[pc]);
		}
		if(_ENVIRON)
		{
			printf("## Attribute : _ENVIRON, Target Process ID : %d ##\n", pid[pc]);
			namelist temp;
			option_m(pid[pc], temp);
		}
	}
}