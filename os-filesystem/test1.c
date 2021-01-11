/*这里创建.和..*/
struct FCB *fcb_now;
struct FCB *fcb_pre;
unsigned short pre = p->number;
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