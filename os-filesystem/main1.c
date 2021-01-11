#include "filesystem.h"

char *myvhard;							   // 虚拟盘块起始地址
struct USEROPEN openfilelist[MAXOPENFILE]; //用户打开文件表数据
int curdir;								   //当前目录文件描述符fd
char currentdir[80];					   //当前目录的目录名（包括目录路径）
char *startp;							   //数据区的起始位置
char *starti;							   //i节点区的起始位置

void startsys();
void my_format();
void my_cd(char *dirname);
void my_mkdir(char *dirname);
void my_rmdir(char *dirname);
void my_ls();
int my_create(char *filename);
void my_rm(char *filename);
int my_open(char *filename);
void my_close();
int my_write(int fd);
int my_do_write(int fd, char *text, int len, char wstyle);
int my_read(int fd, int len);
int my_do_read(int fd, int len, char *text);
void my_exitsys();
void copytouser(int i, struct iNode *p, struct FCB *r);
void getParentInode(char *dir);
void show_openlist();

int main()
{
	system("chcp 65001");
	curdir = 0;
	strcpy(currentdir, "/");
	startsys();
	system("pause");
	// printf("------\n");
	while (1)
	{
		printf("%s>", currentdir);
		char input[1005];
		fgets(input, sizeof(input), stdin);
		char cmd[50][100];
		char *p = strtok(input, " ");
		int i = 0;
		while (p)
		{
			strcpy(cmd[i], p);
			i++;
			p = strtok(NULL, " ");
		}
		for (int j = 0; cmd[i - 1][j] != '\0'; j++)
		{
			if (cmd[i - 1][j] == '\n')
			{
				cmd[i - 1][j] = '\0';
				break;
			}
		}
		if (strcmp(cmd[0], "my_exitsys") == 0)
		{
			my_exitsys();
			break;
		}
		else if (strcmp(cmd[0], "my_create") == 0)
		{
			printf("开始创建文件\n");
			my_create(cmd[1]);
		}
		else if (strcmp(cmd[0], "my_rm") == 0)
		{
			my_rm(cmd[1]);
		}
		else if (strcmp(cmd[0], "my_ls") == 0)
		{
			my_ls();
		}
		else if (strcmp(cmd[0], "my_write") == 0)
		{
			int fd = my_open(cmd[1]);
			if (fd == -1)
			{
				printf("打开失败\n");
				continue;
			}
			my_write(fd);
		}
		else if (strcmp(cmd[0], "my_open") == 0)
		{
			int fd = my_open(cmd[1]);
			if (fd == -1)
			{
				printf("打开失败\n");
			}
			printf("%d\n", fd);
		}
		else if (strcmp(cmd[0], "my_read") == 0)
		{
			int fd = my_open(cmd[1]);
			if (fd == -1)
			{
				printf("打开失败\n");
			}
			char text[MAXBUFFSIZE + 1];
			printf("<< %u >>\n", openfilelist[fd].off);
			int len = 0;
			unsigned short off = 0;
			printf(">>>");
			scanf("%u %d", &off, &len);
			openfilelist[fd].off = off;
			printf("<< %u >>\n", openfilelist[fd].off);
			my_do_read(fd, len, text);
			text[len] = '\0';
			printf("------------------\n");
			printf("%s\n", text);
			printf("------------------\n");
		}
		else if (strcmp(cmd[0], "my_op") == 0)
		{
			for (int i = 0; i < MAXOPENFILE; i++)
			{
				printf("filename:%s\n", openfilelist[i].filename);
				printf("filesize:%d\n", openfilelist[i].filesize);
				printf("dir:%s\n", openfilelist[i].dir);
			}
		}
		else if (strcmp(cmd[0], "show") == 0)
		{
			show_openlist();
		}
		else if (strcmp(cmd[0], "my_mkdir") == 0)
		{
			my_mkdir(cmd[1]);
		}
		else if (strcmp(cmd[0], "my_rmdir") == 0)
		{
			my_rmdir(cmd[1]);
		}
		else if (strcmp(cmd[0], "my_cd") == 0)
		{
			my_cd(cmd[1]);
		}
		else if (strcmp(cmd[0], "\n") != 0)
		{
			printf("命令不存在!\n");
		}
	}
	system("pause");
	return 0;
}

void startsys()
{
	myvhard = (char *)malloc(SIZE);
	memset(myvhard, 0, SIZE);
	startp = myvhard + 18 * BLOCKSIZE;
	starti = myvhard + 2 * BLOCKSIZE;
	FILE *fp;
	//int i;
	//printf("-------\n");
	fp = fopen(SYS_PATH, "r");
	if (fp == NULL)
	{
		//fclose(fp);
		printf("System is not init,now we will install it and create filesystem.\n");
		printf("Please not exit.\n");
		my_format();
		//printf("1111\n");
	}
	else
	{
		fclose(fp);
	}
	fp = fopen(SYS_PATH, "r");
	//fwrite(myvhard, SIZE, 1, fp);
	fread(myvhard, SIZE, 1, fp);
	fclose(fp);
	struct FCB *r = (struct FCB *)(myvhard + 18 * BLOCKSIZE);
	copytouser(0, INODE_OST, r);
	struct iNode *p = (struct iNode *)(myvhard + 2 * BLOCKSIZE);
	LocalTime(p->creattime);
	LocalTime(p->fixtime);
	printf("filesize:%u\n", p->filesize);
	printf("%s\n", r->filename);
	printf("%s\n", openfilelist[0].dir);
	r = (struct FCB *)(myvhard + 18 * BLOCKSIZE);
	printf("%s\n", r->filename);
	printf("id:%d\n", r->id);
}

void my_format()
{
	FILE *fp = fopen(SYS_PATH, "w");
	struct BLOCK0 block;
	strcpy(block.information, "1024,1024");
	block.root = 19;
	block.startblock = startp;
	unsigned int pgraph[32];
	struct iNode *inode;
	memset(pgraph, 0, sizeof(pgraph));
	for (int i = 31; i >= 11; i--)
	{
		pgraph[0] |= (1 << i);
	}
	//pgraph[0]=1;
	struct FCB fcb;
	strcpy(fcb.filename, "/");
	fcb.free = 1;
	fcb.id = 0;
	inode = (struct iNode *)starti;
	inode->count = 1;
	time_t t = time(NULL);
	inode->creattime = time(&t);
	inode->filesize = 0;
	inode->filetype[0] = '0';
	inode->filetype[1] = '3';
	inode->fixtime = inode->creattime;
	inode->number = 20;
	unsigned short index[2] = {21, 65535};
	struct FCB initFCB[64];
	struct FCB init = {"\0", 65535, 0};
	for (int i = 0; i < 64; i++)
	{
		initFCB[i] = init;
	}
	/*strcpy(fcb[1].filename, "usr");
	fcb[1].free = 0;
	fcb[1].id = 1;
	inode = (struct iNode *)(starti + sizeof(struct iNode));
	inode->count = 1;
	inode->creattime = time(&t);
	inode->filesize = 0;
	inode->filetype[0] = '0';
	inode->filetype[1] = '3';
	inode->fixtime = inode->creattime;
	inode->number = 21;
	strcpy(fcb[2].filename, "var");
	fcb[2].free = 0;
	fcb[2].id = 2;
	inode = (struct iNode *)(starti + 2 * sizeof(struct iNode));
	inode->count = 1;
	inode->creattime = time(&t);
	inode->filesize = 0;
	inode->filetype[0] = '0';
	inode->filetype[1] = '3';
	inode->fixtime = inode->creattime;
	inode->number = 22;*/
	fwrite(&block, BLOCKSIZE, 1, fp);
	fwrite(pgraph, BLOCKSIZE, 1, fp);
	fwrite(starti, 16 * BLOCKSIZE, 1, fp);
	fwrite(&fcb, BLOCKSIZE, 1, fp);
	fwrite(index, BLOCKSIZE, 1, fp);
	fwrite(initFCB, BLOCKSIZE, 1, fp);
	fclose(fp);
	// strcmp(openfilelist[0].name,currentdir);
	// openfilelist[0].filesize=0;
	// openfilelist[0].number=20;
	// openfilelist[0].filetype=0;
	// openfilelist[0].count=1;
	// openfilelist[0].createtime=
	return;
}

