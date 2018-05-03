#include "filesystem.h"

FileSystem::FileSystem()
{
    mapSize = Block_Num / (8 * sizeof(int));
//    printf("mapSize: %d\n", mapSize);
    mapBLockNum = Block_Num / ( 8 * Block_Size);
//    printf("mapBlockNUm: %d\n", mapBLockNum);
    inodeBitmap = new int[mapSize];
    blockBitmap = new int[mapSize];

    opt = new int[32];
    for (int i = 0 ; i < 32 ; ++i) {
        opt[i] = 1 << i;
//        printf("%p\n",opt[i]);
    }

    inodeTotal = Block_Num;
    inodeBlockNum = inodeTotal / 8;
    inodeSize = 128;
    pinodes = new pInode[inodeTotal];
    curInode = new inode();


    superBlockSize = sizeof(superblock);
//    printf("superBlockSize: %d\n", superBlockSize);

    soffset = 0;
    impoffset = Block_Size;
    bmpoffset = impoffset + mapBLockNum * Block_Size;
    ioffset = bmpoffset + mapBLockNum * Block_Size;
    boffset = ioffset + inodeBlockNum * Block_Size;
//    printf("soffset: %lu\n"
//           "impoffset: %lu\n"
//           "bmpoffset: %lu\n"
//           "ioffset: %lu\n"
//           "boffset: %lu\n",soffset,impoffset,bmpoffset,ioffset,boffset);

    file_sys_init();

}

FileSystem::~FileSystem()
{
    delete curInode;
    delete []opt;
    delete []inodeBitmap;
    delete []blockBitmap;
    delete [] pinodes;
    for (unsigned int i = 0 ; i < inodeTotal ;++i) {
//        delete pinodes[i];
    }
}

int FileSystem::file_sys_create()
{
    printf("file system creating ...\n");
    file = fopen(DISK,"wb+");
    if (file == NULL) {
        printf("Open disk fail !\n");
        return -1;
    }
    printf("Open disk success !\n");

    //init superblock
    init_superBlock();
    superblock.blockNum = Block_Num;
    superblock.blockSize = Block_Size;
    superblock.inodeNum = 1;
    strcpy(superblock.type,"ext");
    superblock.freeBlockNum = superblock.blockNum - 2 * mapBLockNum - 1;
    updateSuperBlock();
//    printf("freeBlockNum: %d\n", superblock.freeBlockNum);
    printf("init superblock success !\n");

    //init bitmaps
    memset(inodeBitmap,0,mapSize);
    memset(blockBitmap,0,mapSize);
    updateInodeBitmap();
    updateBlockBitmap();

    //init inodes and blocks
    unsigned long length = superblock.freeBlockNum * Block_Size;
    for (unsigned long i = 0 ; i < length ; i++) {
        fputc(0,file);
    }

    //init current inode
    curInode->type = 1;
    strcpy(curInode->name,"/");
    curInode->blockNum = 0;
    curInode->id = 0;
    curInode->pid = 0;
    curInode->fileSize = 0;
    strcpy(curInode->limit,"755");
    updateInode(curInode->id,*curInode);
    updateInodeBitmap(curInode->id);


    fclose(file);
    return 0;
}

void FileSystem::file_sys_init()
{
    if ((file = fopen (DISK , "rb")) == NULL) {
        file_sys_create();
    }
    if ((file = fopen (DISK , "rb+")) == NULL) {
        printf("Open DISK fail !\n");
        return;
    }

    printf("File system initilizing ...\n");
    printf("mapSize: %d\n", mapSize);
    printf("mapBlockNUm: %d\n", mapBLockNum);
    printf("superBlockSize: %d\n", superBlockSize);
    printf("soffset: %lu\n"
           "impoffset: %lu\n"
           "bmpoffset: %lu\n"
           "ioffset: %lu\n"
           "boffset: %lu\n",soffset,impoffset,bmpoffset,ioffset,boffset);

    //load superblock
    //printf("superblock: %d\n",superblock.freeBlockNum);
    loadSuperBlock();
    printf("Load superblock success !\n");

    //load bitmaps
    loadInodeBitmap();
    loadBlockBitmap();
    printf("Load inodeBitmap success !\n"
           "Load blockBitmap success !\n");

    //load inodes
    loadInode();
    printf("root inodes : %s\n"
           "bitmap: %d \n", pinodes[0]->name, inodeBitmap[0]);
    printf("Load inodes success !\n");

    //free inodes
    freeInodes.clear();
    getFreeInodes();

    //free blocks
    freeBlocks.clear();
    getFreeBlocks();

    //load curInode
    curInode = pinodes[0];

//    login();

    command();
}

void FileSystem::updateInodeBitmap()
{
    fseek(file,impoffset,SEEK_SET);
    fwrite(inodeBitmap,mapSize * 4, 1, file);
}

