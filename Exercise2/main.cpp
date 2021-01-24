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
#include <iomanip>

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

bool is_number(const std::string& str)
{
	if (str.empty()) return false;
	if (str.size() == 1 && !std::isdigit(str[0])) return false;
	
	auto p_cnt = 0;

	// All must satisfy the condition:
	// Has maximum of 1 dot if exists (decimal)
	// minus sign at the beginning if exists (negative number)
	// all other characters are digits
	return std::all_of(
		std::begin(str), 
		std::end(str), 
		[&str, &p_cnt](const char c) -> const bool
		{
			if (p_cnt > 1) return false;
			if (c == '.')
			{
				return ++p_cnt > 1 ? false : true;
			}
			if (c == '-')
			{
				if (str[0] != '-') return false;
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
	auto* const str1 = new std::string;
	auto* const str2 = new std::string;
	
	std::cout << "Decimal 1: ";
	std::cin >> *str1;

	std::cout << "\nDecimal 2: ";
	std::cin >> *str2;

	if (!is_number(*str1) || !is_number(*str2))
	{
		delete str1;
		delete str2;
		throw std::exception("Could not convert inputs to decimal values.");
	}

	const auto* const num1 = new double(std::stod(*str1));
	const auto* const num2 = new double(std::stod(*str2));

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
		delete num1; delete num2;
		if (*result < DBL_MIN || *result > DBL_MAX) throw std::exception("Double limits exceeded.");
	}

	std::cout << "\n\nResult: " << *result << std::endl;

	delete result;
}

/// <summary>
/// T4 Write a program that prints inverse of a number that a user has given.
/// Take care that the program works correctly with any input.
/// </summary>
void t4()
{
	const auto fractions = 10;

	std::string input;
	std::cout << "Enter a number (max digits: "<< fractions << ", max fractions: " << fractions  << "): ";
	std::cin >> input;

	if (!is_number(input))
	{
		throw std::exception("Input was not a valid number.");
	}

	// If decimal fractions, check fraction count overflow
	// Validate against limits
	std::string beforeDot;
	std::string afterDot;

	if (input.find('.') != std::string::npos)
	{
		beforeDot = input.substr(0, input.find('.'));
		afterDot = input.substr(input.find('.') + 1, input.size());
	}
	else
	{
		beforeDot = input;
	}
	if (!afterDot.empty() && afterDot.size() > fractions)
	{
		throw std::exception("Too many decimal fractions in number input.");
	}
	if (beforeDot.size() > fractions)
	{
		throw std::exception("Too many digits in the number input.");
	}

	const auto num = std::stod(input);

	std::ostringstream result;
	result << std::setprecision(input.size() > 2 * fractions + 1 ? 2*fractions + 1 : input.size());

	if (num == 0.0)
	{
		result << "Infinity";
	}
	else if (num < 0.0)
	{
		result << "	1\n-	-\n	" << -num;
	}
	else
	{
		result << "1\n-\n" << num;
	}

	std::cout << "\n\nResult:\n" << result.str() << std::endl;
}

void t5()
{
	// Read the data generated in T2 from the same file
	const std::string path = "./random_data.txt";

	// Open the file in read mode
	std::ifstream file;
	file.open(path, std::ios::in);
	if (!file.is_open()) throw std::exception("Could not open target file for write.");

	std::string data;
	char ch;

	// Read the file one char at a time
	// Ensure the char is allowed by the filter
	// If not, discard the character
	while(file.get(ch))
	{
		// Allowed characters: letters, numbers, comma, hyphen
		if ( !std::isalnum(ch)
			|| !std::isalpha(ch)
			|| !std::isdigit(ch)
			|| ch == ','
			|| ch == '-'
			) continue;

		data.push_back(ch);
	}

	// Finally, close the file
	file.close();

	// Output the filtered file contents
	std::cout << "Filtered file contents: " << data << std::endl;
}

int main()
{
	try
	{
		//t2();
		//t3();
		//t4();
		//t5();
	}
	catch (const std::exception &e)
	{
		std::cout << "Caught exception: " << e.what() << std::endl;
	}

	return 0;
}
