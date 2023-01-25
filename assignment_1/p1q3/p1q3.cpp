// Assignment 1 Part 1 Question 3

/*
We are trying to split each input into a token. The tokens are delimited
by field separators. The field separators we are going to be using are
newline whitespace and tab. Assume that each token has at most 20 - 1
characters. We also need to store the  number of tokens obtained in the
variable count. Also specify a simple data structure to store the
obtained tokens.

Function required:
  strtok()
 
Arguments:
  inStr:  The input string.
  WSPACE: Character array of separators. "\n \t".
  count:  The number of obtained tokens.
*/
#include <cstring>
#include <iostream>

using namespace std;
#define MAXLINE 128
#define MAXWORD 20

int main()  {

  // Simple data structure for storing obtained tokens
  typedef struct  {
		/*
		The size of the character matrix is based on the max number of words in the
		string and the max length of a word. The max number of words is the
		character limit per line given in question 2 divided by two, as each word
		must have an accompanying delimiter. The other axis size is based on the
		max length of characters per word. This is just 19 characters + \0 to make
		a string.
		*/
    char toker[MAXLINE/2][MAXWORD];
		
  }  token;

  // Initialize the token type
  token tok;
	// Clearing the structure.
  memset(&tok, 0, sizeof(tok));
	// Sample string for testing
	char inStr[] = "timeout\t10\tdo_work bash.man\n7";
  // Initialize the count
  unsigned int count = 0;
  // The delimiters
  char WSPACE[] = "\n \t";

  // Creating the output in sigular token.
  char *buffet = strtok(inStr, WSPACE);

  while (buffet != NULL)  {
    cout << buffet << endl;
		/*
		Instead of dynamically allocating for this part of the assignment,
		I am taking advantage of the constraint given in the question. I am now
		using the limit of characters per line and per word. Since a word is at
		most 19 characters long, and being a character array, we can use a for
		loop that iterates through the output from strtok and place the
		characters one by one into the struct. Because of the word size limit,
		runtime is not affected as heavily as when there is no limit on size.
		*/
		for (unsigned int i=0; i < sizeof(buffet); i++)  {
			tok.toker[count][i] = buffet[i];
		};
		// Adding to the count
		count++;
    buffet = strtok(NULL, WSPACE);
	};

	for (unsigned int i=0; i<count; i++)  {
		cout << tok.toker[i] << endl;
	}
}


