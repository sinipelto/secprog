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
#include <gcm.h>
#include <ccm.h>
#include <simple.h>

struct user_entry
{
	std::string uname;
	std::string hash;
	std::string salt;
};

const char DELIM = ';';

std::string read_file_contents(const std::string& fname, const bool binary)
{
	std::ifstream file;

	if (binary)
	{
		file.open(fname, std::ios::in | std::ios::binary);
	}
	else
	{
		file.open(fname, std::ios::in);

	}

	if (!file || !file.is_open())
	{
		throw std::exception("Could not open file with provided filename.");
	}

	// Read encryption source file contents
	std::string content{ std::istreambuf_iterator<char>(file), std::istreambuf_iterator<char>() };
	file.close();

	return content;
}

void store_file_contents(const std::string& fname, const bool binary, const std::string& content)
{
	std::ofstream file;

	if (binary)
	{
		file.open(fname, std::ios::out | std::ios::binary);
	}
	else
	{
		file.open(fname, std::ios::out);
	}
	
	file.write(content.c_str(), content.size());
	file.close();
}

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
		return new user_entry{
			*uname,
		line.substr(f + 1, s - f - 1),
		line.substr(s + 1, line.back())
		};
	}

	return nullptr;
}

user_entry* get_user_from_userdb(const std::string& db, const std::string& uname)
{
	std::fstream ufile(db, std::ios::in);
	auto* const entry = get_user_line(&uname, ufile);
	ufile.close();

	if (entry == nullptr)
	{
		std::cout << "Username does not exist" << std::endl;
		return nullptr;
	}

	return entry;
}

void store_user(const user_entry* const entry, const std::string& dest)
{
	std::ofstream file(dest, std::ios::out | std::ios::app);
	
	if (!file || !file.is_open())
	{
		throw std::exception("Could not open file to write new user.");
	}

	const auto line = "\n" + entry->uname + DELIM + entry->hash + DELIM + entry->salt;
	
	file.write(line.c_str(), line.size());
}