void my_exitsys()
{
	printf("bye\n");
	FILE *fp = fopen(SYS_PATH, "w");
	if (fp == NULL)
	{
		printf("保存失败\n");
		return;
	}
	fwrite(myvhard, SIZE, 1, fp);
	fclose(fp);
	printf("文件系统已保存!\n");
	free(myvhard);
}

void copytouser(int i, struct iNode *p, struct FCB *r)
{
	//struct iNode *p = INODE_OST + i;
	//struct FCB* r = ROOT_OST + i;
	strcpy(openfilelist[i].filename, r->filename);
	strcpy(openfilelist[i].filetype, p->filetype);
	openfilelist[i].count = p->count;
	openfilelist[i].filesize = p->filesize;
	openfilelist[i].number = p->number;
	printf("number:%u\n", openfilelist[i].number);
	openfilelist[i].creattime = p->creattime;
	openfilelist[i].fixtime = p->fixtime;
	strcpy(openfilelist[i].dir, currentdir);
	/*
	if (p->filetype[0] == '0') {
		char temp[50];
		strcpy(temp, r->filename);
		strcpy(openfilelist[i].dir, strcat(temp, "/"));
	}*/
	openfilelist[i].off = 0;
	openfilelist[i].fcbstate = 0;
	openfilelist[i].topenfile = 1;
}

unsigned short getFreeAddr()
{
	unsigned int *freebit = PGRAPH_OST;
	int flag, x, y;
	flag = 0;
	for (unsigned short i = 0; i < 32; i++)
	{
		for (unsigned short j = 0; j < 32; j++)
		{
			int b = freebit[i] >> (31 - j) & 1;
			if (b == 0)
			{
				x = i + 1;
				y = j + 1;
				freebit[i] |= (1 << (31 - j));
				flag = 1;
				break;
			}
		}
		if (flag)
			break;
	}
	if (!flag)
	{
		return 65535;
	}
	return (x - 1) * 32 + y;
}

void freeAddr(unsigned short number)
{
	unsigned int *pgraph = PGRAPH_OST;
	unsigned int x, y;
	x = number / 32;
	y = (number % 32) - 1;
	if (y == -1)
	{
		y = 31;
		x -= 1;
	}
	if (x >= 32 || x < 0)
	{
		printf("释放空间失败\n");
		return;
	}
	pgraph[x] &= ~(1 << (31 - y)); //释放文件所占盘块
}

void my_ls()
{
	unsigned int first = openfilelist[curdir].number;		   //当前目录盘块号
	char *start = (char *)(myvhard + (first - 1) * BLOCKSIZE); //当前目录盘块号起始地址,放的是FCB表盘块号
	unsigned short *index = (unsigned short *)start;		   //取FCB盘块号
	while (*index != 65535)
	{
		char *start2 = (char *)(myvhard + (*index - 1) * BLOCKSIZE); //FCB地址
		struct FCB *fcb = (struct FCB *)start2;						 //取FCB
		for (int i = 0; i < 64; i++)
		{
			if ((fcb + i)->free == 1)
			{
				printf("文件名：%s\n", (fcb + i)->filename);
				struct iNode *inode = INODE_OST + (fcb + i)->id; //取i节点编号地址
				printf("文件大小：%u\n", inode->filesize);
				printf("文件创建时间：%u\n", inode->creattime);
				LocalTime(inode->creattime);
				printf("文件最近修改时间：%u\n", inode->fixtime);
				LocalTime(inode->fixtime);
				printf("-----------------\n");
			}
		}
		index += 1;
		system("pause");
	}
}
int my_open(char *filename)
{
	printf("打开文件\n");
	if (strlen(filename) > 13)
	{
		printf("文件名过长!\n");
		return -1;
	}
	//先遍历用户打开表
	int useropen_free = -1;
	for (int i = 0; i < MAXOPENFILE; i++)
	{
		if (openfilelist[i].topenfile)
		{
			if (strcmp(currentdir, openfilelist[i].dir) == 0 && strcmp(filename, openfilelist[i].filename) == 0)
			{
				return i;
			}
		}
		else
		{
			if (useropen_free == -1)
			{
				useropen_free = i;
			}
		}
	}
	//用户打开表不存在则遍历磁盘块
	unsigned short *indexFirst = (unsigned short *)(myvhard + (openfilelist[curdir].number - 1) * BLOCKSIZE);
	struct FCB *fcb = NULL;
	while (*indexFirst != 65535)
	{
		fcb = (struct FCB *)(myvhard + (*indexFirst - 1) * BLOCKSIZE);
		int i;
		for (i = 0; i < 64; i++)
		{
			if ((strcmp(fcb->filename, filename) == 0) && fcb->free)
			{
				break;
			}
			fcb++;
		}
		if (i < 64)
			break;
		indexFirst++;
	}
	unsigned short id = fcb->id;
	struct iNode *inode = INODE_OST + id;
	copytouser(useropen_free, inode, fcb);
	return useropen_free;
}
int my_create(char *filename)
{
	if (sizeof(filename) > 13)
	{
		printf("文件名过长!\n");
		return -1;
	}
	int useropen_free = -1;
	int rename = 0;
	//分配空闲用户打开表项
	//可以预先查找用户打开表，若有重名则不用查找磁盘
	int i;
	for (i = 0; i < MAXOPENFILE; i++)
	{
		if (!openfilelist[i].topenfile)
		{
			if (useropen_free == -1)
			{
				useropen_free = i;
			}
		}
		else
		{
			if (strcmp(currentdir, openfilelist[i].dir) == 0 && strcmp(filename, openfilelist[i].filename) == 0)
			{
				rename = 1;
			}
		}
	}
	if (rename)
	{
		printf("文件名已存在!\n");
		return -1;
	}
	if (useropen_free == -1)
	{
		printf("用户文件打开数超过上限!\n");
		return -1;
	}
	int index = openfilelist[curdir].number;
	printf("%d\n", index);
	unsigned short *indexFirst = (unsigned short *)(myvhard + (index - 1) * BLOCKSIZE);
	struct FCB *fcb_free = NULL;
	int first = 1;
	while (*indexFirst != 65535)
	{
		printf("%u\n", *indexFirst);
		struct FCB *fcb = (struct FCB *)(myvhard + (*indexFirst - 1) * BLOCKSIZE);
		for (int i = 0; i < 64; i++)
		{
			if (!fcb->free)
			{
				if (first)
				{
					fcb_free = fcb;
					first = 0;
				}
			}
			else
			{
				if (strcmp(fcb->filename, filename) == 0)
				{
					rename = 1;
				}
			}
			if (fcb_free != NULL && rename)
				break;
			fcb++;
		}
		if (i < 64)
			break;
		indexFirst++;
	}
	if (rename)
	{
		printf("文件名已存在!\n");
		return -1;
	}
	if (fcb_free == NULL)
	{
		if ((openfilelist[curdir].filesize / 16) % 64 == 0 && (openfilelist[curdir].filesize / 16) / 64 <= 20)
		{
			unsigned short number = getFreeAddr();
			if (number == 65535)
			{
				printf("父目录文件已达上限!\n");
			}
			else
			{
				*indexFirst = number;
				*(indexFirst + 1) = 65535;
				fcb_free = (struct FCB *)(myvhard + (number - 1) * BLOCKSIZE);
			}
		}
	}
	printf("-------------\n");
	strcpy(fcb_free->filename, filename);
	fcb_free->free = 1;
	struct iNode *p = INODE_OST;
	unsigned short count = 0;
	while (count < 1024)
	{
		if (p->count == 0)
		{
			break;
		}
		count++;
		p++;
	}
	fcb_free->id = count;
	printf("id:%u\n", count);
	p->count = 1;
	time_t t = time(NULL);
	p->creattime = time(&t);
	LocalTime(p->creattime);
	p->fixtime = p->creattime;
	p->filesize = 0;
	p->filetype[0] = '1';
	p->filetype[1] = '3';
	unsigned int *freebit = PGRAPH_OST;
	unsigned short n1, n2;
	n1 = getFreeAddr();
	n2 = getFreeAddr();
	if (n1 == 65535 || n2 == 65535)
	{
		printf("磁盘空间不足!创建失败\n");
		if (n1 != 65535)
			freeAddr(n1);
		if (n2 != 65535)
			freeAddr(n2);
		return -1;
	}
	p->number = n1;
	printf("number:%u\n", p->number);
	indexFirst = (unsigned short *)(myvhard + (p->number - 1) * BLOCKSIZE);
	printf("number:%u\n", n2);
	*indexFirst = n2;
	*(indexFirst + 1) = 65535;
	copytouser(useropen_free, p, fcb_free);
	char *data = myvhard + (*indexFirst - 1) * BLOCKSIZE;
	data[0] = EOF;
	//修改父目录信息
	char *q = strtok(currentdir, "/");
	printf("curdir:%d\n", curdir);
	printf("currentdir:%s\n", currentdir);
	struct iNode *pdir = INODE_OST;
	pdir->fixtime = p->creattime;
	//pdir->filesize += p->filesize;
	while (q)
	{
		indexFirst = (unsigned short *)(myvhard + (pdir->number - 1) * BLOCKSIZE);
		struct FCB *fcb = NULL;
		printf("dirname:%s\n", q);
		while (*indexFirst != 65535)
		{
			printf("11111\n");
			printf("index:%u\n", *indexFirst);
			fcb = (struct FCB *)(myvhard + (*indexFirst - 1) * BLOCKSIZE);
			for (int i = 0; i < 64; i++)
			{
				if ((strcmp(fcb->filename, q) == 0) && fcb->free)
				{
					break;
				}
				fcb++;
			}
			indexFirst++;
		}
		pdir = INODE_OST + fcb->id;
		pdir->fixtime = p->creattime;
		//pdir->filesize += p->filesize;
		q = strtok(NULL, "/");
	}
	pdir->filesize += 16;
	return useropen_free;
}

