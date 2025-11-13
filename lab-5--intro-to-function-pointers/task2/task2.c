#include <stdio.h>
#include <stdlib.h>

/* IMPLEMENT ME: Declare your functions here */
int add (int a, int b);
int subtract(int a, int b);
int multiply(int a, int b);
int divide(int a, int b);


int main (void)
{
	/* IMPLEMENT ME: Insert your algorithm here */
	  // Array of function pointers for operations
  int (*operations[])(int, int) = {add, subtract, multiply, divide};

  // Predefined operands
  int a = 6;
  int b = 3;

  printf("Operand 'a': %d | Operand 'b': %d\n", a, b);
  printf("Specify the operation to perform (0:add | 1:subtract | 2:multiply | 3:divide | 4:exit): ");

  // Read operations
  char op;
  scanf(" %c", &op);

  // Get operation index
  int op_index = op - '0';

  // Perform selected operation
  int result = operations[op_index](a, b);

  // Print result based on operation
  const char *op_names[] = {"Adding", "Subtracting", "Multiplying", "Dividing"};
  printf("%s 'a' and 'b' x = %d\n", op_names[op_index], result);

	return 0;
}

/* IMPLEMENT ME: Define your functions here */
int add (int a, int b) { 
	return a + b; 
	}

int subtract(int a, int b){
	return a - b;
}

int multiply(int a, int b){
	return a * b;
}

int divide(int a, int b){
	return a/b;
}
