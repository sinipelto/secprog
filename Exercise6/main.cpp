// STD C++ headers
#include <iostream>
#include <fstream>
#include <string>
#include <iomanip>
#include <string>

using namespace std;

std::string user_input(const std::string& prompt)
{
	std::string input;
	auto ok = false;

	while (!ok)
	{
		std::cout << std::endl << prompt;

		std::getline(std::cin, input, '\n');

		if (input.empty())
		{
			std::cout << "Input was empty." << std::endl;
			continue;
		}

		if (input.size() > input.max_size() - 64)
		{
			std::cout << "Input too long." << std::endl;
			continue;
		}

		ok = true;
	}

	return input;
}

void task3()
{
	string fn1;
	string fn2;
	string fn3;

	ifstream ifile1;
	fstream ifile2;
	ofstream ofile;

	while (!ifile1 || !ifile1.is_open())
	{
		fn1 = user_input("Enter first file name: ");

		ifile1.open(fn1, ios::in);
		if (!ifile1 || !ifile1.is_open())
		{
			cout << "Could not open file: " << fn1 << endl;
			cout << "Try choosing another file." << endl << endl;
		}
	}
	
	while (!ifile2 || !ifile2.is_open())
	{
		fn2 = user_input("Enter second file name: ");
		
		ifile2.open(fn2, ios::in);
		if (!ifile2 || !ifile2.is_open())
		{
			cout << "Could not open file: " << fn2 << endl;
			cout << "Try choosing another file." << endl << endl;
		}
	}

	while (!ofile || !ofile.is_open())
	{
		fn3 = user_input("Enter output file name for write (file will be overwritten!): ");

		ofile.open(fn3, ios::out | ios::trunc);
		if (!ofile || !ofile.is_open())
		{
			cout << "Could not open file: " << fn3 << endl;
			cout << "Try choosing another file." << endl << endl;
		}
	}

	cout << "Starting to mix.." << endl;

	string line;
	
	auto end1 = false;
	auto end2 = false;

	// Read both files in fixed chunks until EOF (prevent buffer overloading)
	try
	{
		while (!end1 && !end2)
		{
			// Clear buffers
			line = "";
			char c = NULL;

			// Read until newline or EOF/IOError
			while (!end1 && c != '\n' && ifile1.get(c))
			{
				line += c;
			}

			if (ifile1.eof()) end1 = true;

			// Write buffer to output file
			ofile.write(line.c_str(), line.size());

			line = "";
			c = NULL;

			while (!end2 && c != '\n' && ifile2.get(c))
			{
				line += c;
			}

			if (ifile2.eof()) end2 = true;
			ofile.write(line.c_str(), line.size());
		}
	}
	catch (const std::exception &e)
	{
		cout << "File processing stopped to an exception: " << e.what() << endl;
		cout << "Cleaning up.." << endl;

		// Delete output file provided by the operation
		try { remove(fn3.c_str()); } catch (...) { cout << "Removing failed output file failed." << endl; }

		try { ifile1.close(); } catch (...) { cout << "Closing first input file failed." << endl; }
		try { ifile2.close(); } catch (...) { cout << "Closing second input file failed." << endl; }
		try { ofile.close(); } catch (...) { cout << "Closing output file failed." << endl; }

		// Throw to main (cannot proceed)
		throw;
	}

	cout << "Cleaning up.." << endl;

	try { ifile1.close(); } catch (...) { cout << "Closing first input file failed." << endl; }
	try { ifile2.close(); } catch (...) { cout << "Closing second input file failed." << endl; }
	try { ofile.close(); } catch (...) { cout << "Closing output file failed." << endl; }

	cout << "Mixing done." << endl;
}

int main()
{
	try
	{
		task3();
	}
	catch (const exception& e)
	{
		cout << "Exception in main: " << e.what() << endl;
	}
}