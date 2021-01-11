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
int main()
{
	system("chcp 65001");
	curdir = 0;
	strcpy(currentdir, "/");
	startsys();
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
			FILE *fp = fopen(SYS_PATH, "w");
			fwrite(myvhard, SIZE, 1, fp);
		}
		else if (strcmp(cmd[0], "my_rm") == 0)
		{
			my_rm(cmd[1]);
		}
		else if (strcmp(cmd[0], "my_write") == 0)
		{
			//int fd = my_open(cmd[1]);
			//my_write(fd);
		}
		else if (strcmp(cmd[0], "my_ls") == 0)
		{
			my_ls();
		}
		else if (strcmp(cmd[0], "my_mkdir") == 0)
		{
			my_mkdir(cmd[1]);
		}
		else if (strcmp(cmd[0], "my_cd") == 0)
		{
			my_cd(cmd[1]);
		}
		else
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
	startp = myvhard + 18 * BLOCKSIZE;
	starti = myvhard + 2 * BLOCKSIZE;
	FILE *fp;
	int i;
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
	//fclose(fp);
	fp = fopen(SYS_PATH, "r");
	//fwrite(myvhard, SIZE, 1, fp);
	fread(myvhard, SIZE, 1, fp);
	struct iNode *p = (struct iNode *)(myvhard + 2 * BLOCKSIZE);
	LocalTime(p->creattime);
	LocalTime(p->fixtime);
	printf("filesize:%u\n", p->filesize);
	struct FCB *r = (struct FCB *)(myvhard + 18 * BLOCKSIZE);
	printf("%s\n", r->filename);
	unsigned short *index = (unsigned short *)(myvhard + 19 * BLOCKSIZE);
	while (*index != 65535)
	{
		printf("%u\n", *index);
		index++;
	}
	fclose(fp);
	copytouser(0, INODE_OST, r);
	printf("%s\n", openfilelist[0].dir);
	r = (struct FCB *)(myvhard + 20 * BLOCKSIZE) + 1;
	printf("%s\n", r->filename);
	printf("%d\n", r->id);
	printf("%u\n", *PGRAPH_OST >> 28 & 1);
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
	struct FCB fcb[3];
	strcpy(fcb[0].filename, "/");
	fcb[0].free = 0;
	fcb[0].id = 0;
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
	fwrite(fcb, BLOCKSIZE, 1, fp);
	fwrite(index, BLOCKSIZE, 1, fp);
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
		return -1;
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
	pgraph[x] &= ~(1 << (31 - y)); //释放文件所占盘块
}
int my_create(char *filename)
{
	int useropen_free = -1;
	//分配空闲用户打开表项
	int i;
	for (i = 0; i < MAXOPENFILE; i++)
	{
		if (!openfilelist[i].topenfile)
		{
			useropen_free = i;
			break;
		}
	}
	if (i == MAXOPENFILE)
	{
		printf("用户文件打开数超过上限!\n");
		return -1;
	}
	int index = openfilelist[curdir].number;
	printf("%d\n", index);
	unsigned short *indexFirst = (unsigned short *)(myvhard + (index - 1) * BLOCKSIZE);
	int rename = 0;
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
		return -1;
	}
	printf("-------------\n");
	strcpy(fcb_free->filename, filename);
	fcb_free->free = 1;
	struct iNode *p = INODE_OST;
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
	p->filesize = 1;
	p->filetype[0] = '1';
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
		return -1;
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
		return -1;
	}
	printf("%u,%u", x, y);
	printf("number:%u\n", (x - 1) * 32 + y);
	*indexFirst = (x - 1) * 32 + y;
	*(indexFirst + 1) = -1;
	copytouser(useropen_free, p, fcb_free);
	
	//修改父目录信息
	char *q = strtok(currentdir, "/");
	struct iNode *pdir = INODE_OST;
	pdir->fixtime = p->creattime;
	pdir->filesize += p->filesize;
	while (q)
	{

		indexFirst = (unsigned short *)(myvhard + (pdir->number - 1) * BLOCKSIZE);
		struct FCB *fcb = NULL;
		while (*indexFirst != -1)
		{
			fcb = (struct FCB *)(myvhard + (*indexFirst - 1) * BLOCKSIZE);
			printf("filename:%s\n",fcb->filename);
			printf("q:%s\n",q);
			system("pause");
			for (int i = 0; i < 64; i++)
			{
				if (strcmp(fcb->filename, q) == 0)
				{
					break;
				}
				fcb++;
			}
		}
		pdir = INODE_OST + fcb->id;
		pdir->fixtime = p->creattime;
		pdir->filesize += p->filesize;
		q = strtok(NULL, "/");
	}
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
		x = inode->number / 32;
		y = inode->number % 32 - 1;
		if (y == -1)
		{
			y = 31;
			x -= 1;
		}
		pgraph[x] &= ~(1 << (31 - y)); //释放索引表
		while (*index != 65535)
		{
			x = *index / 32;
			y = (*index % 32) - 1;
			if (y == -1)
			{
				y = 31;
				x -= 1;
			}
			pgraph[x] &= ~(1 << (31 - y)); //释放文件所占盘块
			index++;
		}
	}
	//修改父目录信息
	char *q = strtok(currentdir, "/");
	struct iNode *pdir = INODE_OST;
	pdir->fixtime = inode->fixtime;
	pdir->filesize -= inode->filesize;
	while (q)
	{
		index = (unsigned short *)(myvhard + (pdir->number - 1) * BLOCKSIZE);
		struct FCB *fcb = NULL;
		while (*index != -1)
		{
			fcb = (struct FCB *)(myvhard + (*index - 1) * BLOCKSIZE);
			int i = 0;
			for (i = 0; i < 64; i++)
			{
				if (strcmp(fcb->filename, q) == 0)
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
		pdir->filesize -= inode->filesize;
		q = strtok(NULL, "/");
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
		int off = 0;
		while (index[off++] != -1)
			;
		off -= 2;
		char *readp = (myvhard + (index[off] - 1) * BLOCKSIZE);
		off *= BLOCKSIZE;
		while (*readp != EOF)
		{
			readp++;
			off++;
		}
		printf("off:%d\n", off);
		openfilelist[fd].off = off;
	}
	else if (wstyle == EOF)
	{
		printf("退出编写模式!\n");
		return 0;
	}
	unsigned short filesize = openfilelist[fd].filesize;
	while ((buff[i++] = getchar()) != EOF)
	{
		if (filesize + i > MAXBUFFSIZE)
		{
			printf("文件大小已达上限!\n");
			break;
		}
	}
	my_do_write(fd, buff, strlen(buff), wstyle); //实际写
}

int my_do_write(int fd, char *text, int len, char wstyle)
{
	//重新分配、回收数据磁盘块
	int e_blocknum = len / BLOCKSIZE;
	int e_rest = len % BLOCKSIZE;
	int r_blocknum = openfilelist[fd].filesize / BLOCKSIZE;
	int r_rest = openfilelist[fd].filesize % BLOCKSIZE;
	int lastIndex; //记录文件索引表最后一项的索引
	if (r_rest != 0)
		lastIndex = r_blocknum + 1;
	unsigned short *indexFirst = (unsigned short *)(myvhard + (openfilelist[fd].number - 1) * BLOCKSIZE);
	int index = openfilelist[fd].off / BLOCKSIZE;
	int offset = openfilelist[fd].off % BLOCKSIZE;
	unsigned short filesize = openfilelist[fd].filesize;
	if (wstyle == '0')
	{
		if (e_rest != 0)
			e_blocknum += 1;
		if (r_rest != 0)
			r_blocknum += 1;
		int n = e_blocknum - r_blocknum;
		int k;
		unsigned short addrId;
		if (n > 0)
		{
			for (k = 0; k < n; k++)
			{
				addrId = getFreeAddr();
				if (addrId == -1)
				{
					printf("磁盘空间不足!申请失败\n");
					//return -1;
					break;
				}
				indexFirst[lastIndex] = addrId;
				lastIndex++;
				//这里不需要判断索引表是否已达上限，因为文件最大占用磁盘块数为20
			}
			indexFirst[lastIndex] = -1;
		}
		else if (n < 0)
		{
			n = -n;
			for (k = 0; k < n; k++)
			{
				lastIndex--;
				unsigned short num = indexFirst[lastIndex];
				freeAddr(num);
			}
			indexFirst[lastIndex] = -1;
		}
	}
	else if (wstyle == '1')
	{
		unsigned short addrId;
		r_blocknum = r_blocknum - index - 1;
		r_rest = BLOCKSIZE - offset;
		int m = e_blocknum - r_blocknum;
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
				if (addrId == -1)
				{
					printf("磁盘空间不足!申请失败\n");
					//return -1;
					break;
				}
				indexFirst[lastIndex] = addrId;
				lastIndex++;
				//这里不需要判断索引表是否已达上限，因为文件最大占用磁盘块数为20
			}
			indexFirst[lastIndex] = -1;
		}
		else if (m < 0)
		{
			if (restn > 0)
			{
				m = -m - 1;
			}
			for (k = 0; k < m; k++)
			{
				lastIndex--;
				unsigned short num = indexFirst[lastIndex];
				freeAddr(num);
			}
			indexFirst[lastIndex] = -1;
		}
		else if (m == 0)
		{
			if (restn > 0)
			{
				addrId = getFreeAddr();
				if (addrId == -1)
				{
					printf("磁盘空间不足!创建失败\n");
					//return -1;
				}
				else
				{
					indexFirst[lastIndex] = addrId;
					indexFirst[++lastIndex] = -1;
				}
			}
		}
	}
	else if (wstyle == '2')
	{
		int k = 0;
		r_rest = BLOCKSIZE - r_rest;
		int restn = e_rest - r_rest;
		unsigned short addrId;
		if (restn > 0)
		{
			e_blocknum += 1;
		}
		for (k = 0; k < e_blocknum; k++)
		{
			addrId = getFreeAddr();
			if (addrId == -1)
			{
				printf("磁盘空间不足!创建失败\n");
				//return -1;
				break;
			}
			indexFirst[lastIndex] = addrId;
			lastIndex++;
		}
		indexFirst[lastIndex] = -1;
	}
	//写磁盘
	char *buffFirst = (char *)malloc(BLOCKSIZE * sizeof(char));
	int i = 0;
	int loopnum = lastIndex - index;
	char *p = text;
	char *datap;
	int q = 0;
	unsigned short readp = openfilelist[fd].off;
	while (i < loopnum)
	{
		unsigned short textSize = strlen(p);
		unsigned short writeSize = textSize < BLOCKSIZE ? textSize : BLOCKSIZE;
		memcpy(&buffFirst[q], p, writeSize);
		p += writeSize;
		int index, offset;
		while (q < BLOCKSIZE)
		{
			index = readp / BLOCKSIZE;
			offset = readp % BLOCKSIZE;
			textSize = BLOCKSIZE - q;
			writeSize = textSize < BLOCKSIZE - offset ? textSize : BLOCKSIZE - offset;
			datap = (myvhard + (indexFirst[index] - 1) * BLOCKSIZE) + offset;
			memcpy(datap, &buffFirst[q], writeSize);
			q += writeSize;
			//openfilelist[fd].off += writeSize;
			readp += writeSize;
		}
		i++;
	}
	//修改自身信息以及父节点信息
	openfilelist[fd].filesize = readp;
	time_t t = time(NULL);
	unsigned int fixtime = time(&t);
	char dir[100];
	strcpy(dir, currentdir);
	char *dirq = strtok(strcat(dir, openfilelist[fd].filename), "/");
	struct iNode *pdir = INODE_OST;
	pdir->fixtime = fixtime;
	pdir->filesize = pdir->filesize - filesize + readp;
	unsigned short *indexp;
	while (dirq)
	{
		indexp = (unsigned short *)(myvhard + (pdir->number - 1) * BLOCKSIZE);
		struct FCB *fcb = NULL;
		while (*indexp != -1)
		{
			fcb = (struct FCB *)(myvhard + (*indexp - 1) * BLOCKSIZE);
			int i = 0;
			for (i = 0; i < 64; i++)
			{
				if (strcmp(fcb->filename, dirq) == 0)
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
		pdir->filesize = pdir->filesize - filesize + readp;
		dirq = strtok(NULL, "/");
	}
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
				struct iNode *inode = INODE_OST + (fcb + i)->id; //取i节点地址
				printf("文件大小：%u\n", inode->filesize);
				printf("文件类型：");
				if (inode->filetype[0] == '1')
					printf("普通文件\n");
				else if (inode->filetype[0] == '2')
					printf("目录文件\n");
				printf("文件创建时间：");
				LocalTime(inode->creattime);
				printf("文件最近修改时间：");
				LocalTime(inode->fixtime);
				printf("-----------------\n");
			}
		}
		index += 1;
	}
}