void my_rm(char *filename)
{
	unsigned short indexnum = openfilelist[curdir].number;							  //索引表块号
	unsigned short *index = (unsigned short *)(myvhard + (indexnum - 1) * BLOCKSIZE); //索引表指针
	unsigned short id;
	while (*index != 65535)
	{
		int i = 0;
		struct FCB *fcb = (struct FCB *)(myvhard + (*index - 1) * BLOCKSIZE);
		for (i = 0; i < 64; i++)
		{
			if (fcb->free && strcmp(fcb->filename, filename) == 0)
			{
				fcb->free = 0;
				id = fcb->id;
				break;
			}
			fcb++;
		}
		if (i < 64)
			break;
		index++;
	}
	if (*index == 65535)
	{
		printf("该文件不存在\n");
		return;
	}
	struct iNode *inode = INODE_OST + id;
	inode->count--;
	time_t t = time(NULL);
	inode->fixtime = time(&t);
	printf("文件已删除\n");
	int x, y;
	unsigned int *pgraph = PGRAPH_OST;
	if (inode->count == 0)
	{
		index = (unsigned short *)(myvhard + (inode->number - 1) * BLOCKSIZE);
		/*x = inode->number / 32;
		y = inode->number % 32 - 1;
		if (y == -1) {
			y = 31;
			x -= 1;
		}
		if (x >= 32 || x < 0) {
			printf("释放空间失败!\n");
			return;
		}
		pgraph[x] &= ~(1 << (31 - y));//释放索引表*/
		freeAddr(inode->number);
		while (*index != 65535)
		{
			/*	x = *index / 32;
			y = (*index % 32) - 1;
			if (y == -1) {
				y = 31;
				x -= 1;
			}
			pgraph[x] &= ~(1 << (31 - y));//释放文件所占盘块*/
			freeAddr(*index);
			index++;
		}
	}
	//修改父目录信息以及调整父目录的磁盘块
	char *q = strtok(currentdir, "/");
	struct iNode *pdir = INODE_OST;
	pdir->fixtime = inode->fixtime;
	//pdir->filesize -= inode->filesize;
	while (q)
	{
		index = (unsigned short *)(myvhard + (pdir->number - 1) * BLOCKSIZE);
		struct FCB *fcb = NULL;
		while (*index != 65535)
		{
			fcb = (struct FCB *)(myvhard + (*index - 1) * BLOCKSIZE);
			int i = 0;
			for (i = 0; i < 64; i++)
			{
				if ((strcmp(fcb->filename, q) == 0) && fcb->free)
				{
					break;
				}
				fcb++;
			}
			if (i < 64)
				break;
		}
		pdir = INODE_OST + fcb->id;
		pdir->fixtime = inode->fixtime;
		//pdir->filesize -= inode->filesize;
		q = strtok(NULL, "/");
	}
	unsigned short *indexFirst = (unsigned short *)(myvhard + (pdir->number - 1) * BLOCKSIZE);
	int flags[512] = {0};
	int i = 0;
	while (indexFirst[i] != 65535)
	{
		int j = 0;
		struct FCB *fcb = (struct FCB *)(myvhard + (*indexFirst - 1) * BLOCKSIZE);
		for (j = 0; j < 64; j++)
		{
			if (fcb[j].free == 1)
			{
				break;
			}
		}
		if (j >= 64)
			flags[i] = 1;
		i++;
	}
	flags[i] = -1;
	i = 0;
	while (flags[i] != -1)
	{
		if (flags[i] == 1)
		{
			freeAddr(indexFirst[i]);
			indexFirst[i] = 65535;
			flags[i] = -1;
		}
		else if (flags[i] == 0)
		{
			if (i != 0)
			{
				int j = i - 1;
				while (flags[j] != 0 || j == 0)
					j--;
				if (j == 0)
				{
					indexFirst[j] = indexFirst[i];
					flags[j] = 0;
					if (flags[0] != 0)
					{
						indexFirst[i] = 65535;
						flags[i] = -1;
					}
				}
				else
				{
					indexFirst[j + 1] = indexFirst[i];
					flags[j + 1] = 0;
					if (j + 1 != i)
					{
						indexFirst[i] = 65535;
						flags[i] = -1;
					}
				}
			}
		}
		i++;
	}
}

