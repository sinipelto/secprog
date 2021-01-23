#include <cstdlib>
#include <cstdio>
#include <cstring>
#include <iostream>

// *** T1 ***
// Overflowing a char array "unlimitedly" with longer string than it would fit
void unlim_buf_ovf()
{
	char p[4];
	strcpy(p, "Hello World"); // Unlimited overflow
}

// *** T2 ***
// Out of array bounds by one
void off_by_one_ovf()
{
	char buf[3] = { 'a', 'b', 'c' };
	buf[3] = 'd'; // Off by one overflow
}

// *** T3 ***
// Writing to a memory slot that was already released.
// Reading dynamic memory over bounds => reading memory that might not belong to this program at all
// Possibly reading memory area allocated for another process and reveal secrets from them?
void heap_ovf()
{
	// Dynamically allocate two pointers
	char* p;
	p = (char*)malloc(sizeof(char) * 4);
	char* q;
	q = (char*)malloc(sizeof(char) * 4);

	// Tell the OS to free the allocated memory from this process
	free(p);
	// Write to that memory area with a valid input, after it has been already released
	strcpy(p, "abc");

	// Read a dynamic memory over bounds, to see whats in the memory outside of the allocated memory area
	char buf[32];
	memcpy(buf, q, 32);

	// Print the result
	printf(q);
}

// *** T4 ***
// Overflow a function pointer to target an invalid location
void f()
{
	printf("hello from another function.");
}

void func_ptr_ovf()
{
	// First a Valid function location address to a object pointer
	// Add + 0xF to the pointer address value (4 byte memory slot offsets) converted into a char pointer and convert back into function pointer
	// Try to call the function pointer value
	//((void(*)())((char*)&func_ptr_ovf + 0x0F + 0xAA))(); // urctbased.dll function
	((void(*)())((char*)&func_ptr_ovf + 0xAA))();
}

// *** T5 ***
// Call prinft function with a string containing format specifiers.
// These specifiers cause the printf to leak data which it is not supposed to be aware of.
void printf_vuln()
{
	char input[] = "helloworld %s%s%s";
	printf(input);
}

int main()
{
	//unlim_buf_ovf();
	//off_by_one_ovf();
	heap_ovf();
	//func_ptr_ovf();
	//printf_vuln();
}