void close_openlist(char *name1)
{
	for (int i = 0; i < MAXOPENFILE; i++)
	{
		char name[105] = "";
		strcat(name, openfilelist[i].dir);
		strcat(name, openfilelist[i].filename);
		if (strcmp(name, name1) == 0)
		{
			openfilelist[i].topenfile = 0;
			return;
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
			bool flag = 0;
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
			bool flag = 0;
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
										if (strcmp(qq, openfilelist[j].dir) == 0)
										{
											strcpy(currentdir, qq);
											strcat(qq, "/");
											curdir = j;
											break;
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
		char *q = strtok(dirname, "/");
		unsigned short pre = openfilelist[0].number;
		unsigned short *index;
		bool flag1 = 0; //判断是否重名
		bool flag2 = 0; //判断是否最后一个目录依然重名
		struct FCB *fcb_free;
		bool first;
		struct iNode *p;
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
				p->filesize = 0;
				p->filetype[0] = '2';
				p->filetype[1] = '3';
				unsigned int *freebit = PGRAPH_OST;
				unsigned short x, y;
				bool flag = 0;
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
				struct FCB *fcb_now;
				struct FCB *fcb_pre;
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
		char *q4 = strtok(currentdir, "/");
		struct iNode *pdir = INODE_OST;
		pdir->fixtime = p->creattime;
		pdir->filesize += p->filesize;
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
			pdir->filesize += p->filesize;
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
		bool rename = 0;
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
		p->filesize = 0;
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
		struct FCB *fcb_now;
		struct FCB *fcb_pre;
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
		char *q = strtok(currentdir, "/");
		struct iNode *pdir = INODE_OST;
		pdir->fixtime = p->creattime;
		pdir->filesize += p->filesize;
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
			pdir->filesize += p->filesize;
			q = strtok(NULL, "/");
		}
	}
}