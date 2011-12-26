#ifndef MY_FS_H
#define MY_FS_H
/*this file is the entry of my fileSystem. it defines many consts and types*/

/* define types */
typedef unsigned int block_t;
typedef unsigned int zone_t;
// typedef int int;
typedef int d_size;
// typedef int off_t;
typedef unsigned long bit_t;
// typedef int int;



#define NR_TZONES 14

/* filp area */
#define NR_FILPS 128
#define OPEN_MAX 128
struct filp {
  int filp_count;		/* how many file descriptors share this slot?*/
  int filp_pos;
  int filp_type;
  struct inode *filp_ino;	/* pointer to the inode */
};

struct filp filp[NR_FILPS];
#define FILP_CLOSED	0	/* filp_mode: associated device closed */

#define NIL_FILP (struct filp *) 0	/* indicates absence of a filp slot */

#define READ 1
#define WRITE 2
#define RDWR 3



/*  d_inode area*/
typedef struct {		/* V2.x disk inode */
    int d_mode;
    int d_size;		/* current file size in bytes */
    zone_t d_zone[NR_TZONES];	/* block nums for direct, ind, and dbl ind */
} d_inode;


/*struct direct area*/
#define DIR_ENTRY_SIZE 256
#define DIRSIZ	252

struct direct {
  int d_ino;
  char d_name[DIRSIZ];
};



/* buf area */
#define NR_INDIRECT_BLOCK 128
#define NR_INDIRECT_BLOCK_2 16384
#define NR_DIR_BLOCK 2
#define NR_BIT_MAP 128
#define INODE_PER_BLOCK 8
#define NR_BUFS 128
#define NR_BUF_HASH 128
#define MAX_BLOCK_SIZE 512
struct buf {
  /* Data portion of the buffer. */
  union {
    char b__data[MAX_BLOCK_SIZE];		     /* ordinary user data */
/* directory block */
    struct direct b__dir[NR_DIR_BLOCK];
/* indirect block */
    zone_t  b__ind[NR_INDIRECT_BLOCK];
/* inode block */
    d_inode b__ino[INODE_PER_BLOCK];
/* bit map block */
    int b__bitmap[NR_BIT_MAP];
  } b;

  /* Header portion of the buffer. */
  struct buf *b_next;		/* used to link all free bufs in a chain */
  struct buf *b_prev;		/* used to link all free bufs the other way */
  struct buf *b_hash;		/* used to link bufs on hash chains */
  block_t b_blocknr;		/* block number of its (minor) device */
  int b_count;
  char b_dirt;			/* CLEAN or DIRTY */
};
struct buf buf[NR_BUFS];
struct buf *buf_hash[NR_BUFS];	/* the buffer hash table */

struct buf *front;	/* points to least recently used free block */
struct buf *rear;	/* points to most recently used free block */
int bufs_in_use;		/* # bufs currently in use (not on free list)*/

/* A block is free if b_dev == NO_DEV. */

#define NIL_BUF ((struct buf *) 0)	/* indicates absence of a buffer */

/* These defs make it possible to use to bp->b_data instead of bp->b.b__data */
#define b_data   b.b__data
#define b_dir    b.b__dir
#define b_ind b.b__ind
#define b_ino b.b__ino
#define b_bitmap b.b__bitmap



#define INODE_BLOCK        0				 /* inode block */
#define DIRECTORY_BLOCK    1				 /* directory block */
#define INDIRECT_BLOCK     2				 /* pointer block */
#define MAP_BLOCK          3				 /* bit map */
#define FULL_DATA_BLOCK    5		 	 	 /* data, fully used */
#define PARTIAL_DATA_BLOCK 6 				 /* data, partly used*/

#define HASH_MASK (NR_BUF_HASH - 1)	/* mask for hashing block numbers */




/*super block area*/
#define S_NINODES 10000
#define S_ZONES (400 * 1024)
#define S_IMAP_BLOCKS 3
#define S_ZMAP_BLOCKS 100
#define S_FIRSTDATAZONE 1354
#define S_BLOCK_SIZE 512
#define START_BLOCK 1

struct super_block{
  /* The following items are only used when the super_block is in memory. */
  bit_t s_isearch;		/* inodes below this bit number are in use */
  bit_t s_zsearch;		/* all zones below this bit number are in use*/
};

struct super_block super_block;
#define IMAP		0	/* operating on the inode bit map */
#define ZMAP		1	/* operating on the zone bit map */

#define NO_BIT 0




/*inode area*/
#define NR_INODES 128
struct inode {
  int i_mode;
  int i_size;			/* current file size in bytes */
  zone_t i_zone[NR_TZONES]; /* zone numbers for direct, ind, and dbl ind */

  /* The following items are not present on the disk. */
  int i_num;			/* inode number on its (minor) device */
  int i_count;			/* # times inode used; 0 means slot is free */
  char i_dirt;			/* CLEAN or DIRTY */
};

struct inode inode[NR_INODES];
#define NIL_INODE (struct inode *) 0	/* indicates absence of inode slot */

#ifndef DIRTY
#define DIRTY 1
#define CLEAN 0
#endif
#define NO_ZONE 0
#define NO_BLOCK 0
#define READING 0
#define WRITING 1

/*int bits if it is directory*/
#define I_DIRECTORY 0x00000002
#define I_NOT_ALLOC 0x00000000
#define I_ALLOC 0x00000001



/*for indirect index block*/
#define DIRECT_ZONE 10
#define INDEX1_ZONE 138
#define INDEX2_ZONE_1 16522
#define INDEX2_ZONE_2 32906
#define INDEX3_ZONE 102400


/* public functions to be called outside */
int my_open(char * path);
int my_create(char * path);
int my_read(int fd, char * buf, int count);
int my_write(int fd, char * buf, int count);
int my_close(int fd);
int my_remove(char * name);
int my_rename(char * oldname, char * newname);
int my_mkdir(char * name);
int my_rmdir(char * name);
void init();
void show_file_list();
void mkroot(); /*called only once*/
void my_strcpy(char des[DIRSIZ], const char * src);
void reset_disk();
void my_mkfs();


#endif /*MY_FS_H*/