int my_write(int fd)
{
	char buff[MAXBUFFSIZE];
	memset(buff, '\0', MAXBUFFSIZE);
	int i = 0;
	if (fd == -1)
	{
		printf("文件不存在!\n");
		return -1;
	}
	printf("截断写:0，覆盖写:1，追加写:2，退出:Ctrl+Z");
	printf("请选择指令:");
	char wstyle;
	scanf("%c", &wstyle);
	unsigned short *index = (unsigned short *)(myvhard + (openfilelist[fd].number - 1) * BLOCKSIZE);
	//读写指针修改
	if (wstyle == '0')
	{
		openfilelist[fd].count = 0 * BLOCKSIZE + 0;
	}
	else if (wstyle == '1')
	{
	}
	else if (wstyle == '2')
	{
		/*
		int off = 0;
		while (index[off++] != -1);
		off -= 2;
		char * readp = (myvhard + (index[off] - 1)*BLOCKSIZE);
		off *= BLOCKSIZE;
		while (*readp != EOF) {
			readp++;
			off++;
		}
		printf("off:%d\n", off);
		*/
		if (openfilelist[fd].filesize > 0)
			openfilelist[fd].off = openfilelist[fd].filesize - 1;
		else
			openfilelist[fd].off = 0;
	}
	else if (wstyle == EOF)
	{
		printf("退出编写模式!\n");
		return 0;
	}
	unsigned short filesize = openfilelist[fd].filesize;
	getchar();
	i = 0;
	while ((buff[i++] = getchar()) != EOF)
	{
		if (filesize + i > MAXBUFFSIZE)
		{
			printf("文件大小已达上限!\n");
			break;
		}
	}
	int size = strlen(buff);
	printf("%c", buff[0]);
	printf("%c你好", buff[size - 2]);
	buff[size - 2] = EOF;
	buff[size - 1] = '\0';
	printf("%c你好", buff[size - 2]);
	char s[100] = {'1', '2', '3', '\n', '\0'};
	s[3] = EOF;
	printf("%s", s);
	printf("%d\n", strlen(s));
	printf("buff:%d\n", strlen(buff));
	return my_do_write(fd, buff, strlen(buff), wstyle); //实际写
}

int my_do_write(int fd, char *text, int len, char wstyle)
{
	//重新分配、回收数据磁盘块
	int e_blocknum = len / BLOCKSIZE;
	int e_rest = len % BLOCKSIZE;
	int r_blocknum = 0;
	int r_rest = openfilelist[fd].filesize % BLOCKSIZE;
	int lastIndex = 0; //记录文件索引表最后一项的索引
	printf("%d\n", openfilelist[fd].number - 1);
	unsigned short *indexFirst = (unsigned short *)(myvhard + (openfilelist[fd].number - 1) * BLOCKSIZE);
	while (indexFirst[lastIndex] != 65535)
		lastIndex++;
	r_blocknum = lastIndex;
	int index = openfilelist[fd].off / BLOCKSIZE;
	int offset = openfilelist[fd].off % BLOCKSIZE;
	unsigned short filesize = openfilelist[fd].filesize;
	int m;
	int n = 0;
	printf("当前索引表:");
	while (indexFirst[n] != 65535)
	{
		printf("%u ", indexFirst[n]);
		n++;
	}
	printf("\n");
	if (wstyle == '0')
	{
		if (e_rest != 0)
			e_blocknum += 1;
		int n = e_blocknum - r_blocknum;
		int k;
		unsigned short addrId;
		if (n > 0)
		{
			for (k = 0; k < n; k++)
			{
				addrId = getFreeAddr();
				if (addrId == 65535)
				{
					printf("磁盘空间不足!申请失败\n");
					//return -1;
					break;
				}
				indexFirst[lastIndex] = addrId;
				lastIndex++;
				//这里不需要判断索引表是否已达上限，因为文件最大占用磁盘块数为20
			}
			indexFirst[lastIndex] = 65535;
		}
		else if (n < 0)
		{
			n = -n;
			for (k = 0; k < n; k++)
			{
				lastIndex--;
				unsigned short num = indexFirst[lastIndex];
				printf("free:%d\n", num);
				freeAddr(num);
			}
			indexFirst[lastIndex] = 65535;
		}
	}
	else if (wstyle == '1')
	{
		unsigned short addrId;
		r_blocknum = r_blocknum - index - 1;
		r_rest = BLOCKSIZE - offset;
		m = e_blocknum - r_blocknum;
		int restn = e_rest - r_rest;
		int k = 0;
		if (m > 0)
		{
			if (restn > 0)
			{
				m += 1;
			}
			for (k = 0; k < m; k++)
			{
				addrId = getFreeAddr();
				if (addrId == 65535)
				{
					printf("磁盘空间不足!申请失败\n");
					//return -1;
					break;
				}
				indexFirst[lastIndex] = addrId;
				lastIndex++;
				//这里不需要判断索引表是否已达上限，因为文件最大占用磁盘块数为20
			}
			indexFirst[lastIndex] = 65535;
		}
		else if (m < 0)
		{
			text[len - 1] = '\0';
			/*for (k = 0; k < m; k++) {
				lastIndex--;
				unsigned short num = indexFirst[lastIndex];
				freeAddr(num);
			}
			indexFirst[lastIndex] = -1;*/
		}
		else if (m == 0)
		{
			if (restn > 0)
			{
				addrId = getFreeAddr();
				if (addrId == 65535)
				{
					printf("磁盘空间不足!创建失败\n");
					//return -1;
				}
				else
				{
					indexFirst[lastIndex] = addrId;
					indexFirst[++lastIndex] = 65535;
				}
			}
			else if (e_rest < 0)
			{
				text[len - 1] = '\0';
			}
		}
		if (e_rest)
		{
			if (restn > 0)
			{
				e_blocknum += 2;
			}
			else
			{
				e_blocknum += 1;
			}
		}
	}
	else if (wstyle == '2')
	{
		int k = 0;
		r_rest = BLOCKSIZE - offset;
		printf("%u\n", r_rest);
		if (e_rest != 0)
			e_blocknum += 1;
		int restn = e_rest - r_rest;
		unsigned short addrId;
		if (restn > 0)
		{
			e_blocknum += 1;
		}
		for (k = 0; k < e_blocknum - 1; k++)
		{
			addrId = getFreeAddr();
			if (addrId == 65535)
			{
				printf("磁盘空间不足!创建失败\n");
				//return -1;
				break;
			}
			indexFirst[lastIndex] = addrId;
			lastIndex++;
		}
		indexFirst[lastIndex] = 65535;
	}
	printf("当前索引表:");
	n = 0;
	while (indexFirst[n] != 65535)
	{
		printf("%u ", indexFirst[n]);
		n++;
	}
	printf("\n");
	//如果是覆盖写且原来空间足够，则需要将缓冲区中的文件结束符去除
	//写磁盘
	char *buffFirst = (char *)malloc(BLOCKSIZE * sizeof(char));
	memset(buffFirst, '\0', BLOCKSIZE);
	int i = 0;
	//注意写数据不能用
	int loopnum = (lastIndex - index) < e_blocknum ? (lastIndex - index) : e_blocknum;
	printf("loopnum:%d\n", loopnum);
	char *p = text;
	char *datap;
	unsigned short q = 0;
	unsigned short readp = openfilelist[fd].off;
	printf("readp:%u\n", readp);
	while (i < loopnum)
	{
		unsigned short textSize = strlen(p);
		unsigned short writeSize = textSize < BLOCKSIZE ? textSize : BLOCKSIZE;
		unsigned short writeSize1;
		printf("buffwritesize:%u\n", writeSize);
		memcpy(&buffFirst[q], p, writeSize);
		p += writeSize;
		while (q < writeSize)
		{
			index = readp / BLOCKSIZE;
			offset = readp % BLOCKSIZE;
			textSize = writeSize - q;
			writeSize1 = textSize < (BLOCKSIZE - offset) ? textSize : (BLOCKSIZE - offset);
			datap = (myvhard + (indexFirst[index] - 1) * BLOCKSIZE) + offset;
			printf("writeIndex:%d offset:%d\n", indexFirst[index], offset);
			memcpy(datap, &buffFirst[q], writeSize1);
			q += writeSize1;
			//openfilelist[fd].off += writeSize;
			printf("blockwritesize:%u\n", writeSize1);
			printf("writedata:%s\n", datap);
			readp += writeSize1;
		}
		i++;
		q = 0;
	}
	//修改自身信息以及父节点信息
	if (wstyle == '1')
		openfilelist[fd].filesize = readp > filesize ? readp : filesize;
	else
		openfilelist[fd].filesize = readp;
	time_t t = time(NULL);
	unsigned int fixtime = time(&t);
	char dir[100];
	strcpy(dir, currentdir);
	char *dirq = strtok(strcat(dir, openfilelist[fd].filename), "/");
	struct iNode *pdir = INODE_OST;
	pdir->fixtime = fixtime;
	//pdir->filesize = pdir->filesize - filesize + openfilelist[fd].filesize;
	unsigned short *indexp;
	while (dirq)
	{
		indexp = (unsigned short *)(myvhard + (pdir->number - 1) * BLOCKSIZE);
		struct FCB *fcb = NULL;
		while (*indexp != 65535)
		{
			fcb = (struct FCB *)(myvhard + (*indexp - 1) * BLOCKSIZE);
			int i = 0;
			for (i = 0; i < 64; i++)
			{
				if ((strcmp(fcb->filename, dirq) == 0) && fcb->free)
				{
					break;
				}
				fcb++;
			}
			if (i < 64)
				break;
		}
		pdir = INODE_OST + fcb->id;
		pdir->fixtime = fixtime;
		//pdir->filesize = pdir->filesize - filesize + openfilelist[fd].filesize;
		dirq = strtok(NULL, "/");
	}
	if ((readp - openfilelist[fd].off) == 0 && len > 0)
		return -1;
	printf("writecount:%d\n", readp - openfilelist[fd].off);
	return readp - openfilelist[fd].off;
}

