/*
"Dangling else" shift-reduce conflict
from http://www.quut.com/c/ANSI-C-grammar-y-2011.html

selection_statement
	: IF '(' expression ')' statement ELSE statement
	| IF '(' expression ')' statement
*/
void dangling_else() {
	if(1) 
		if (1)
			return;
		else
			return;
}
/*
ATOMIC shift-reduce conflict
from http://www.quut.com/c/ANSI-C-grammar-y-2011.html

atomic_type_specifier
	: ATOMIC '(' type_name ')'
	
type_qualifier
	| ATOMIC
*/
_Atomic(int) a;	
_Atomic int b;