void FileSystem::updateInodeBitmap(unsigned int id)
{
    int row, col, bits;
    row = id / 32;
    col = id % 32;
    bits = inodeBitmap[row] ^ opt[col];
    inodeBitmap[row] = bits;
    fseek(file,impoffset + sizeof(int) * row,SEEK_SET);
    fwrite(&bits,4, 1, file);
}

void FileSystem::updateBlockBitmap()
{
    fseek(file,bmpoffset,SEEK_SET);
    fwrite(blockBitmap,mapSize * 4, 1, file);
}

void FileSystem::updateBlockBitmap(unsigned int id)
{
    int row, col, bits;
    row = id / 32;
    col = id % 32;
    bits = blockBitmap[row] ^ opt[col];
    blockBitmap[row] = bits;
    fseek(file,bmpoffset + sizeof(int) * row,SEEK_SET);
    fwrite(&bits,4, 1, file);
}

void FileSystem::loadInodeBitmap()
{
    fseek(file,impoffset,SEEK_SET);
    fread(inodeBitmap,mapSize * 4, 1, file);
}

void FileSystem::loadBlockBitmap()
{
    fseek(file,bmpoffset,SEEK_SET);
    fread(blockBitmap,mapSize * 4, 1, file);
}

void FileSystem::updateSuperBlock()
{
    fseek(file,soffset,SEEK_SET);
    fwrite(&superblock,superBlockSize,1,file);
}

void FileSystem::init_superBlock()
{
    fseek(file,soffset,SEEK_SET);
    for (int i = 0 ; i < Block_Size ; ++i) fputc(0,file);
}

void FileSystem::loadSuperBlock()
{
    fseek(file,soffset,SEEK_SET);
    fread(&superblock,superBlockSize,1,file);
}

void FileSystem::updateInode(int index, const inode &ide)
{
    fseek(file,ioffset + index * inodeSize,SEEK_SET);
    fwrite(&ide,sizeof(ide),1,file);
}

void FileSystem::loadInode()
{
//    fseek(file,ioffset,SEEK_SET);
    for (unsigned int i = 0 ; i < inodeTotal ; i++) {
        pinodes[i] = new inode();
        fseek(file,ioffset + inodeSize * i,SEEK_SET);
        fread(pinodes[i],sizeof(inode),1,file);
    }
}

void FileSystem::freeInode(int index)
{
    pfcb tmpInode = pinodes[index];
    tmpInode->blockNum = 0;
    tmpInode->fileSize = 0;
    tmpInode->id = 0;
    strcpy(tmpInode->name,"\0");
    tmpInode->pid = 0;
    tmpInode->type = 0;
    tmpInode->uid = 0;
    for (int i = 0 ; i < 14 ; ++i) {
        tmpInode->i_addr[i] = 0;
    }
    updateInode(index,*tmpInode);
    updateInodeBitmap(index);
}

void FileSystem::curTime(unsigned short *ptime)
{
    time_t now;
    struct tm * tm_now;
    time(&now);
    tm_now = localtime(&now);
    ptime[0] = (unsigned short)(tm_now->tm_year + 1900);
    ptime[1] = (unsigned short)(tm_now->tm_mon + 1);
    ptime[2] = (unsigned short)(tm_now->tm_mday);
    ptime[3] = (unsigned short)(tm_now->tm_hour);
    ptime[4] = (unsigned short)(tm_now->tm_min);
}

void FileSystem::getFreeInodes()
{
    getFreeID(&freeInodes,inodeBitmap);
}

bool FileSystem::isFree(int row, int col, int *bitmap)
{
    return (bitmap[row] & opt[col]) == 0 ? true : false;
}

void FileSystem::getFreeID(std::deque<int> *deque, int *bitmap)
{
    for (int row = 0 ; row < mapSize ; ++row) {
        for (int col = 0 ; col < 32 ; ++col) {
            if (isFree(row,col,bitmap)) {
                deque->push_back(row * 32 + col);
//                printf("%d ",row *32 +col);
//                freeInodes.pop_front();
            }
        }
    }
}

pfcb FileSystem::findPath(std::__cxx11::string dirname)
{
    //get path and filename
    std::string pattern("/");
    std::string dir("");
    std::vector<std::string> path = split(dirname,pattern);
    pfcb tmpInode = curInode;
    pfcb ret;

    for (size_t i = 0 ; i < path.size() - 1 ; ++i) {
        dir.append(path[i]).append("/");
//        std::cout << path[i] << std::endl;
    }
//    std::cout << dir << std::endl;

    //change to the file/s dir
    if (cd_helper(dir) < 0 ) {
        printf("read: cd dir fail !\n");
        return NULL;
    }

    //find the file
//    for (i = 0 ; i < curInode->blockNum ; ++i) {
//        if (path[path.size() - 1] == pinodes[curInode->i_addr[i]]->name
//                && pinodes[curInode->type == 0]) {
//            ret = pinodes[curInode->i_addr[i]];
//            printf("read: find file success !\n");
//            filefound = true;
//            break;
//        }
//    }

//    if (!filefound) {
//        printf("read: no such file !\n");
//        curInode = tmpInode;
//        return NULL;
//    }
    ret = curInode;
    curInode = tmpInode;
    return ret;
}