int my_do_read(int fd, int len, char *text)
{
	unsigned short off = openfilelist[fd].off;
	int e_blocknum = off / BLOCKSIZE; // 开始读取位置所在盘块
	int e_rest = off % BLOCKSIZE;	  // 开始读取位置盘块偏移

	len = min(len, MAXBUFFSIZE);
	len = min(len, openfilelist[fd].filesize);
	int n_blocknum = (len + off) / BLOCKSIZE; // 结束读取位置所在盘块
	int n_rest = (len + off) % BLOCKSIZE;	  // 结束读取位置盘块偏移

	char buf[BLOCKSIZE + 1];
	int lenth = 0;
	unsigned short *indexFirst = (unsigned short *)(myvhard + (openfilelist[fd].number - 1) * BLOCKSIZE); // 找到文件索引表起始地址
	char *curChar;
	for (int i = e_blocknum; i < n_blocknum; i++)
	{
		if (i == e_blocknum)
		{
			// 当前指针在第一块要读的盘块中
			// 将指针到该盘块末尾复制到text中
			curChar = (indexFirst[e_blocknum] - 1) * BLOCKSIZE + myvhard + e_rest;
			memcpy(buf, curChar, (BLOCKSIZE - e_rest) * sizeof(char));
			memcpy(text + lenth, buf, (BLOCKSIZE - e_rest) * sizeof(char));
			lenth += BLOCKSIZE - e_rest;
			buf[BLOCKSIZE - e_rest] = '\0';
			printf("<1>-------------------\n");
			printf("%s\n", buf);
			printf("<1>-------------------\n");
		}
		else
		{
			// 当前指针在盘块起始位置
			// 将整块复制到text中
			curChar = (indexFirst[i] - 1) * BLOCKSIZE + myvhard;
			memcpy(buf, curChar, BLOCKSIZE * sizeof(char));
			memcpy(text + lenth, buf, BLOCKSIZE * sizeof(char));
			lenth += BLOCKSIZE;
			buf[BLOCKSIZE] = '\0';
			printf("<2>-------------------\n");
			printf("%s\n", buf);
			printf("<2>-------------------\n");
		}
	}
	if (e_blocknum == n_blocknum)
	{
		// 如果第一块和最后一块为同一块
		curChar = (indexFirst[e_blocknum] - 1) * BLOCKSIZE + myvhard + e_rest;
		memcpy(buf, curChar, (n_rest - e_rest + 1) * sizeof(char));
		memcpy(text + lenth, buf, (n_rest - e_rest + 1) * sizeof(char));
		lenth += n_rest - e_rest + 1;
		buf[n_rest - e_rest + 1] = '\0';
		printf("<3>-------------------\n");
		printf("%s\n", buf);
		printf("<3>-------------------\n");
	}
	else
	{
		// 最后一块，仅读到n_rest为止
		curChar = (indexFirst[n_blocknum] - 1) * BLOCKSIZE + myvhard;
		memcpy(buf, curChar, (n_rest + 1) * sizeof(char));
		memcpy(text + lenth, buf, (n_rest + 1) * sizeof(char));
		lenth += n_rest + 1;
		buf[n_rest + 1] = '\0';
		printf("<4>-------------------\n");
		printf("%s\n", buf);
		printf("<4>-------------------\n");
	}
	printf("readlen: %d\n", lenth);
}

void close_openlist(char *name1)
{
	for (int i = 0; i < MAXOPENFILE; i++)
	{
		if (openfilelist[i].topenfile == 1)
		{
			char name[105] = "";
			strcpy(name, openfilelist[i].dir);
			strcat(name, "/");
			strcat(name, openfilelist[i].filename);
			if (strcmp(name, name1) == 0)
			{
				openfilelist[i].topenfile = 0;
				return;
			}
		}
	}
}