/// <summary>
/// Function for processing user input.
/// Type 0 : Username input
/// Type 1 : Password input
/// Type 2 : Normal text input
/// </summary>
/// <param name="input">Pointer to string where to store the user input</param>
/// <param name="prompt">What to prompt from the user</param>
/// <param name="type">Type of the prompt (username, password, normal text ...)</param>
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

		if (type == 1 && input->size() > 512)
		{
			std::cout << "Input too long." << std::endl;
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

std::string gen_salt(const size_t len, CryptoPP::SecByteBlock* const &out, const bool use_out)
{
	using namespace CryptoPP;
	
	if (use_out)
	{
		// Generate new salt for out param
		OS_GenerateRandomBlock(true, *out, out->size());

		// Return the salt as Hex
		std::string salt_hex;
		ArraySource as(*out, out->size(), true, new HexEncoder(new StringSink(salt_hex)));
		return salt_hex;
	}

	// Create sec byte block for salt
	SecByteBlock salt(len);

	// Use OS PSRNG to generate new random salt
	OS_GenerateRandomBlock(true, salt, salt.size());
	
	// Return the salt as Hex
	std::string salt_hex;
	ArraySource as(salt, salt.size(), true, new HexEncoder(new StringSink(salt_hex)));
	return salt_hex;
}

bool password_hash(user_entry* const entry, const std::string* passwd)
{
	// Dive into cryptopp namespace
	using namespace CryptoPP;

	// Copy the password string into a secure byte block
	SecByteBlock passwd_block(reinterpret_cast<const byte*>(passwd->data()), passwd->size());

	// Salt size: 32 bytes = 256 bits => big enough keyspace for ensuring unique salts
	SecByteBlock salt(SHA3_512::DIGESTSIZE / 2);

	// If salt empty, generate new
	if (entry->salt.empty())
	{
		entry->salt = gen_salt(salt.size(), &salt, true);
	}
	// If salt exists, read existing salt value
	else
	{
		// Retrieve existing salt from entry
		StringSource ss(entry->salt, true, new HexDecoder(new ArraySink(salt, salt.size())));
	}
	
	// Instantiate secure password hasher (PBKDF2-HMAC using SHA3_512 for hashing)
	const PKCS5_PBKDF2_HMAC<SHA3_512> pkcs5_pbkdf2_hmac;

	// Store hash into byteblock
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
		5'000); // key function iterations (1 000 < i < 10 000)

	// Collect the derived key as hex digest (digest size == 64 by == 512 b)
	std::string hash_hex;
	ArraySource ss(hash, SHA3_512::DIGESTSIZE, true, new HexEncoder(new StringSink(hash_hex)));

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

user_entry* derive_new_hash(user_entry* const entry, const std::string& passwd)
{
	// Remove user personal password hash to use different password
	entry->hash = "";

	// Get derived key from the password input (using PBKDF2)
	password_hash(entry, &passwd);

	// Return updated entry
	return entry;
}

user_entry* create_new_user(const std::string& uname)
{
	// Allocate new user entry with current username
	return new user_entry{ uname };
}

void debug_print(const CryptoPP::SecByteBlock& key, const CryptoPP::SecByteBlock& iv, const bool mode)
{
	using namespace CryptoPP;

	std::cout << std::endl;

	if (mode)
	{
		std::cout << std::endl << "ENC KEY: ";
		ArraySource akey(key, key.size(), true, new HexEncoder(new FileSink(std::cout)));

		std::cout << std::endl << "ENC IV: ";
		ArraySource aiv(iv, iv.size(), true, new HexEncoder(new FileSink(std::cout)));
	}
	else
	{
		std::cout << std::endl << "DEC KEY: ";
		ArraySource akey(key, key.size(), true, new HexEncoder(new FileSink(std::cout)));

		std::cout << std::endl << "DEC IV: ";
		ArraySource aiv(iv, iv.size(), true, new HexEncoder(new FileSink(std::cout)));
	}

	std::cout << std::endl;
}

std::string aes_cbc_encrypt(const CryptoPP::SecByteBlock& key, const CryptoPP::SecByteBlock& iv, const std::string& content)
{
	using namespace CryptoPP;

	// Create AES CBC decryptor object with provided key and IV
	CBC_Mode<AES>::Encryption enc(key, key.size(), iv);

	// Encrypted output as c++ string
	std::string output;

	// Encrypt the provided content
	StringSource s(content, true,
		new StreamTransformationFilter(enc,
			new StringSink(output)
		)
	);

	std::cout << "Plain content: " << content << std::endl;

	// Store IV to file as cleartext into the file end
	ArraySource ivs(iv, iv.size(),
		true,
		new StringSink(output)
	);

	return output;
}

std::string aes_cbc_decrypt(const CryptoPP::SecByteBlock& key, const CryptoPP::SecByteBlock& iv, std::string& content)
{
	using namespace CryptoPP;
	
	// Create AES decryptor object with key and IV
	CBC_Mode<AES>::Decryption dec(key, key.size(), iv);

	// Remove IV from file contents to decrypt
	content.erase(content.size() - AES::BLOCKSIZE, AES::BLOCKSIZE);

	std::string output;
	
	StringSource s(content, true,
		new StreamTransformationFilter(dec,
			new StringSink(output)
		)
	);

	std::cout << "Decrypted content: " << output << std::endl;

	return output;
}

CryptoPP::SecByteBlock create_aes_key(const std::string& data, const size_t len)
{
	using namespace CryptoPP;

	// Derived key from user input password, cut to correct length
	return SecByteBlock(reinterpret_cast<const byte*>(data.data()), len); // 16 bytes = 128 bits
}

CryptoPP::SecByteBlock create_aes_iv(const void* data, const size_t len)
{
	using namespace CryptoPP;

	if (data == nullptr)
	{
		SecByteBlock iv(len);
		OS_GenerateRandomBlock(true, iv, iv.size());
		return iv;
	}
	else
	{
		return SecByteBlock(static_cast<const byte*>(data), len);
	}
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
	const std::string user_file = "./userdata.txt";

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

			std::cout << "User '" + *input + "' does not exist in file." << std::endl;

			// Allocate new user entry with current username
			udata = create_new_user(*input);

			// Prompt for new password for user
			user_input(input, "Enter NEW password for user '" + *input + "'(max 512 characters): ", 1);
		}
		else
		{
			// USER FOUND
			user_exists = true;
			user_input(input, "Enter password for user '" + *input + "': ", 1);
		}

		// Try to authenticate and generate hash and salt
		const auto auth_ok = password_hash(udata, input);

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

		try
		{
			store_user(udata, user_file);
		}
		catch (...)
		{
			delete udata;
			throw;
		}

		std::cout << "New username and password stored in file." << std::endl;
	}

	delete input;
}

