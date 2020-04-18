#include <iostream>
#include <unistd.h>
#include <fcntl.h>
#include <fstream>
#include <string.h>
#include <typeinfo>
#include <iomanip>
#include<sstream>
#include <bits/stdc++.h>

using namespace std;

// Function prototypes
// This is the function read the header of the disk
struct VDIFile *vdiOpen(const char *);

// This is the partition open function
struct PartitionFile *partitionOpen(struct VDIFile *, struct PartitionEntry);

// This is the displayBufferPage function
void displayBufferPage (uint8_t *, uint32_t, uint32_t, uint64_t);

// This is the displayBuffer function1
void displayBuffer(uint8_t *, uint32_t, uint64_t);

// This is the vdiClose function
void vdiClose (struct VDIFile *);

// This is the vdiRead function
ssize_t vdiRead (struct VDIFile *, void *, int);

// This is the vdiWrite function
ssize_t vdiWrite (struct VDIFile *, void *, int);

// This is the vdiSeek function
off_t vdiSeek (VDIFile *, off_t, int);

// This is the function to close the partition
void partitionClose(struct PartitionFile *);

// This is the function partitionRead
ssize_t partitionRead(struct PartitionFile *,void *, int);

// This is the function partitionWrite
ssize_t partitionWrite(struct PartitionFile *,void *, size_t);

// This is the function partitionSeek
off_t partitionSeek(struct PartitionFile *, off_t, int);

// This is the ext2Open function
struct Ext2File *ext2Open(char *fn, int32_t pNum);

void ext2Close (struct Ext2File *f);

int32_t fetchBlock(struct Ext2File *,uint32_t, void *);

int32_t writeBlock(struct Ext2File *,uint32_t , void *);

int32_t fetchSuperblock(struct Ext2File *,uint32_t, struct Ext2Superblocks *);

int32_t writeSuperblock(struct Ext2File *,uint32_t , struct Ext2Superblock *);

void dumpSuperBlock(Ext2Superblock *);

int32_t fetchBGDT(struct Ext2File *,uint32_t, struct Ext2BlockGroupDescriptor *);

int32_t writeBGDT(struct Ext2File *,uint32_t, struct Ext2BlockGroupDescriptor *);

int32_t fetchInode(struct Ext2File * ,uint32_t, struct Inode *);

int32_t writeInode(struct Ext2File *,uint32_t, struct Inode *);

// This is to convert char into hex
struct HexCharStruct {
	unsigned char c;
	HexCharStruct (unsigned char _c) : c(_c) {

	}
};

inline std::ostream& operator << (std::ostream& o, const HexCharStruct& hs) {
	return (o << std::hex << (int) hs.c);
}

inline HexCharStruct hex (unsigned char _c) {
	return HexCharStruct(_c);
}

// Structures declaration

struct HeaderStructure {
    /** Just text info about image type, for eyes only. */
    char szFileInfo[64];
    /** The image signature (VDI_IMAGE_SIGNATURE). */
    uint32_t u32Signature;
    /** The image version (VDI_IMAGE_VERSION). */
    uint32_t u32Version;
     /** Size of this structure in bytes. */
    uint32_t cbHeader;
    /** The image type (VDI_IMAGE_TYPE_*). */
    uint32_t u32Type;
    /** Image flags (VDI_IMAGE_FLAGS_*). */
    uint32_t fFlags;
    /** Image comment. (UTF-8) */
    char szComment[256];
    /** Offset of blocks array from the beginning of image file.
     * Should be sector-aligned for HDD access optimization. */
    uint32_t offBlocks;
    /** Offset of image data from the beginning of image file.
     * Should be sector-aligned for HDD access optimization. */
    uint32_t offData;
    /** Cylinders. */
    uint32_t    cCylinders;
    /** Heads. */
    uint32_t    cHeads;
    /** Sectors per track. */
    uint32_t    cSectors;
    /** Sector size. (bytes per sector) */
    uint32_t    cbSector;
    /** Was BIOS HDD translation mode, now unused. */
    uint32_t u32Dummy;
    /** Size of disk (in bytes). */
    uint64_t cbDisk;
    /** Block size. (For instance VDI_IMAGE_BLOCK_SIZE.) Should be a power of 2! */
    uint32_t cbBlock;
    /** Size of additional service information of every data block.
     * Prepended before block data. May be 0.
     * Should be a power of 2 and sector-aligned for optimization reasons. */
    uint32_t cbBlockExtra;
    /** Number of blocks. */
    uint32_t cBlocks;
    /** Number of allocated blocks. */
    uint32_t cBlocksAllocated;
    /** UUID of image. */
    char uuidCreate[16];
    /** UUID of image's last modification. */
    char uuidModify[16];
    /** Only for secondary images - UUID of previous image. */
    char uuidLinkage[16];
    /** Only for secondary images - UUID of previous image's last modification. */
    char uuidParentModify[16];
};

