#include <iostream>
#include <unistd.h>
#include <fcntl.h>
#include <fstream>
#include <string>
#include <typeinfo>
#include <iomanip>

using namespace std;

// Function prototypes
// This is the function read the header of the disk
void readHeader();

// This is the function to get the array which contains characters get from the disk
char getArray(int, int);

// This is the displayBufferPage function
void displayBufferPage (uint32_t, uint32_t, uint64_t);

// This is the displayBuffer function1
void displayBuffer(char *, uint32_t, uint64_t);

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

struct UUID {
    uint32_t
        timeLow;
    uint16_t
        timeMid,
        timeHigh,
        clock;
    uint8_t
        node[6];
};

struct HeaderStructure {
    /** The signature */
    uint32_t Signature;
    /** Version */
    uint32_t version;
    /** Size of header*/
    uint32_t sizeOfHeader;
    /** The image type (VDI_IMAGE_TYPE_*). */
    uint32_t Type;
    /** Image flags (VDI_IMAGE_FLAGS_*). */
    uint32_t fFlags;
    /** Size of disk (in bytes). */
    uint64_t cbDisk;
    /** Block size. (For instance VDI_IMAGE_BLOCK_SIZE.) */
    uint32_t BlockSize;
    /** Number of blocks. */
    uint32_t numOfBlocks;
    /** Number of allocated blocks. */
    uint32_t numOfBlocksAllocated;
    /** Offset block*/
    uint32_t offsetBlocks;
    /** Offset data */
    uint32_t offsetData;
    /** Cylinders*/
    uint32_t cylinders;
    /** Heads*/
    uint32_t heads;
    /** Sectors*/
    uint32_t sectors;
    /** SectorSize*/
    uint32_t sectorSize;
    /** UUID of image. */
    UUID uuidVDI;
    /** UUID of image's last SNAP. */
    UUID uuidLastSnap;
    /** UUID link */
    UUID UUIDLink;
    /** UUID Parent*/
    UUID UUIDParent;
};

struct vdiFile {
    int fileDescriptor;
    HeaderStructure header;
    int cursor;
};

int main () {

    char *buffer;

    //displayBufferPage(256, 0, 0);

    readHeader();
}

void displayBufferPage (uint32_t count, uint32_t start, uint64_t offset) {
	// Establish connection to the disk
	int fileIndex = open ("Test-dynamic-1k.vdi", O_RDONLY);

	// Seek to a random location in the disk
	cout << lseek(fileIndex, 256, SEEK_CUR) << endl;

	char charArray [256];

	// Instantiate the array with all 0
	for (int i = 0; i < 256; i++) {
        charArray[i] = '0';
	}

	cout << read(fileIndex, charArray, count) << endl;

	cout << "   00 01 02 03 04 05 06 07 08 09 0a 0b 0c 0d 0e 0f    0...4...8...c..." << endl;
	cout << "  +-----------------------------------------------+  +----------------+" << endl;

	// First row
	for (int j = 0; j < 16; j++) {
		cout << setfill('0') << setw(2) << right << hex << hex(j) << "|";
		for (int i = (j*16); i < ((j*16) + 15); i++) {
			cout << setfill('0') << setw(2) << right << hex << hex(charArray[i]) << " ";
		}
		cout << setfill('0') << setw(2) << right << hex << hex(charArray[(j*16) + 15]) <<"|" << setfill('0') << setw(2) << right << hex << hex(j) <<"|";

		for (int i = (j*16); i < ((j*16) + 16); i++) {
			if (isprint(charArray[i])) {
				cout << charArray[i];
			} else {
				cout << " ";
			}
		}
		cout << "|";
		cout << endl;
	}

	cout << endl;
}

char getArray (int offset, int index) {
    // Establish connection to the disk
	int fileIndex = open ("Test-dynamic-1k.vdi", O_RDONLY);

	// Seek to a random location in the disk
	lseek(fileIndex, offset, SEEK_CUR);

	char charArray [256];

	// Instantiate the array with all 0
	for (int i = 0; i < 256; i++) {
        charArray[i] = '0';
	}

	read(fileIndex, charArray, 256);

	return charArray[index];
}

