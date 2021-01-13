#include <cstdlib>
#include <iostream>

// *** T1 ***
// Overflowing a char array "unlimitedly" with longer string than it would fit
void unlim_buf_ovf()
{
	char p[4];
	strcpy(p, "Hello World");
}

// *** T2 ***
// Out of array bounds by one
void off_by_one_ovf()
{
	char input[3];
	input[0] = 'a'; input[1] = 'b'; input[2] = 'c'; // input = "abc"

	char buf[2];
	strcpy(buf, input);
}

// *** T3 ***
// Continuously allocate memory from the heap
// without releasing it
void heap_ovf()
{
	for (auto i = 0; i < 10'000'000; i++)
	{
		new char();
	}
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
	//func_ptr_ovf();
	//printf_vuln();
}