struct Inode {
    uint16_t i_mode;
    uint16_t i_uid;
    uint32_t i_size;
    uint32_t i_atime;
    uint32_t i_ctime;
    uint32_t i_mtime;
    uint32_t i_dtime;
    uint16_t i_gid;
    uint16_t i_links_count;
    uint32_t i_blocks;
    uint32_t i_flags;
    uint32_t i_osd1;
    uint32_t i_block[15];
    uint32_t i_generation;
    uint32_t i_file_acl;
    uint32_t i_sizeHigh;
    uint32_t i_faddr;
    uint16_t i_blocksHigh;
    uint16_t reserved16;
    uint16_t i_uidHigh;
    uint16_t i_gidHigh;
    uint32_t reserved32;
};

struct PartitionEntry {
    /** status */
    uint8_t status;
    /** chsStart */
    uint8_t chsStart[3];
    /** Type */
    uint8_t type;
    /** chsEnd */
    uint8_t chsEnd[3];
    /** LBA Start */
    uint32_t LBAStart;
    /** nSectors */
    uint32_t nSectors;
};

struct VDIFile {
    /** File descriptor */
    int fileDescriptor;
    /** Header structure */
    HeaderStructure header;
    /** Cursor */
    int cursor;
};

struct PartitionFileTables {
    /** VDI File */
    VDIFile * vdiFile;
    /** Partition table*/
    PartitionEntry table[4];
};

struct PartitionFile {
    /** VDI File */
    VDIFile * vdiFile;
    /** Partition Entry */
    PartitionEntry partitionEntry;
};

struct Ext2BlockGroupDescriptor {
    uint32_t bg_block_bitmap;
    uint32_t bg_inode_bitmap;
    uint32_t bg_inode_table;
    uint16_t bg_free_blocks_count;
    uint16_t bg_free_inodes_count;
    uint16_t bg_used_dirs_count;
    uint16_t bg_pad;
    char bg_reserved[12];
};

struct Ext2Superblocks {
    uint32_t s_inodes_count;
    uint32_t s_blocks_count;
    uint32_t s_r_blocks_count;
    uint32_t s_free_blocks_count;
    uint32_t s_free_inodes_count;
    uint32_t s_first_data_block;
    uint32_t s_log_block_size;
    uint32_t s_log_frag_size ;
    uint32_t s_blocks_per_group;
    uint32_t s_frags_per_group;
    uint32_t s_inodes_per_group;
    time_t s_mtime;
    time_t s_wtime;
    uint16_t s_mnt_count;
    uint16_t s_max_mnt_count;
    uint16_t s_magic;
    uint16_t s_state;
    uint16_t s_errors;
    uint16_t s_minor_rev_level;
    time_t s_lastcheck;
    time_t s_checkinterval;
    uint32_t s_creator_os;
    uint32_t s_rev_level;
    uint16_t s_def_resuid;
    uint16_t s_def_resgid;
    uint32_t s_first_ino;
    uint16_t s_inode_size;
    uint16_t s_block_group_nr;
    uint32_t s_feature_compat;
    uint32_t s_feature_incompat;
    uint32_t s_feature_ro_compat;
    char        s_uuid[16];
    char s_volume_name[16];
    char        s_last_mounted[64];
    uint32_t s_algo_bitmap;
    unsigned char s_prealloc_blocks;
    unsigned char_prealloc_dir_blocks;
    unsigned short s_padding_1;
    unsigned  char s_journal_uuid[16];
    uint32_t s_journal_inum;
    uint32_t        s_journal_dev;
    uint32_t s_last_orphan;
    uint32_t        s_hash_seed[4];
    uint8_t s_def_hash_version;
    unsigned char s_reserved_char_pad;
    unsigned short s_reserved_word_pad;
    uint32_t        s_default_mount_options;
    uint32_t s_first_meta_bg;
    unsigned int s_reserved[190];
};

