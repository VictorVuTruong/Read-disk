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

int main () {

    char *buffer;

    displayBufferPage(256, 0, 0);
}

void displayBufferPage (uint32_t count, uint32_t start, uint64_t offset) {
	// Establish connection to the disk
	int fileIndex = open ("Test-fixed-4k.vdi", O_RDONLY);

	// Seek to a random location in the disk
	//lseek(fileIndex, 200, SEEK_CUR);

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