void show_openlist()
{
	for (int i = 0; i < MAXOPENFILE; i++)
	{
		if (openfilelist[i].topenfile == 1)
		{
			if (strcmp(openfilelist[i].dir, "/") != 0)
				printf("%s/%s\n", openfilelist[i].dir, openfilelist[i].filename);
			else if (strcmp(openfilelist[i].filename, "/") != 0)
				printf("%s%s\n", openfilelist[i].dir, openfilelist[i].filename);
			else
				printf("/\n");
		}
	}
}
/*绝对路径不支持./..*/
void my_cd(char *dirname)
{
	//绝对路径,一步步打开

	if (dirname[0] == '/')
	{
		char *q = strtok(currentdir, "/");
		char qq[105] = "/";
		while (q) //先全关了
		{
			strcat(qq, q);
			close_openlist(qq);
			strcat(qq, "/");
			q = strtok(NULL, "/");
		}

		curdir = 0;
		strcpy(currentdir, "/");
		char *q1 = strtok(dirname, "/");
		char qq1[105] = "/";
		while (q1)
		{
			strcat(qq1, q1);
			int num = -1;
			for (int i = 0; i < MAXOPENFILE; i++)
			{
				if (openfilelist[i].topenfile == 0)
				{
					num = i;
					break;
				}
			}
			if (num == -1)
			{
				printf("文件打开太多，打开失败\n");
			}
			unsigned short now = openfilelist[curdir].number;
			char *start = (char *)myvhard + (now - 1) * BLOCKSIZE;
			unsigned short *index = (unsigned short *)start; //取FCB盘块号
			int flag = 0;
			while (*index != 65535)
			{
				char *start2 = (char *)(myvhard + (*index - 1) * BLOCKSIZE); //FCB地址
				struct FCB *fcb = (struct FCB *)start2;						 //取FCB
				for (int i = 0; i < 64; i++)
				{
					if ((fcb + i)->free == 1)
					{
						//printf("文件名：%s\n", (fcb + i)->filename);
						if (strcmp((fcb + i)->filename, q1) == 0)
						{
							struct iNode *inode = INODE_OST + (fcb + i)->id; //取i节点地址
							if (inode->filetype[0] == '2')					 //必须得是目录文件
							{
								copytouser(num, inode, fcb + i);
								curdir = num;
								strcpy(currentdir, qq1);
								strcat(qq1, "/");
								flag = 1;
								break;
							}
						}
					}
				}
				if (flag)
					break;
				index += 1;
			}
			if (flag == 0)
			{
				printf("没有这个路径！\n");
				return;
			}
			q1 = strtok(NULL, "/");
		}
	}
	else //相对路径
	{
		char *q = strtok(dirname, "/");
		char qq[105] = "";
		strcpy(qq, currentdir);
		if (strcmp(qq, "/") != 0)
			strcat(qq, "/");
		while (q)
		{
			int num = -1;
			if (strcmp(q, ".") != 0)
			{
				strcat(qq, q);
			}

			if (strcmp(q, "..") == 0)
			{
				int len = strlen(currentdir);
				int len2 = 0;
				for (int i = len - 1; i >= 0; i--)
				{
					if (currentdir[i] == '/')
					{
						len2 = i;
						break;
					}
				}
				if (len2 == 0)
				{
					strcpy(qq, "/");
					//printf("%s\n",qq);
				}
				else
				{
					strcpy(qq, currentdir);
					qq[len2] = '\0';
				}
				close_openlist(currentdir);
				num = -2;
			}

			if (strcmp(q, ".") != 0 && strcmp(q, "..") != 0)
			{
				num = -1;
				for (int i = 0; i < MAXOPENFILE; i++)
				{
					if (openfilelist[i].topenfile == 0)
					{
						num = i;
						break;
					}
				}
				if (num == -1)
				{
					printf("文件打开太多，打开失败\n");
					return;
				}
			}
			unsigned short now = openfilelist[curdir].number;
			char *start = (char *)myvhard + (now - 1) * BLOCKSIZE;
			unsigned short *index = (unsigned short *)start; //取FCB盘块号
			int flag = 0;
			while (*index != 65535)
			{
				char *start2 = (char *)(myvhard + (*index - 1) * BLOCKSIZE); //FCB地址
				struct FCB *fcb = (struct FCB *)start2;						 //取FCB
				for (int i = 0; i < 64; i++)
				{
					if ((fcb + i)->free == 1)
					{
						//printf("文件名：%s\n", (fcb + i)->filename);
						if (strcmp((fcb + i)->filename, q) == 0)
						{
							struct iNode *inode = INODE_OST + (fcb + i)->id; //取i节点地址
							if (inode->filetype[0] == '2')					 //必须得是目录文件
							{
								if (num != -1 && num != -2) //不是./..
								{
									copytouser(num, inode, fcb + i);
									strcpy(currentdir, qq);
									strcat(qq, "/");
									curdir = num;
								}
								else if (num == -1) //.
								{
									//
								}
								else if (num == -2) //..
								{
									for (int j = 0; j < MAXOPENFILE; j++)
									{
										if (openfilelist[j].topenfile == 1)
										{
											char temp[105] = "";
											strcpy(temp, openfilelist[j].dir);
											if (strcmp(openfilelist[j].dir, "/") != 0)
												strcat(temp, "/");
											if (strcmp(openfilelist[j].filename, "/") != 0)
												strcat(temp, openfilelist[j].filename);
											if (strcmp(qq, temp) == 0)
											{
												strcpy(currentdir, qq);
												strcat(qq, "/");
												curdir = j;
												return;
											}
										}
									}
								}
								flag = 1;
								break;
							}
						}
					}
				}
				if (flag)
					break;
				index += 1;
			}
			if (flag == 0)
			{
				printf("没有这个路径！\n");
				return;
			}
			q = strtok(NULL, "/");
		}
	}
}

