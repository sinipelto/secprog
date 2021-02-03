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

// STD C++ headers
#include <iostream>
#include <fstream>
#include <string>
#include <iomanip>
#include <string>
#include <algorithm>
#include <sstream>
#include <vector>

// CryptoPP headers
#include <cryptlib.h>
#include <secblock.h>
#include <osrng.h>
#include <modes.h>
#include <sha3.h>
#include <hex.h>
#include <pwdbased.h>
#include <filters.h>
#include <threefish.h>
#include <aes.h>
#include <files.h>
#include <simple.h>

struct user_entry
{
	std::string uname;
	std::string hash;
	std::string salt;
};

const char DELIM = ';';

user_entry* get_user_line(const std::string * const &uname, std::fstream& source)
{
	if (!source || !source.is_open()) 
		throw std::exception("Input source file stream was not open.");
	
	for (std::string line; std::getline(source, line);)
	{
		const auto f = line.find(DELIM, 0);

		std::string user;

		// Check if user matches the one in file line
		if (*uname != line.substr(0, f)) continue;

		// Safely assume that both hash and salt found in the file
		const auto s = line.find(DELIM, f+1);

		// Form the user entry object: uname, hash, salt
		auto* const entry = new user_entry{
			*uname,
		line.substr(f + 1, s - f - 1),
		line.substr(s + 1, line.back())
		};

		return entry;
	}

	return nullptr;
}

void store_user(const user_entry* const entry, std::fstream& dest)
{
	if (!dest || !dest.is_open()) 
		throw std::exception("Target file stream was not open.");

	const auto line = "\n" + entry->uname + DELIM + entry->hash + DELIM + entry->salt;
	
	dest.write(line.c_str(), line.size());
}

/// <summary>
/// Function for processing user input.
/// Type 0 : Username input
/// Type 1 : Password input
/// </summary>
/// <param name="input">Pointer to string where to store the user input</param>
/// <param name="prompt">What to prompt from the user</param>
/// <param name="type">Type of the prompt (username, password, ...)</param>
void user_input(std::string* const input, const std::string& prompt, const int type)
{
	auto ok = false;
	
	while (!ok)
	{
		std::cout << std::endl << prompt;
		
		std::getline(std::cin, *input, '\n');

		if (input->empty())
		{
			std::cout << "Input was empty." << std::endl;
			continue;
		}
		
		if (type == 0 && input->find(DELIM) != std::string::npos)
		{
			std::cout << "Username should not contain the following inputs: [" << DELIM << "]" << std::endl;
			continue;
		}
		
		ok = true;
	}
}