//Initializing I/O functions for Ext2File access
struct blocks{};

struct Ext2File {

    /** VDI File */
    VDIFile vdiFile;

    /** Partition File */
    PartitionFile partitionFile;

    ssize_t numberofBlockGroups;

    /** One super block */
    Ext2Superblocks superBlocks;

    /** One array of block group descriptors */
    Ext2BlockGroupDescriptor * blockGroupDescriptorstable;

    blocks blocks;
};

int main () {
    Ext2File *ext2File= ext2Open("Test-dynamic-1k.vdi",0);

    /**
    cout << (*(ext2File -> blockGroupDescriptorstable + 15)).bg_block_bitmap << endl;

    cout << (*ext2File).superBlocks.s_inodes_per_group;

    Inode * inodePtr = new Inode;

    fetchInode(ext2File, 11000, inodePtr);

    return 0;
    */
    // Group number where the desired inode is located
    int group = (11000 - 1) / (*ext2File).superBlocks.s_inodes_per_group;

    // Find the index inside the target group
    int index = (11000 - 1) % (*ext2File).superBlocks.s_inodes_per_group;

    // Find the number of inode per group
    int numOfInodePerGroup = (*ext2File).superBlocks.s_inode_size / (*ext2File).superBlocks.s_log_block_size;

    // Find the block number inside the group
    int block = index / numOfInodePerGroup;

    // Array of Inodes
    //Inode * arrayOfInodes = new Inode[1024];
    char arrayOfInodes [1024];

    // Index of the desired inode inside the block
    index = index % numOfInodePerGroup;

    cout << "Block number to fetch " << (*(ext2File -> blockGroupDescriptorstable + group)).bg_inode_table + block << endl;
    cout << "Number of inodes per group " << numOfInodePerGroup;

    // Fetch one block
    fetchBlock(ext2File, 8192, arrayOfInodes);

    //printf("%x", arrayOfInodes[index]);
}

void displayBufferPage (uint8_t *buffer, uint32_t count, uint32_t start, uint64_t offset) {

    int fileHandler = open("Test-dynamic-1k.vdi", O_RDONLY);

    lseek(fileHandler, offset, SEEK_CUR);

    read(fileHandler, buffer, count);

	cout << "   00 01 02 03 04 05 06 07 08 09 0a 0b 0c 0d 0e 0f     0...4...8...c..." << endl;
	cout << "  +------------------------------------------------+  +----------------+" << endl;

	// First row
	for (int j = 0; j < 16; j++) {
		cout << setfill('0') << setw(2) << right << hex << hex(j) << "|";
		for (int i = (j*16); i < ((j*16) + 16); i++) {
			if(start <= offset && i + 1 <= start + count){
				cout << setfill('0') << setw(2) << right << hex << hex(*(buffer + i));
					cout << " ";
			} else {
				cout << "   ";
			}
		}

		cout << "|" << setfill('0') << setw(2) << right << hex << hex(j) <<"|";
		for (int i = (j*16); i < ((j*16) + 16); i++) {
			if(start <= offset &&  i + 1 + offset <= start + count){
				if (isprint(*(buffer + i + offset))) {
					cout << *(buffer + i + offset);
				} else {
					cout << " ";
				}
			} else {
				cout << " ";
			}
		}
		cout << "|";
		cout << endl;
	}
	cout << endl;
}

