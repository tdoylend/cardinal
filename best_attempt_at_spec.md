- a PROGRAM consists of a series of STATEMENTs.

- a STATEMENT is one of the following succeeded by a NEWLINE:

	(1) an EXPRESSION;

	(2) the keyword "break";

	(3) the keyword "continue";

	(4) the keyword "for" followed by a FOR LOOP;

	(5) the keyword "while" followed by a WHILE LOOP;

	(6) the keyword "if" followed by an IF STATEMENT;

	(7) the keyword "import" followed by an IMPORT CLAUSE;

	(8) the keyword "return" followed by an optional EXPRESSION;

	(9) the keyword "var" followed by an identifier, optionally followed by an "=" and an
		EXPRESSION (both, or neither).

 - a FOR LOOP is "(", an identifier, "in", an EXPRESSION, ")", and a CONTROL BLOCK BODY.

 - a WHILE LOOP is "(", an EXPRESSION, ")", and a CONTROL BLOCK BODY.

 - a CONTROL BLOCK BODY is one of the following:

	- a single STATEMENT;

	- "{", an optional NEWLINE, zero or more STATEMENTs, and "}".

 - an EXPRESSION is one of the following:

	(1) a number;

	(2) a STRING;

	(3) "true";

	(4) "false"; 

	(5) "null";

	(6) a LIST;

	(7) a VARIABLE;

	(8) a MAP.

	(9) a PREFIX OPERATOR followed by an EXPRESSION of tighter or equal precedence;

	(10) an EXPRESSION followed by a POSTFIX OPERATOR of looser or equal precedence;

	(11) an EXPRESSION, the "?" symbol, another EXPRESSION, the ":" symbol, and a third EXPRESSION;

	(12) an EXPRESSION followed by a METHOD CALL;

	(13) two EXPRESSIONs surrounding an INFIX OPERATOR of looser or equal precedence;

	(14) a CLOSURE.

 - a CLOSURE is: "{", an optional CLOSURE PARAMETER BLOCK, one of the following:

		(1) an EXPRESSION;

		(2) a NEWLINE followed by zero or more STATEMENTs;

        (3) nothing.

	...followed by "}".

 - a METHOD CALL is a "." followed by one of:

	(1) an identifier.

	(2) an identifier followed by "(", followed by zero or more EXPRESSIONs separated by ",",
		followed by a closing ")".
	
	(3) an identifier followed by "=", followed by an EXPRESSION.

	(4) an identifier followed by "[", followed by one or more EXPRESSIONs separated by ",",
		followed by a closing "]".
	
	(5) an identifier followed by "[", followed by one or more EXPRESSIONs separated by ",",
		followed by a closing "]", followed by "=", followed by an EXPRESSION;
	
	(6) an identifier followed by a CLOSURE.


var 