void readHeader () {
    // Data from the first 256 bytes
    char charArray1 [256];

    for (int i = 0; i < 256; i++) {
        charArray1[i] = getArray(0, i);
    }

    // Data from the next 256 bytes
    char charArray2 [256];

    for (int i = 0; i < 256; i++) {
        charArray2[i] = getArray(256, i);
    }

    // Print the name
    cout << "Name: ";
    for (int i = 0; i < 48; i++) {
        if (isprint(charArray1[i])) {
            cout << charArray1[i];
        }
    }
    cout << endl;

    // Print the Image signature
    cout << "Signature: ";
    for (int i = 64; i < 68; i++) {
        cout << setfill('0') << setw(2) << right << hex << hex(charArray1[i]);
    }
    cout << endl;

    // Print the version
    cout << "Version: ";
    for (int i = 68; i < 72; i++) {
        cout << setfill('0') << setw(2) << right << hex << hex(charArray1[i]);
    }
    cout << endl;

    // Print the size of header
    cout << "Size of header: ";
    for (int i = 72; i < 76; i++) {
        cout << setfill('0') << setw(2) << right << hex << hex(charArray1[i]);
    }
    cout << endl;

    // Print the image type
    cout << "Image type: ";
    for (int i = 76; i < 80; i++) {
        cout << setfill('0') << setw(2) << right << hex << hex(charArray1[i]);
    }
    cout << endl;

    // Print the image flag
    cout << "Image flag: ";
    for (int i = 80; i < 84; i++) {
        cout << setfill('0') << setw(2) << right << hex << hex(charArray1[i]);
    }
    cout << endl;

    // Image Description
    cout << "Image Description: ";
    for (int i = 84; i < 96; i++) {
        cout << setfill('0') << setw(2) << right << hex << hex(charArray1[i]);
    }
    cout << endl;

    //offsetBlocks
    cout << "offsetBlocks: ";
    for (int i = 84; i < 88; i++) {
        cout << setfill('0') << setw(2) << right << hex << hex(charArray2[i]);
    }
    cout << endl;

    // offsetData
    cout << "offsetData: ";
    for (int i = 88; i < 92; i++) {
        cout << setfill('0') << setw(2) << right << hex << hex(charArray2[i]);
    }
    cout << endl;

    // Cylinders
    cout << "Cylinders: ";
    for (int i = 92; i < 96; i++) {
        cout << setfill('0') << setw(2) << right << hex << hex(charArray2[i]);
    }
    cout << endl;

    // Heads
    cout << "Heads: ";
    for (int i = 96; i < 100; i++) {
        cout << setfill('0') << setw(2) << right << hex << hex(charArray2[i]);
    }
    cout << endl;

    // Sectors
    cout << "Sectors: ";
    for (int i = 100; i < 104; i++) {
        cout << setfill('0') << setw(2) << right << hex << hex(charArray2[i]);
    }
    cout << endl;

    // SectorSize
    cout << "SectorSize ";
    for (int i = 104; i < 108; i++) {
        cout << setfill('0') << setw(2) << right << hex << hex(charArray2[i]);
    }
    cout << endl;

    //DiskSize
    cout << "DiskSize: ";
    for (int i = 112; i < 120; i++) {
        cout << setfill('0') << setw(2) << right << hex << hex(charArray2[i]);
    }
    cout << endl;

    //BlockSize
    cout << "BlockSize: ";
    for (int i = 120; i < 124; i++) {
        cout << setfill('0') << setw(2) << right << hex << hex(charArray2[i]);
    }
    cout << endl;

    // Block Extra Data
    cout << "Block Extra Data: ";
    for (int i = 124; i < 128; i++) {
        cout << setfill('0') << setw(2) << right << hex << hex(charArray2[i]);
    }
    cout << endl;

    //Number of BlocksInHDD
    cout << "Number of BlocksInHDD: ";
    for (int i = 128; i < 132; i++) {
        cout << setfill('0') << setw(2) << right << hex << hex(charArray2[i]);
    }
    cout << endl;

    // Number of BlocksAllocated
    cout << "Number of BlocksAllocated: ";
    for (int i = 132; i < 136; i++) {
        cout << setfill('0') << setw(2) << right << hex << hex(charArray2[i]);
    }
    cout << endl;

    // UUID of this VDI
    cout << "UUID of this VDI: ";
    for (int i = 136; i < 152; i++) {
        cout << setfill('0') << setw(2) << right << hex << hex(charArray2[i]);
    }
    cout << endl;

    // UUID of last SNAP
    cout << "UUID of last SNAP: ";
    for (int i = 152; i < 168; i++) {
        cout << setfill('0') << setw(2) << right << hex << hex(charArray2[i]);
    }
    cout << endl;

    // UUID link
    cout << "UUID link: ";
    for (int i = 168; i < 184; i++) {
        cout << setfill('0') << setw(2) << right << hex << hex(charArray2[i]);
    }
    cout << endl;

    // UUID Parent
    cout << "UUID Parent: ";
    for (int i = 184; i < 200; i++) {
        cout << setfill('0') << setw(2) << right << hex << hex(charArray2[i]);
    }
    cout << endl;
}

