// Crypto++ Library
#ifdef _DEBUG
#  pragma comment ( lib, "cryptlibd" )
#else
#  pragma comment ( lib, "cryptlib" )
#endif

#include <cryptlib.h>
#include <osrng.h>
#include <modes.h>
#include <threefish.h>
#include <aes.h>
#include <limits>
#include <iostream>
#include <fstream>

/// <summary>
/// T2: Program that writes crypto safe random data into a file
/// </summary>
void t2()
{
	using namespace CryptoPP;

	const std::string path = "./random_data.txt";
	const auto block_size = 1024;

	// Initialize true random seed from OS random source
	SecByteBlock seed(32 + 16);
	SecByteBlock result(block_size);
	OFB_Mode<AES>::Encryption rng;

	// Initialize IV for AES
	OS_GenerateRandomBlock(false, seed, seed.size());
	rng.SetKeyWithIV(seed, 32, seed + 32, 16);

	// Use AES Block Cipher to generate a true random block of random data bytes
	rng.GenerateBlock(result, result.size());

	// Convert bytes into readable text (skip any invalid character bytes)
	std::string text;
	for (auto x : result)
	{
		if (std::isprint(x))
		{
			text += x;
		}
	}

	// Open the file for writing in overwrite mode
	std::ofstream file;
	file.open(path,std::ios::out | std::ios::trunc);
	if (!file.is_open()) throw std::exception("Could not open target file for write.");
	// Write the text into the file
	file.write(text.c_str(), text.length());
	// Close the file
	file.close();
}

/// <summary>
/// T3 Take 2 decimal inputs, multiply them together and return the result.
/// Check any invalid or out of bound inputs.
/// </summary>
void t3()
{
	double num1;
	double num2;
	double temp;

	std::cout << "Decimal 1: ";
	std::cin >> num1;
	if (!std::cin.good() || std::cin.bad() || std::cin.fail())
	{
		throw std::exception("Input for Decimal 1 was not a valid double.");
	}

	std::cout << "\nDecimal 2: ";
	std::cin >> num2;
	if (!std::cin.good() || std::cin.bad() || std::cin.fail())
	{
		throw std::exception("Input for Decimal 2 was not a valid double.");
	}

	// try to convert result into long double (to maintain a highest possible precision
	long double result;

	// If either of the values were 0, the resulting value will be zero.
	if (num1 == 0.0 || num2 == 0.0)
	{
		result = 0;
	}
	else if (num1 < DBL_MIN || num2 < DBL_MIN)
	{
		throw std::exception("Doubles were under minimum value.");
	}
	else if (num1 > DBL_MAX || num2 > DBL_MAX)
	{
		throw std::exception("Doubles were over maximum value.");
	}
	else
	{
		result = num1 * num2;
		if (result < DBL_MIN || result > DBL_MAX) throw std::exception("Double limits exceeded.");
	}

	std::cout << "\nResult: " << result << std::endl;
}

void t4()
{
}

void t5()
{
	// Read the data generated in T2 from the same file
	const std::string path = "./random_data.txt";
	// Define a list of allowed characters (letters, numbers, comma, hyphen)
	const std::string allowed_chars = "0123456789abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ,-";

	// Open the file in read mode
	std::ifstream file;
	file.open(path, std::ios::in);
	if (!file.is_open()) throw std::exception("Could not open target file for write.");

	std::string data;
	char val;

	// Read the file one char at a time
	// Ensure the char is allowed by the filter
	// If not, discard the character
	while(file.get(val))
	{
		if (allowed_chars.find(val) != std::string::npos)
		{
			data.push_back(val);
		}
	}

	// Finally, close the file
	file.close();

	// Output the filtered file contents
	std::cout << "Processed file data: " << data << std::endl;
}

int main()
{
	try
	{
		//t2();
		t3();
		//t5();
	}
	catch (const std::exception &e)
	{
		std::cout << "Caught exception: " << e.what() << std::endl;
	}

	return 0;
}
