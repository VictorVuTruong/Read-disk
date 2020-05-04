#include <iostream>
#include <unistd.h>
#include <fcntl.h>
#include <math.h>
#include <stdio.h>
#include <cstring>
#include <fstream>
#include <string.h>
#include <typeinfo>
#include <iomanip>
#include<sstream>
#include <bits/stdc++.h>

using namespace std;

// Function prototypes
struct VDIFile *vdiOpen(const char *);

struct PartitionFile *partitionOpen(struct VDIFile *, struct PartitionEntry);

void displayBufferPage (uint8_t *, uint32_t, uint32_t, uint64_t);

void displayBuffer(uint8_t *, uint32_t, uint64_t);

void vdiClose (struct VDIFile *);

ssize_t vdiRead (struct VDIFile *, void *, int);

ssize_t vdiWrite (struct VDIFile *, void *, int);

off_t vdiSeek (VDIFile *, off_t, int);

void partitionClose(struct PartitionFile *);

ssize_t partitionRead(struct PartitionFile *,void *, int);

ssize_t partitionWrite(struct PartitionFile *,void *,size_t);

off_t partitionSeek(struct PartitionFile *, off_t, int);

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

int32_t writeBlock(struct Ext2File *,uint32_t , void *);

int32_t inodeInUse(struct Ext2File *,uint32_t);

int32_t freeInode(struct Ext2File * ,uint32_t);

uint32_t allocateInode(struct Ext2File *,int32_t);

int32_t fetchBlockFromFile(struct Inode *i,uint32_t bNum, void *buf);

int32_t writeBlockToFile(struct Inode *i,uint32_t bNum, void *buf);

int32_t fetchBlockFromFile(struct Ext2File *, struct Inode *, uint32_t, void *);

int32_t writeBlockToFile(struct Ext2File *, struct Inode *, uint32_t, void *, uint32_t);

bool getNextDirent(struct Directory *, uint32_t &, char *);

struct Directory *openDir(struct Ext2File *, uint32_t);

void rewindDir(struct Directory *);

void closeDir(struct Directory *);

uint32_t searchDir(struct Ext2File *f,uint32_t iNum,char *target);

uint32_t traversePath(Ext2File *, char *);

uint32_t copyVDIFileToHost(Ext2File*, char *, char *);

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

struct Inode {
    uint16_t
        i_mode,
        i_uid;
    uint32_t
        i_size,
        i_atime,
        i_ctime,
        i_mtime,
        i_dtime;
    uint16_t
        i_gid,
        i_links_count;
    uint32_t
        i_blocks,
        i_flags,
        i_osd1,
         i_block[15],
        i_generation,
        i_file_acl,
        i_sizeHigh,
        i_faddr;
    uint16_t
        i_blocksHigh,
        reserved16,
        i_uidHigh,
        i_gidHigh;
    uint32_t
    reserved32;
};

struct Dirent {
    uint32_t iNum;
    uint16_t recLen;
    uint8_t nameLen, fileType,name[1];
};

struct Directory{
    ssize_t  cursor;
    uint32_t inodeNum;
    Inode inode;
    uint8_t *pointer;
    Ext2File *ext2File;
    Dirent *dirent;
};

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
};

void displayBufferPage (uint8_t *buffer, uint32_t count, uint32_t start, uint64_t offset) {

    int fileHandler = open("Test-dynamic-1k.vdi", O_RDONLY);

    lseek(fileHandler, offset, SEEK_SET);

    //read(fileHandler, buffer, count);

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
    lseek(f -> fileDescriptor, f -> cursor + f -> header.offData, SEEK_SET);

    // Read the given number of bytes into the buffer passed to this function as a parameter
    int numOfBytesRead = read(f->fileDescriptor, buf, counts);

    // Return the number of bytes read
    if(numOfBytesRead >= 0){
       return 0;
    } else return -1;
}

ssize_t vdiWrite(struct VDIFile *f, void *buf, int counts) {
    // Use lseek to seek the cursor to the location need to be read
    // The location to start writing is the current value of VDIFile cursor plus the data offset
    lseek(f -> fileDescriptor, f -> cursor, SEEK_SET);

    // Write the given number of bytes from the given buffer to the disk
    int numOfBytesWritten = write((*f).fileDescriptor, buf, counts);

    // Return the number of bytes read
    return numOfBytesWritten;
}

