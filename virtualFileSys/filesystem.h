#ifndef FILESYSTEM_H
#define FILESYSTEM_H

#include <vector>
#include <iostream>
#include <cstdio>
#include <string>
#include <cstring>
#include <deque>
#include <queue>
#include <vector>
#include <unistd.h>
#include <ctime>


#define Block_Num 102400
#define Block_Size 1024
#define DISK "/home/haines/os/lab5/virtualdisk"
#define SCREEN "/home/haines/os/lab5/vitrualscreen"

#define HELP    0
#define OPEN    1
#define CLOSE   2
#define READ    3
#define WRITE   4
#define CP      5
#define RM      6
#define CD      7
#define LS      8
#define MKDIR   9
#define TOUCH   10
#define SYSINFO 11
#define CLEAR   12
#define TREE    13
#define MV      14
#define CAT     15
#define CHMOD   16
#define FORMAT  17
#define EXIT    -1


/*SuperBlock
 * describe super block
 * size: 24 Byte
*/
struct SuperBlock {
    unsigned short blockSize;
    char type[10];                 //ext or fat
    unsigned int blockNum;
    unsigned int inodeNum;
    unsigned int freeBlockNum;
};

/*inode
 *use an array to store inodes
 *size: 126 Bytes
*/
struct inode {
    char name[30];                 //name, max length : 50
    unsigned int fileSize;         //max:4GB-1
    unsigned int blockNum;         //

    unsigned int id;               //index
    unsigned int pid;              //index of parent

    char limit[4];                 //limits: 00000|000
    char type;                     //file or dir : 0 or 1
    unsigned short uid;            //file owner
    unsigned short time[5];        //time:y-m-d-h-mn

    unsigned int i_addr[14];       //0~11: direct index, 12: one, 13: two (store blocknums)
};

/*dentry
 *use an array to store dentries
 *in memory
*/
struct dentry {

};

/*file
*use an array to store files
* in memory
*/
struct file {

};

typedef inode * pfcb;
typedef inode * pInode;

class FileSystem
{
public:
    FileSystem();
    ~FileSystem();

private:
    int file_sys_create();
    void file_sys_init();

    //bitmaps: Block_Num bits
    int mapSize;
    int mapBLockNum;
    int* inodeBitmap;
    int* blockBitmap;
    int* opt;
    void updateInodeBitmap();
    void updateInodeBitmap(unsigned int id);
    void updateBlockBitmap();
    void updateBlockBitmap(unsigned int id);
    void loadInodeBitmap();
    void loadBlockBitmap();

    //file pointer
    FILE* file;
//    FILE* screenfile;
//    FILE* ofile;

    //superblock
    SuperBlock superblock;
    void updateSuperBlock();
    void init_superBlock();
    void loadSuperBlock();
    unsigned int superBlockSize;


    //offset: superblock, bitmap, inodes, blocks
    unsigned long soffset;
    unsigned long bmpoffset;
    unsigned long impoffset;
    unsigned long ioffset;
    unsigned long boffset;

    //inodes
    unsigned int inodeTotal;
    unsigned int inodeBlockNum;
    unsigned int inodeSize;
    void updateInode(int index, const inode & ide);
    void loadInode();
    void freeInode(int index);
    pInode* pinodes;
    pInode curInode;
    void curTime(unsigned short *ptime);

    //free inodes manager
    std::deque<int> freeInodes;
    void getFreeInodes();

    //free blocks manager
    std::deque<int> freeBlocks;
    void getFreeBlocks();
    void freeBlock(size_t id);

    bool isFree(int row, int col, int *bitmap);
    void getFreeID(std::deque<int> *deque, int * bitmap);

    //tools
    pfcb findPath(std::string dirname);

    //get commands
    void command();
    void open_helper();
    void close_helper();
    void read_helper(std::__cxx11::string filename);
    void write_helper(std::string filename);
    void cp_helper(std::__cxx11::string file_1, std::__cxx11::string file_2);
    void rm_helper(std::string filename);
    void exit_helper();
    int cd_helper(std::string dirname);
    void ls_helper();
    void ls_helper(std::string option);
    void mkdir_helper(std::string dirname);
    int touch_helper(std::string filename);
    void mv_helper(std::__cxx11::string src, std::__cxx11::string dest);
    void cat_helper(std::string filename);
    void chmod_helper(std::string mod, std::string filename);
    void format_helper();
    std::vector<std::string> split(std::string str, std::string pattern);

    //ui
    int cmd_handle(std::vector<std::string> cmd);
    void showPath();
    void showHelper();
    void showSysInfo();
    void clr();
    void tree();
    void printDir(pfcb pnd, size_t depth);
    void login();

protected:


};

#endif // FILESYSTEM_H
