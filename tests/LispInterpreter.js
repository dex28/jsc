/////////////////////////////////////////////////////////////////////////////
// JavaScript Lisp Interpreter
// by Joe Ganley, http://ganley.org/
// Email: ganley@cadence.com (OR) ganley@hotmail.com
// Copyright (C) 1997 J. L. Ganley.  All rights reserved.
// changed alert to print, also removed document. references - by MBK
/////////////////////////////////////////////////////////////////////////////

// some character classification functions
function isDigit(ch) {
    if (ch == "") return false;
    return ("0123456789".indexOf(ch) >= 0);
}

function isAlpha(ch) {
    if (ch == "") return false;
    return ("ABCDEFGHIJKLMNOPQRSTUVWXYZ".indexOf(ch.toUpperCase()) >= 0);
}

function isAlphaDigit(ch) {
    if (ch == "") return false;
    return (isDigit(ch) || isAlpha(ch));
}

function isWhite(ch) {
    if (ch == "") return false;
    return (" \t\n".indexOf(ch) >= 0);
}


Array.prototype.ix = 0;
Array.prototype.listP = true;
Array.prototype.lambdaP = true;

// symbol table
symTab = new Array();
symTab.ix = 0;
symTab[0] = new Array();

// takes a string and returns an array of tokens
function jslTokenize(str) {
    var i, ret;

    ret = new Array();
    ret.ix = 0;
    i = 0;
    while (i < str.length) {
	if ((str.charAt(i) == "+") || (str.charAt(i) == "-")) {
	    if (isDigit(str.charAt(i + 1))) {
	        ret[ret.length] = str.charAt(i++);
	    	while ((i < str.length) && (isDigit(str.charAt(i)))) {
		    ret[ret.length - 1] += str.charAt(i++);
	        }
	    }
	    else {
		ret[ret.length] = str.charAt(i++);
	    }
	}
	else if (isDigit(str.charAt(i))) {
	    ret[ret.length] = "";
	    while ((i < str.length) && (isDigit(str.charAt(i)))) {
		ret[ret.length - 1] += str.charAt(i++);
	    }
	}
	else if (isAlpha(str.charAt(i))) {
	    ret[ret.length] = "";
	    while ((i < str.length) && (isAlphaDigit(str.charAt(i)))) {
		ret[ret.length - 1] += str.charAt(i++);
	    }
	}
	else if (isWhite(str.charAt(i))) {
	    ++i;
	}
	else {
	    ret[ret.length] = str.charAt(i++);
	}
    }
    return ret;
} // jslTokenize


// look up the given symbol in the symbol table.  Start with the current
// level and work backward as necessary if the symbol is not found.
// (dynamic scoping a la Lisp, rather than Scheme)
function jslLookup(sym) {
    var i;

    try {
	    for (i = symTab.ix; i >= 0; --i)
	    {
	   	    if (symTab[i][sym])
	        {
		        return symTab[i][sym];
		        }
	        }
    	}
    catch( e )
    {
        }

    return "nil";
} // jslLookup


// parse the array of tokens into the standard Lisp list structure
// (implemented as arrays, since they're dynamic in JS)
function jslParse(tok) {
    var i, ret, arg;

    if (tok[tok.ix] == "(") {
        ++tok.ix;
	if (tok[tok.ix] == ")") {
    	    ++tok.ix;
	    ret = new Array(1);
    	    ret[0] = null;
	    ret.listP = true;
    	    ret.lambdaP = false;
	}
	else {
            ret = new Array();
	    ret.listP = true;
    	    ret.lambdaP = false;
	    while ((tok.ix < tok.length) && (tok[tok.ix] != ")")) {
	        ret[ret.length] = jslParse(tok);
	    }
	    if (tok[tok.ix] == ")") {
	    	++tok.ix;
	    }
	    else {
	    	print("Missing \')\'");
	    }
	}
    }
    else if (tok[tok.ix] == "\'") {
	++tok.ix;
	arg = jslParse(tok);
    	if (jslNullP(arg)) {
    	    ret = "nil";
    	}
    	else {
	    ret = new Array(2);
	    ret.listP = true;
    	    ret.lambdaP = false;
	    ret[0] = "quote";
	    ret[1] = arg;
    	}
    }
    else {		// atom
	ret = tok[tok.ix++];
    }

    return ret;
} // jslParse


