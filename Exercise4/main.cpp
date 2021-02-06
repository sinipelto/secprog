// STD C++ headers
#include <csignal>
#include <cstdlib>
#include <cstdio>
#include <cstring>
#include <iostream>
#include <fstream>
#include <sstream>
#include <iomanip>
#include <algorithm>
#include <vector>
#include <string>

// This function will ensure that temporary file used during the program
// is securely overwritten with nonsense values and deleted after
void ensure_secure_delete(FILE* fp)
{
	try
	{
		// First, seek to file end to get file size
		fseek(fp, 0L, SEEK_END);
		const auto size = ftell(fp);

		// If file empty, do nothing
		if (size <= 0)
		{
			return;
		}

		// Seek back to file start
		rewind(fp);

		// Create a buffer of file size filled with 0xFF
		auto* const buf = new unsigned char[size];
		std::memset(buf, 0xFF, size * sizeof(unsigned char));

		// Overwrite file contents with 0xFF bytes
		fwrite(buf, sizeof(char), size, fp);

		// Release heap mem
		delete[] buf;

		std::cout << "FILE SECURELY DELETED" << std::endl;
	}
	catch (...)
	{
		// We must terminate the program at this point in any case,
		// the error might be unrecoverable at program level
		exit(EXIT_FAILURE);
	}
}

// File pointer to a temp file
FILE* tmp;

// Here we ensure that the file is deleted before we exit
void signal_handler(const int sig)
{
	std::cout << "RECEIVED SIGNAL: " << sig << std::endl;
	ensure_secure_delete(tmp);
	std::exit(sig);
}

using namespace std;

int main()
{
	// We use the enhanced version of tmpfile_s (_s = secure)
	// It handles race conditions, deletion and correct access perms.

	// However, the file deletion is impl-def,
	// we need to ensure that in case of unhandled exception, the file is deleted

	// As a Plus, we also create a custom signal handler for e.g. SIGSEGV (Segmentation Fault)
	// that ensures the file is deleted also in that case the signal occurs.
	// Program termination signals are naturally excluded.
	signal(SIGFPE, signal_handler);
	signal(SIGILL, signal_handler);
	signal(SIGSEGV, signal_handler);

	// File pointer to the temp file
	FILE* fp;

	try
	{
		// At the very beginning, we create the temporary file in OS (or user) temp folder
		// Once the file is open, it cannot be opened by other processes (file locking in place)

		//if (fp = fopen("test.txt", "w+b")) // TODO REMOVE
		if (tmpfile_s(&fp)) // MS Docs: Returns 0 if successful
		{
			cout << "Could not create temp file." << endl;
			// At this point, just exit. We could not create a temp file at all.
			return EXIT_FAILURE;
		}

		if (!fp || fp == nullptr)
		{
			cout << "Error while opening temp file." << endl;
			return EXIT_FAILURE;
		}

		const auto p = fputs("justsometeststringtoinputintothetempfile\n", fp);
		const auto f = fflush(fp);

		if (p + f != 0)
		{
			cout << "Could not write and flush." << endl;
		}

		// Ensure reference stored globally
		tmp = fp;
		cout << "Temp file created." << endl;

		// Write something to the temp file
		const auto* const txt = "Address of Main: ";
		auto* const buf = new char[64];
		strcpy_s(buf, 64, txt);
		snprintf(buf + strlen(txt), sizeof(&main), "%p\n", &main);
		
		if (fputs(buf, fp) == EOF)
		{
			cout << "Error writing to temp file stream." << endl;
		}

		//delete[] buf;

		if (fflush(fp) == EOF)
		{
			cout << "Could not flush file stream to disk." << endl;
		}

		// Figure out current file size
		if (fseek(fp, 0L, SEEK_END))
		{
			cout << "File seek to end failed" << endl;
		}

		const auto fsize = ftell(fp);
		rewind(fp);

		// Output current file contents
		cout << "Data read from temp file:\n\n\"";
		while (true)
		{
			auto const ch = fgetc(fp);
			if (feof(fp))
				break;
			cout << static_cast<unsigned char>(ch);
		}
		cout << "\"\n" << endl;

		const auto ptr = reinterpret_cast<unsigned char*>(0x0); // Assign a uchar ptr to address 0x0000000000000000
		*ptr = 0; // Write value 0 to that address (SIGSEGV)
	}
	catch (...) // catch ANY exception
	{
		// Delete file, but signal as non-error signal
		ensure_secure_delete(fp);
		
		// Inform user something went wrong
		return EXIT_FAILURE;
	}

	// No errors, terminate normally
	// wipe the temp file contents
	ensure_secure_delete(fp);
	return EXIT_SUCCESS;
}