void FileSystem::getFreeBlocks()
{
    getFreeID(&freeBlocks,blockBitmap);
}

void FileSystem::freeBlock(size_t id)
{
    char buf[Block_Size] = "\0";
    fseek(file,boffset + Block_Size * id, SEEK_SET);
    fwrite(buf,Block_Size,1,file);
    updateBlockBitmap(id);
}

void FileSystem::command()
{
    std::string input;
    int ret;
    while (1) {
        showPath();
        std::getline(std::cin , input);
        std::string pattern(" ");
        std::vector<std::string> res = split(input,pattern);
//        for (std::size_t i = 0 ; i < res.size() ; ++i) {
//            std::cout << res[i] << std::endl;
//        }
        ret = cmd_handle(res);
        if (res.size() == 0) continue ;
        if (ret == EXIT|| ret == FORMAT) return;
//        printf("%s\n",input);
//        if(strcmp(input,"exit\n") == 0) return;
    }
//    cmd_handle(input);
    return;
}

void FileSystem::open_helper()
{
    printf("open file ...\n");
}

void FileSystem::close_helper()
{
    printf("close file ...\n");
}

void FileSystem::read_helper(std::string filename)
{
    FILE *sfile = fopen(SCREEN,"w+");
    printf("read file ...\n");

    size_t blockcount = 0, i;
    bool filefound = false;
    size_t filesize = 0;
    char c;

    //get path and filename
    std::string pattern("/");
    std::string dir("");
    std::vector<std::string> path = split(filename,pattern);
    pfcb tmpInode = curInode;
    for (i = 0 ; i < path.size() - 1 ; ++i) {
        dir.append(path[i]).append("/");
//        std::cout << path[i] << std::endl;
    }
//    std::cout << dir << std::endl;

    //change to the file/s dir
    if (cd_helper(dir) < 0 ) {
        printf("read: cd dir fail !\n");
        return;
    }

    //find the file
    for (i = 0 ; i < curInode->blockNum ; ++i) {
        if (path[path.size() - 1] == pinodes[curInode->i_addr[i]]->name
                && pinodes[curInode->type == 0]) {
            curInode = pinodes[curInode->i_addr[i]];
//            printf("read: find file success !\n");
            filefound = true;
            break;
        }
    }

    if (!filefound) {
        printf("read: no such file !\n");
        curInode = tmpInode;
        return;
    }

    if (curInode->limit[0] == '7' || curInode->limit[0] == '5' || curInode->limit[0] == '4' || curInode->limit[0] == '6') {
        //write in blocks the file has firstly
        for (blockcount = 0 ; blockcount < curInode->blockNum ; ++blockcount) {
            fseek(file, boffset + curInode->i_addr[blockcount] * Block_Size , SEEK_SET);
            for (i = 0 ; i < Block_Size ; ++i) {
                if ((c = fgetc(file)) != EOF) {
                    filesize++;
//                    printf("%c",c);
                    fputc(c,sfile);
                } else {
                    fputc(EOF,sfile);
                    curInode = tmpInode;
                    fclose(sfile);

//                    printf("\n");
                    return;
                }
            }
        }
    } else {
        printf("Cannot read the file!\n");
        fclose(sfile);
    }

    curInode = tmpInode;
}