// is the given list 'nil' in any of its guises?
function jslNullP(tree) {
    return (!tree) || (tree.toString() == "nil")
    	    || ((tree.listP) && ((tree.length == 0) || (tree[0] == null)));
} // jslNullP


// evaluate the given list    
function jslEval(tree) {
    var i, j, ret, arg, arg2, formals, found;

    if (jslNullP(tree)) {
    	ret = "nil";
    }
    else if (tree.listP) {
    	if (tree[0] == "quote") {
    	    ret = tree[1];
    	}
    	else if ((tree[0] != "") && ("+-*/".indexOf(tree[0]) >= 0)) {
	    ret = parseInt(jslEval(tree[1]));
    	    for (i = 2; i < tree.length; ++i) {
		if (tree[0] == "+") { ret += parseInt(jslEval(tree[i])); }
		else if (tree[0] == "-") { ret -= parseInt(jslEval(tree[i])); }
		else if (tree[0] == "*") { ret *= parseInt(jslEval(tree[i])); }
		else if (tree[0] == "/") {
		    ret = Math.floor(ret / parseInt(jslEval(tree[i])));
		}
	    }
	}
	else if ((tree[0] == "eq") || (tree[0] == "=")) {
    	    ret = "t";
    	    arg = jslEval(tree[1]);
    	    for (i = 2; (i < tree.length) && (ret == "t"); ++i) {
    	    	arg2 = jslEval(tree[i]);
    	    	if (arg != arg2) {
    	    	    ret = "nil";
    	    	}
    	    }
	}
	else if ((tree[0] == "<") || (tree[0] == ">")) {
    	    arg2 = parseInt(jslEval(tree[1]));
    	    ret = "t";
    	    for (i = 2; (i < tree.length) && (ret == "t"); ++i) {
    	    	arg = arg2;
    	    	arg2 = parseInt(jslEval(tree[i]));
    	    	if (((tree[0] == "<") && (arg >= arg2))
    	    	    || ((tree[0] == ">") && (arg <= arg2))) {
    	    	    ret = "nil";
    	    	}
    	    }
	}
	else if (tree[0] == "and") {
	    ret = "t";
    	    for (i = 1; (i < tree.length) && (ret == "t"); ++i) {
    	    	arg = jslEval(tree[i]);
    	    	if (jslNullP(arg)) {
    	    	    ret = "nil";
    	    	}
    	    }
	}
	else if (tree[0] == "or") {
	    ret = "nil";
    	    for (i = 1; (i < tree.length) && (ret == "nil"); ++i) {
    	    	arg = jslEval(tree[i]);
    	    	if (!jslNullP(arg)) {
    	    	    ret = "t";
    	    	}
    	    }
    	}
	else if (tree[0] == "cond") {
	    ret = "nil";
    	    found = false;
    	    for (i = 1; (i < tree.length) && (!found); ++i) {
    	    	if (tree[i].listP) {
    	    	    arg = jslEval(tree[i][0]);
    	    	    if (!jslNullP(arg)) {
    	    	    	if (tree[i].length > 1) {
    	    	    	    ret = jslEval(tree[i][1]);
    	    	    	}
    	    	    	else {
    	    	    	    ret = arg;
    	    	    	}
    	    	    	found = true;
    	    	    }
    	    	}
    	    	else {
    	    	    print( "Syntax error in cond" );
    	    	}
    	    }
    	}
	else if (tree[0] == "car") {
	    arg = jslEval(tree[1]);
	    if (arg.listP) {
		if (arg.length == 0) {
		    ret = "nil";
		}
    	    	else {
		    ret = arg[0];
    	    	}
	    }
    	    else {
    	    	ret = "nil";
    	    }
	}
	else if (tree[0] == "cdr") {
	    arg = jslEval(tree[1]);
	    if (arg.listP) {
		if (arg.length <= 1) {
		    ret = "nil";
		}
    	    	else {
		    ret = new Array(arg.length - 1);
		    ret.listP = true;
    	    	    ret.lambdaP = false;
		    for (i = 1; i < arg.length; ++i) {
		    	ret[i - 1] = arg[i];
    	    	    }
		}
	    }
    	    else {
    	    	ret = "nil";
    	    }
	}
	else if (tree[0] == "cons") {
	    arg = jslEval(tree[1]);
    	    if (jslNullP(arg)) { arg = "nil"; }
	    arg2 = jslEval(tree[2]);
    	    if (jslNullP(arg2)) {
    	    	arg2 = new Array();
    	    	arg2.listP = true;
    	    	arg2.lambdaP = false;
    	    }
	    if (arg2.listP) {
		ret = new Array(arg2.length + 1);
		ret.listP = true;
    	    	ret.lambdaP = false;
		for (i = 0; i < arg2.length; ++i) {
		    ret[i + 1] = arg2[i];
		}
		ret[0] = arg;
	    }
    	    else {
	    	ret = "nil";
    	    }
	}
	else if (tree[0] == "atom") {
    	    arg = jslEval(tree[1]);
	    if ((arg.listP) && (!jslNullP(arg))) { ret = "nil"; }
    	    else    	    	             	 { ret = "t"; }
	}
	else if (tree[0] == "list") {
	    ret = new Array();
	    ret.listP = true;
    	    ret.lambdaP = false;
    	    for (i = 1; i < tree.length; ++i) {
    	    	ret[ret.length] = jslEval(tree[i]);
    	    }
	}
	else if (tree[0] == "set") {
	    arg = jslEval(tree[1]);
	    if ((arg.toString() == "nil") || (!isAlpha(arg.charAt(0)))) {
		print("Invalid first operand to set: " + arg);
		ret = "nil";
	    }
    	    else {
	    	arg2 = jslEval(tree[2]);

	    	symTab[symTab.ix][arg] = arg2;
    	    	ret = arg2;
    	    }
	}
	else if (tree[0] == "eval") {
	    ret = jslEval(jslEval(tree[1]));
	}
	else if (tree[0] == "lambda") {
	    ret = new Array();
	    ret.listP = true;
	    ret.lambdaP = true;
    	    for (i = 1; i < tree.length; i++) {
		ret[i - 1] = tree[i];
	    }
	}

    	else {
	    // if we get here, then it's either a function call or an error
	    arg = jslEval(tree[0]);
	    if ((arg) && (arg.listP) && (arg.lambdaP)) {
		formals = new Array(arg[0].length);
	    	for (i = 0; i < arg[0].length; ++i) {
		    formals[i] = jslEval(tree[i + 1]);
		}
		symTab[++symTab.ix] = new Array();
		for (i = 0; i < arg[0].length; ++i) {
		    symTab[symTab.ix][arg[0][i]] = formals[i];
	    	}
	    	for (i = 1; i < arg.length; ++i) {
		    ret = jslEval(arg[i]);
	    	}
		--symTab.ix;
	    }
	    else {
	    	print("Invalid function call: " + jslPrint(tree[0]));
	    	ret = "nil";
	    }
    	}
    }
    else {  	    	    	    // it's an atom
    	if (isAlpha(tree.charAt(0))) {
	    if (tree.toString() == "t") {
	    	ret = "t";
	    }
    	    else if (jslNullP(tree)) {
    	    	ret = "nil";
    	    }
    	    else {
    	    	ret = jslLookup(tree);
    	    }
	}
    	else {
    	    ret = tree;
    	}
    }

    return ret;
} // jslEval