/*以/开头的为绝对路径，否则为相对路径（相对路径就直接文件名，不可二层，因为二层不知道是判断无还是创建2个）*/
void my_mkdir(char *dirname)
{
	if (dirname[0] == '/') //指定目录
	{
		char temp[105] = "";
		strcpy(temp, dirname);
		char *q = strtok(temp, "/");
		unsigned short pre = openfilelist[0].number;
		unsigned short *index;
		int flag1 = 0; //判断是否重名
		int flag2 = 0; //判断是否最后一个目录依然重名
		struct FCB *fcb_free;
		int first;
		struct iNode *p = NULL;
		while (q)
		{
			first = 1;
			fcb_free = NULL;
			index = (unsigned short *)(myvhard + (pre - 1) * BLOCKSIZE);
			flag1 = 0;
			while (*index != 65535)
			{
				struct FCB *fcb = (struct FCB *)(myvhard + (*index - 1) * BLOCKSIZE);
				for (int i = 0; i < 64; i++)
				{
					if (!fcb->free)
					{
						if (first)
						{
							fcb_free = fcb;
							first = 0;
						}
					}
					else
					{
						if (strcmp(fcb->filename, q) == 0)
						{
							flag1 = 1;
							pre = ((struct iNode *)INODE_OST + fcb->id)->number;
							break;
						}
					}
					fcb++;
				}
				if (flag1)
					break;
				index++;
			}

			if (!flag1)
			{
				flag2 = 1;
				strcpy(fcb_free->filename, q);
				fcb_free->free = 1;
				p = INODE_OST;
				unsigned short count = 0;
				while (p->count != -1)
				{
					if (p->count == 0)
					{
						break;
					}
					count++;
					p++;
				}
				fcb_free->id = count;

				printf("id:%u\n", count);
				p->count = 1;
				time_t t = time(NULL);
				p->creattime = time(&t);
				LocalTime(p->creattime);
				p->fixtime = p->creattime;
				p->filesize = 32;
				p->filetype[0] = '2';
				p->filetype[1] = '3';
				unsigned int *freebit = PGRAPH_OST;
				unsigned short x, y;
				int flag = 0;
				for (unsigned short i = 0; i < 32; i++)
				{
					for (unsigned short j = 0; j < 32; j++)
					{
						int b = freebit[i] >> (31 - j) & 1;
						if (b == 0)
						{
							x = i + 1;
							y = j + 1;
							freebit[i] |= (1 << (31 - j));
							flag = 1;
							break;
						}
					}
					if (flag)
						break;
				}
				if (!flag)
				{
					printf("磁盘空间不足!创建失败\n");
					return;
				}
				p->number = (x - 1) * 32 + y;
				printf("%u,%u", x, y);
				printf("number:%u\n", p->number);
				index = (unsigned short *)(myvhard + (p->number - 1) * BLOCKSIZE);
				flag = 0;
				for (unsigned short i = 0; i < 32; i++)
				{
					for (unsigned short j = 0; j < 32; j++)
					{
						int b = freebit[i] >> (31 - j) & 1;
						if (b == 0)
						{
							x = i + 1;
							y = j + 1;
							freebit[i] |= (1 << (31 - j));
							flag = 1;
							break;
						}
					}
					if (flag)
						break;
				}
				if (!flag)
				{
					printf("磁盘空间不足!创建失败\n");
					return;
				}
				printf("%u,%u", x, y);
				printf("number:%u\n", (x - 1) * 32 + y);
				*index = (x - 1) * 32 + y;
				*(index + 1) = -1;
				pre = p->number;
				/*这里创建.和..*/
				struct FCB *fcb_now = NULL;
				struct FCB *fcb_pre = NULL;
				index = (unsigned short *)(myvhard + (pre - 1) * BLOCKSIZE);
				struct FCB *fcb_old = (struct FCB *)(myvhard + (*index - 1) * BLOCKSIZE);
				while (*index != 65535)
				{
					struct FCB *fcb = (struct FCB *)(myvhard + (*index - 1) * BLOCKSIZE);
					for (int i = 0; i < 64; i++)
					{
						if (!fcb->free)
						{
							fcb_now = fcb;
							break;
						}
						fcb++;
					}
					index++;
				}
				strcpy(fcb_now->filename, ".");
				fcb_now->free = 1;
				fcb_now->id = fcb_free->id;
				struct iNode *p1 = (struct iNode *)INODE_OST + fcb_now->id;
				p1->count += 1;
				index = (unsigned short *)(myvhard + (pre - 1) * BLOCKSIZE);
				while (*index != 65535)
				{
					struct FCB *fcb = (struct FCB *)(myvhard + (*index - 1) * BLOCKSIZE);
					for (int i = 0; i < 64; i++)
					{
						if (!fcb->free)
						{
							fcb_pre = fcb;
							break;
						}
						fcb++;
					}
					index++;
				}
				strcpy(fcb_pre->filename, "..");
				fcb_pre->free = 1;
				fcb_pre->id = fcb_old->id;
				struct iNode *p2 = (struct iNode *)INODE_OST + fcb_old->id;
				p2->count += 1;
			}
			q = strtok(NULL, "/");
		}
		if (flag2 == 0)
		{
			printf("文件已存在！\n");
			return;
		}
		/*修改父目录信息*/
		int len2 = 0;
		int len = strlen(dirname);
		for (int i = 0; i < len; i++)
		{
			if (dirname[i] == '/')
				len2 = i;
		}
		strcpy(temp, dirname);
		temp[len2] = '\0';
		char *q4 = strtok(temp, "/");
		struct iNode *pdir = INODE_OST;
		pdir->fixtime = p->creattime;
		pdir->filesize += 16 + p->filesize;
		while (q4)
		{
			index = (unsigned short *)(myvhard + (pdir->number - 1) * BLOCKSIZE);
			struct FCB *fcb = NULL;
			while (*index != 65535)
			{
				fcb = (struct FCB *)(myvhard + (*index - 1) * BLOCKSIZE);
				for (int i = 0; i < 64; i++)
				{
					if (strcmp(fcb->filename, q4) == 0)
					{
						break;
					}
					fcb++;
				}
				index++;
			}
			pdir = INODE_OST + fcb->id;
			pdir->fixtime = p->creattime;
			pdir->filesize += p->filesize + 16;
			q4 = strtok(NULL, "/");
		}
	}
	else //当前目录
	{
		int len = strlen(dirname);
		int tt = 0;
		for (int i = 1; i < len; i++)
			if (dirname[i] == '/')
				tt++;
		if (tt)
		{
			printf("只能创建一级目录！\n");
			return;
		}
		unsigned short index = openfilelist[curdir].number;
		unsigned short *indexFirst = (unsigned short *)(myvhard + (index - 1) * BLOCKSIZE);
		int rename = 0;
		struct FCB *fcb_free = NULL;
		int first = 1;
		while (*indexFirst != 65535)
		{
			struct FCB *fcb = (struct FCB *)(myvhard + (*indexFirst - 1) * BLOCKSIZE);
			for (int i = 0; i < 64; i++)
			{
				if (!fcb->free)
				{
					if (first)
					{
						fcb_free = fcb;
						first = 0;
					}
				}
				else
				{
					if (strcmp(fcb->filename, dirname) == 0)
					{
						rename = 1;
						break;
					}
				}
				fcb++;
			}
			indexFirst++;
		}
		if (rename)
		{
			printf("文件名已存在!\n");
			return;
		}
		strcpy(fcb_free->filename, dirname);
		fcb_free->free = 1;
		struct iNode *p = INODE_OST;
		unsigned short count = 0;
		while (p->count != 65535)
		{
			if (p->count == 0)
			{
				break;
			}
			count++;
			p++;
		}
		fcb_free->id = count;
		printf("id:%u\n", count);
		p->count = 1;
		time_t t = time(NULL);
		p->creattime = time(&t);
		LocalTime(p->creattime);
		p->fixtime = p->creattime;
		p->filesize = 32;
		p->filetype[0] = '2';
		p->filetype[1] = '3';
		unsigned int *freebit = PGRAPH_OST;
		unsigned short x, y;
		int flag = 0;
		for (unsigned short i = 0; i < 32; i++)
		{
			for (unsigned short j = 0; j < 32; j++)
			{
				int b = freebit[i] >> (31 - j) & 1;
				if (b == 0)
				{
					x = i + 1;
					y = j + 1;
					freebit[i] |= (1 << (31 - j));
					flag = 1;
					break;
				}
			}
			if (flag)
				break;
		}
		if (!flag)
		{
			printf("磁盘空间不足!创建失败\n");
			return;
		}
		p->number = (x - 1) * 32 + y;
		printf("%u,%u", x, y);
		printf("number:%u\n", p->number);
		indexFirst = (unsigned short *)(myvhard + (p->number - 1) * BLOCKSIZE);
		flag = 0;
		for (unsigned short i = 0; i < 32; i++)
		{
			for (unsigned short j = 0; j < 32; j++)
			{
				int b = freebit[i] >> (31 - j) & 1;
				if (b == 0)
				{
					x = i + 1;
					y = j + 1;
					freebit[i] |= (1 << (31 - j));
					flag = 1;
					break;
				}
			}
			if (flag)
				break;
		}
		if (!flag)
		{
			printf("磁盘空间不足!创建失败\n");
			return;
		}
		printf("%u,%u", x, y);
		printf("number:%u\n", (x - 1) * 32 + y);
		*indexFirst = (x - 1) * 32 + y;
		*(indexFirst + 1) = 65535;

		/*这里创建.和..*/
		struct FCB *fcb_now = NULL;
		struct FCB *fcb_pre = NULL;
		unsigned short pre = p->number;
		indexFirst = (unsigned short *)(myvhard + (pre - 1) * BLOCKSIZE);
		struct FCB *fcb_old = (struct FCB *)(myvhard + (*indexFirst - 1) * BLOCKSIZE);
		while (*indexFirst != 65535)
		{
			struct FCB *fcb = (struct FCB *)(myvhard + (*indexFirst - 1) * BLOCKSIZE);
			for (int i = 0; i < 64; i++)
			{
				if (!fcb->free)
				{
					fcb_now = fcb;
					break;
				}
				fcb++;
			}
			indexFirst++;
		}
		strcpy(fcb_now->filename, ".");
		fcb_now->free = 1;
		fcb_now->id = fcb_free->id;
		struct iNode *p1 = (struct iNode *)INODE_OST + fcb_now->id;
		p1->count += 1;
		indexFirst = (unsigned short *)(myvhard + (pre - 1) * BLOCKSIZE);
		while (*indexFirst != 65535)
		{
			struct FCB *fcb = (struct FCB *)(myvhard + (*indexFirst - 1) * BLOCKSIZE);
			for (int i = 0; i < 64; i++)
			{
				if (!fcb->free)
				{
					fcb_pre = fcb;
					break;
				}
				fcb++;
			}
			indexFirst++;
		}
		strcpy(fcb_pre->filename, "..");
		fcb_pre->free = 1;
		fcb_pre->id = fcb_old->id;
		struct iNode *p2 = (struct iNode *)INODE_OST + fcb_old->id;
		p2->count += 1;

		/*修改父目录信息*/
		char temp[105];
		strcpy(temp, currentdir);
		char *q = strtok(temp, "/");
		struct iNode *pdir = INODE_OST;
		pdir->fixtime = p->creattime;
		pdir->filesize += 16 + p->filesize;
		while (q)
		{
			indexFirst = (unsigned short *)(myvhard + (pdir->number - 1) * BLOCKSIZE);
			struct FCB *fcb = NULL;
			while (*indexFirst != 65535)
			{
				printf("%u\n", *indexFirst);
				fcb = (struct FCB *)(myvhard + (*indexFirst - 1) * BLOCKSIZE);
				for (int i = 0; i < 64; i++)
				{
					if (strcmp(fcb->filename, q) == 0)
					{
						break;
					}
					fcb++;
				}
				indexFirst++;
			}
			pdir = INODE_OST + fcb->id;
			pdir->fixtime = p->creattime;
			pdir->filesize += 16 + p->filesize;
			q = strtok(NULL, "/");
		}
	}
}

