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
struct PartitionFile *partitionOpen(struct VDIFile *,struct PartitionEntry);

// This is the displayBufferPage function
void displayBufferPage (uint8_t *, uint32_t, uint32_t, uint64_t);

// This is the displayBuffer function1
void displayBuffer(uint8_t *, uint32_t, uint64_t);

// This is the vdiClose function
void vdiClose (struct VDIFile *);

// This is the vdiRead function
ssize_t vdiRead (struct VDIFile *, PartitionEntry *, int);

// This is the vdiWrite function
ssize_t vdiWrite (struct VDIFile *, PartitionEntry *, int);

// This is the vdiSeek function
off_t vdiSeek (VDIFile *, off_t, int);

// This is the function to close the partition
void partitionClose(struct PartitionFile *);

// This is the function partitionRead
ssize_t partitionRead(struct PartitionFile *,PartitionEntry *,size_t);

// This is the function partitionWrite
ssize_t partitionWrite(struct PartitionFile *,PartitionEntry *,size_t);

// This is the function partitionSeek
off_t partitionSeek(struct PartitionFile, off_t, int);

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

struct PartitionFile {
    /** VDI File */
    VDIFile * vdiFile;
    /** Partition Entry 1*/
    //PartitionEntry arrayOfPartitionEntry1;
    /** Partition Entry 2*/
    //PartitionEntry arrayOfPartitionEntry2;
    /** Partition Entry 3*/
    //PartitionEntry arrayOfPartitionEntry3;
    /** Partition Entry 4*/
    //PartitionEntry arrayOfPartitionEntry4;
    PartitionEntry table[4];
};

int main () {

	// char buffer to hold the bytes to be displayed by displayBufferPage
	//char * buffer = new char[256];

	// Display buffer
    //displayBuffer((uint8_t*)buffer, 256, 0);

    // char buffer to hold the bytes to be read by read function (Let it be 256 for now)
    //char *bufferRead = new char[256];

    VDIFile * vdiFileStruct = vdiOpen("Test-fixed-4k.vdi");

    struct PartitionEntry partitionEntry = {};

    PartitionFile * partitionFile = partitionOpen(vdiFileStruct,partitionEntry);

    cout << (*partitionFile).table[0].type << endl;

    return 0;
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

    // Header structureTest-dynamic-1k.vdi
    struct HeaderStructure header = {};

	// Seek to a random location in the disk
	lseek(fileIndex, 0, SEEK_CUR);

	// Read the header into the header structure
	read(fileIndex, &header, 400);

	// VDIFile header
	struct VDIFile vdiFileStruct = {fileIndex, header, 0};

	// Pointer to the vdiFile structure
	VDIFile *ptr;

	// Make the pointer points to the structure
	ptr = &vdiFileStruct;

	// Return the pointer to the structure
	return ptr;
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

    // Pointer to the partitionFile structure
	PartitionFile *ptr1 = new PartitionFile;
	ptr1->vdiFile = vdiFile;

    // Use vdiSeek to seek to the location of the partition table
    vdiSeek(vdiFile, 446, SEEK_SET);

    // Use vdiRead to read the data of the partition table into the array of entries
    vdiRead(vdiFile, ptr1->table, 64);

	// Return the pointer
	return ptr1;
}

ssize_t partitionRead(struct PartitionFile *f, PartitionEntry *buf, size_t count) {
    if (((*f).table[0].LBAStart * 512 < ((*f).vdiFile -> cursor)) && ((count + (*f).vdiFile -> cursor) < (*f).table[0].nSectors * 512) ||
        ((*f).table[1].LBAStart * 512 < ((*f).vdiFile -> cursor)) && ((count + (*f).vdiFile -> cursor) < (*f).table[1].nSectors * 512) ||
        ((*f).table[2].LBAStart * 512 < ((*f).vdiFile -> cursor)) && ((count + (*f).vdiFile -> cursor) < (*f).table[2].nSectors * 512) ||
        ((*f).table[3].LBAStart * 512 < ((*f).vdiFile -> cursor)) && ((count + (*f).vdiFile -> cursor) < (*f).table[3].nSectors * 512)) {
            return vdiRead(f -> vdiFile, buf, count);
        } else {
            return 0;
        }
}

ssize_t partitionWrite(struct PartitionFile *f, PartitionEntry *buf, size_t count) {
    if (((*f).table[0].LBAStart * 512 < ((*f).vdiFile -> cursor)) && ((count + (*f).vdiFile -> cursor) < (*f).table[0].nSectors * 512) ||
        ((*f).table[1].LBAStart * 512 < ((*f).vdiFile -> cursor)) && ((count + (*f).vdiFile -> cursor) < (*f).table[1].nSectors * 512) ||
        ((*f).table[2].LBAStart * 512 < ((*f).vdiFile -> cursor)) && ((count + (*f).vdiFile -> cursor) < (*f).table[2].nSectors * 512) ||
        ((*f).table[3].LBAStart * 512 < ((*f).vdiFile -> cursor)) && ((count + (*f).vdiFile -> cursor) < (*f).table[3].nSectors * 512)) {
            return vdiWrite(f -> vdiFile, buf, count);
        } else {
            return 0;
        }
}

off_t partitionSeek(struct PartitionFile *f,off_t offset,int anchor) {
    int cur = f -> vdiFile -> cursor;

    // Place where the cursor may be moved to
    int expectedLocation = vdiSeek((*f).vdiFile, offset + (*f).table[0].LBAStart, SEEK_SET);

    // If the location given is invalid, move the cursor back to the original location
    if (expectedLocation < ((*f).table[0].LBAStart * 512) ||
        expectedLocation >= ((*f).table[0].LBAStart * 512) + ((*f).table[0].nSectors * 512)) {
        f -> vdiFile -> cursor = cur;
    }
}


void vdiClose(struct VDIFile *f) {
    // Close the file whose file handler is given
    close((*f).fileDescriptor);

    // Deallocate created memory regions
    delete f;
}

ssize_t vdiRead(struct VDIFile *f, PartitionEntry *buf, int counts) {
    // Use lseek to seek the cursor to the location need to be read
    // The location to start reading is the current value of the VDIFile cursor plus the data offset
    lseek((*f).fileDescriptor, (*f).cursor + (*f).header.offData, SEEK_SET);

    // Read the given number of bytes into the buffer passed to this function as a parameter
    int numOfBytesRead = read((*f).fileDescriptor, buf, counts);

    // Advance the cursor by the number of bytes read
    (*f).cursor += numOfBytesRead;

    // Return the number of bytes read
    return numOfBytesRead;
}

ssize_t vdiWrite(struct VDIFile *f, PartitionEntry *buf, int counts) {
    // Use lseek to seek the cursor to the location need to be read
    // The location to start writing is the current value of VDIFile cursor plus the data offset
    lseek((*f).fileDescriptor, (*f).cursor + (*f).header.offData, SEEK_SET);

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