void displayBuffer(uint8_t *buffer, uint32_t count, uint64_t offset){
	uint64_t originalCount = count;
	uint64_t currentOffset = offset;
	cout << "Num of time to loop " << count/256 + 1 << endl;
	for (int i = 0; i < ((int)count/256)+1; i++){
		if(originalCount >= 256){
			displayBufferPage(buffer, 256, 0, currentOffset);
			originalCount -= 256;
			currentOffset += 256;
		} else if (originalCount < 256){
			displayBufferPage(buffer, 256-originalCount, 0, currentOffset);
		}
	}
}

struct VDIFile *vdiOpen (const char * fn) {
    // Establish connection to the disk
	int fileIndex = open (fn, O_RDONLY);

    struct VDIFile *vdiFileStruct = new VDIFile;

	// Read the header into the header structure
	read(fileIndex, &(vdiFileStruct -> header), 400);

	vdiFileStruct -> fileDescriptor = fileIndex;
	vdiFileStruct -> cursor = 0;

	// Return the pointer to the structure
	return vdiFileStruct;
}

ssize_t vdiRead(struct VDIFile *f, void *buf, int counts) {
    // Use lseek to seek the cursor to the location need to be read
    // The location to start reading is the current value of the VDIFile cursor plus the data offset
    lseek(f -> fileDescriptor, f -> cursor, SEEK_SET);

    // Read the given number of bytes into the buffer passed to this function as a parameter
    int numOfBytesRead = read(f->fileDescriptor, buf, counts);

    // Advance the cursor by the number of bytes read
    (*f).cursor += numOfBytesRead;

    // Return the number of bytes read
    return numOfBytesRead;
}

ssize_t vdiWrite(struct VDIFile *f, void *buf, int counts) {
    // Use lseek to seek the cursor to the location need to be read
    // The location to start writing is the current value of VDIFile cursor plus the data offset
    lseek(f -> fileDescriptor, f -> cursor, SEEK_SET);

    // Write the given number of bytes from the given buffer to the disk
    int numOfBytesWritten = write((*f).fileDescriptor, buf, counts);

    // Advance the cursor by the number of bytes read
    (*f).cursor += numOfBytesWritten;

    // Return the number of bytes read
    return numOfBytesWritten;
}

off_t vdiSeek (struct VDIFile *f, off_t offset, int anchor) {
    // New location of the cursor
    off_t newLoc;

    // Set the right new location based on the anchor
    if (anchor == SEEK_SET) {
        newLoc = offset + f -> header.offData;
    } else if (anchor == SEEK_CUR) {
        newLoc = offset + (*f).cursor;
    } else if (anchor == SEEK_END) {
        newLoc = offset + (*f).header.cbDisk;
    } else {
        newLoc = (*f).cursor;
    }

    // Check if new location is valid or not
    if (newLoc >= 0 && newLoc <= (*f).header.cbDisk) {
        (*f).cursor = newLoc;
    }

    // Return the new location
    return newLoc;
}

void vdiClose(struct VDIFile *f) {
    // Close the file whose file handler is given
    close((*f).fileDescriptor);

    // Deallocate created memory regions
    delete f;
}

void partitionClose (struct PartitionFile *f) {
    // Pointer to the vdiFile of the PartitionFile
    VDIFile * vdiFile;

    vdiFile = (*f).vdiFile;

    // Close the file whose handler is given
    close((*vdiFile).fileDescriptor);

    // Deallocate created memory regions
    delete f;
}

struct PartitionFile *partitionOpen(struct VDIFile * vdiFile, struct PartitionEntry partitionEntry) {
	// Pointer to the PartitionFile structure
	PartitionFile *ptr2 = new PartitionFile;
    ptr2->vdiFile = vdiFile;
    ptr2->partitionEntry = partitionEntry;

	// Return the pointer
	return ptr2;
}

