/*
EBNF for the language with rule numbers.

1			<PROGRAM>	::= {<FUNCPROTO>} MAINTOK '(' ')'  {'{'<STATMT>'}'} {<FUNCDECL>}
2,3,4,5		<STATMT>	::= <ASSIGNSTAT> | <IFSTAT> | <READSTAT> | <WRITESTAT> | <LOOPSTAT>
6,7,8		|<VARDECL>|<RETSTAT>
9			<VARDECL>	::=	<TYPE> IDTOK [ '[' LITTTOK ']' ] ';'
10			<FUNCDECL>	::=	<TYPE> IDTOK '(' {<PRAMLIST> { ',' <PRAMLIST>} ')' '{' {<STATMT>} '}'
11			<PRAMLIST>	::=	TYPETOK IDTOK
12			<ASSIGNSTAT>::= IDTOK['[' NUMTOK ']']	ASTOK <EXPRESS>
13			<IFSTAT>	::= IFTOK '(' <EXPRESS>')' '{' {<STATMT>} '}' [ELSETOK '{' {<STATMT>} '}' ]
15			<WRITESTAT>	::= PRINTTOK '(' (STRINGTOK|<EXPRESS>) ')' ';'
16			<LOOPSTAT>	::=	WHILETOK '(' <EXPRESS> ')' '{'  {<STATMT>}'}'
17			<EXPRESS>	::= <TERM> {ADDOPTOK <TERM> }
18			<TERM>		::=	<RELFACT> { MULOPTOK <RELFACT> }
19			<RELFACT>	::=	<FACTOR>[RELOPTOK<FACTOR>]
20,21,22,23	<FACTOR>	::=	<IDNONTERM> | LITTOK | '{' <EXPRESS> '}' | BOOLTOK
24			<IDNONTERM>	::= IDTOK ['[' LITTOK ']' | '(' { <EXPRESS> { <EXPRESS> } }')']
25			<RETSTAT>	::= RETTOK <IDNONTERM>
26			<FUNCPROTO>	::= TYPETOK ID '(' {<PRAMLIST>} ')' ';'
*/

/*
This program generates the MIPS assembly code for my language "Mini C". For now it only works for integer types.
The program first matches the type of the operand. Both operand should be of INTEGER type.
It then genrates the equivalent MIPS code for while loops, if statements, read statements, write statements,
add operator, subtract operator, multiply operator, divide operator, remainder operator, or operator, and 'and operator' and functions.
The parameters are passed by value using the $a registers. So a maximum of 4 parameters can be passed to a function. If there are
more than 4 parameters, the rest are simply ignored.
For reference
1: int
2: bool
3: double
*/

#include<iostream>
#include "Scanner.h"
#include<string>
#include <fstream> 
#include "Scanner.h"
#include "HashTable.h"
#include "stack.h"

using namespace std;
void FunProto(Token);	// The Prototype of the function used
void Program(Token);	// The start state of the program
void Statmt(Token);		// Function to check for valid statement
void VarDecl(Token);	// Function to check for valid variable declaration
void FunDecl(Token);	// Function to check for valid function declaration
void PramList(Token);	// Function to check for the parameter list a function takes
void AssignStat(Token);	// Function to check for valid assignment statemments
void IfStat(Token);		// Function for valid if-else statements
void ReadStat(Token);	// Function to check for valid read statements
void WriteStat(Token);	// Function to check for valid Write statements
void LoopStat(Token);	// Function to check for valid Loops
void RetStat(Token);	// Function to check for valid Return statements
void Express(Token);	// Function to check for valid Expressions
void Term(Token);		// Function for non terminal term
void RelFact(Token);	// Function for non terminal RelFact
void Factor(Token);		// Function for non terminal Factor
void Idnonterm(Token);	// Function for non terminal Idnonterm
bool IsFirstStat();		// Function to check the first of statement
void match(int);		// Function to check whether the given token number matches the current token no.
void codeGenProlog();	// Function to write the starting lines of the assembly code to a text file 
void codeGenPostlog();	// Function to write the ending lines of the assembly code to a text file
void codeGen(string input);	//Function to write the code generated to a text file.
string genStrLabel();		// Function to generate label for a given strings in the write statements.

/*
Expression record structure to get the type and location of an identifier
*/
struct ExpessionRec {
	int type;
	int loc;
}expRec;

ofstream codefile;	//Declaring codefile to open the output file and write to it.
Token token;	// variable to store the current token	
Scanner sc;		// The variable sc represents the object of the scanner class that is used to get the next token
int regNo = 0;	// Variable to indicate the index of register a. This is used for parameter passing
HashTable hashTable(13);	// Hash table to store the details of the identifiers
stack activeBlock;				// Creating a stack to store the list of braces that are currently opened but not closed.(Active blocks)
stack retAddr;					// Creating a stack to store the location of a function's return address in the stack
int currentBlock = -1;			// currentBlock represents the current block in which the identifier is present. It is initialized to -1.
int scope, type, arraySize, idType;			// Temporary variable to store the scope, type, size in case of array and type 
string name, funcName, pram, errorMsg = "";	// Temporary variable to store the name, parameters and error messages