/*删除同理，要想跨目录删就必须用绝对路径*/
void my_rmdir(char *dirname)
{

	int sum1 = 0;
	int sum2 = 0;
	char temp[105];
	strcpy(temp,dirname);
	char *q_ = strtok(temp, "/");
	while (q_) //sum1==sum2用来判断是否要删
	{
		sum1++;
		q_ = strtok(NULL, "/");
	}

	if (strcmp(dirname, "/") == 0)
	{
		printf("不可删除根目录!\n");
		return;
	}
	int big_flag = 0; //指定路径还是当前路径
	if (dirname[0] == '/')
		big_flag = 1; //指定
	else
		big_flag = 0; //当前
	strcpy(temp,dirname);
	char *q = strtok(temp, "/");
	char qq[105] = "";
	if (!big_flag)
		strcpy(qq, currentdir);
	else
		strcpy(qq, "/");
	if (strcmp(qq, "/") != 0)
		strcat(qq, "/");
	struct iNode *p;
	char name[105] = "";

	unsigned short now;
	if (!big_flag)
		now = openfilelist[curdir].number;
	else
		now = openfilelist[0].number;

	while (q)
	{
		sum2++;
		strcpy(name, qq);
		strcat(name, q);
		for (int i = 0; i < MAXOPENFILE; i++)
		{
			if (openfilelist[i].topenfile == 1)
			{
				char name1[105] = "";
				strcpy(name1, openfilelist[i].dir);
				strcat(name1, "/");
				strcat(name1, openfilelist[i].filename);
				if (strcmp(name, name1) == 0)
				{
					printf("文件已被打开，不可删除！\n");
					return;
				}
			}
		}
		char *start = (char *)myvhard + (now - 1) * BLOCKSIZE;
		unsigned short *index = (unsigned short *)start; //取FCB盘块号
		int flag = 0;									 //定义是否重名
		struct FCB *fcb;
		while (*index != 65535)
		{
			char *start2 = (char *)(myvhard + (*index - 1) * BLOCKSIZE); //FCB地址
			fcb = (struct FCB *)start2;									 //取FCB
			for (int i = 0; i < 64; i++)
			{
				if (fcb->free == 1)
				{
					if (strcmp(fcb->filename, q) == 0)
					{
						struct iNode *inode = INODE_OST + fcb->id; //取i节点地址
						if (inode->filetype[0] == '2')			   //必须得是目录文件
						{
							now=inode->number;
							strcat(qq, "/");
							flag = 1;
							break;
						}
					}
				}
				fcb++;
			}
			if (flag)
				break;
			index += 1;
		}

		if (flag == 0)
		{
			printf("没有这个路径！\n");
			return;
		}
		if (sum2 == sum1) //这时候才真的可以删了
		{
			p = INODE_OST + fcb->id;
			if (p->filesize > 32)
			{
				printf("不为空目录，不能删除！\n");
				return;
			}
			p->count--;
			time_t t = time(NULL);
			p->fixtime = time(&t);
			if (p->count == 0)
			{
				index = (unsigned short *)(myvhard + (p->number - 1) * BLOCKSIZE);
				freeAddr(p->number);
				while (*index != 65535)
				{
					freeAddr(*index);
					index++;
				}
			}
			fcb->free = 0;
		}
		q = strtok(NULL, "/");
	}
	/*修改父目录信息*/
	int len=strlen(dirname);
	int len2=0;
	for(int i=0;i<len;i++){
		if(dirname[i]=='/') len2=i;
	}
	if(!big_flag){
		strcpy(temp, currentdir);
		if(strcmp(currentdir,"/")!=0)strcat(temp,"/");
		char tt[105]="";
		strcpy(tt,dirname);
		tt[len2]='\0';
		strcat(temp,tt);
	}	
	else{
		strcpy(temp,dirname);
		temp[len2]='\0';
	}
	//以上都是在确定父目录
	printf("temp:%s\n",temp);
	q = strtok(temp, "/");
	struct iNode *pdir = INODE_OST;
	pdir->fixtime = p->fixtime;
	pdir->filesize -= (p->filesize + 16);
	while (q)
	{
		unsigned short *indexFirst = (unsigned short *)(myvhard + (pdir->number - 1) * BLOCKSIZE);
		struct FCB *fcb = NULL;
		while (*indexFirst != 65535)
		{
			printf("%u\n", *indexFirst);
			fcb = (struct FCB *)(myvhard + (*indexFirst - 1) * BLOCKSIZE);
			for (int i = 0; i < 64; i++)
			{
				if (strcmp(fcb->filename, q) == 0)
				{
					break;
				}
				fcb++;
			}
			indexFirst++;
		}
		pdir = INODE_OST + fcb->id;
		pdir->fixtime = p->creattime;
		pdir->filesize -= (p->filesize + 16);
		q = strtok(NULL, "/");
	}
}
