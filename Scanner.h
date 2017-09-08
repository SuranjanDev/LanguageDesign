#ifndef SCANNER
#define SCANNER

#include<iostream>
#include<string>
#include <fstream>
using namespace std;

const string KEYWORDS[] = { "int","bool","double","if",		// List of Keywords
"else","while","true",
"false","void","return",
"print","scan","const","char","main" };
const string FILENAME = "input.txt"; // Name of input file.
char buffer[4096];	// Decalring the buffer array of size 4096.
char *currentChar;	// Pointer to the current character in the buffer
int state = 0;		// Initial state of the DFA
					/*
					Structure of the token. It has a name and a number.
					*/
struct Token {
	string TokenName;
	int TokenNumber;
}CurrentToken;	// CurrentToken holds the current token found.
				/*
				The Scanner class implements methods to read from a file, fill the buffer and return the current token found.
				*/
class Scanner {
public:
	Scanner() {
		currentChar = buffer;// Initialize the currentChar pointer to the begining of the buffer
		FillBuffer();// Fill the buffer
	}
	/*
	The FillBuffer function fills the buffer
	*/
	void FillBuffer() {
		int index = 0;
		ifstream fileOpen(FILENAME);// Opens the file
		while (fileOpen.get(buffer[index]))
			index++;
		fileOpen.close();//Closes the file
	}
	/*
	GetChar function returns the next character in the buffer.
	*/
	char GetChar() {
		char c = *currentChar;
		currentChar++;
		return c;
	}
	/*
	CheckKeyword function checks for valid keywords and returns the appropriate token number.
	If its not a keyword it returns 16 which id the token number for an identifier
	*/
	int CheckKeyword(string word) {
		for (int i = 0; i <15; i++) {
			if (KEYWORDS[i] == word) {
				return (i + 1);
			}
		}
		return 16;
	}
	/*
	GetToken function returns the next token in the buffer
	*/
	Token GetToken() {
		char ch = GetChar(); // Get the character from the buffer 
		CurrentToken.TokenName = "";//Initializing the token name to be empty
		CurrentToken.TokenNumber = NULL;//Initalizing the token number to be NULL.
										/*
										Skip all the white spaces, newlines and tab
										*/
		while (ch == ' ' || ch == '\n' || ch == '\t') {
			ch = GetChar();
		}
		/*
		Skip all the comments
		*/
		if (ch == '/') {
			ch = GetChar();
			if (ch == '/') // Check if the two consecutive characters are //
				while (ch != '\n') // Loop till the end of line
					ch = GetChar();
			else
				currentChar--;// Incase it's not a comment backtrack.
			ch = GetChar();	// Get the next character
		}
		switch (state) {
		case 0:	// The start state
			if (isalpha(ch))		state = 1;// For keywords and identifiers
			else if (isdigit(ch))	state = 2;//For digits
			else if (ch == '=')		state = 3;//For assignment operator
			else if (ch == ';')		state = 4;//For end of statement operator
			else if (ch == '<')		state = 5;//For relational operator
			else if (ch == '>')		state = 6;//For relational operator
			else if (ch == '{')		state = 7;// Opening { bracces
			else if (ch == '(')		state = 8;// Opening ( bracces
			else if (ch == '[')		state = 9;// Opening [ bracces
			else if (ch == '}')		state = 10;//closing } braces
			else if (ch == ')')		state = 11;//closing ) braces
			else if (ch == ']')		state = 12;//closing ] braces
			else if (ch == '+' || ch == '-') state = 13;// For add and subtract operators
			else if (ch == '*' || ch == '%' || ch == '|') state = 14;// For mul, div and rem operators
			else if (ch == '"') state = 15;// For strings
			else if (ch == ',') state = 16;// For comma
			else if (ch == '&') state = 17;// For and
			else if (ch == '|') state = 18;// For or
			else if (ch == '\0')	state = 22;// End of file
			else state = 20;// Error state
			currentChar--;	// Back track one character.
			CurrentToken = GetToken();// To avoid returning null token right after state 0.
			break;
		case 1:// For Keywords and identifiers
			while (isalpha(ch) || isdigit(ch)) {//Loop till the current character is an alphabet or a digit
				CurrentToken.TokenName += ch;
				ch = GetChar();// Get the next character
			}
			currentChar--;//Back track one character
			CurrentToken.TokenNumber = CheckKeyword(CurrentToken.TokenName);//Get ctoken number from the CheckKeyword func.
			state = 0;//Back to start state
			break;
		case 2:// For digits
			while (isdigit(ch)) {//Loop till the current character is a digit
				CurrentToken.TokenName += ch;
				ch = GetChar();// Get the next character
			}
			CurrentToken.TokenNumber = 17;// Token num for digits of type int
			if (ch == '.') {//Check for double type
				CurrentToken.TokenName += ch;
				ch = GetChar();
				while (isdigit(ch)) {//Loop till the current char is a digit
					CurrentToken.TokenName += ch;
					ch = GetChar();
				}
				CurrentToken.TokenNumber = 30;//Token num for digits of type double
			}
			currentChar--;//Back track one character			
			state = 0;
			break;
		case 3://For '=' and '=='
			CurrentToken.TokenName = ch;
			CurrentToken.TokenNumber = 18;//Token number for assignment operator
			ch = GetChar();
			if (ch == '=') {//If = is followed by another = thenits a relational op.
				CurrentToken.TokenName += ch;
				CurrentToken.TokenNumber = 19;//token no for relational op.
			}
			else currentChar--;//Back track one character
			state = 0;
			break;
		case 4:// For ';'
			CurrentToken.TokenName = ch;
			CurrentToken.TokenNumber = 20;//token number for ';'
			state = 0;
			break;
		case 5://for <,<= and <>
			CurrentToken.TokenName = ch;
			CurrentToken.TokenNumber = 19;//Token no for '<'
			ch = GetChar();
			if (ch == '=') {
				CurrentToken.TokenName += ch;
				CurrentToken.TokenNumber = 19;//Token no for '<='
			}
			else if (ch == '>') {
				CurrentToken.TokenName += ch;
				CurrentToken.TokenNumber = 19;//Token no for '<>'
			}
			else currentChar--;//Back track one character
			state = 0;
			break;
		case 6://For '>'
			CurrentToken.TokenName = ch;
			CurrentToken.TokenNumber = 19;//Token number for '>'
			ch = GetChar();
			if (ch == '=') {
				CurrentToken.TokenName += ch;
				CurrentToken.TokenNumber = 19;//TOken number for '>='
			}
			else currentChar--;// Back track one character
			state = 0;
			break;
		case 7:		//For'{'
			CurrentToken.TokenName = ch;
			CurrentToken.TokenNumber = 21;//Token number for '{'
			state = 0;
			break;
		case 8:		//For '('
			CurrentToken.TokenName = ch;
			CurrentToken.TokenNumber = 22;//Token number for '('
			state = 0;
			break;
		case 9:		//For '['
			CurrentToken.TokenName = ch;
			CurrentToken.TokenNumber = 23;//Token number for '['
			state = 0;
			break;
		case 10:	// For '}'
			CurrentToken.TokenName = ch;
			CurrentToken.TokenNumber = 24;//Token number for '}'
			state = 0;
			break;
		case 11:	// For ')'
			CurrentToken.TokenName = ch;
			CurrentToken.TokenNumber = 25;//Token number for ')'
			state = 0;
			break;
		case 12:	//For ']'
			CurrentToken.TokenName = ch;
			CurrentToken.TokenNumber = 26;//Token number for ']'
			state = 0;
			break;
		case 13:	// For '+' or '-'
			CurrentToken.TokenName = ch;
			CurrentToken.TokenNumber = 27;//Token number for '+' or '-'
			state = 0;
			break;
		case 14:	// For '*', '/' or '%'
			CurrentToken.TokenName = ch;
			CurrentToken.TokenNumber = 28;//Token number for '*','/' or '%'
			state = 0;
			break;
		case 15:	// For strings
			ch = GetChar();
			while (ch != '"'&& ch != '\n') {// Loop till you encounter either " or reach end of line
				CurrentToken.TokenName += ch;
				ch = GetChar();
			}
			if (ch == '"')// Valid string if it ends with "
				CurrentToken.TokenNumber = 29;
			else// invalid string
				CurrentToken.TokenNumber = -2;
			state = 0;
			break;
		case 16:	// For ','
			CurrentToken.TokenName = ch;
			CurrentToken.TokenNumber = 31;//Token number for ','
			state = 0;
			break;
		case 17:
			CurrentToken.TokenName = ch;
			ch = GetChar();
			if (ch == '&') {
				CurrentToken.TokenName += ch;
				CurrentToken.TokenNumber = 28;
			}
			else {
				currentChar--;
				CurrentToken.TokenNumber = -2;
			}
			state = 0;
			break;
		case 18:
			CurrentToken.TokenName = ch;
			ch = GetChar();
			if (ch == '|') {
				CurrentToken.TokenName += ch;
				CurrentToken.TokenNumber = 27;
			}
			else {
				currentChar--;
				CurrentToken.TokenNumber = -2;
			}
			state = 0;
			break;
		case 22:	// For end of file token
			CurrentToken.TokenNumber = -1;
			break;
		case 20:	// Error state
			CurrentToken.TokenName = ch;
			CurrentToken.TokenNumber = -2;
			state = 0;
			break;
		}
		return CurrentToken;	// Returns the current token found
	}
};
#endif