static int strLabel = 0;				// Variable used to generate unique string names
int ifCount = 1, loopCount = 1, setVal = 0;	//Variables to print label for IF and LOOP statements respectively
string stringOutput = "";				// Variable to store the list of strings in the write statement.
int funType;	// Variable to match the return value of a function with its return type.
bool codeIsGood = true;	// To check if any warning is generated. If so then no output file will be generated.
int offset = 0;	// Set the current offset to zero.
/*
The main runs the parser and displays if the program is legal or not.
If the program is legal then it throws temporary warnings if there is any.
*/
int main() {
	token = sc.GetToken();	//Get the first token
	Program(token);			// Pass it to the start state
	cout << endl << "**Legal Program**  ";// Display Legal if no error were thrown. The program would terminate immediately incase of any error.
	cout << endl << errorMsg;

	hashTable.display(currentBlock); // Display the symbol table
	return 0;
}
/* The Function matches a given token number with the current token and if it doesn't
match it exits the program
*/
void match(int TokenNum) {
	if (TokenNum == token.TokenNumber)
		token = sc.GetToken();	// If the two tokens match then get the next token
	else {						// Else just display error message and terminate the program
		cout << endl << "Syntax Error  ";
		exit(0);
	}
}
/*
Function to write the prolog(starting part of the assembly code) to the SPIM file.
*/
void codeGenProlog() {
	codefile.open("output.s", std::ofstream::out);
	codefile << "#Prolog:\n";
	codefile << ".text \n";
	codefile << ".globl  main \n";
	codefile << "main: \n";
	codefile << "move  $fp,  $sp \n";
	codefile << "la  $a0,  ProgStart \n";
	codefile << "li  $v0, 4 \n";
	codefile << "syscall \n";
	codefile << "#End of Prolog\n";
}
/*
Function to write the postlog(ending part of the assembly code) to the SPIM file.
*/
void codeGenPostlog() {
	// If there are no warnings then write the output
	if (codeIsGood) {
		codefile << "Postlog:\n";
		codefile << "la $a0, ProgEnd\n";
		codefile << "li $v0, 4\n";
		codefile << "syscall\n";
		codefile << "li $v0, 10\n";
		codefile << "syscall\n";
		codefile << ".data\n";
		codefile << stringOutput;
		codefile << "ProgStart:  .asciiz	\"Program Start\\n\"\n";
		codefile << "ProgEnd:  .asciiz	\"\\nProgram End\\n\"\n";
		codefile.close();
	}
	// If there are warnings then simply delete the file
	else {
		codefile.close();
		remove("output.s");
	}
}
/*
Function to write the intermediate code to the SPIM file
*/
void codeGen(string code) {
	codefile << code << '\n';
}
/*
Function to generate an unique string label for each write function.
*/
string genStrLabel(string lab) {
	strLabel++;
	return lab + to_string(strLabel);
}
/*
Checks for the First(Statement).
*/
bool IsFirstStat() {
	return (token.TokenNumber == 16 || token.TokenNumber == 4 || token.TokenNumber == 11
		|| token.TokenNumber == 12 || token.TokenNumber == 6 || token.TokenNumber == 10
		|| token.TokenNumber == 1 || token.TokenNumber == 2 || token.TokenNumber == 3);
}
/*	The start state. The rules are
<PROGRAM>	::= {<FUNCPROTO>} MAINTOK '(' ')'  {'{'<STATMT>'}'} {<FUNCDECL>}
*/
void Program(Token currToken) {
	cout << "Rules:  ";
	codeGenProlog();
	currentBlock++;		//Increment the current block
	activeBlock.push(currentBlock);//Push it onto the stack of current blocks
	while (token.TokenNumber == 1 || token.TokenNumber == 2 || token.TokenNumber == 3 || token.TokenNumber == 9)
		FunProto(token);

	cout << 1 << " ";
	match(15);	// Match token number for the keyword main
	match(22);	// match token number for '('
	match(25);	// match token number for ')'
	match(21);	// match token number for '{'

	/*
	Check for the FIRST(statement) which can be an IDTOK, IFTOK, READTOK,WRITETOK
	LOOPTOK,RETTOK,INTTOK,DOUBLETOK or BOOLTOK.
	Loop for multiple statements
	*/
	while (IsFirstStat())
		Statmt(token);
	match(24);// match token number for '}'
	activeBlock.pop();
	/*
	Check for FIRST(FunDecl) which can be int, bool, double or void.
	Loop for multiple function declarations.
	*/
	while (token.TokenNumber == 1 || token.TokenNumber == 2 || token.TokenNumber == 3 || token.TokenNumber == 9) {
		codeGen("j Postlog"); // In case there are any functions. The code should directly jump to the postlog to avoid infinite loops.
		FunDecl(token);
	}
	codeGenPostlog();
	match(-1);// match token number for end of file token.
}
/* Check for valid statements. The rules are
<STATMT>	::= <ASSIGNSTAT> | <IFSTAT> | <READSTAT> | <WRITESTAT> | <LOOPSTAT>
|<VARDECL>|<RETSTAT>
*/
void Statmt(Token currToken) {
	switch (currToken.TokenNumber) {
	case 16:	// If it starts with an identifier
		cout << 2 << " ";
		AssignStat(token);// Call assignment statement
		break;
	case 4:		// If its a if statement
		cout << 3 << " ";
		IfStat(token);// call if statement
		break;
	case 12:	// If its a read statement
		cout << 4 << " ";
		ReadStat(token);// call read statement
		break;
	case 11:	// If its a write statement
		cout << 5 << " ";
		WriteStat(token);// call write statement
		break;
	case 6:		// If its a loop statement
		cout << 6 << " ";
		LoopStat(token);// Call loop statement
		break;
	case 10:	// If its a return statement
		cout << 8 << " ";
		RetStat(token);//Call return statement
		break;
	case 1:		// If it starts with an integer token
		cout << 7 << " ";
		VarDecl(token);// Call variable declaration
		break;
	case 2:		// If it starts with a bool token
		cout << 7 << " ";
		VarDecl(token);// Call variable declaration
		break;
	case 3:		// If it starts with a double token
		cout << 7 << " ";
		VarDecl(token);// call variable declaration
		break;
	default:	// else just throw an error and exit
		cout << endl << "Syntax Error  ";
		exit(0);
	}
}
/*
Function to check for valid assignment statements. The rules are
<ASSIGNSTAT>::= IDTOK['[' NUMTOK ']']	ASTOK <EXPRESS>
*/
void AssignStat(Token currToken) {
	cout << 12 << " ";
	int identType, identLoc;	// To get the identifier type and location
	identifier *ident = nullptr;// To get the pointer returned from the symbol table when an identifier is searched.
	if (currToken.TokenNumber == 16)//If the current token is an identifier, look it up in the symbol table
		ident = hashTable.findInAll(currToken.TokenName, activeBlock, "", 0);
	if (ident != nullptr) {
		identType = (*ident).type;		// Store the type in a temporary vaiable
		identLoc = (*ident).location;	// Store the location in a temporary variable
	}
	//cout << endl << identType << " " << identLoc << endl;
	match(16);			//match an identifier
	if (token.TokenNumber == 23) {// match token number for '['. Incase it's an array
		match(23);
		arraySize = stoi(token.TokenName);
		identLoc -= arraySize * 4;	// Get the appropriate memory location as per array index 
		match(17);// match token number for a valid digit(Size of array)
		match(26);// match token number for ']'
	}
	match(18);			// Check for the assignment token '='	
	Express(token);		// Check for valid expression		
	if (identType != expRec.type) { //Throw an error message if types don't match
		errorMsg += "Warning: Types don't match: " + to_string(identType) + " to " + to_string(expRec.type) + '\n';
		codeIsGood = false;
	}
	// Load the value from the stack pointer to the register t0 and then store it at the location of the identifier
	codeGen("lw $t0," + to_string(expRec.loc) + "($sp)");
	codeGen("sw $t0," + to_string(identLoc) + "($sp)");
	match(20);			// Must end in ';'
}
/*
Check for valid If statements. The rules are
<IFSTAT>	::= IFTOK '(' <EXPRESS>')' '{' {<STATMT>} '}' [ELSETOK '{' {<STATMT>} '}' ]
*/
void IfStat(Token currToken) {
	cout << 13 << " ";
	int ifCtr = ifCount;// Temporary counter to generate an if label. Necessary for nested if statements.
	ifCount++;
	match(4);			// Must start with the keyword 'if'
	match(22);			// match token number for '(' 
	Express(token);		// Must be a valid expression 
	codeGen("if" + to_string(ifCtr) + ": ");	// Generate the label if
	codeGen("lw $t2," + to_string(expRec.loc) + "($sp)");//Get the value of t2. 0 for false and 1 for true
	codeGen("beq $t2,$0,else" + to_string(ifCtr));// If t2 is 0 then go to else.
	match(25);			// match token number for ')'
	match(21);			// match token number for '{'
	currentBlock++;		// Increment the current block
	activeBlock.push(currentBlock);
	Statmt(token);		// The if block must have atleast one statement
	/*
	Check for the FIRST(statement) which can be an IDTOK, IFTOK, READTOK,WRITETOK
	LOOPTOK,RETTOK,INTTOK,DOUBLETOK or BOOLTOK.
	Loop for multiple statements
	*/
	while (IsFirstStat())
		Statmt(token);
	codeGen("j endif" + to_string(ifCtr));	// Jump to the end of the if statement. This is necessary to skip else part.

	match(24);			// match token number for '}'
	activeBlock.pop();
	codeGen("else" + to_string(ifCtr) + ": ");// Generate the else label
	/*
	Check for the optional else statement
	*/
	if (token.TokenNumber == 5) {
		match(5);		// Match for else statement
		match(21);		// match token number for '{'
		currentBlock++;
		activeBlock.push(currentBlock);

		Statmt(token);	// Call statement with the current token
	/*
	Check for the FIRST(statement) which can be an IDTOK, IFTOK, READTOK,WRITETOK
	LOOPTOK,RETTOK,INTTOK,DOUBLETOK or BOOLTOK.
	Loop for multiple statements
	*/
		while (IsFirstStat())
			Statmt(token);

		match(24);		// match token number for '}'
		activeBlock.pop();
	}
	codeGen("endif" + to_string(ifCtr) + ": ");// Generate the endif label
}
/*
Check for valid variable declarations. The rules are
<VARDECL>	::=	<TYPE> IDTOK [ '[' LITTTOK ']' ] ';'
*/
void VarDecl(Token currToken) {
	cout << 9 << " ";
	arraySize = -1; idType = 0; pram = ""; // Intializing all the temporary vaiables. These are used as parameters to insert in the symbol table.
	type = currToken.TokenNumber;
	switch (currToken.TokenNumber) {
	case 1:		// Check if the variable is of integer type
		match(1);
		name = token.TokenName;
		match(16);// match token number for a valid identifier
		if (token.TokenNumber == 23) {// match token number for '['. Incase it's an array
			idType = 1;
			match(23);
			arraySize = stoi(token.TokenName);
			match(17);// match token number for a valid digit(Size of array)
			match(26);// match token number for ']'
		}
		match(20);		// Must end in a ';'
		break;
	case 2:		// Check if the variable is of bool type
		match(2);
		name = token.TokenName;
		match(16);// match token number for a valid identifier
		if (token.TokenNumber == 23) {// match token number for '['. Incase it's an array
			idType = 1;
			match(23);
			arraySize = stoi(token.TokenName);
			match(17);// match token number for a valid digit(Size of array)
			match(26);// match token number for ']'
		}
		match(20);		// Must end in a ';'
		break;
	case 3:	// Check if the variable is of double type
		match(3);
		name = token.TokenName;
		match(16);// match token number for a valid identifier
		if (token.TokenNumber == 23) {// match token number for '['. Incase it's an array
			idType = 1;
			match(23);
			arraySize = stoi(token.TokenName);
			match(17);// match token number for a valid digit(Size of array)
			match(26);// match token number for ']'
		}
		match(20);		// Must end in a ';'
		break;
	}
	/*
	Check if the variable is aready declared. If not then store all the details in the symbol table and
	then decrement the offset by 4. If the identifier is an array then subtract the offset by 4*size of array
	*/
	if (hashTable.findInCurrent(name, activeBlock.currentBlock(), "", idType) == nullptr) {
		hashTable.insert(name, activeBlock.currentBlock(), type, offset, pram, arraySize, idType);
		if (idType == 0)
			offset = offset - 4;
		else if (idType == 1)
			offset = offset - arraySize * 4;
	}
	else {
		errorMsg += "Warning: Variable  " + name + " already declared in the current scope" + '\n';
		codeIsGood = false;
	}

}
/*
Check for valid read statements. The rules are
<READSTAT>	::= SCANTOK '(' <IDNONTERM> ')' ';'
*/
void ReadStat(Token currToken) {
	cout << 14 << " ";
	ExpessionRec ex1;
	match(12);	// Must start with the keyword scan
	match(22);	// match token number for '('
	codeGen("li $v0,5");	// The number 5 is loaded to v0. 5 is used to read for integers.
	codeGen("syscall");
	codeGen("sw $v0," + to_string(offset) + "($sp)");// Store the value entered by the user in the stack pointer
	ex1.loc = offset;	// Get the location in a temp expression record.
	ex1.type = 1;		// Get the type in a temp expression record.
	offset -= 4;
	Idnonterm(token);// Call Idnonterm with the current token
	if (ex1.type = expRec.type) {
		codeGen("lw $t0," + to_string(ex1.loc) + "($sp)");	// Get the value entered by the user from the stack pointer
		codeGen("sw $t0," + to_string(expRec.loc) + "($sp)");// Assign it to the identifier
	}
	match(25);	// match token number for ')'
	match(20);	//Must end with a ';'
}
/*
Check for valid write statements. The rules are
<WRITESTAT>	::= PRINTTOK '(' (STRINGTOK|<EXPRESS>) ')' ';'
*/
void WriteStat(Token currToken) {
	cout << 15 << " ";
	match(11);	// Must start with the keyword print
	match(22);	// match token number for '('
	/*
	Check if the current token is a string and convert it to equivalent assembly
	*/
	if (token.TokenNumber == 29) {
		string label = genStrLabel("write");	// Generate a unique label
		codeGen("la $a0," + label);	// load the address to register a0
		codeGen("li $v0,4");
		codeGen("syscall");	// Print the string
		stringOutput += label + ":" + " .asciiz \"\\n" + token.TokenName + "\\n\"" + '\n';// Generate the label with the string
		match(29);
	}
	/*
	Check if the current token is an expression and convert it to equivalent assembly
	*/
	else {
		Express(token);	// Else check for valid expression
		codeGen("lw $a0," + to_string(expRec.loc) + "($sp)");	// To print an expre
		codeGen("li $v0,1");
		codeGen("syscall");
	}
	match(25);// match token number for ')'
	match(20);//Must end with a ';'
}
/*
Check for valid while loops. The rules are
<LOOPSTAT>	::=	WHILETOK '(' <EXPRESS> ')' '{'  {<STATMT>}'}'
*/
void LoopStat(Token currToken) {
	cout << 16 << " ";
	int loopCtr = loopCount;	// Variable to keep track of the no. of loops. Necessary if there are nested loops.
	loopCount++;
	match(6);	//Must start with the keyword while
	match(22);	// match token number for '('
	codeGen("while" + to_string(loopCtr) + ": ");
	Express(token);//Must have a valid condition
	codeGen("lw $t2," + to_string(expRec.loc) + "($sp)");// Get the value of t2. It is false if its value is 0.
	codeGen("beq $t2,0,exitLoop" + to_string(loopCtr));//Exit the loop if t2 is false 
	match(25);// match token number for ')'
	match(21);// match token number for '{'
	currentBlock++;
	activeBlock.push(currentBlock);
	/*
	The loop can have zero or more statements
	Check for the FIRST(statement) which can be an IDTOK, IFTOK, READTOK,WRITETOK
	LOOPTOK,RETTOK,INTTOK,DOUBLETOK or BOOLTOK.
	Loop for multiple statements
	*/
	while (IsFirstStat())
		Statmt(token);
	codeGen("j while" + to_string(loopCtr));// Go back to the beginning of the while loop
	codeGen("exitLoop" + to_string(loopCtr) + ": ");// Label for exiting the loop
	match(24);// match token number for '}'
	activeBlock.pop();
}
/*
Check for a valid return statement. The rules are
<RETSTAT>	::= RETTOK <IDNONTERM>
*/
void RetStat(Token currToken) {
	cout << 25 << " ";
	match(10);	// must start with the keyword return
	Express(token);// Call Idnonterm with the current token
	if (expRec.type != funType) { //check if the type of the expession returned matches with the function. 
		errorMsg += "Warning: Return type of the function don't match: " + to_string(funType) + " to " + to_string(expRec.type) + '\n';
		codeIsGood = false;
	}
	codeGen("lw $t0," + to_string(expRec.loc) + "($sp)");//Get the returned value of the expression to register $t0
	codeGen("lw $t1,0($sp)");	// Get the dynamic link (Old top of stack)
	codeGen("lw $t2,-4($sp)");	// Get the return address to be jumped back

	codeGen("move $sp,$t1");	// Set the stack pointer to the old top of stack
	codeGen("move $fp,$t1");	// Set the frame pointer to the old top of stack
	codeGen("sw $t0," + to_string(retAddr.currentBlock()) + "($sp)");// Store the result of the expression returned in the appropriate location
	retAddr.pop();	// Pop it to get the next return address. Necessary for nested function calls.

	codeGen("jr $t2");// Jump back to the retuen address which was stored at register $t2
	match(20);	// Must end with a ';'
}
/*
Check for valid Expressions. The rules are
<EXPRESS>	::= <TERM> {ADDOPTOK <TERM> }
*/
void Express(Token currToken) {
	cout << 17 << " ";
	ExpessionRec exp1;	// Temporary variable to store the expression record
	string op;			// Variable to get the operator
	Term(token);// Call term with the current token
	exp1 = expRec;
	while (token.TokenNumber == 27) { // Can have zero or more expressions starting with a '+' or '-'
		switch (token.TokenName[0]) {
		case '+': op = "add"; break;	// Check the operator for add
		case '-': op = "sub"; break;	// Check the operator for subtract
		case '|': op = "or"; break;		// Check the operator for or
		}
		match(27);// match token number for '+' or '-'
		Term(token);//Call term with the current token
		if (exp1.type != expRec.type) {
			errorMsg += "Warning: Types don't match: " + to_string(exp1.type) + " to " + to_string(expRec.type) + '\n';
			codeIsGood = false;
		}
		/*
		If both the types match then load the value from the stack pointer to registers t0 and t1.
		Then perform the operation on both registers and store the output in register t0.
		*/
		if (exp1.type == 1 && expRec.type == 1) {
			codeGen("lw $t0," + to_string(exp1.loc) + "($sp)");
			codeGen("lw $t1," + to_string(expRec.loc) + "($sp)");
			codeGen(op + " $t0,$t0,$t1");
			codeGen("sw $t0" + to_string(offset) + "($sp)");
			expRec.type = 1;
			expRec.loc = offset;
			offset -= 4;
		}
		exp1 = expRec; // Save the current expression record.
	}
}
/*
Function for non terminal Term. The rules are
<TERM>		::=	<RELFACT> { MULOPTOK <RELFACT> }
*/
void Term(Token currToken) {
	cout << 18 << " ";
	ExpessionRec exp1;	// Temporary variable to store the expression record
	string op;			// variable to store the operator
	RelFact(token);// Call RelFact with the current token
	exp1 = expRec;
	while (token.TokenNumber == 28) {// Can have zero or more expressions starting with a '*','/' or '%'
		switch (token.TokenName[0]) {
		case '*': op = "mult"; break;	// Check for multiple operator
		case '|': op = "div"; break;	// Check for division operator
		case '%': op = "rem"; break;	// Check for remainder operator
		case '&': op = "and"; break;	// Check for and operator
		}
		match(28);// match token number for '*','/' or '%'
		RelFact(token);//Call RelFact with the current token
		if (exp1.type != expRec.type) {// Check if the type of
.		both expression matches
			errorMsg += "Warning: Types don't match: " + to_string(exp1.type) + " to " + to_string(expRec.type) + '\n';
			codeIsGood = false;
		}
		/*
		Load the operator in register t0 and t1 from the stack pointer.
		Then perfprm the desired operation both registers and store them back in the stack pointer at a new location.
		*/
		if (exp1.type == 1 && expRec.type == 1) {
			if (op == "mult") {
				codeGen("lw $t0," + to_string(exp1.loc) + "($sp)");
				codeGen("lw $t1," + to_string(expRec.loc) + "($sp)");
				codeGen(op + " $t0,$t1");// Multiply t0 and t1
				codeGen("mfhi $t0");	// Get the high order bits to register t0
				codeGen("mflo $t0");	// Get the lower order bits to register t0
				codeGen("sw $t0" + to_string(offset) + "($sp)");
			}
			else if (op == "div") {
				codeGen("lw $t0," + to_string(exp1.loc) + "($sp)");
				codeGen("lw $t1," + to_string(expRec.loc) + "($sp)");
				codeGen(op + " $t0,$t1");// Divide the two numbers
				codeGen("mflo $t0");	// Get the quotient to register t0
				codeGen("sw $t0" + to_string(offset) + "($sp)");
			}
			else if (op == "rem") {
				codeGen("lw $t0," + to_string(exp1.loc) + "($sp)");
				codeGen("lw $t1," + to_string(expRec.loc) + "($sp)");
				codeGen("div $t0,$t1");	// Divide the two numbers
				codeGen("mfhi $t0");	// Get the remainder to register t0
				codeGen("sw $t0" + to_string(offset) + "($sp)");
			}
			else if (op == "and") {
				codeGen("lw $t0," + to_string(exp1.loc) + "($sp)");
				codeGen("lw $t1," + to_string(expRec.loc) + "($sp)");
				codeGen("and $t0,$t0,$t1");// Perform the bit-wise and operation
				codeGen("sw $t0" + to_string(offset) + "($sp)");
			}
			expRec.type = 1;
			expRec.loc = offset;
			offset -= 4;
		}
		exp1 = expRec;	// Save the current expression record
	}
}
/*
Function for non terminal RelFact. The rules are
<RELFACT>	::=	<FACTOR>[RELOPTOK<FACTOR>]
*/
void RelFact(Token currToken) {
	cout << 19 << " ";
	ExpessionRec exp1;	//Temporary variable to store the expression record
	string relop;		// Variable to store the relational operator
	Factor(token);//Call Factor with the current token
	exp1 = expRec;
	if (token.TokenNumber == 19) {// Check for relational operators
		relop = token.TokenName;
		setVal++;
		match(19);
		Factor(token);// Call factor with the current token
		if (exp1.type != expRec.type) {	// Check if both the expressions are of the same type
			errorMsg += "Warning: Types don't match: " + to_string(exp1.type) + " to " + to_string(expRec.type) + '\n';
			codeIsGood = false;
		}
		if (expRec.type == 1 && exp1.type == 1) {
			/*
			Load the value to temp registers t0 and t1.
			*/
			codeGen("lw $t1," + to_string(expRec.loc) + ("($sp)"));
			codeGen("lw $t0," + to_string(exp1.loc) + ("($sp)"));
			if (relop == "<")
				codeGen("slt $t2,$t0,$t1");// If t0 is less than t1 set t2 as 1 else 0.
			else if (relop == ">=") {
				codeGen("slt $t2,$t0,$t1");// If t0 is less than t1 set t2 as 1 else 0.
				codeGen("xori $t2,$t2,1");	// Reverse the value stored at t2.
			}
			else if (relop == "==") {
				codeGen("beq $t0,$t1,setVal" + to_string(setVal));// If t0 and t1 are equal then branch.
				codeGen("li $t2,0");	// If t0 and t1 are not equal. Set t2 as 0. 
				codeGen("j next" + to_string(setVal));// Jump to the next statement
				codeGen("setVal" + to_string(setVal) + ": ");
				codeGen("li $t2,1");	// If t0 and t1 are equal. Set t2 as 1.
				codeGen("next" + to_string(setVal) + ": ");
			}
			else if (relop == "<>") {
				codeGen("bne $t0,$t1,setVal" + to_string(setVal));// If t0 and t1 are not equal then branch.
				codeGen("li $t2,0");// If t0 and t1 are equal. Set t2 as 0. 
				codeGen("j next" + to_string(setVal));// Jump to the next statement.
				codeGen("setVal" + to_string(setVal) + ": ");
				codeGen("li $t2,1");// If t0 and t1 are not equal. Set t2 as 1.
				codeGen("next" + to_string(setVal) + ": ");
			}
			else if (relop == "<=") {
				codeGen("ble $t0,$t1,setVal" + to_string(setVal));// If t0 <= t1  branch.
				codeGen("li $t2,0");// If t0 > t1 are equal. Set t2 as 0. 
				codeGen("j next" + to_string(setVal));// Jump to the next statement.
				codeGen("setVal" + to_string(setVal) + ": ");
				codeGen("li $t2,1");// If t0 <=t1 are not equal. Set t2 as 1.
				codeGen("next" + to_string(setVal) + ": ");
			}
			else if (relop == ">") {// If t0 > t1  branch.
				codeGen("bgt $t0,$t1,setVal" + to_string(setVal));
				codeGen("li $t2,0");// If t0 <= t1 are equal. Set t2 as 0. 
				codeGen("j next" + to_string(setVal));// Jump to the next statement.
				codeGen("setVal" + to_string(setVal) + ": ");
				codeGen("li $t2,1");// If t0 <=t1 are not equal. Set t2 as 1.
				codeGen("next" + to_string(setVal) + ": ");
			}
			codeGen("sw $t2," + to_string(offset) + "($sp)"); // t2=1 indicates true and t2=0 indicates false
			expRec.loc = offset;
			expRec.type = 1;
			offset -= 4;
		}
		exp1 = expRec;
	}
}
/*
Function for non terminal Factor. The rules are
<FACTOR>	::=	<IDNONTERM> | LITTOK | '{' <EXPRESS> '}' | BOOLTOK
*/
void Factor(Token currToken) {
	switch (currToken.TokenNumber) {
	case 16:	// If it starts with an identifier
		cout << 20 << " ";
		Idnonterm(token);// Call Idnonterm with the current token
		break;
	case 17:	// If it starts with an integer
		cout << 21 << " ";
		expRec.loc = offset;
		offset -= 4;
		expRec.type = 1;
		// Using the Stack pointer assign a memory to the Literal2
		codeGen("li $t0," + token.TokenName);
		codeGen("sw $t0," + to_string(expRec.loc) + "($sp)");
		match(17);// Just match it to go to next token
		break;
	case 30:	// If it starts with a double value
		cout << 21 << " ";
		expRec.loc = offset;
		offset -= 4;
		expRec.type = 3;
		// Using the Stack pointer assign a memory to the Literal
		codeGen("li $t0," + token.TokenName);
		codeGen("sw $t0," + to_string(expRec.loc) + "($sp)");
		match(30);// Just match it to go to the next token
		break;
	case 22:	// If it starts with a '('
		cout << 22 << " ";
		match(22);// Match it to go to the next token
		Express(token);// Call Express with the current token
		match(25);// Match token number for ')'
		break;
	case 7:		// Match token for keyword true
		cout << 23 << " ";
		expRec.loc = offset;
		offset -= 4;
		expRec.type = 2;
		// Using the Stack pointer assign a memory to the Literal
		codeGen("li $t0," + token.TokenName);
		codeGen("sw $t0," + to_string(expRec.loc) + "($sp)");
		match(7);
		break;
	case 8:		// Match token for keyword false
		cout << 23 << " ";
		expRec.loc = offset;
		offset -= 4;
		expRec.type = 2;
		// Using the Stack pointer assign a memory to the Literal
		codeGen("li $t0," + token.TokenName);
		codeGen("sw $t0," + to_string(expRec.loc) + "($sp)");
		match(8);
		break;
	default:	// Throw an error if it's anything else
		cout << endl << "Syntax Error  ";
		exit(0);
	}
}
/*
Function for non terminal Idnonterm. The rules are
<IDNONTERM>	::= IDTOK ['[' LITTOK ']' | '(' { <EXPRESS> { <EXPRESS> } }')']
*/
void Idnonterm(Token currToken) {
	cout << 24 << " ";
	identifier *Ident;// To store the pointer to the location of the identifier in the symbol table
	idType = 0;// id type 0 indicates an identifier that is not an array. To distinguish between array and identifier in a symbol table
	pram = "";
	name = token.TokenName;
	string tempPram, tempName = token.TokenName;// Temporary variable to store the list of parameters and name of a function
	match(16);	// Match for an Identifier token
	if (token.TokenNumber == 23) {// Check if it's followed by '['
		idType = 1;	// id type 1 indicates a array
		match(23);	
		arraySize = stoi(token.TokenName);
		match(17);	// Match for a valid digit for an array
		match(26);	// Match for ']'
	}
	else if (token.TokenNumber == 22) {//Check if the idnetifier if followed by '('
		match(22);
		tempPram = "";	
		/*
		Check for the FIRST(Express). It can start with an identifier, a digit or a '('.
		There can be zero or more expressions.
		*/
		if (token.TokenNumber == 16 || token.TokenNumber == 17 || token.TokenNumber == 30 || token.TokenNumber == 22) {
			regNo = 0;// To indicate which a register is in use. Range is between 0 and 3
			Express(token);// Parameters to the function.
			codeGen("lw $a" + to_string(regNo) + "," + to_string(expRec.loc) + "($sp)");// Load the expression from the stack pointer to register a0
			regNo++;// indicates which a rgister is in use. range is from 0 to 3
			tempPram += to_string(expRec.type);// To generate a unique string based on functions parameters
			/*
			Check for ',' between expressions
			*/
			while (token.TokenNumber == 31) {
				match(31);
				Express(token);// Call Express with the current token
				if (regNo<4)// Maximum value of the register number of a is 3
					codeGen("lw $a" + to_string(regNo) + "," + to_string(expRec.loc) + "($sp)");
				regNo++;
				tempPram += to_string(expRec.type);// To generate a unique string based on functions parameters
			}
		}
		name = tempName;// Move back the name from the temporary variable to store in symbol table
		pram = tempPram;// Move back the unique parameter string  from the temporary variable to store in symbol table
		idType = 2;// id type 2 indicates the name of a function

		match(25); // Match the token number for ')'
		Ident = hashTable.findInCurrent(name, 0, pram, idType);
		if (Ident == nullptr) {
			errorMsg += "Warning: Function " + name + " not declared " + '\n';
			codeIsGood = false;
		}
		/*
		To store the location and type of the value that will be returned by the function
		*/
		else {
			expRec.type = (*Ident).type;
			expRec.loc = offset;
			retAddr.push(offset);// Push the return address to a stack. The function called will return the value to this address after restoring the stack pointer.
			offset -= 4;
		}
		codeGen("jal " + name);// Jump and link to the function's name
	}
	/*
	Check if the variable is declared if not throw a warning.
	*/
	if (idType != 2) {
		Ident = hashTable.findInAll(name, activeBlock, pram, idType);
		if (Ident == nullptr) {
			errorMsg += "Warning: Variable " + name + " not declared " + '\n';
			codeIsGood = false;
		}
		/*
		Get the location and tye of the identifier. In case of array access memory as per
		the array index
		*/
		else {
			expRec.type = (*Ident).type;
			if ((*Ident).id == 0)
				expRec.loc = (*Ident).location;
			else if ((*Ident).id == 1)
				expRec.loc = (*Ident).location - 4 * arraySize;
		}
	}
}
/*
Function to check for a valid function declaration. The rules are
<FUNCDECL>	::=	<TYPE> IDTOK '(' {<PRAMLIST> { ',' <PRAMLIST>} ')' '{' {<STATMT>} '}'
*/
void FunDecl(Token currToken) {
	cout << 10 << " ";
	pram = ""; idType = 2;// Initialize the pram and idType for functions to default.
	identifier *Ident;	// To get the pointer to the location of the identifier in the symbol table 
	type = currToken.TokenNumber;
	funType = type;	// Initialize funType. It is necessary to compare the return type of the function with the returned value.
	match(currToken.TokenNumber);// Just match the current token with itsef to skip the return type because it was already checked earlier.
	funcName = token.TokenName;
	codeGen(funcName + ":");// Label for the funtion's name
	codeGen("addi $sp,$sp," + to_string(offset));//Move the current stack pointer down
	//retAddr.push(offset);
	offset = 0;	// Move the offset to 0
	codeGen("sw $fp," + to_string(offset) + "($sp)");// Store the previous location of stack pointer in the dynamic link
	offset -= 4;
	codeGen("move $fp,$sp");// Move the frame pointer down
	codeGen("sw $ra," + to_string(offset) + "($sp)");// Store the return address in the stack
	offset -= 4;
	match(16);// Match the token number for a valid identifier
	match(22);// Match the token number for '('
	currentBlock++;
	activeBlock.push(currentBlock);
	/*
	Check for the list of parameters that can begin with int, bool or double.
	*/
	if (token.TokenNumber == 1 || token.TokenNumber == 2 || token.TokenNumber == 3) {
		regNo = 0;// To indicate which a register is in use. It's range is from 0 to 3
		PramList(token);// Call parameter list with the current token
		while (token.TokenNumber == 31) {// Check ',' for multiple parameters
			match(31);
			PramList(token);// Call parameter list with the current token
		}
	}
	match(25);// Match token number for ')'
	/*
	Check if a prototype exists for the function
	*/
	Ident = hashTable.findInCurrent(funcName, 0, pram, idType);
	if (Ident == nullptr) {
		errorMsg += "Warning: Function prototype " + funcName + " not defined" + '\n';
		codeIsGood = false;
	}
	
	
	//else
	//	retAddr.push((*Ident).location);
	
	match(21);// Match token number for '{'
	/*
	The function body can have 0 or more statements.
	Check for the FIRST(statement) which can be an IDTOK, IFTOK, READTOK,WRITETOK
	LOOPTOK,RETTOK,INTTOK,DOUBLETOK or BOOLTOK.
	Loop for multiple statements
	*/
	while (IsFirstStat())
		Statmt(token);
	match(24);// Match token number for '}'
	activeBlock.pop();
}
/*
Function to check for a valid function declaration. The rules are
<FUNCPROTO>	::= TYPETOK ID '(' {<PRAMLIST>} ')' ';'
*/
void FunProto(Token currToken) {
	cout << 26 << " ";
	pram = "";// String to generate a unique number based on the type and number of parameters a function takes.
	idType = 2;// id type is 2 for a function
	type = currToken.TokenNumber;
	match(currToken.TokenNumber);// Just match the current token with itsef to skip the return type because it was already checked earlier.
	name = token.TokenName;
	match(16);// Match the token number for a valid identifier
	match(22);// Match the token number for '('
	/*
	Check for the list of parameters that can begin with int, bool or double.
	*/
	while (token.TokenNumber == 1 || token.TokenNumber == 2 || token.TokenNumber == 3) {
		pram += to_string(token.TokenNumber);//Append the type of the identifier to a string that helps in generating a unique string
		match(token.TokenNumber);
		match(16);
		while (token.TokenNumber == 31) {// Check ',' for multiple parameters
			match(31);
			pram += to_string(token.TokenNumber);//Append the type of the identifier to a string that helps in generating a unique string
			if (token.TokenNumber == 1 || token.TokenNumber == 2 || token.TokenNumber == 3)
				match(token.TokenNumber);
			match(16);
		}
	}
	match(25);// Match token number for ')'
	match(20);	// Must end with a ';'
	/*
	Check if the function prototype is already defined.
	*/
	if (hashTable.findInCurrent(name, activeBlock.currentBlock(), pram, 2) == nullptr)
		hashTable.insert(name, activeBlock.currentBlock(), type, offset, pram, -1, 2);
	else {
		errorMsg += "Warning: Function prototype " + name + " already defined" + '\n';
		codeIsGood = false;
	}
	offset -= 4;
}
/*
Check for valid paramters. The rules are
<PRAMLIST>	::=	TYPETOK IDTOK
*/
void PramList(Token currToken) {
	cout << 11 << " ";
	pram += to_string(token.TokenNumber);// Append the type of the identifier to a string, helps in generating a unique string.
	// Must start with int, bool or double
	if (token.TokenNumber == 1 || token.TokenNumber == 2 || token.TokenNumber == 3)
		type = token.TokenNumber;
	expRec.type = type;
	expRec.loc = offset;

	match(currToken.TokenNumber);
	name = token.TokenName;
	match(16);// Must be accompanied by a valid identifier
	/*
	Check if the variable is there in the current scope. If not add it to the symbol table.
	*/
	if (hashTable.findInCurrent(name, activeBlock.currentBlock(), "", 0) == nullptr) {
		hashTable.insert(name, activeBlock.currentBlock(), expRec.type, expRec.loc, "", -1, 0);
		if (regNo < 4) { // Store the value of corresponding $a register to the location of the identifier on the stack.
			codeGen("sw $a" + to_string(regNo) + "," + to_string(expRec.loc) + "($sp)");
			regNo++;// Go to the next a register. It can be between 0 to 3. The rest are ignored
			offset -= 4;
		}
	}
	// If the identifier already exists then generate a warning
	else {
		errorMsg += "Warning: Variable  " + name + " already declared in the current scope" + '\n';
		codeIsGood = false;
	}
}