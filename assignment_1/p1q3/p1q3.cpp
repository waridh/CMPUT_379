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

#define MAXLINE 128
#define MAXWORD 20

int main()  {

  // Simple data structure for storing obtained tokens
  typedef struct  {
    char toker[MAXLINE/2][MAXWORD];
    unsigned int count;

  }  token;
  // Initialize the token type
  token tok;

  memset(&tok, 0, sizeof(tok));

  // The count
  tok.count = 0;
  // The delimiters
  char WSPACE[] = "\n \t";

  // Creating the output in sigular token.
  char *tok = strtok(inStr, WSPACE);

  while (tok != NULL)  {
    // Adding to the count
    count++;
    tok = strtok(NULL, WSPACE);
}
}


