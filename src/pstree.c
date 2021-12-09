#include<stdio.h>
#include<string.h>
#include<assert.h>
#include<stdlib.h>
#include<sched.h>
#include<dirent.h>

#define LENGTH 101
#define PROC 1
#define THREAD 2
unsigned int _p = 0, _n = 0, _V = 0;

typedef struct Procblock{
	unsigned int Pid, PPid, Type, thread;
	char* Name;
	unsigned childNum;
	struct Procblock* children[LENGTH];
	struct Procblock* parent;
} procblock;

unsigned int Num = 0;
procblock* head[LENGTH];
procblock* root;

void test(int argc, char **argv) {
	int test = 0;
	for(int i = 1; i < argc; ++i) {
		if(strcmp(argv[i], "-test") == 0) 
			test = 1;
	}
	if(test){
		printf("Test pstree, argc = %d \n", argc);
		for(int i = 0; i < argc; ++i){
			printf("argv[%d]: %s\n", i, argv[i]);
		}
		exit(0);
	}
}
void argInit(int argc, char **argv) {
	for(int i = 1; i < argc; ++i){
		if(strcmp(argv[i], "-V") == 0 || strcmp(argv[i], "--version") == 0)
			_V = 1;
		if(strcmp(argv[i], "-p") == 0 || strcmp(argv[i], "--show-pids") == 0)
			_p = 1;
		if(strcmp(argv[i], "-n") == 0 || strcmp(argv[i], "--numeric-sort") == 0)
			_n = 1;
	}
	if(_V == 1) {
		printf("The pstree with AsilenceBTF\n\n");
		printf("If you have any suggestions for this very clumsy implementation, please contact me!\n");
		exit(0);
	}
}


void prinHead() {
	for(int i = 0; i < Num; ++i) {
		printf("ProName = %s, Pid = %u, PPid = %u \n", head[i]->Name, head[i]->Pid, head[i]->PPid);
	}
}

unsigned int cTou(char *c) {
	unsigned int arr[LENGTH], n = 0, res = 0;
	for(; *c != '\n' && *c != '\0'; c++) {
		if(*c >= '0' && *c <= '9')
		arr[n++] = *c - '0';
	}
	for(int i = 0; i < n; ++i) {
		res = res * 10 + arr[i];
	}
	return res;
}
void procAdd(char *status, unsigned int Type, unsigned int PPid) {
	procblock *proc;
	proc = (procblock *)malloc(sizeof(procblock));
	proc->Type = Type;
	proc->childNum = 0;
	FILE *fp = NULL;
	fp = fopen(status, "r");
	char buff[255];
	while(fgets(buff, 255, (FILE*)fp) != NULL) {
		char tmp[5];
		strncpy(tmp, buff, 4);
		if(strcmp(tmp, "Name") == 0) {
			proc->Name = malloc(strlen(buff + 6) + 1);
			strcpy(proc->Name, buff + 6);
			char *c = proc->Name;
			while(*c != '\n') c++;
			*c = '\0';
		} else if(strcmp(tmp, "Pid:") == 0) {
			proc->Pid = cTou(buff + 5);
		} else if(strcmp(tmp, "PPid") == 0) {
			proc->PPid = cTou(buff + 6);
		}
	}
	fclose(fp);
	for(int i = 0; i < Num; ++i) {
		if(proc->Pid == head[i]->Pid)
			return;
	}	
	if(PPid != 0)
			proc->thread = PPid;
	head[Num++] = proc;
}
void proInit(char * path, unsigned int Type){
	DIR *dir = opendir(path); // 遍历procfile
	if(dir == NULL) return;
	assert(dir);
	struct dirent* p = NULL;
	char tmp[LENGTH];
	while((p = readdir(dir))!= NULL){
		if(p->d_name[0] >= '0' && p->d_name[0] <= '9') { //
			strcpy(tmp, path);
			strcat(tmp, "/");
			strcat(tmp, p->d_name);
			strcat(tmp, "/status");
			if(Type == 2) {
				procAdd(tmp, Type, cTou(path));
			}	else {
				procAdd(tmp, Type, 0);
			}
			
			strcpy(tmp, path);
			strcat(tmp, "/");
			strcat(tmp, p->d_name);
			strcat(tmp, "/task");
			proInit(tmp, 2);
		}
	}
	closedir(dir);
}

void treeInit() {
	// get ps-tree root 
	for(int i = 0; i < Num; ++i) {
		if(head[i]->Pid == 1) {
			root = head[i];
		}
	}
	for(int i = 0; i < Num; ++i) {
			for(int j = 0; j < Num; ++j) {
			if(i == j) continue;	
			if(head[j]->thread == head[i]->Pid) {
				head[j]->Name = head[i]->Name;
				head[j]->PPid = head[i]->Pid;
			}
		}
	}	
	for(int i = 0; i < Num; ++i) {
			for(int j = 0; j < Num; ++j) {
			if(i == j) continue;	
			if(head[j]->PPid == head[i]->Pid) {
				head[j]->parent = head[i];
				head[i]->children[head[i]->childNum++] = head[j];
			}
		}
	}
}

int vis[LENGTH];
void printTree(procblock *proc, int y) {
	if(proc->Type == 1) {
		printf("%s", proc->Name);
		if(_p == 1) {
			printf("(%d)", proc->Pid);
		} 
	}
	else {
		if(_p == 0 && _n == 1) {
			int tmp = 0;
			for(int j = 0; j < proc->parent->childNum; ++j) {
				if(proc->Name == proc->parent->children[j]->Name) 
					tmp++;
			}
			if(tmp > 1) {
				printf("%d[{%s}]", tmp, proc->Name);
			} else {
				printf("{%s}", proc->Name);
			}
			printf("\n");
			return;
		} else {
			printf("{%s}", proc->Name);
			if(_p == 1) {
				printf("(%d)", proc->Pid);
			}
		}
	}
	printf("\n");
	y += 3;
	vis[y] = 1;
	for(int i = 0; i < proc->childNum; ++i) {
		if(proc->children[i]->Type == 2 && _p == 0 && _n == 1) {
			int flag = 0;
			for(int j = 0; j < i; ++j) {
				if(proc->children[i]->Name == proc->children[j]->Name)
					flag = 1;
			}
			if(flag) 
				continue;
		}
		for(int i = 0; i < y; ++i) printf("%c", vis[i] ? '|' : ' ');
		if(i + 1 < proc->childNum){
			printf("├─");
		}
		else {
			vis[y] = 0;
			printf("└─");
		}
		printTree(proc->children[i], y);
	}
}
int main(int argc, char *argv[]){
	test(argc, argv);
	argInit(argc, argv);
	proInit("/proc", 1);
	// prinHead();
	treeInit();
	printTree(root, 0);
	return 0;
}



