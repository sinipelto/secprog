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
#include <iostream>
#include <fstream>
#include <string>

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

bool check_decimal(const std::string* const str)
{
	auto p_cnt = 0;

	// All must satisfy the condition:
	// Has only 1 comma or dot, all other characters are digits
	return std::all_of(
		std::begin(*str), 
		std::end(*str), 
		[&p_cnt](const char& c) -> const bool
		{
			if (p_cnt > 1) return false;
			if (c == '.')
			{
				if (++p_cnt > 1) return false;
				return true;
			}
			if (std::isdigit(c)) return true;
			return false;
		}
	);
}

/// <summary>
/// T3 Take 2 decimal inputs, multiply them together and return the result.
/// Check any invalid or out of bound inputs.
/// Everything is stored in heap so stack overflowing is not possible
/// </summary>
void t3()
{
	auto* str1 = new std::string;
	auto* str2 = new std::string;
	
	std::cout << "Decimal 1: ";
	std::cin >> *str1;

	std::cout << "\nDecimal 2: ";
	std::cin >> *str2;

	if (!check_decimal(str1) || !check_decimal(str2))
	{
		delete str1;
		delete str2;
		throw std::exception("Could not convert inputs to decimal values.");
	}
	
	double* num1;
	double* num2;
	
	try
	{
		num1 = new double(std::stod(*str1));
		num2 = new double(std::stod(*str2));
	}
	catch (...)
	{
		throw;
	}

	delete str1;
	delete str2;

	// try to convert result into long double (to maintain a highest possible precision)
	auto* result = new long double;

	// If either of the values were 0, the resulting value will be zero.
	if (*num1 == 0.0 || *num2 == 0.0)
	{
		*result = 0;
	}
	else if (*num1 < DBL_MIN || *num2 < DBL_MIN)
	{
		throw std::exception("Doubles were under minimum value.");
	}
	else if (*num1 > DBL_MAX || *num2 > DBL_MAX)
	{
		throw std::exception("Doubles were over maximum value.");
	}
	else
	{
		*result = *num1 * *num2;
		if (*result < DBL_MIN || *result > DBL_MAX) throw std::exception("Double limits exceeded.");
	}

	delete num1;
	delete num2;

	std::cout << "\n\nResult: " << *result << std::endl;

	delete result;
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