bool authenticate_user(user_entry* const entry, const std::string* passwd)
{
	// Dive into cryptopp namespace
	using namespace CryptoPP;

	// Copy the password string into a secure byte block
	SecByteBlock passwd_block(reinterpret_cast<const byte*>(passwd->data()), passwd->size());

	// Salt size: 16 bytes = 128 bits => big enough key space for ensuring unique salts
	SecByteBlock salt(16);
	auto x = salt.size();
	auto y = salt.SizeInBytes();

	// If salt empty, generate new
	if (entry->salt.empty())
	{
		// use PSRNG to generate new random salt
		OS_GenerateRandomBlock(true, salt, salt.size());

		// store salt to user entry
		StringSource ss(salt, salt.size(), true, new HexEncoder(new StringSink(entry->salt)));
	}
	// If salt exists, read salt
	else
	{
		// Retrieve existing salt from entry
		StringSource ss(entry->salt, true, new HexDecoder(new ArraySink(salt, salt.size())));
	}

	// Instantiate secure password hasher (PBKDF2-HMAC)
	const PKCS5_PBKDF2_HMAC<SHA3_512> pkcs5_pbkdf2_hmac;

	SecByteBlock hash(SHA3_512::DIGESTSIZE);

	// Derive key from the password input and salt
	pkcs5_pbkdf2_hmac.DeriveKey(
				hash, // key output
		hash.size(), // key output size
		0x0, // purpose
		passwd_block, // password input
		passwd_block.size(), // password input size
				salt, // salt
		salt.size(), //salt size
		10'000); // key function iterations

	// Collect the derived key as hex
	std::string hash_hex;
	StringSource ss(hash, hash.size(), true, new HexEncoder(new StringSink(hash_hex)));

	// Check if hash matched to stored one TODO
	if (hash_hex == entry->hash)
	{
		// User password correct
		return true;
	}

	// If no hash was previously stored - store new password hash
	if (entry->hash.empty())
	{
		entry->hash = hash_hex;
	}

	return false;
}

/// <summary>
/// T2 Implement a program that authenticates a user with a user name and password.
/// Hashes of passwords are stored in a file. Please notice to use random salt.
/// Using Crypto++ crypto library with SHA3-512 (SHA3 with 512 bits long hash) hashing standard to provide the hash.
/// SHA3 (Secure Hashing Algorithm 3) is the latest hashing standard.
/// The length of the hash helps making it more difficult to crack and allows more broader "key space".
/// It uses Keccak hash algorithm under hood.
/// PBKDF2 is used for the password derivation function
/// </summary>
void t2()
{
	const std::string user_file = "./users.txt";

	// Variable for std input
	// Keep in the heap for sake of size and privacy
	auto* const input = new std::string();

	while (true)
	{
		std::cout << std::endl << "*********************************" << std::endl;
		
		// Collect username from stdin
		user_input(input, "Enter username: ", 0);

		std::fstream file_handle(user_file, std::ios::in);

		// User database entry variable
		user_entry* udata = nullptr;

		// Try to open file as readonly first
		// If fails, try to create the file
		if (!file_handle || !file_handle.is_open())
		{
			// FILE DID NOT EXIST OR INACCESSIBLE
			file_handle.open(user_file, std::ios::out | std::ios::trunc);

			// If could not create the user database file, abort
			if (!file_handle || !file_handle.is_open())
			{
				throw std::exception("Could not access user file.");
			}

			// FILE OPEN FOR WRITE
			file_handle.close();
		}
		else
		{
			// FILE OPEN FOR READ
			udata = get_user_line(input, file_handle);
			file_handle.close();
		}

		bool user_exists;

		// User not found from db or file didn't exist
		if (udata == nullptr)
		{
			user_exists = false;
			// Allocate new user entry with current username
			udata = new user_entry {*input};
			
			std::cout << "User '" + *input + "' does not exist in file." << std::endl;

			user_input(input, "Enter NEW password for user '" + *input + "'(max 512 characters): ",1);
		}
		else
		{
			// USER FOUND
			user_exists = true;

			user_input(input, "Enter password for user '" + *input + "': ", 1);
		}

		// Try to authenticate and generate hash and salt
		const auto auth_ok = authenticate_user(udata, input);

		// user_exists => udata != nullptr
		if (user_exists && auth_ok)
		{
			std::cout << std::endl << "Correct password. Auth SUCCESS" << std::endl;
			delete udata;
			break;
		}
		if (user_exists)
		{
			std::cout << std::endl << "Invalid password provided. Auth FAILED" << std::endl;
			delete udata;
			continue;
		}

		// Store user data into the user database
		file_handle.open(user_file, std::ios::out | std::ios::app);
		if (!file_handle || !file_handle.is_open())
		{
			delete udata;
			throw std::exception("Could not open user file for write");
		}
		store_user(udata, file_handle);
		file_handle.close();

		delete udata;

		std::cout << "New username and password stored in file." << std::endl;
	}

	delete input;
}

/// <summary>
/// T3 Implement a program that encrypts and decrypts a file.
/// If you made exercise 2 you may use the authentication to help encryption.
/// t2 is not used in this answer.
/// The file contents are encrypted or decrypted, based on user prompt.
/// Encryption algorithm X is used from CryptoPP library
/// </summary>
void t3()
{
}

int main()
{
	try
	{
		t2();
		t3();
	}
	catch (const std::exception& e)
	{
		std::cout << "Exception: " << e.what() << std::endl;
	}
}