ssize_t partitionRead(struct PartitionFile *f, void *buf, int counts) {
    if ((*f).partitionEntry.LBAStart * 512 <= (*f).vdiFile->cursor &&
     ((counts + (*f).vdiFile->cursor) <=  ((*f).partitionEntry.LBAStart+ (*f).partitionEntry.nSectors)*512)){
        lseek(f->vdiFile->fileDescriptor,f->vdiFile->cursor,SEEK_SET);
        int num = vdiRead(f->vdiFile,buf,counts);
        return num;
    }
    else {
        return -1;
    }
}

ssize_t partitionWrite(struct PartitionFile *f, void *buf, size_t count) {
    if (((*f).partitionEntry.LBAStart < ((*f).vdiFile -> cursor)) && ((count + (*f).vdiFile -> cursor) < (*f).partitionEntry.nSectors)) {
            return vdiWrite(f -> vdiFile, buf, count);
        } else {
            return 0;
        }
}

off_t partitionSeek(struct PartitionFile *f,off_t offset,int anchor) {
    //create a new location within the file
    off_t location;

    //Seek starts from the location of the first sector of the partition
    location = vdiSeek((*f).vdiFile , offset + (*f).partitionEntry.LBAStart*512, SEEK_SET);

    //no change in location of cursor if cursor is moved outside the first partition
    if (location < (*f).partitionEntry.LBAStart * 512 &&
         location >=  ( (*f).partitionEntry.LBAStart+ (*f).partitionEntry.nSectors)*512) {
        location = (*f).vdiFile->cursor;
        cout << "too big";
    }
    else {
        (*f).vdiFile->cursor = location;
    }

    return (*f).vdiFile->cursor;
}

void readPartitionEntry( struct VDIFile* vdiFile,PartitionEntry partitionEntry[4]){
    // Use vdiSeek to seek to the location of the partition table
    vdiSeek(vdiFile, 446, SEEK_SET);
    // Use vdiRead to read the data of the partition table into the array of entries
    vdiRead(vdiFile, partitionEntry,64);
}

struct Ext2File *ext2Open(char *fn, int pNum) {
    Ext2File *ptr = new Ext2File;

    // VDIFile pointer
    VDIFile * vdiFile = vdiOpen (fn);
    PartitionEntry *partitionEntry = new PartitionEntry[4];

    // Use vdiSeek to seek to the location of the partition table
    vdiSeek(vdiFile, 446, SEEK_SET);
    // Use vdiRead to read the data of the partition table into the array of entries
    vdiRead(vdiFile, partitionEntry,64);

    //open the partition which the user wants to open
    ptr->partitionFile= *partitionOpen(vdiFile,partitionEntry[pNum]);

    //seek to the super block of the partition and read into the  superblock of new Ext2File it
    partitionSeek(&ptr->partitionFile,1024,SEEK_SET);
    partitionRead(&ptr->partitionFile,&ptr->superBlocks, 1024);

    //update two fields in the superblock
    ptr->superBlocks.s_log_block_size= 1024* pow (2,ptr->superBlocks.s_log_block_size);
    ptr->numberofBlockGroups = (ptr->superBlocks.s_blocks_count/ptr->superBlocks.s_blocks_per_group);

    //set the size of the block group descriptor table based on the number of block groups in the file
    ptr->blockGroupDescriptorstable = new Ext2BlockGroupDescriptor[ptr->numberofBlockGroups+1];

    //reading block descriptor table of each block group
    for (int i = 0; i <= ptr->numberofBlockGroups; ++i) {
        //seek to the second block of the block group
        partitionSeek(&ptr->partitionFile,1024*2+32*i,SEEK_SET);

        //read into the block group descriptor of that block group so that each block group descriptor entries fill the block descriptor table
        partitionRead(&ptr->partitionFile,&ptr->blockGroupDescriptorstable[i],32);
    }

    // Return the pointer
    return ptr;
};

void ext2Close (struct Ext2File *f){
    delete(f);
}