/// <summary>
/// T3 Implement a program that encrypts and decrypts a file.
/// If you made exercise 2 you may use the authentication to help encryption.
/// The file contents are encrypted or decrypted, based on user prompt.
/// Using symmetric crypto because its faster.
/// Encryption algorithm X is used from CryptoPP library
/// </summary>
void t3()
{
	const std::string userdb = "./userdata.txt";
	auto* const input = new std::string();

	user_input(input, "Encrypt or Decrypt (e/d)? ", 3); // TODO HACK
	//*input = "e"; // TODO HACK

	bool do_encrypt;

	// Main control flow
	if (*input == "e")
	{
		do_encrypt = true;
	}
	else if (*input == "d")
	{
		do_encrypt = false;
	}
	else
	{
		std::cout << "Input not recogrnized as an action." << std::endl;
		delete input;
		return;
	}

	// BEGIN
	
	if (do_encrypt)
	{
		user_input(input, "File name to encrypt: ", 3);
	}
	else
	{
		user_input(input, "File name to decrypt: ", 3);
	}

	const auto filename(*input);

	std::string content;
	
	if (do_encrypt)
	{
		// Read input as normal text
		content = read_file_contents(filename, false);
	}
	else
	{
		// Treat encrypted file as binary input
		content = read_file_contents(filename, true);
	}

	user_input(input, "Enter username: ", 0);
	auto* entry = get_user_from_userdb(userdb, *input);

	// User was not found
	if (entry == nullptr)
	{
		// Create new user and store it
		std::cout << "Creating new user '" << *input << "'" << std::endl;
		entry = create_new_user(*input);
		entry->salt = gen_salt(32, nullptr, false);
		store_user(entry, userdb);
	}

	if (do_encrypt)
	{
		user_input(input, "Enter file encryption password: ", 1);	
	}
	else
	{
		user_input(input, "Enter file decryption password: ", 1);
	}
	
	derive_new_hash(entry, *input);
	//derive_new_hash(entry, "salasana123");

	if (do_encrypt)
	{
		user_input(input, "Enter file name for encrypted output: ", 2);
	}
	else
	{
		user_input(input, "Enter file name for decrypted output: ", 2);
	}

	const auto fname = *input;

	// Derived key from user input password, cut to correct AES key length
	const auto key = create_aes_key(entry->hash, CryptoPP::AES::DEFAULT_KEYLENGTH); // 16 bytes = 128 bits

	std::string output;
	
	if (do_encrypt)
	{
		// Generate random IV
		const auto iv = create_aes_iv(nullptr, CryptoPP::AES::BLOCKSIZE); // 16 bytes = 128 bits

		// Encrypt the content using AES-CBC
		output = aes_cbc_encrypt(key, iv, content);
	}
	else
	{
		// Use the IV from the last 16 bytes of the encrypted file
		const auto iv = create_aes_iv(reinterpret_cast<const CryptoPP::byte*>(&*(content.end() - CryptoPP::AES::BLOCKSIZE)), CryptoPP::AES::BLOCKSIZE); // 16 = 128 bits

		// Decrypt the content using AES-CBC
		output = aes_cbc_decrypt(key, iv, content);
	}

	if (do_encrypt)
	{
		// Treat encrypted string as binary
		store_file_contents(fname, true, output);
		std::cout << "Encrypted content saved into: " << fname << std::endl;
	}
	else
	{
		store_file_contents(fname, false, output);
		std::cout << "Decrypted content saved into: " << fname << std::endl;
	}

	delete input;
}

int main()
{
	try
	{
		//t2();
		t3();
	}
	catch (const std::exception& e)
	{
		std::cout << std::endl << std::endl << "EXCEPTION: " << e.what() << std::endl;
	}
}