void FileSystem::write_helper(std::__cxx11::string filename)
{
    printf("write file ...\n");

//    char buf[Block_Size];
    size_t blockcount = 0, i;
    bool filefound = false;
    size_t filesize = 0;
    char c;

    FILE *sfile = fopen(SCREEN,"r+");

    //get path and filename
    std::string pattern("/");
    std::string dir("");
    std::vector<std::string> path = split(filename,pattern);
    pfcb tmpInode = curInode;
    for (i = 0 ; i < path.size() - 1 ; ++i) {
        dir.append(path[i]).append("/");
//        std::cout << path[i] << std::endl;
    }
//    std::cout << dir << std::endl;

    //change to the file/s dir
    if (cd_helper(dir) < 0 ) {
        printf("write: cd dir fail !\n");
        return;
    }

    //find the file
    for (i = 0 ; i < curInode->blockNum ; ++i) {
        if (path[path.size() - 1] == pinodes[curInode->i_addr[i]]->name
                && pinodes[curInode->type == 0]) {
            curInode = pinodes[curInode->i_addr[i]];
//            printf("write: find file success !\n");
            filefound = true;
            break;
        }
    }

    if (!filefound) {
        printf("write: no such file !\n");
        curInode = tmpInode;
        return;
    }

    if (curInode->limit[0] == '7' || curInode->limit[0] == '3' || curInode->limit[0] == '2' || curInode->limit[0] == '6') {
        //write in blocks the file has firstly
        for (blockcount = 0 ; blockcount < curInode->blockNum ; ++blockcount) {
            fseek(file, boffset + curInode->i_addr[blockcount] * Block_Size , SEEK_SET);
            for (i = 0 ; i < Block_Size ; ++i) {
                if ((c = fgetc(sfile)) != EOF) {
                    filesize++;
//                    printf("%c",c);
                    fputc(c,file);
                } else {
                    curInode->blockNum = blockcount + 1;
                    curInode->fileSize = filesize;
                    updateInode(curInode->id,*curInode);

                    fputc(EOF,file);
                    curInode = tmpInode;

                    fclose(sfile);

//                    printf("\n");
                    return;
                }
            }
        }

        //if file is larger than origin file, get new block
        while (1) {
            size_t new_id = freeBlocks.front();
            freeBlocks.pop_front();

            curInode->i_addr[blockcount] = new_id;
            updateBlockBitmap(new_id);

            fseek(file,boffset + Block_Size * new_id, SEEK_SET);

            blockcount++;

            for (i = 0 ; i < Block_Size ; ++i) {
                if ((c = fgetc(sfile)) != EOF) {
                    filesize++;
//                    printf("%c",c);
                    fputc(c,file);
                } else {
                    curInode->blockNum = blockcount;
                    curInode->fileSize = filesize;
                    fputc(EOF,file);
                    updateInode(curInode->id,*curInode);
                    for (size_t i = 0 ; i < curInode->blockNum ; ++i) {
                        printf("write: %d\n",curInode->i_addr[i]);
                    }

                    curInode = tmpInode;
                    fclose(sfile);

//                    printf("\n");

                    return;
                }
            }
        }
    } else {
        printf("Cannot write this file!\n");
        curInode = tmpInode;
        fclose(sfile);
        return;
    }

}

void FileSystem::cp_helper(std::string file_1, std::string file_2)
{
    printf("copy file ...\n");
    pfcb sourceInode, destInode, tmpInode;
    tmpInode = curInode;

    int dest_id = touch_helper(file_2);
    if (dest_id < 0) return;
    destInode = pinodes[dest_id];

    curInode = tmpInode;

    printf("cp: %s %s\n",curInode->name,destInode->name);

    printf("read file ...\n");

    size_t i;
    bool filefound = false;

    //get path and filename
    std::string pattern("/");
    std::string dir("");
    std::vector<std::string> path = split(file_1,pattern);
    for (i = 0 ; i < path.size() - 1 ; ++i) {
        dir.append(path[i]).append("/");
//        std::cout << path[i] << std::endl;
    }
//    std::cout << dir << std::endl;

    //change to the file/s dir
    if (cd_helper(dir) < 0 ) {
        printf("cp: cd dir fail !\n");
        curInode = tmpInode;
        return;
    }

    //find the file
    for (i = 0 ; i < curInode->blockNum ; ++i) {
        if (path[path.size() - 1] == pinodes[curInode->i_addr[i]]->name
                && pinodes[curInode->type == 0]) {
            sourceInode = pinodes[curInode->i_addr[i]];
//            printf("cp: find file_1 success !\n");
            filefound = true;
            curInode = tmpInode;
            break;
        }
    }

    if (!filefound) {
        printf("cp:file_1 is not been found !\n");
        curInode = tmpInode;
        return;
    }

    size_t old_blockNum, new_blockNum, block_index;
    char buf[Block_Size] = "\0";
    old_blockNum = destInode->blockNum;
    new_blockNum = sourceInode->blockNum;

//    printf("cp: src blocknum: %d name : %s\n",sourceInode->blockNum, sourceInode->name);
//    printf("cp: dst blocknum: %d name : %s\n",destInode->blockNum, destInode->name);

    if (new_blockNum > old_blockNum) {
        for (size_t i = old_blockNum ; i < new_blockNum ; ++i) {
            block_index = freeBlocks.front();
            freeBlocks.pop_front();
            destInode->i_addr[i] = block_index;
            destInode->blockNum +=1;
            updateBlockBitmap(block_index);
        }
    } else if (new_blockNum < old_blockNum) {
        for (size_t i = new_blockNum ; i < old_blockNum ; ++i) {
            freeBlock(destInode->i_addr[i]);
            destInode->blockNum -= 1;
        }
    }

//    printf("cp: src blocknum: %d name : %s\n",sourceInode->blockNum, sourceInode->name);
//    printf("cp: dst blocknum: %d name : %s\n",destInode->blockNum, destInode->name);

    for (size_t i = 0 ; i < sourceInode->blockNum ; ++i) {
        fseek(file,boffset + sourceInode->i_addr[i] * Block_Size, SEEK_SET);
        fread(buf,Block_Size,1,file);
        printf("cp: src block nums: %d i: %d name : %s\n",sourceInode->i_addr[i],i,sourceInode->name);
//        printf("cp: %s\n",buf);
        fseek(file,boffset + destInode->i_addr[i] * Block_Size, SEEK_SET);
        fwrite(buf,Block_Size,1,file);
    }


    curInode = tmpInode;
}