int32_t fetchBlock(struct Ext2File *f,uint32_t blockNum, void *buf){
    //seek to the block whose block number is given
    partitionSeek(&f->partitionFile,blockNum*f->superBlocks.s_log_block_size ,SEEK_SET);
    //read that entire block into the buffer
    int32_t readSuccess = partitionRead(&f->partitionFile,buf, f->superBlocks.s_log_block_size);

    if (readSuccess==0){
        cout << "Fetched the given block" <<endl;
        return 0;
    }else{
        cout <<" Failure to fetch the block" << endl;
        return -1;
    }


}

int32_t writeBlock(struct Ext2File *ext2File, uint32_t blockNum, void *buf){
    partitionSeek(&ext2File->partitionFile, (blockNum * (*ext2File).superBlocks.s_log_block_size), SEEK_SET);

    int result = partitionWrite(&ext2File->partitionFile, buf, ext2File->superBlocks.s_log_block_size);

    if(result != 0){
        cout << "Fail to fetch!" << endl;
        return -1;
    }

    return result;
}

int32_t fetchSuperblock(struct Ext2File *ext2File,uint32_t blockNum, struct Ext2Superblocks *ext2SuperBlock){
    //find the group in which the given block is contained.

    int result = round(blockNum / ext2File->superBlocks.s_blocks_per_group);

    if (result != 0){
        //create a buffer of the size of a block in the partition
        void *ptr = malloc(ext2File->superBlocks.s_log_block_size);

        int blockCount = result * ext2File->superBlocks.s_blocks_per_group;

        fetchBlock(ext2File, blockCount, ptr);

        //cout << ext2SuperBlock->s_log_block_size << endl;
        memcpy(ext2SuperBlock,ptr,1024);
        //cout << ext2SuperBlock->s_log_block_size << endl;


        (ext2SuperBlock->s_log_block_size) = 1024 * pow(2,ext2SuperBlock->s_log_block_size);

    }
    //else if accessing the copies of super block in other blocks,
    // find the position where the superblock begins based on the number of block groups before that superblock
    else{
        //seek to the end of the block before superblock
        partitionSeek(&ext2File->partitionFile,ext2File->superBlocks.s_first_data_block *ext2File->superBlocks.s_log_block_size,SEEK_SET);

        //read 1024 bytes from there into the passed superblock
        partitionRead(&ext2File->partitionFile,ext2SuperBlock,1024);
        (ext2SuperBlock->s_log_block_size) = 1024 * pow(2,ext2SuperBlock->s_log_block_size);

    }
    cout << "The superblock has been fetched from the block group of the given block." << endl;
    return 0;
}
int32_t writeSuperblock(struct Ext2File *ext2File,uint32_t blockNum, struct Ext2Superblocks *ext2Superblock){
    int result = blockNum / ext2File->superBlocks.s_blocks_per_group;

    if(result != 0){
        int blockCount = result * ext2File->superBlocks.s_blocks_per_group;

        void *ptr = malloc(ext2File->superBlocks.s_blocks_per_group);
        writeBlock(ext2File, result, ptr);

        memcpy(ext2File, ptr, 1024);

    } else {
        partitionSeek(&ext2File->partitionFile, (*ext2File).superBlocks.s_log_block_size, SEEK_SET);
        partitionWrite(&ext2File->partitionFile, ext2Superblock, 1024);
        (ext2Superblock->s_log_block_size) = 1024 * pow(2,ext2Superblock->s_log_block_size);
    }
}


