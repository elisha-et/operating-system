#include <stdio.h>
#include <stdlib.h>

/* IMPLEMENT ME: Declare your functions here */
int add (int a, int b);
int subtract (int a, int b);
int multiply (int a, int b);
int divide (int a, int b);
int exit_prog (int a, int b);

int main (void)
{
	/* IMPLEMENT ME: Insert your algorithm here */
	int a = 6;
	int b = 3;
	char input;

	// Define an array of function pointers corresponding to 0, 1, 2, 3, 4
	int (*operations[5])(int, int) = {add, subtract, multiply, divide, exit_prog};

	printf("Operand 'a' : %d | Operand 'b' : %d\n", a, b);
	printf("Specify the operation to perform (0 : add | 1 : subtract | 2 : Multiply | 3 : divide | 4 : exit): ");
	
	// Read the character input
	scanf(" %c", &input);

	// Convert the character ('0'-'4') to an integer index (0-4)
	int choice = input - '0';

	// Execute the function at the chosen index and get the result
	int x = operations[choice](a, b);

	printf("x = %d\n", x);

	return 0;
}

/* IMPLEMENT ME: Define your functions here */
int add (int a, int b) { 
    printf ("Adding 'a' and 'b'\n"); 
    return a + b; 
}

int subtract (int a, int b) { 
    printf ("Subtracting 'b' from 'a'\n"); 
    return a - b; 
}

int multiply (int a, int b) { 
    printf ("Multiplying 'a' and 'b'\n"); 
    return a * b; 
}

int divide (int a, int b) { 
    printf ("Dividing 'a' by 'b'\n"); 
    return a / b; 
}

int exit_prog (int a, int b) { 
    exit(0); 
    return 0; 
}