void FileSystem::rm_helper(std::__cxx11::string filename)
{
    printf("delelte file ...\n");
    char buf[Block_Size] = "\0";
    size_t i;
    bool isFound = false;


    std::string pattern("/");
    std::string dir("");
    std::vector<std::string> path = split(filename,pattern);
    pfcb tmpInode = curInode;
    for (i = 0 ; i < path.size() - 1 ; ++i) {
        dir.append(path[i]).append("/");
//        std::cout << path[i] << std::endl;
    }
//    std::cout << dir << std::endl;


    if (cd_helper(dir) < 0 ) {
        printf("rm: cd dir fail !\n");
        curInode = tmpInode;
        return ;
    }

//    printf("rm: %s\n",curInode->name);
//    printf("rm: %s\n",path[path.size()-1].data());
//    printf("rm: %d\n",curInode->blockNum);

    for (i = 0 ; i < curInode->blockNum ; ++i) {
        if (path[path.size() - 1] == pinodes[curInode->i_addr[i]]->name) {
//            printf("Find file !\n");
            isFound = true;
            pfcb tt = pinodes[curInode->i_addr[i]];
            curInode->i_addr[i] = 0;
            curInode->blockNum -= 1;
            updateInode(curInode->id,*curInode);
            curInode = tt;
            break;
        }
    }

    if (!isFound) {
        printf("No such file !\n");
        curInode = tmpInode;
        return;
    }

    printf("rm: %s\n",curInode->name);

    for (i = 0 ; i < curInode->blockNum ; ++i) {
        fseek(file,boffset + Block_Size * curInode->i_addr[i],SEEK_SET);
        fwrite(buf,Block_Size,1,file);
        updateBlockBitmap(curInode->i_addr[i]);
    }

    //free inode
    freeInode(curInode->id);

    curInode = tmpInode;
}

void FileSystem::exit_helper()
{
    printf("exit file system ...\n");
    fclose(file);
}

int FileSystem::cd_helper(std::string dirname)
{
    bool haveItem = false;
    bool isDir;
    std::size_t i, j;
    std::string pattern("/");
//    std::cout << dirname << std::endl;
    std::vector<std::string> path = split(dirname,pattern);
    pfcb tmpInode, recInode;
    recInode = curInode;
//    for (i = 0 ; i < path.size() ; ++i) std::cout << path[i] << std::endl;
//    std::cout << path.size() <<std::endl;
//    std::cout << dirname << std::endl;
    if (dirname.data()[0] == '/') {
        tmpInode = pinodes[0];
        for (i = 0; i < path.size() ; ++i) {
            if (path[i] == std::string("")) continue;
//            std::cout << path[i] << std::endl;
            haveItem = false;
            isDir = false;
            for (j = 0 ; j < tmpInode->blockNum ; ++j) {
                if (std::string(pinodes[tmpInode->i_addr[j]]->name) == path[i]) {
                    haveItem = true;
                    if (pinodes[tmpInode->i_addr[j]]->type == 1) {
                        isDir = true;
                        tmpInode = pinodes[tmpInode->i_addr[j]];
                    } else isDir = false;
                    break;
                }
            }
            if (!haveItem) {
                printf("No such director !\n");
                curInode = recInode;
                return -1;
            }
            if (!isDir) {
                std::cout << pinodes[tmpInode->i_addr[j]]->name << " is not a directory" << std::endl;
                curInode = recInode;
                return -1;
            }
//            if (i == path.size() - 1) curInode = tmpInode;
            curInode = tmpInode;
        }
    } else {
        for (i = 0 ; i < path.size() ; ++i) {
            if (path[i] == std::string("")) continue;
            haveItem = false;
            isDir = false;
            for (j = 0 ; j < curInode->blockNum ; ++j) {
                if (std::string(pinodes[curInode->i_addr[j]]->name) == path[i]) {
                    haveItem = true;
                    if (pinodes[curInode->i_addr[j]]->type == 1) {
                        isDir = true;
                        curInode = pinodes[curInode->i_addr[j]];
                    } else isDir = false;
                    break;
                }
            }
            if (!haveItem) {
                printf("No such director !\n");
                curInode = recInode;
                return -1;
            }
            if (!isDir) {
                std::cout << pinodes[curInode->i_addr[j]]->name << " is not a directory" << std::endl;
                curInode = recInode;
                return -1;
            }
        }
    }
    return 0;
}