int32_t fetchBGDT(struct Ext2File *ext2File, uint32_t blockNum, Ext2BlockGroupDescriptor *bgdt) {

    int groupNumber = round(blockNum/ ext2File->superBlocks.s_blocks_per_group);

    bgdt->bg_inode_bitmap = ext2File->blockGroupDescriptorstable[groupNumber].bg_inode_bitmap;
    bgdt->bg_block_bitmap = ext2File->blockGroupDescriptorstable[groupNumber].bg_block_bitmap;
    bgdt->bg_free_blocks_count = ext2File->blockGroupDescriptorstable[groupNumber].bg_free_blocks_count;
    bgdt->bg_free_inodes_count = ext2File->blockGroupDescriptorstable[groupNumber].bg_free_inodes_count;
    bgdt->bg_used_dirs_count = ext2File->blockGroupDescriptorstable[groupNumber].bg_used_dirs_count;
    bgdt->bg_pad = ext2File->blockGroupDescriptorstable[groupNumber].bg_pad;
    bgdt->bg_inode_table = ext2File->blockGroupDescriptorstable[groupNumber].bg_inode_table;

    for (int i = 1; i <3 ; ++i) {
        bgdt->bg_reserved[i] = ext2File->blockGroupDescriptorstable[groupNumber].bg_pad;
    }

    return 0;
}

int32_t writeBGDT(struct Ext2File *ext2File,uint32_t blockNum,Ext2BlockGroupDescriptor *bgdt) {

    int groupNumber = round(blockNum/ ext2File->superBlocks.s_blocks_per_group);

    ext2File->blockGroupDescriptorstable[groupNumber].bg_inode_bitmap = bgdt->bg_inode_bitmap;
    ext2File->blockGroupDescriptorstable[groupNumber].bg_block_bitmap = bgdt->bg_block_bitmap;
    ext2File->blockGroupDescriptorstable[groupNumber].bg_free_blocks_count = bgdt->bg_free_blocks_count;
    ext2File->blockGroupDescriptorstable[groupNumber].bg_free_inodes_count = bgdt->bg_free_inodes_count;
    ext2File->blockGroupDescriptorstable[groupNumber].bg_used_dirs_count = bgdt->bg_used_dirs_count;
    ext2File->blockGroupDescriptorstable[groupNumber].bg_pad = bgdt->bg_pad;
    ext2File->blockGroupDescriptorstable[groupNumber].bg_inode_table = bgdt->bg_inode_table;

    for (int i = 0; i <3 ; ++i) {
         ext2File->blockGroupDescriptorstable[groupNumber].bg_pad = bgdt->bg_reserved[i];
    }

    return 0;
}

int32_t fetchInode(struct Ext2File *ext2File, uint32_t iNum, struct Inode *buf) {

    // Group number where the desired inode is located
    int group = (iNum - 1) / (*ext2File).superBlocks.s_inodes_per_group;

    // Find the index inside the target group
    int index = (iNum - 1) % (*ext2File).superBlocks.s_inodes_per_group;

    // Find the number of inode per group
    int numOfInodePerGroup = (*ext2File).superBlocks.s_log_block_size / (*ext2File).superBlocks.s_inode_size;

    // Find the block number inside the group
    int block = index / numOfInodePerGroup;

    // Array of Inodes
    Inode * arrayOfInodes = new Inode[numOfInodePerGroup];

    // Index of the desired inode inside the block
    index = index % numOfInodePerGroup;

    // Fetch one block
    fetchBlock(ext2File, (*(ext2File -> blockGroupDescriptorstable + group)).bg_inode_table + block, arrayOfInodes);

    cout << arrayOfInodes[index].i_mode;
}

int32_t writeInode(struct Ext2File *f,uint32_t iNum, struct Inode *buf) {
    // Group number where the desired inode is located
    int group = (iNum - 1) / (*ext2File).superBlocks.s_inodes_per_group;

    // Find the index inside the target group
    int index = (iNum - 1) % (*ext2File).superBlocks.s_inodes_per_group;

    // Find the number of inode per group
    int numOfInodePerGroup = (*ext2File).superBlocks.s_log_block_size / (*ext2File).superBlocks.s_inode_size;

    // Find the block number inside the group
    int block = index / numOfInodePerGroup;

    // Array of Inodes
    Inode * arrayOfInodes = new Inode[numOfInodePerGroup];

    // Index of the desired inode inside the block
    index = index % numOfInodePerGroup;

    // Fetch one block
    fetchBlock(ext2File, (*(ext2File -> blockGroupDescriptorstable + group)).bg_inode_table + block, arrayOfInodes);

    cout << arrayOfInodes[index].i_mode;
}
