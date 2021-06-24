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

#define FLAG_f 1
#define FLAG_t 2
#define FLAG_c 4
#define FLAG_n 8
#define FLAG_m 16
#define FLAG_k 32
#define FLAG_w 64
#define FLAG_e 128
#define FLAG_l 256
#define FLAG_v 512
#define FLAG_r 1024
#define FLAG_s 2048
#define FLAG_o 4096

typedef struct names{
	int count;
	char name[64][256];
}namelist;

typedef struct opt_table{
	int bit;
	namelist fPID;
	namelist tPID;
	namelist cPID;
	namelist nPID;
	namelist mPID;
	namelist key;
	namelist Attributes;
	namelist Redirect;
}options;


void option_f(int fPID);
void option_t(int tPID);
void option_c(int cPID);
void option_n(int nPID);
void option_m(int mPID, namelist key);
void option_w();
void option_e();
void option_l();
void option_v();
void option_s(namelist Attributes);