void FileSystem::ls_helper()
{
    for (unsigned int i = 0 ; i < curInode->blockNum ; ++i) {
        if (pinodes[curInode->i_addr[i]]->type == 0)
            printf("%s\t",pinodes[curInode->i_addr[i]]->name);
        else
            printf("\033[;34m%s\033[0m\t",pinodes[curInode->i_addr[i]]->name);
        if (i == curInode->blockNum - 1) printf("\n");
    }
}

//drwxr-xr-x   2 root root 0 3月   5 08:02 block
void FileSystem::ls_helper(std::__cxx11::string option)
{
    if (option == "-l") {
        for (unsigned int i = 0 ; i < curInode->blockNum ; ++i) {
            if (pinodes[curInode->i_addr[i]]->type == 1) printf("d");
            else printf("-");
            for (size_t j = 0 ; j < 3 ; ++j) {
                if (pinodes[curInode->i_addr[i]]->limit[j] == '7')
                    printf("rwx");
                if (pinodes[curInode->i_addr[i]]->limit[j] == '6')
                    printf("rw-");
                if (pinodes[curInode->i_addr[i]]->limit[j] == '5')
                    printf("r-x");
                if (pinodes[curInode->i_addr[i]]->limit[j] == '4')
                    printf("r--");
                if (pinodes[curInode->i_addr[i]]->limit[j] == '3')
                    printf("-wx");
                if (pinodes[curInode->i_addr[i]]->limit[j] == '2')
                    printf("-w-");
                if (pinodes[curInode->i_addr[i]]->limit[j] == '1')
                    printf("--x");
            }
            printf("\t");
            printf("2\thaines\thaines\t");
            if (pinodes[curInode->i_addr[i]]->type == 1)
                printf("%d\t",pinodes[curInode->i_addr[i]]->blockNum*Block_Size);
            else
                printf("%d\t",pinodes[curInode->i_addr[i]]->fileSize);
            for (size_t j = 0 ; j < 4; ++j) printf("%d.",pinodes[curInode->i_addr[i]]->time[j]);
            printf("%d\t",pinodes[curInode->i_addr[i]]->time[4]);
            if (pinodes[curInode->i_addr[i]]->type == 0)
                printf("%s\n",pinodes[curInode->i_addr[i]]->name);
            else
                printf("\033[;34m%s\033[0m\n",pinodes[curInode->i_addr[i]]->name);
        }
    } else return;


}

void FileSystem::mkdir_helper(std::__cxx11::string dirname)
{
    int new_id = freeInodes.front();
//    std::cout << "mkdir: " << new_id << std::endl;
    pfcb pnd = pinodes[new_id];
    freeInodes.pop_front();

    //update bitmap
    updateInodeBitmap(new_id);

    //update inode in memory
    pnd->id = new_id;
    pnd->blockNum = 0;
    pnd->fileSize = 0;
    pnd->pid = curInode->id;
    pnd->type = 1;
    strcpy(pnd->name,dirname.data());
    strcpy(pnd->limit,"755");

    unsigned short time[5];
    curTime(time);
    for (size_t i = 0 ; i < 5 ; ++i) pnd->time[i] = time[i];

    //update parent inode
    curInode->i_addr[curInode->blockNum] = new_id;
    curInode->blockNum += 1;

    //update inodes in disk
    updateInode(curInode->id,*curInode);
    updateInode(pnd->id,*pnd);
}

int FileSystem::touch_helper(std::__cxx11::string filename)
{
    size_t i;
    int new_id = freeInodes.front();
//    std::cout << "touch: " << new_id << std::endl;
    pfcb pnd = pinodes[new_id];
    freeInodes.pop_front();

    std::string pattern("/");
    std::string dir("");
    std::vector<std::string> path = split(filename,pattern);
    pfcb tmpInode = curInode;
    for (i = 0 ; i < path.size() - 1 ; ++i) {
        dir.append(path[i]).append("/");
//        std::cout << path[i] << std::endl;
    }
//    std::cout << dir << std::endl;


    if (cd_helper(dir) < 0 ) {
        printf("touch: cd dir fail !\n");
        return -1;
    }

    for (i = 0 ; i < curInode->blockNum ; ++i) {
        if (path[path.size() - 1] == pinodes[curInode->i_addr[i]]->name) {
            printf("File exist !\n");

            return curInode->i_addr[i];
        }
    }

    //update bitmap
    updateInodeBitmap(new_id);

    //update inode in memory
    pnd->id = new_id;
    pnd->blockNum = 0;
    pnd->fileSize = 0;
    pnd->pid = curInode->id;
    pnd->type = 0;                      //file
    strcpy(pnd->limit,"755");

    unsigned short time[5];
    curTime(time);
    for (size_t i = 0 ; i < 5 ; ++i) pnd->time[i] = time[i];
    strcpy(pnd->name,path[path.size() - 1].data());

    //update parent inode
    curInode->i_addr[curInode->blockNum] = new_id;
    curInode->blockNum += 1;

    //update inodes in disk
    updateInode(curInode->id,*curInode);
    updateInode(pnd->id,*pnd);

    //restore curInode
    curInode = tmpInode;

    return new_id;
}

