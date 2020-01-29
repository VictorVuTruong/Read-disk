#include <iostream>
#include <unistd.h>
#include <fcntl.h>
#include <fstream>
#include <string>

using namespace std;

// Function prototypes
// This is the function to get the buffer
uint8_t * readFile (string);

// This is the displayBufferPage function 
void displayBufferPage (uint8_t, uint32_t, uint32_t, uint64_t);

// This is the displayBuffer function
void displayBuffer(uint8_t, uint32_t, uint64_t);

int main () {

	uint8_t * buffer = readFile("Test-fixed-1k.vdi");
	
	for (int i = 0; i < length; i++) {
		cout << *(buffer + i) << endl;
	}

	return 0;
}

// Implementation for the readFile function
uint8_t * readFile (string fileName) {
	// Open the file	
	ifstream is (fileName, std::ifstream::binary);

	// Get length of the file by bringing the cursor the end of file and back
	is.seekg (0, is.end);
	int length = is.tellg();
	is.seekg (0, is.beg);
	
	// Pointer to an array of characters	
	char * buffer = new char [length];
	
	if (is) {
		
		// Read data as a block
		is.read (buffer, length);

		if (is) {
			cout << "All characters read successfully" << endl;
		}
		else {
			cout << "Some error occured" << endl;
		}
	}

	// Pointer to an array of int 
	uint8_t * bufferInt = new uint8_t [length];

	// THe for loop that traverse through the array of char to convert to an array of int
	for (int i = 0; i < length; i++) {
		* (bufferInt + i) = (uint8_t)(buffer + i);
	}

	cout << "Number of bytes read " << is.gcount() << endl; 

	// Return the pointer to the array 
	return bufferInt;
}

