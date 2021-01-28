// Crypto++ (CryptoPP) Library (cryptlib)
// Compiled as static debug and release builds
// linked statically to this project
// This project built with static linking
// Linking debug/release build, depending on this project setting
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
#include <sstream>
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

	std::cout << "Random data written to file." << std::endl;
}

/// <summary>
/// This functions checks if the input string
/// is a valid number (integer or dotted decimal)
/// and returns a boolean value correspondingly.
/// </summary>
/// <param name="str">The number as string to be validated.</param>
/// <returns>The string input is a valid number (integer or double)</returns>
bool is_number(const std::string& str)
{
	// Initial checks on number string
	// If size is 1, can only be a digit
	if (str.empty()) return false;
	if (str.size() == 1 && !std::isdigit(str[0])) return false;

	// Keep track of the dot counts
	//(legit dot might occur anywhere in the string)
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
	// Initialize variables for inputs
	auto* const str1 = new std::string;
	auto* const str2 = new std::string;

	// Read first decimal value into a string variable
	std::cout << "Decimal 1: ";
	std::cin >> *str1;

	// Read second decimal value into a string variable
	std::cout << "\nDecimal 2: ";
	std::cin >> *str2;

	// Ensure inputs are valid decimal numbers
	if (!is_number(*str1) || !is_number(*str2))
	{
		// If not, deallocate memory and throw an exception
		delete str1;
		delete str2;
		throw std::exception("Could not convert inputs to decimal values.");
	}

	// Initialize double variables to store inputs as double type
	const auto* const num1 = new double(std::stod(*str1));
	const auto* const num2 = new double(std::stod(*str2));

	// If conversion succeeded, release the string variables' memory
	// (no longer needed)
	delete str1;
	delete str2;

	// try to convert result into long double (to maintain a highest possible precision)
	auto* result = new long double;

	// If either of the values were 0, the resulting value will be zero.
	if (*num1 == 0.0 || *num2 == 0.0)
	{
		*result = 0;
	}
	// If limits were crossed, throw an exception
	else if (*num1 < DBL_MIN || *num2 < DBL_MIN)
	{
		throw std::exception("Doubles were under minimum value.");
	}
	else if (*num1 > DBL_MAX || *num2 > DBL_MAX)
	{
		throw std::exception("Doubles were over maximum value.");
	}
	// Otherwise, calculate the result (multiplication)
	else
	{
		*result = *num1 * *num2;
		// Free input variables, as result already stored
		delete num1; delete num2;
		// Ensure result within bounds
		if (*result < DBL_MIN || *result > DBL_MAX) throw std::exception("Double limits exceeded.");
	}

	// Output the result into STDOUT
	std::cout << "\n\nResult: " << *result << std::endl;

	// Finally, free the result variable memory
	delete result;
}

/// <summary>
/// T4 Write a program that prints inverse of a number that a user has given.
/// Take care that the program works correctly with any input.
/// Carefully checks and validates the input first.
/// After that, converts it as an inversion of the value if the value was a valid number
/// </summary>
void t4()
{
	// Limit the amount of numbers in the input
	// to prevent overflowing
	const auto num_size = 10;

	// Collect the input as string first
	std::string input;
	std::cout << "Enter a number (max digits: "<< num_size << ", max fractions: " << num_size  << "): ";
	std::cin >> input;

	// Validate the input as a number (int or double)
	if (!is_number(input))
	{
		// If invalid, throw an exception, caught in Main
		throw std::exception("Input was not a valid number.");
	}

	// If decimal fractions, check fraction count overflow
	// Validate against limits
	std::string beforeDot;
	std::string afterDot;

	// Ensure the limits were respected
	// First, check if the number was a decimal value
	if (input.find('.') != std::string::npos)
	{
		// Collect the number whole and decimals parts separately
		beforeDot = input.substr(0, input.find('.'));
		afterDot = input.substr(input.find('.') + 1, input.size());
	}
	// If was not a decimal, only collect the whole part of the number, as the complete input
	else
	{
		beforeDot = input;
	}
	// Ensure the decimal part does not over limit, if provided
	if (!afterDot.empty() && afterDot.size() > num_size)
	{
		throw std::exception("Too many decimal fractions in number input.");
	}
	// Ensure the whole part does not over the limit
	if (beforeDot.size() > num_size)
	{
		throw std::exception("Too many digits in the number input.");
	}

	// Convert the input into a double
	const auto num = std::stod(input);

	// Form the output result stdout buffer
	std::ostringstream result;
	// Set the correct output precision, depending on the input size
	// If the size was less than limit, use the size, otherwise truncate to the limit
	result << std::setprecision(input.size() > 2 * num_size + 1 ? 2*num_size + 1 : input.size());

	// If value was 0 -> return inf
	if (num == 0.0)
	{
		result << "Infinity";
	}
	// If value was negative number, add minus sign to the beginning of the result
	else if (num < 0.0)
	{
		result << "	1\n-	-\n	" << -num;
	}
	// Form the result
	else
	{
		result << "1\n-\n" << num;
	}

	// Spit the result to STDOUT
	std::cout << "\n\nResult:\n" << result.str() << std::endl;
}

/// <summary>
/// T5 Write a program that reads a file
/// and filters all characters except letter’s, numbers, commas and hyphen.
/// Print the result on the screen. If you made exercise 2, use that data.
/// </summary>
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
	// If not, discard the character and move to next
	while(file.get(ch))
	{
		// Allowed characters: letters, numbers, comma, hyphen
		if (!std::isalnum(ch) && ch != ',' && ch != '-') continue;
		data.push_back(ch);
	}

	// Finally, close the file
	file.close();

	// Output the filtered file contents
	std::cout << "Filtered file contents: " << data << std::endl;
}

/// <summary>
/// The main function of this exercise.
/// Used to control the executed exercise tasks.
/// </summary>
/// <returns>Program end state (0 = success, not 0 = error)</returns>
int main()
{
	try
	{
		t2();
		//t3();
		//t4();
		//t5();
	}
	// Catch any exceptions occurred during execution
	catch (const std::exception &e)
	{
		// Log the exception information to STDOUT and exit
		std::cout << "Caught exception: " << e.what() << std::endl;
	}

	// Ensure <key> pressed before program exits
	char c;
	std::cout << "\nPress any key to exit." << std::endl;
	std::cin.ignore(std::numeric_limits<int>::max(), '\n');
	std::cin.get(&c, 0);
	
	return 0;
}