void FileSystem::mv_helper(std::__cxx11::string src, std::__cxx11::string dest)
{
    pfcb srcInode = findPath(src);
    pfcb destInode = findPath(dest);
    std::string pattern("/");
    std::vector<std::string> srcp, destp;
    srcp = split(src,pattern);
    destp = split(dest,pattern);
    std::string  srcfile = srcp.back();
    std::string destfile = destp.back();
    for (size_t i = 0 ; i < srcInode->blockNum ; ++i) {
        if (pinodes[srcInode->i_addr[i]]->name == srcfile) {
            srcInode = pinodes[srcInode->i_addr[i]];
            break;
        }
    }

    printf("mv: %s %s\n",srcInode->name,destInode->name);

    destInode->i_addr[destInode->blockNum] = srcInode->id;
    pinodes[srcInode->pid]->blockNum -=1;
    updateInode(pinodes[srcInode->pid]->id,*pinodes[srcInode->pid]);
    srcInode->pid = destInode->id;
    destInode->blockNum += 1;
    updateInode(destInode->id, *destInode);
    updateInode(srcInode->id, *srcInode);
}

void FileSystem::cat_helper(std::__cxx11::string filename)
{
    size_t blockcount = 0, i;
    bool filefound = false;
    char c;

    pfcb tmpInode = curInode;

    curInode = findPath(filename);

    std::string pattern("/");
    std::vector<std::string> path;
    path = split(filename,pattern);
    std::string  nfile = path.back();

    //find the file
    for (i = 0 ; i < curInode->blockNum ; ++i) {
        if (nfile == pinodes[curInode->i_addr[i]]->name
                && pinodes[curInode->type == 0]) {
            curInode = pinodes[curInode->i_addr[i]];
//            printf("read: find file success !\n");
            filefound = true;
            break;
        }
    }

    if (!filefound) {
        printf("read: no such file !\n");
        curInode = tmpInode;
        return;
    }

    //write in blocks the file has firstly
    for (blockcount = 0 ; blockcount < curInode->blockNum ; ++blockcount) {
        fseek(file, boffset + curInode->i_addr[blockcount] * Block_Size , SEEK_SET);
        for (i = 0 ; i < Block_Size ; ++i) {
            if ((c = fgetc(file)) != EOF) {
                printf("%c",c);
            } else {
                curInode = tmpInode;
                printf("\n");
                return;
            }
        }
    }

    curInode = tmpInode;
}

void FileSystem::chmod_helper(std::__cxx11::string mod, std::__cxx11::string filename)
{
    size_t i;
    bool filefound = false;

    pfcb tmpInode = curInode;

    curInode = findPath(filename);

    std::string pattern("/");
    std::vector<std::string> path;
    path = split(filename,pattern);
    std::string  nfile = path.back();

    //find the file
    for (i = 0 ; i < curInode->blockNum ; ++i) {
        if (nfile == pinodes[curInode->i_addr[i]]->name
                && pinodes[curInode->type == 0]) {
            curInode = pinodes[curInode->i_addr[i]];
//            printf("read: find file success !\n");
            filefound = true;
            break;
        }
    }

    if (!filefound) {
        printf("read: no such file !\n");
        curInode = tmpInode;
        return;
    }

    strcpy(curInode->limit,mod.data());

    updateInode(curInode->id,*curInode);

    curInode = tmpInode;
}

void FileSystem::format_helper()
{
    printf("Format this file system ? [y/n]\n");
    char c;
    scanf("%c",&c);
    if (c == '\n' || c == 'y'){
        file_sys_create();
        file_sys_init();
    }
    else
        return;
}


//split input string
std::vector<std::__cxx11::string> FileSystem::split(std::__cxx11::string str, std::__cxx11::string pattern)
{
    std::string::size_type pos;
    std::vector<std::string> result;
    str+=pattern;//扩展字符串以方便操作
    size_t size=str.size();

    for(size_t i=0; i<size; i++)
    {
      pos=str.find(pattern,i);
      if(pos<size)
      {
        std::string s=str.substr(i,pos-i);
        result.push_back(s);

        i=pos+pattern.size()-1;
      }
    }
    return result;
}