// translate the list expression back into printable form
function jslPrint(sexpr) {
    var i, ret;

    if (sexpr) {
    	if (sexpr.listP) {
	    if (sexpr.length == 0) {
	    	ret = "nil";
	    }
    	    if (sexpr.lambdaP) { ret = "(lambda "; }
    	    else    	       { ret = "("; }
	    ret += jslPrint(sexpr[0]);
	    for (i = 1; i < sexpr.length; ++i) {
	    	ret += " " + jslPrint(sexpr[i]);
	    }
	    ret += ")";
    	}
    	else {
    	    ret = sexpr.toString();
    	}
    }
    else {
    	ret = "";
    }

    return ret;
} // jslPrint


// the main interpreter driver
function jslInterp(str) {
    return jslPrint(jslEval(jslParse(jslTokenize(str))));
} // jslInterp


function jslComposites() {
    jslInterp("(set 'len (lambda (s) (cond ((null s) 0) (t (+ 1 (len (cdr s)))))))");
    jslInterp("(set 'flatten (lambda (s) (cond ((null s) nil) ((atom (car s)) (cons (car s) (flatten (cdr s)))) (t (append (flatten (car s)) (flatten (cdr s)))))))");

    // Portions copyright (c) 1988, 1990 Roger Rohrbach
    jslInterp("(set 'cadr (lambda (e) (car (cdr e))))");
    jslInterp("(set 'cddr (lambda (e) (cdr (cdr e))))");
    jslInterp("(set 'caar (lambda (e) (car (car e))))");
    jslInterp("(set 'cdar (lambda (e) (cdr (car e))))");
    jslInterp("(set 'cadar (lambda (e) (car (cdr (car e)))))");
    jslInterp("(set 'caddr (lambda (e) (car (cdr (cdr e)))))");
    jslInterp("(set 'cddar (lambda (e) (cdr (cdr (car e)))))");
    jslInterp("(set 'cdadr (lambda (e) (cdr (car (cdr e)))))");
    jslInterp("(set 'null (lambda (e) (eq e nil)))");
    jslInterp("(set 'not (lambda (e) (eq e nil)))");
    jslInterp("(set 'equal (lambda (x y) (or (and (atom x) (atom y) (eq x y)) (and (not (atom x)) (not (atom y)) (equal (car x) (car y)) (equal (cdr x) (cdr y))))))");
    jslInterp("(set 'append (lambda (x y) (cond ((null x) y) (t (cons (car x) (append (cdr x) y))))))");
    jslInterp("(set 'member (lambda (x y) (and (not (null y)) (or (equal x (car y)) (member x (cdr y))))))");
    jslInterp("(set 'last (lambda (e) (cond ((atom e) nil) ((null (cdr e)) (car e)) (t (last (cdr e))))))");
    jslInterp("(set 'reverse (lambda (x) (qreverse x nil)))");
    jslInterp("(set 'qreverse (lambda (x y) (cond ((null x) y) (t (qreverse (cdr x) (cons (car x) y))))))");
    jslInterp("(set 'remove (lambda (e l) (cond ((null l) nil) ((equal e (car l)) (remove e (cdr l))) (t (cons (car l) (remove e (cdr l)))))))");
    jslInterp("(set 'mapcar (lambda (f l) (cond ((null l) nil) (t (cons (eval (list f (list 'quote (car l)))) (mapcar f (cdr l)))))))");
    jslInterp("(set 'apply (lambda (f args) (cond ((null args) nil) (t (eval (cons f (mapcar '(lambda (a) (list 'quote a)) args)))))))");
} // jslComposites


print( "Running" );
print( jslInterp( "(cdr '( 1  2 3 4 5 6 ) )" ) );