off_t vdiSeek (struct VDIFile *f, off_t offset, int anchor) {
    // New location of the cursor
    off_t newLoc;

    // Set the right new location based on the anchor
    if (anchor == SEEK_SET) {
        //newLoc = offset + f -> header.offData;
        newLoc = offset;
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

ssize_t partitionRead(struct PartitionFile *partitionFile, void *buf, int count) {
    // If the number of bytes want to read is with the partition, continue to read the number of bytes
    // to the buffer
    if ((*partitionFile).partitionEntry.LBAStart * 512 <= (*partitionFile).vdiFile->cursor &&
     ((count + (*partitionFile).vdiFile->cursor) <=  ((*partitionFile).partitionEntry.LBAStart+ (*partitionFile).partitionEntry.nSectors)*512)){
        lseek(partitionFile->vdiFile->fileDescriptor,partitionFile->vdiFile->cursor,SEEK_SET);
        // Read information into the buffer
        int num = vdiRead(partitionFile->vdiFile,buf,count);

        // Return number of bytes read
        return num;
    } // Number of bytes want to read doesn't within the partition
    else {
        // Return -1 since nothing is read
        return -1;
    }
}

ssize_t partitionWrite(struct PartitionFile *partitionFile, void *buf, size_t count) {
    // If the number of bytes want to write is with the partition, continue to write the number of bytes
    // from the buffer to the partition
    if ((*partitionFile).partitionEntry.LBAStart * 512 <= (*partitionFile).vdiFile->cursor &&
     ((count + (*partitionFile).vdiFile->cursor) <=  ((*partitionFile).partitionEntry.LBAStart+ (*partitionFile).partitionEntry.nSectors)*512)) {
        // Return the number of bytes that were written into the partition
        return vdiWrite(partitionFile -> vdiFile, buf, count);
    } else {
        // Return -1 since nothing is written into the partition
        return -1;
    }
}

off_t partitionSeek(struct PartitionFile *f, off_t offset, int anchor) {
    // Desired location that we want to seek to
    off_t newLoc;

    // Seek to the desired offset location plus the location of the first sector
    newLoc = vdiSeek((*f).vdiFile , offset + (*f).partitionEntry.LBAStart*512, SEEK_SET);

    // If the desired location is outside of the current partition, reset the position
    // so that the current position does not change
    // The desired location will be consider out of partition space if it is greater than number of sectors in the partition plus the location of the first sector
    // or less than the location of the first sector
    if (newLoc < (*f).partitionEntry.LBAStart * 512 ||
        newLoc >= ((*f).partitionEntry.LBAStart + (*f).partitionEntry.nSectors)*512) {
        newLoc = (*f).vdiFile->cursor;
    } // If the desired location is within the partition, set the cursor in the vdiFile to the new desired location
    else {
        (*f).vdiFile->cursor = newLoc;
    }

    // Return the number of bytes change
    return (*f).vdiFile -> cursor - f->partitionEntry.LBAStart*512;
}

struct Ext2File *ext2Open(char *fn, int pNum) {
    // Pointer to the Ext2File which going to be opened and returned
    Ext2File *ext2File = new Ext2File;

    // Pointer to the VDIFile
    VDIFile * vdiFile = vdiOpen (fn);

    // Array to hold 4 partition entries
    PartitionEntry *partitionEntry = new PartitionEntry[4];

    // Use vdiSeek to seek to the location of the partition table
    vdiSeek(vdiFile, 446, SEEK_SET);

    // Use vdiRead to read the data of the partition table into the array of entries
    vdiRead(vdiFile, partitionEntry, 64);

    //open the partition which the user wants to open
    ext2File -> partitionFile = *partitionOpen(vdiFile, partitionEntry[pNum]);

    //seek to the super block of the partition and read into the  superblock of new Ext2File it
    partitionSeek(&ext2File -> partitionFile, 1024, SEEK_SET);
    partitionRead(&ext2File -> partitionFile, &ext2File -> superBlocks, 1024);

    //update two fields in the Ext2File structure
    ext2File -> superBlocks.s_log_block_size = 1024* pow (2, ext2File -> superBlocks.s_log_block_size);
    ext2File -> numberofBlockGroups = (ext2File -> superBlocks.s_blocks_count / ext2File -> superBlocks.s_blocks_per_group);

    //set the size of the block group descriptor table based on the number of block groups in the file
    ext2File -> blockGroupDescriptorstable = new Ext2BlockGroupDescriptor[ext2File -> numberofBlockGroups + 1];

    //reading block descriptor table of each block group
    for (int i = 0; i <= ext2File -> numberofBlockGroups; ++i) {
        //seek to the second block of the block group
        partitionSeek(&ext2File -> partitionFile, (ext2File -> superBlocks.s_first_data_block + 1)*ext2File -> superBlocks.s_log_block_size + 32 * i, SEEK_SET);

        //read into the block group descriptor of that block group so that each block group descriptor entries fill the block descriptor table
        partitionRead(&ext2File -> partitionFile, &ext2File -> blockGroupDescriptorstable[i], 32);
    }

    // Return the pointer to the ext2File that just opened
    return ext2File;
};

void ext2Close (struct Ext2File *f){
    delete(f);
}

int32_t fetchBlock(struct Ext2File *ext2File, uint32_t blockNum, void *buf){
    // Use partitionSeek to seek to the location of the desired block
    partitionSeek(&ext2File->partitionFile, (blockNum * (*ext2File).superBlocks.s_log_block_size), SEEK_SET);

    // Use partitionRead to read information of the desired block into the buffer
    int32_t bytesRead = partitionRead(&ext2File->partitionFile, buf, ext2File->superBlocks.s_log_block_size);

    // bytesRead 0 indicate that nothing is read
    if(bytesRead != 0){
        cout << "Fail to fetch block!" <<endl ;
        return -1;
    }

    // Return the number of bytes read
    return bytesRead;
}

int32_t writeBlock(struct Ext2File *ext2File, uint32_t blockNum, void *buf){
    // Use partitionSeek to seek to the location of the desired block
    partitionSeek(&ext2File->partitionFile, (blockNum * (*ext2File).superBlocks.s_log_block_size), SEEK_SET);

    // Use partitionWrite to write information from the buffer to the desired block
    int bytesWritten = partitionWrite(&ext2File->partitionFile, buf, ext2File->superBlocks.s_log_block_size);

    // bytesWritten 0 indicate that nothing is written
    if(bytesWritten != 0){
        cout << "Fail to write block!" << endl;
        return -1;
    }

    // Return the number of bytes written
    return bytesWritten;
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
    // Get the block group descriptor table where the block number is located
    int group = round(blockNum/ ext2File->superBlocks.s_blocks_per_group);

    // Populate all fields in the block group descriptor table structure which is treated as a buffer in this function
    bgdt -> bg_inode_bitmap = ext2File -> blockGroupDescriptorstable[group].bg_inode_bitmap;
    bgdt -> bg_block_bitmap = ext2File -> blockGroupDescriptorstable[group].bg_block_bitmap;
    bgdt -> bg_free_blocks_count = ext2File -> blockGroupDescriptorstable[group].bg_free_blocks_count;
    bgdt -> bg_free_inodes_count = ext2File -> blockGroupDescriptorstable[group].bg_free_inodes_count;
    bgdt -> bg_used_dirs_count = ext2File -> blockGroupDescriptorstable[group].bg_used_dirs_count;
    bgdt -> bg_pad = ext2File -> blockGroupDescriptorstable[group].bg_pad;
    bgdt -> bg_inode_table = ext2File -> blockGroupDescriptorstable[group].bg_inode_table;
    for (int i = 1; i <3 ; ++i) {
        bgdt->bg_reserved[i] = ext2File->blockGroupDescriptorstable[groupNumber].bg_pad;
    }

    return 0;
}

int32_t writeBGDT(struct Ext2File *ext2File,uint32_t blockNum,Ext2BlockGroupDescriptor *bgdt) {
    // Get the block group descriptor table where the block number is located
    int groupNumber = round(blockNum/ ext2File->superBlocks.s_blocks_per_group);

    // Write all fields in the block group descriptor table with information from the block group descriptor table
    // which is passed to this function as a buffer
    ext2File -> blockGroupDescriptorstable[groupNumber].bg_inode_bitmap = bgdt -> bg_inode_bitmap;
    ext2File -> blockGroupDescriptorstable[groupNumber].bg_block_bitmap = bgdt -> bg_block_bitmap;
    ext2File -> blockGroupDescriptorstable[groupNumber].bg_free_blocks_count = bgdt -> bg_free_blocks_count;
    ext2File -> blockGroupDescriptorstable[groupNumber].bg_free_inodes_count = bgdt -> bg_free_inodes_count;
    ext2File -> blockGroupDescriptorstable[groupNumber].bg_used_dirs_count = bgdt -> bg_used_dirs_count;
    ext2File -> blockGroupDescriptorstable[groupNumber].bg_pad = bgdt -> bg_pad;
    ext2File -> blockGroupDescriptorstable[groupNumber].bg_inode_table = bgdt -> bg_inode_table;
    for (int i = 0; i <3 ; ++i) {
         ext2File->blockGroupDescriptorstable[groupNumber].bg_pad = bgdt->bg_reserved[i];
    }

    return 0;
}

int32_t fetchInode(struct Ext2File *ext2File, uint32_t iNum, struct Inode *buf) {
    // Group number where the desired inode is located
    int group = (iNum - 1) / ext2File -> superBlocks.s_inodes_per_group;

    // Adjust the iNum
    iNum = (iNum - 1) % ext2File -> superBlocks.s_inodes_per_group;

    // Number of inodes per block
    int numOfInodesPerBlock = ext2File -> superBlocks.s_log_block_size / ext2File -> superBlocks.s_inode_size;

    // Find the block where the given inode is located
    int inodeBlockNum = (iNum / numOfInodesPerBlock) + ext2File -> blockGroupDescriptorstable[group].bg_inode_table;

    // Find offset of inode in the block
    int offsetInBlock = iNum % numOfInodesPerBlock;

    // The temp block
    uint8_t* tempBlock = new uint8_t [ext2File->superBlocks.s_log_block_size];

    // Fetch the block into the temp buffer
    fetchBlock(ext2File, inodeBlockNum, tempBlock);

    // Read the tempBlock to the buffer
    *buf = ((Inode *)tempBlock)[offsetInBlock];

}



int32_t writeInode(struct Ext2File *ext2File,uint32_t iNum, void *buf) {
    // Group number where the desired inode is located
    int group = (iNum -  1) / (*ext2File).superBlocks.s_inodes_per_group;

    // Find the index inside the target group
    int index = (iNum - 1) % (*ext2File).superBlocks.s_inodes_per_group;

    // Find the number of inode per group
    int numOfInodePerGroup = (*ext2File).superBlocks.s_log_block_size / (*ext2File).superBlocks.s_inode_size;

    // Find the block number inside the group
    int block = index / numOfInodePerGroup;

    // Index of the desired inode inside the block
    index = index % numOfInodePerGroup;

    // Fetch one block
    writeBlock(ext2File, (*(ext2File -> blockGroupDescriptorstable + group)).bg_inode_table + block, buf);

    return index;
}

int32_t inodeInUse(struct Ext2File *ext2File, uint32_t iNum){
    // Group number where the desired inode is located
    int group = (iNum -  1) / (*ext2File).superBlocks.s_inodes_per_group;

    // Find the inode position inside the target group
    int inodePosition = (iNum - 1) % (*ext2File).superBlocks.s_inodes_per_group;

    // Find where the bitmap is within the group
    int bitmapPostition = (*(ext2File -> blockGroupDescriptorstable + group)).bg_inode_bitmap;

    // Find the inode's position within the found group
    // This is also the bit to find in the bitmap
    int bitToFindInBitMap = inodePosition % (*ext2File).superBlocks.s_inodes_per_group;

    // The array to hold the bitmap
    uint8_t bitmap [1024];

    // Use fetchBlock to fetch the bitmap in the group
    fetchBlock(ext2File, bitmapPostition, bitmap);

    // Find the byte with the bit currently looking for
    int byteWithBit = bitToFindInBitMap / 8;

    // Find the bit in the byte
    int bitInByte = bitToFindInBitMap % 8;

    // This will hold the result about if the inode is in use or not
    int32_t isInUsed;

    // Get the result
    isInUsed = bitmap[byteWithBit] && (1 << inodePosition % 8);

    // Return the result
    return isInUsed;
}

uint32_t allocateInode(struct Ext2File *ext2File,int32_t group) {
    // Current number of inode
    int currentINum = 0;

    // Go through the group to see which inode is free and which one is not
    // If an inode is free, break out of the loop and mark that inode
    while (inodeInUse(ext2File, currentINum) != 0) {
        currentINum += 1;
    }

    // Once the free inode is found, allocate it by marking it
    // Find where the bitmap is within the group
    int bitmapPostition = (*(ext2File -> blockGroupDescriptorstable + group)).bg_inode_bitmap;

    // Find the inode's position within the found group
    // This is also the bit to find in the bitmap
    int bitToFindInBitMap = currentINum % (*ext2File).superBlocks.s_inodes_per_group;

    // The array to hold the bitmap
    uint8_t bitmap [1024];

    // Use fetchBlock to fetch the bitmap in the group
    fetchBlock(ext2File, bitmapPostition, bitmap);

    // Find the byte with the bit currently looking for
    int byteWithBit = bitToFindInBitMap / 8;

    // Find the bit in the byte
    int bitInByte = bitToFindInBitMap % 8;

    // Byte to manipulate
    uint8_t byteToManipulate = bitmap[byteWithBit];

    // Set the target bit in the given bytes to mark the inode
    byteToManipulate |= 1UL << bitInByte;

    // Bring the manipulated byte back to the array of bitmap
    bitmap[byteWithBit] = byteToManipulate;

    // Write the array with manipulated bitmap back to the group
    writeBlock(ext2File, bitmapPostition, bitmap);
}

int32_t freeInode(struct Ext2File *ext2File, uint32_t iNum) {
    // Group number where the desired inode is located
    int group = (iNum -  1) / (*ext2File).superBlocks.s_inodes_per_group;

    // Find the inode position inside the target group
    int inodePosition = (iNum - 1) % (*ext2File).superBlocks.s_inodes_per_group;

    // Find where the bitmap is within the group
    int bitmapPostition = (*(ext2File -> blockGroupDescriptorstable + group)).bg_inode_bitmap;

    // Find the inode's position within the found group
    // This is also the bit to find in the bitmap
    int bitToFindInBitMap = inodePosition % (*ext2File).superBlocks.s_inodes_per_group;

    // The array to hold the bitmap
    uint8_t bitmap [1024];

    // Use fetchBlock to fetch the bitmap in the group
    fetchBlock(ext2File, bitmapPostition, bitmap);

    // Find the byte with the bit currently looking for
    int byteWithBit = bitToFindInBitMap / 8;

    // Find the bit in the byte
    int bitInByte = bitToFindInBitMap % 8;

    // Byte to manipulate
    uint8_t byteToManipulate = bitmap[byteWithBit];

    // Clear the target bit in the given bytes to free the inode
    byteToManipulate &= ~(1UL << bitInByte);

    // Bring the manipulated byte back to the array of bitmap
    bitmap[byteWithBit] = byteToManipulate;

    // Write the array with manipulated bitmap back to the group
    writeBlock(ext2File, bitmapPostition, bitmap);
}



int32_t fetchBlockFromFile(struct Ext2File *ext2File, struct Inode *inode, uint32_t bNum, void *buf) {
    // Pointer which will be used to point to the blockList
    void * blockListPointer;

    int index;

    // Find the number k which is the block size divided by 4
    int k = ext2File -> superBlocks.s_log_block_size / 4;

    if (bNum < 12) {
        // Make the blockList pointer points to the i_block
        blockListPointer = inode -> i_block;

        // Goto direct
        goto direct;
    } else if (bNum < 12 + k) {
        if (*(uint32_t*)(inode -> i_block + 12) == 0) {
            return false;
        }

        // Fetch the SIB block
        fetchBlock(ext2File, *(uint32_t*)(inode -> i_block + 12), buf);

        // Set up an array to read from
        blockListPointer = buf;

        // Adjust block number for nodes skipped over
        bNum = bNum - 12;

        // Goto direct
        goto direct;
    } else if (bNum < 12 + k + k*k) {
        if (*(uint32_t*)(inode -> i_block + 13) == 0) {
            return false;
        }

        // Fetch the DIB block
        fetchBlock(ext2File, *(uint32_t*)(inode -> i_block + 13), buf);

        // Set up an array to read from
        blockListPointer = buf;

        // Adjust block number for nodes skipped over
        bNum = bNum - 12 - k;

        // Goto single
        goto single;
    } else {
        if (*(uint32_t*)(inode -> i_block + 14) == 0) {
            return false;
        }

        // Fetch the TIB block
        fetchBlock(ext2File, *(uint32_t*)(inode -> i_block + 14), buf);

        // Set up an array to read from
        blockListPointer = buf;

        // Adjust block number for nodes skipped over
        bNum = bNum - 12 - k - (k*k);
    }

    // Determine which DIB to fetch
    index = bNum / (k*k);

    // Determine which block under that DIB we want
    bNum = bNum % (k*k);

    if (*(uint32_t*)(blockListPointer + index) == 0) {
        return false;
    }

    // Fetch the DIB and point to it
    fetchBlock(ext2File, *(uint32_t*)(blockListPointer + index), buf);
    blockListPointer = buf;

    // Single label
    // Give a DIB, fetch a proper SIB
    single: {
        // Determine which SIB to fetch
        index = bNum / k;

        // Determine which block under that SIB we want
        bNum = bNum % k;

        if (*(uint32_t*)(blockListPointer + index) == 0) {
            return false;
        }

        // Fetch the DIB and point to it
        fetchBlock(ext2File, *(uint32_t*)(blockListPointer + index), buf);
        blockListPointer = buf;
    }

    // Direct label
    direct: {
        if (*(uint32_t*)(blockListPointer + bNum) == 0) {
            return false;
        }

        // Fetch the data block
        fetchBlock(ext2File, *(uint32_t*)(blockListPointer + bNum), buf);
    }

    return true;
}


int32_t writeBlockToFile(struct Ext2File *ext2File, struct Inode *inode, uint32_t bNum, void *buf, uint32_t iNum) {
    // Find the number k which is the block size divided by 4
    int k = ext2File -> superBlocks.s_log_block_size / 4;

    // Figure out the block group that the iNum inode is located
    int blockGroupToAllocate = (iNum -  1) / (*ext2File).superBlocks.s_inodes_per_group;

    // A temporary block that you must allocate
    void * tmp;

    // Pointer which will be used to point to the blockList
    void * blockListPointer;

    int ibNum;
    int index;

    if (bNum < 12) {
        // If block not there, allocate it
        if (*(uint32_t*)(inode -> i_block + bNum) == 0) {
            // Allocate
            allocateInode(ext2File, blockGroupToAllocate);

            // Write the inode
            writeInode(ext2File, iNum, inode);
        }

        // Set up the array to read from
        blockListPointer = inode -> i_block;

        // Goto direct
        goto direct;
    } else if (bNum < 12 + k) {
        // If block not there, allocate it
        if (*(uint32_t*)(inode -> i_block + 12) == 0) {
            // Allocate
            allocateInode(ext2File, blockGroupToAllocate);

            // Write the inode
            writeInode(ext2File, iNum, inode);
        }

        // Fetch SIB
        fetchBlock(ext2File, *(uint32_t*)(inode -> i_block + 12), tmp);

        // Set up the array to read from
        ibNum = *(uint32_t*)(inode -> i_block + 12);
        blockListPointer = tmp;

        // Adjust b for nodes skipped over
        bNum = bNum - 12;

        // Goto direct
        goto direct;
    } else if (bNum < 12 + k + (k*k)) {
        if (*(uint32_t*)(inode -> i_block + 13) == 0) {
            // Allocate
            allocateInode(ext2File, blockGroupToAllocate);

            // Write the inode
            writeInode(ext2File, iNum, inode);
        }

        // Fetch DIB
        fetchBlock(ext2File, *(uint32_t*)(inode -> i_block + 13), tmp);

        // Set up the array to read from
        ibNum = *(uint32_t*)(inode -> i_block + 13);
        blockListPointer = tmp;

        // Adjust b for nodes skipped over
        bNum = bNum - 12 - k;

        // Goto single
        goto single;
    } else {
        if (*(uint32_t*)(inode -> i_block + 14) == 0) {
            // Allocate
            allocateInode(ext2File, blockGroupToAllocate);

            // Write the inode
            writeInode(ext2File, iNum, inode);
        }

        // Fetch TIB
        fetchBlock(ext2File, *(uint32_t*)(inode -> i_block + 14), tmp);

        // Set up the array to read from
        ibNum = *(uint32_t*)(inode -> i_block + 14);
        blockListPointer = tmp;

        // Adjust b for nodes skipped over
        bNum = bNum - 12 - k - (k*k);
    }

    // Determine which DIB to fetch
    index = bNum / (k*k);

    // Determine which block under that DIB we want
    bNum = bNum % (k*k);

    if (*(uint32_t*)(blockListPointer + index) == 0) {
        // Allocate
        allocateInode(ext2File, blockGroupToAllocate);

        // Write the inode
        writeInode(ext2File, iNum, inode);
    }

    // Fetch the DIB and point to it
    ibNum = *(uint32_t*)(blockListPointer + index);
    fetchBlock(ext2File, *(uint32_t*)(blockListPointer + index), tmp);
    blockListPointer = tmp;

    // Single label
    single: {
        // Determine which SIB to fetch
        index = bNum / k;

        // Determine which block under that SIB we want
        bNum = bNum % k;

        if (*(uint32_t*)(blockListPointer + index) == 0) {
            // Allocate
            allocateInode(ext2File, blockGroupToAllocate);

            // Write the inode
            writeInode(ext2File, ibNum, blockListPointer);
        }

        // Fetch the SIB and point to it
        ibNum = *(uint32_t*)(blockListPointer + index);
        fetchBlock(ext2File, *(uint32_t*)(blockListPointer + index), tmp);
        blockListPointer = tmp;
    }

    // Direct label
    direct: {
        if (*(uint32_t*)(blockListPointer + bNum) == 0) {
            // Allocate
            allocateInode(ext2File, bNum);

            // Write the inode
            writeInode(ext2File, ibNum, blockListPointer);
        }

        // Write the data block
        writeBlock(ext2File, *(uint32_t*)(blockListPointer + bNum), buf);
    }
}

struct Directory *openDir(struct Ext2File * ext2File, uint32_t iNum) {
    // create new directory pointer
    Directory *directory = new Directory;

    // Set the cursor to 0
    directory -> cursor = 0;

    // Set the iNum in the directory structure to the desired inode number
    directory -> inodeNum = iNum;

    // Fetch the inode we want and bring the information into the inode of the directory structure
    fetchInode(ext2File, iNum, &directory -> inode);

    // Make the pointer of the directory structure point to the array with size is size of the block
    directory -> pointer = new uint8_t[ext2File -> superBlocks.s_log_block_size];

    // Make the ext2File pointer in the directory structure point to the ext2File of this function
    directory -> ext2File = ext2File;

    // After all fields in the directory structure is populated, return the pointer to the directory
    return directory;
}

bool getNextDirent(struct Directory * directory, uint32_t &iNum, char *name){
    //Loop through everything in the directory
    while (directory -> cursor < directory -> inode.i_size){
        //find the block in the file which contains the directory
        int bNum = directory -> cursor/ directory -> ext2File -> superBlocks.s_log_block_size;

        //fetch the directory
        fetchBlockFromFile(directory -> ext2File, &directory -> inode, bNum, directory -> pointer);

        //find the offset of the directory entry from the start of the directory
        int offset = directory -> cursor % directory -> ext2File -> superBlocks.s_log_block_size;

        // Modify the structure information
        //point to the directory entry we want to access
        directory -> dirent = (Dirent *) (directory -> pointer + offset);
        // Move the cursor to the next directory entry
        directory -> cursor += directory -> dirent -> recLen;

        //copy the name to the passed array if the inode is not zero
        if (directory -> dirent -> iNum !=0) {
            iNum = directory -> dirent -> iNum;

            for (int i = 0; i < directory -> dirent -> nameLen; i++) {
                name[i]= directory -> dirent -> name[i];
            }

            name[directory -> dirent -> nameLen] = 0;
            return true;
        }
    }
    return false;
}

void rewindDir(struct Directory * directory) {
    // Reset the directory cursor to 0
    directory -> cursor = 0;
}

void closeDir(struct Directory *d){
    close((*(d->ext2File)).partitionFile.vdiFile->fileDescriptor);
}


uint32_t searchDir(struct Ext2File *ext2File,uint32_t iNum,char *target){
    char name[256];
    Directory *d;
    d = openDir(ext2File,iNum);

    while (getNextDirent(d,iNum, name)) {

        int result = strcmp(target, name);
        if(result == 0){
            return iNum;
        }
    }
    return 0;
}

uint32_t traversePath(Ext2File *ext2File, char *path) {
    // Start
    int start = 1;

    // Length of the path
    int len = strlen(path);

    // Inode number
    int iNum = 2;

    while (start < len && iNum != 0) {
        // End index of the path
        int endPath = start + 1;
        while (path[endPath] != 0 && path[endPath] != '/') {
            endPath ++;
        }
        path[endPath] = 0;
        iNum = searchDir(ext2File, iNum, path + start);
        start = endPath + 1;
    }
    cout << "Inum: " << iNum << endl;
    return iNum;
}

int main () {

    Ext2File *ext2File = ext2Open("Test-fixed-1k.vdi",0);

    /** Test Step 4 **/
    //Super Block

    //Ext2Superblocks ext2Superblock;
    //cout << ext2Superblock.s_log_block_size<<endl;
    //fetchSuperblock(ext2File,8200,&ext2Superblock);
    //dumpSuperBlock(&ext2Superblock);

    //BGDT
    //Ext2BlockGroupDescriptor ext2BGDT;
    //fetchBGDT(ext2File, 16385, &ext2BGDT);
    //dumpBGDT(ext2File);

    /** Test Step 5 **/
    /*
    Inode inode;
    char buffer [1024];
    fetchInode(ext2File, 30481, &inode);
    fetchBlockFromFile(ext2File, &inode, 0, buffer);
    */

    /** Test Step 6 **/

    /*
    char name[256];
    uint32_t iNum;
    Directory *d;

    cout << "Root directory contents" << endl;
    d = openDir(ext2File,2);
    while (getNextDirent(d, iNum,name)) {
        cout << "Inode: " << iNum << " name: [" << name << "]" << endl;
    }
    //closeDir(d);

    cout << "\n\nlost+found directory contents" << endl;
    d = openDir(ext2File,11);
    while (getNextDirent(d, iNum,name)) {
        cout << "Inode: " << iNum << " name: [" << name << "]" << endl;
    }
    //closeDir(d);
    */


    /** Test Step 7 **/
    //char target[]= "/examples/05.Control/IfStatementConditional/IfStatementConditional.ino";
    //traversePath(ext2File, target);

    /** Test Step 8 **/
    char fileName[]= "/examples/08.Strings/StringComparisonOperators/StringComparisonOperators.txt";
    char copyPath[] = "test.png";
    copyVDIFileToHost(ext2File, copyPath, fileName);
}

uint32_t copyVDIFileToHost(Ext2File* ext2File, char *path, char *fileName){
    Inode inode;
    uint8_t *block = new uint8_t[ext2File->superBlocks.s_log_block_size];

    int inodeNum = traversePath(ext2File, fileName);
    fetchInode(ext2File, inodeNum, &inode);

    int fileDescriptor = open(path, O_WRONLY|O_CREAT, 0666); //The fileDescriptor should be 4
    cout << "File Descriptor: " << fileDescriptor <<endl;

    if(fileDescriptor == 4){
        uint32_t blockSize = ext2File->superBlocks.s_log_block_size;
        int bytesToWrite = blockSize;
        int bytesLeft = inode.i_size;
        for (int i = 0; i < blockSize; i++) {
            fetchBlockFromFile(ext2File, &inode, i, block);
            if(bytesToWrite < blockSize){
                bytesToWrite = bytesLeft;
            }

            int w = write(fileDescriptor, block, bytesToWrite);
        }
    } else {
        cout << "There's an error when copying VDI File to the host machine" <<endl;
    }

    cout << "File size: " << inode.i_size <<endl;

}