int FileSystem::cmd_handle(std::vector<std::string> cmd)
{
    const char* tmpStr = cmd[0].data();

    if (strcmp(tmpStr,"help") == 0){
        showHelper();
        return HELP;
    }
    if (strcmp(tmpStr,"exit") == 0){
        exit_helper();
        return EXIT;
    }
    if (strcmp(tmpStr,"open") == 0){
        open_helper();
        return OPEN;
    }
    if (strcmp(tmpStr,"close") == 0){
        close_helper();
        return CLOSE;
    }
    if (strcmp(tmpStr,"read") == 0){
        read_helper(cmd[1]);
        return READ;
    }
    if (strcmp(tmpStr,"write") == 0){
        write_helper(cmd[1]);
        return WRITE;
    }
    if (strcmp(tmpStr,"cp") == 0){
        cp_helper(cmd[1],cmd[2]);
        return CP;
    }
    if (strcmp(tmpStr,"rm") == 0){
        rm_helper(cmd[1]);
        return RM;
    }
    if (strcmp(tmpStr,"cd") == 0){
        if (cmd.size() == 2) {
            if (cmd[1] == std::string("..")) {
                curInode = pinodes[curInode->pid];
            } else if (cmd[1] == std::string(".")) {

            } else if (cmd[1] == std::string("~") || cmd[1] == std::string("/")) {
                curInode = pinodes[0];
            } else cd_helper(cmd[1]);
        }
        return CD;
    }
    if (strcmp(tmpStr,"ls") == 0){
        cmd.size() == 1 ? ls_helper() : ls_helper(cmd[1]);
        return LS;
    }
    if (strcmp(tmpStr,"mkdir") == 0){
        mkdir_helper(cmd[1]);
        return MKDIR;
    }
    if (strcmp(tmpStr,"touch") == 0) {
        touch_helper(cmd[1]);
        return TOUCH;
    }
    if (strcmp(tmpStr,"sysinfo") == 0) {
        showSysInfo();
        return SYSINFO;
    }
    if (strcmp(tmpStr,"clear") == 0) {
        clr();
        return CLEAR;
    }
    if (strcmp(tmpStr,"tree") == 0) {
        tree();
        return TREE;
    }
    if (strcmp(tmpStr,"mv") == 0) {
        mv_helper(cmd[1],cmd[2]);
        return MV;
    }
    if (strcmp(tmpStr,"cat") == 0) {
        cat_helper(cmd[1]);
        return CAT;
    }
    if (strcmp(tmpStr,"chmod") == 0) {
        chmod_helper(cmd[1],cmd[2]);
        return CHMOD;
    }
    if (strcmp(tmpStr,"format") == 0) {
        format_helper();
        return FORMAT;
    }
    return 100;
}

void FileSystem::showPath()
{
    printf("%s:",curInode->name);
}

void FileSystem::showHelper()
{
    printf("command: \n\
    help    ---  show help menu \n\
    sysinfo ---  show system base information \n\
    clear   ---  clear the screen \n\
    ls      ---  list the digest of the directory's children \n\
    ls -l   ---  list the detail of the directory's children \n\
    cd      ---  change directory \n\
    mkdir   ---  make directory   \n\
    touch   ---  create a new file \n\
    cat     ---  read a file \n\
    write   ---  write something to a file \n\
    rm      ---  delete a directory or a file \n\
    cp      ---  cp a directory file to another directory or file (not finish)\n\
    chmod   ---  change the authorizatino of a directory or a file \n\
    mv      ---  move a file to another dir \n\
    format  ---  format this file system \n\
    exit    ---  exit this system\n");
}

void FileSystem::showSysInfo()
{
    SuperBlock tmpSurperBlock;
    fseek(file,soffset,SEEK_SET);
    fread(&tmpSurperBlock,superBlockSize,1,file);
    printf("System Information:\n"
           "type: %s\n"
           "blocknum: %d\n"
           "blocksize: %d\n",tmpSurperBlock.type,tmpSurperBlock.blockNum,tmpSurperBlock.blockSize);
}

void FileSystem::clr()
{
    system("clear");
}

void FileSystem::tree()
{
    printDir(curInode,0);
}

void FileSystem::printDir(pfcb pnd, size_t depth)
{
    if (depth > 0) {
        for (size_t i = 0 ; i < depth * 4 - 4; ++i) printf(" ");
        printf("|");
        for (size_t i = 0 ; i < 3 ;++i) printf("-");
    }
    printf("%s\n",pnd->name);
    if (pnd->blockNum == 0 || pnd->type == 0) return;
    for (size_t i = 0 ; i < pnd->blockNum ; ++i) {
//        printf("%s\n",pinodes[pnd->i_addr[i]]->name);
        printDir(pinodes[pnd->i_addr[i]],depth + 1);
    }
}

void FileSystem::login()
{
    char password[50];
    char user[50];
    while (1) {
        std::cout << "user:";
        scanf("%s",user);
        std::cout << "password:";
        scanf("%s",password);
        if (strcmp(user,"haines") == 0 && strcmp(password,"123456") == 0) break;
    }
}



