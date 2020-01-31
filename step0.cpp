#include <iostream>
#include <unistd.h>
#include <fcntl.h>
#include <fstream>
#include <string>
#include <typeinfo>
#include <iomanip>

using namespace std;

// Function prototypes
// This is the function to get the buffer
void readFile ();

// This is the displayBufferPage function
void displayBufferPage (uint8_t *, uint32_t, uint32_t, uint64_t);

// This is the displayBuffer function1
void displayBuffer(uint8_t *, uint32_t, uint64_t);

// /*
// 	These are the 5 basic functions I/O function
// */
struct VDIFile *vdiOpen(char *fn);
void vdiClose(struct VDIFile *f);
ssize_t vdiRead(struct VDIFile *f, void *buf, size_t count);
//ssizt_t vdiWrite(struct VDIFile *f, void *buf, size_t count);
off_t vdiSeek(VDIFile *f, off_t offset, int anchor);




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

int main () {

	// Establish connection to the disk
	//int fileIndex = open ("Test-fixed-4k.vdi", O_RDONLY);
	ifstream is ("Test-fixed-4k.vdi", std::ifstream::binary);

	// Seek to a random location in the disk
	//lseek(fileIndex, 0, SEEK_CUR);

	//Get length of the fileIndex
	is.seekg(0, is.end);
	int length = is.tellg();
	is.seekg(0, is.beg);


	char * buffer = new char[length];

	//Read file
	is.read(buffer, length);


	//char charArray [1024];

	//cout << read(fileIndex, charArray, 1024) << endl;

  displayBuffer((uint8_t*) buffer, 400, 0);
}


// struct VDIFile{
// 	int fileDescriptor;
// 	VDIHeaderStructure headerStructure;
// 	size_t cursor;
// };
//
struct HeadrStructure{
	
};


// struct VDIFile *vdiOpen(char *fn){
// 	struct
// 	int fileDescriptor = open ("Test-fixed-4k.vdi", O_RDONLY);
// }



void displayBufferPage (uint8_t *buffer, uint32_t count, uint32_t start, uint64_t offset) {


	cout << "   00 01 02 03 04 05 06 07 08 09 0a 0b 0c 0d 0e 0f    0...4...8...c..." << endl;
	cout << "  +-----------------------------------------------+  +----------------+" << endl;

	// First row
	for (int j = 0; j < 16; j++) {
		cout << setfill('0') << setw(2) << right << hex << hex(j) << "|";
		for (int i = (j*16); i < ((j*16) + 16); i++) {
			if(start <= offset && i + 1 + offset <= start + count){
				cout << setfill('0') << setw(2) << right << hex << hex(*(buffer + i + offset));
				if(i + 1 < ((j*16) + 16)) {
					cout << " ";
				}
			} else {
				cout << "  ";
				if(i + 1 < ((j*16) + 16)) {
					cout << " ";
				}
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
	for (int i = offset; i < count; i++){
		if(count >= 256){
			displayBufferPage(buffer, originalCount, 0, offset);
			count -= 256;
			offset += 256;
		} else if (count < 256){
			displayBufferPage(buffer, originalCount, 0,offset);
			break;
		}
	}
}
