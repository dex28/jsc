
test_name = "!?";

function panic( msg )
{
    System.stderr.writeln( test_name + ":" + msg + " -- failed!" );
    System.stderr.flush ();
    //System.exit( 1 );
    }

///////////////////////////////////////////////////////////////////////////////
// Test Boolean

function Foo ()
{
    }

function TestBoolean ()
{
    test_name = "Boolean";

    var b;
    var f = new Foo ();

    b = new Boolean( f.cant_be_found );

    if ( b )
        test_fail( "new Boolean( undefined )" );

    b = new Boolean( null );

    if ( b )
        test_fail( "new Boolean( null )" );

    b = new Boolean( false );

    if ( b )
        test_fail( "new Boolean( false )" );

    b = new Boolean( true );

    if ( ! b )
        test_fail( "new Boolean( true )" );

    b = new Boolean( "" );

    if ( b )
        test_fail( "new Boolean( \"\" )" );

    b = new Boolean( "foo" );

    if ( ! b )
        test_fail( "new Boolean( \"foo\" )" );

    b = new Boolean( 0 );

    if ( b )
        test_fail( "new Boolean( 0 )" );

    b = new Boolean( 1 );

    if ( ! b )
        test_fail( "new Boolean( 1 )" );

    if ( true.toString () != "true" )
        test_fail( "true.toString ()" );

    if ( false.toString () != "false" )
        test_fail( "false.toString ()" );

    // 15.6
    b = Boolean( 2 );
    if ( !( b && typeof( b )=="boolean" ) )
        panic( "boolean.1" );

    if ( !( ! Boolean () ) )
        panic( "boolean.2" );

    if ( !( ! Boolean( 0 ) ) )
        panic( "boolean.3" );

    ob = new Boolean( "true" );
    if ( !( ob && typeof( ob ) == "boolean" ) )
        panic( "boolean.4" );
/*
    if ( !( Boolean.prototype.constructor == Boolean ) )
        panic( "boolean.5" );
*/
    ob = new Boolean();
    if ( !( ob.toString () == "false" ) )
        panic( "boolean.6" );

    ob = new Boolean( true );
    v = ob.valueOf ();
    if ( !( v && typeof( v ) == "boolean" ) )
        panic( "boolean.7" );

    if ( !( Boolean.prototype ) )
        panic( "boolean.8" );
    }

///////////////////////////////////////////////////////////////////////////////
// Test VM opcodes

// Check items <a> and <b> for equality.  They should be equal, so if
// == returns false or != returns true, that's an error.
///
function cmp_eq ( a, b )
{
    if ( ! ( a == b ) )
        panic( "`cmp_eq " + typeof a + " " + typeof b + "' returns false" );

    if ( a != b )
        panic( "`cmp_ne " + typeof a + " " + typeof b + "' returns true" );
    }

// Check items <a> and <b> for strict equality.
//
function cmp_seq ( a, b )
{
    if ( ! ( a === b ) )
        panic( "`cmp_seq " + typeof a + " " + typeof b + "' returns false" );

    if ( a !== b )
        panic( "`cmp_sne " + typeof a + " " + typeof b + "' returns true" );
    }

function TestOpcodes ()
{
    test_name = "Opcodes";

    // cmp_{eq,ne}
    cmp_eq ( null, null );
    cmp_eq ( null, new Object ().foo );
    cmp_eq ( new Object ().foo, new Object ().bar );
    cmp_eq ( 1, 1 );
    cmp_eq ( 1.0, 1 );
    cmp_eq ( 1, 1.0 );
    cmp_eq ( 0, -0 );
    cmp_eq ( -0, 0 );
    cmp_eq ( -0, -0 );
    cmp_eq ( "foo", "foo" );
    cmp_eq ( true, true );
    cmp_eq ( false, false );

    var a = new Object ();
    cmp_eq ( a, a );
    cmp_eq ( Object, Object );
    cmp_eq ( TestOpcodes, TestOpcodes );

    cmp_eq ( 1, "1" );
    cmp_eq ( "1", 1 );
    cmp_eq ( 1, "1.0" );
    cmp_eq ( "1.0", 1.0 );

    cmp_eq ( true, 1 );
    cmp_eq ( false, 0 );

    // TODO: Object , string/number

    // cmp_{seq,sne}
    cmp_seq ( 1, 1 );
    cmp_seq ( 1.0, 1 );
    cmp_seq ( 1, 1.0 );
    cmp_seq ( "foobar", "foobar" );
    cmp_seq ( true, true );
    cmp_seq ( false, false );
    cmp_seq ( a, a );
    cmp_seq ( Object, Object );
    cmp_seq ( TestOpcodes, TestOpcodes );

    // div
    if ( ! isNaN( NaN / 1 ) || ! isNaN( 1 / NaN ) || ! isNaN( NaN / NaN ) )
        panic( "NaN in `div' didn't gave NaN" );

    if ( ! isNaN( Infinity / Infinity ) )
        panic( "`div Infinity Infinity' didn't gave NaN" );

    if ( isFinite( Infinity / 0 ) )
        panic( "`div Infinity 0' didn't gave Infinity" );

    if ( 42 / Infinity != 0 )
        panic( "`div nonzero Infinity1 didn't gave 0" );

    if ( ! isNaN( 0 / 0 ) )
        panic( "`div 0 0' didn't gave NaN" );

    if ( isFinite ( 42 / 0 ) )
        panic( "`div nonzero 0' didn't gave Infinity" );

    // mod 
    if ( ! isNaN( NaN % 0 ) || ! isNaN( NaN % NaN ) || ! isNaN( NaN % Infinity ) )
        panic( "NaN in `mod' didn't gave NaN" );

    if ( ! isNaN( Infinity % 7 ) || ! isNaN( -Infinity % 2 ) )
        panic( "Infinity in dividend of `mod' didn't gave NaN" );

    if ( ! isNaN( 42 % 0 ) )
        panic( "0 in divisor of `mod' didn't gave NaN" );

    if ( 42 % Infinity != 42 || 42.2 % Infinity != 42.2 )
        panic( "finite % Infinity didn't gave finite" );

    if ( 0 % 42.0 != 0 || 0.0 % 42 != 0.0 )
        panic( "finite % Infinity didn't gave finite" );

    if ( 42 % 7 != 0 )
        panic( "positive % positive didn't work" );

    if ( 41 % 7 != 6 )
        panic( "positive % positive didn't work" );

    if ( -41 % 7 != -6 )
        panic( "negative % positive didn't work" );

    if ( -4.2 % 7 != -4.2 )
        panic( "negative % positive didn't work" );
    }

///////////////////////////////////////////////////////////////////////////////
// Test Statements

function Foo ( value )
{
    this.value = value;
    this.hello = Foo$hello;
    }

function Foo$hello ()
{
    return "Hello, world!";
    }

function testScope ()
{
    s1.sb = 22;
    s1.sc = 33;
    s2.sc=333;

    with ( s1 ) 
    {
        with (s2) 
        {
            return sa==1 && sb == 22 && sc == 333;
            }
        }
    }

function r1( x )
{
    1; return x + 2; 5;
    }

function r2( x )
{   
    1; 
    for ( var i = 0; i < 5; i++ ) 
    {
        if ( i == 3 ) return x;
        x ++;
        }
    return "aa";
    }

function r3( x )
{
    1; 
    for ( var i = 0; i < x; i ++)
    {
        aa += 2;
        if ( i == 3 ) return;
        }

    a = "wrong";
    return;
    }

function sin( x )
{
    return "a";
    }

function TestStatements ()
{
    test_name = "Statements";

    // 12.1

    { a = 1; { 3; b = 4; }; c = b; }

    if ( !( c == 4 ) )
        panic( "block.1" );

    // Just test proper parsing of empty blocks
    {}

    // 12.2
    v2 = 4;
    v1 = 3;
    var v1 = 5, v2 = 3, v4;
    if ( !( typeof v4 == "undefined" ) )
        panic( "var.1" );

    if ( !( v2 == 3 ) )
        panic( "var.2" );

    if ( !( v1 == 5 ) )
        panic( "var.3" );

    // 12.3
    ;;;true;

    // 12.4
    34 + 23; 12 * 4 / ( 3 + 4 ); true;

    // 12.5
    if ( 2 == 2 ) true;

    if ( 2 == 3 ) false; else true;

    if ( 0 ) ; true;

    if ( 23 + 4 - 27 ) { 1; false;} { false; true; }

    if ( "" ) false; else true;

    if ( "false" ) true; else false;

    a = false;
    if ( ! a ) a = true;

    if ( ! a )
        panic( "if.1" );

    // 12.6
    // 12.6.2
    a = 1;
    while ( a < 5) a++;

    if ( !( a == 5 ) )
        panic( "while.1" );

    a = 1;
    while ( a < 1 ) a++;

    if ( !( a == 1 ) )
        panic( "while.2" );

    a = 1;
    while ( a < 5 ) { a++; if ( a == 3 ) break; }

    if ( !( a == 3 ) )
        panic( "while.3" );

    a = 1;
    b = true;
    while ( b ) { a++; if ( a==5 ) b = false; }

    if ( !( a == 5 ) )
        panic( "while.4" );

    ir = -1;
    var i = 0;
    while ( i < 4 )
    {
        ir = i;
        if ( i > 10 ) break;
        i++;
        continue;
        ir = -1;
        }

    if ( !( ir == 3 ) )
        panic( "while.5" );

    // 12.6.2
    for ( a = 0; a < 5; a++ ) ;
    if ( !( a == 5 ) )
        panic( "for.1" );

    a = 0;
    for (; a<6; a++ ) ;

    if ( !( a == 6 ) )
        panic( "for.2" );

    for ( a = 0 ;; a++ ) if ( a == 5 ) break;

    if ( !( a == 5 ) )
        panic( "for.3" );

    a = 0;
    for ( a = 0; a < 5; ) a++;

    if ( !( a == 5 ) )
        panic( "for.4" );

    for ( a = 5; a > 0; a-- ) ;

    if ( !( a == 0 ) )
        panic( "for.5" );

    a = 3;
    for ( var a = 0; a < 5; a++ ) ;

    if ( !( a == 3 ) )
        panic( "for.6" );

    a = 3;
    for ( var a = 0 ;; a++ ) if ( a == 5 ) break;

    if ( !( a == 3 ) )
        panic( "for.7" );

    a = 0;
    for ( var a = 0; a < 5; ) a++;

    if ( !( a == 0 ) )
        panic( "for.8" );

    for ( var a = 5; a > 0; a -- ) ;

    if ( !( a == 0 ) )
        panic( "for.9" );

    b = 5;
    for ( var a = 0; a < 5; b++ ) a++;
    if ( !( b == 10 ) )
        panic( "for.10" );

    var ir = 0;
    for ( var i = 0; i < 4; i++ )
    {
	    ir = i; 
	    if ( i > 10 ) break;
	    continue ;
	    ir = -1;
        }
    if ( !( ir == 3 ) )
        panic( "for.11" );

    // 11.6.3
    o = new Object();
    o.a = 1;
    o.b = 2;
    o.c = 4;
    a = 0;
    for ( i in o ) { a += o[ i ]; }

    if ( !( a == 7 ) )
        panic( "forin.1" );

/*
    a = new Array();
    a[2] = 3;
    a[4] = 5;
    v = 0;
    for ( i in a )  v += a[ i ];
    if ( !( v == 8 ) )
        panic( "forin.2" );
*/

    o = new Object();
    o.a = 1;
    o.b = 2;
    o.c = 4;
    a = 0;
    for ( var i in o ) { a += o[ i ]; }

    if ( !( a == 7 ) )
        panic( "forin.3" );

/*
    a = new Array();
    a[ 2 ] = 3;
    a[ 4 ] = 5;
    v = 0;
    i = 5;
    for ( var i in a)  v += a[ i ];

    if ( !( v == 8 ) )
        panic( "forin.4" );
*/

    // 12.9
    if ( !( r1( 3 ) == 5 ) )
        panic( "return.1" );

    if ( !( r2( 3 ) == 6 ) )
        panic( "return.2" );

    aa = 0;
    r3( 5 );
    if ( !( aa == 8 ) )
        panic( "return.3" );

    // 12.10;
    with ( Math )
    {
        a = sin(2);
        }

    o = new Object();
    o.sin = sin;

    with (o)
    {
        if ( !( sin( 1 ) == "a" ) )
            panic( "with.1" );
        }

    with ( Math ) with ( o ) { a = sin( 1 ); }

    if ( !( a == "a" ) )
        panic( "with.3" );

    with ( o ) with ( Math ) { a = sin( 1 ); }

    if ( !( 0.8 < a && a < 0.9 ) )
        panic( "with.4" );

    with (o) { gl=123; }

    if ( ! ( gl == 123  ) )
        panic( "with.5" );

    var reference, val;

    reference = Math.PI;
    with ( System )
        with ( Math )
            val = PI;

    if ( val != reference )
        panic( "with.6" );

    with ( System )
        with ( Math )
            with ( File )
                val = byteToString ( 32 );

    if ( val != " " )
        panic( "with.7" );

    var o = new Foo ( 42 );
    reference = o.value;
    with ( o )
        val = value;

    if ( val != reference )
        panic( "with.8" );

    with ( o )
        val = hello ();

    if ( val != "Hello, world!" )
        panic( "with.9" );

/*
    // The scoping (with) implementation has a peculiarity which makes
    // the following tests worthwhile
    panic( "scope.1
    sa = 1;
    sb = 2;
    sc = 3;
    }

    s1 = new Number(99);
    s2 = new Number(98)
    testScope();

    panic( "scope.2" );
    s1 = new String("abc");
    s2 = new String("def")
    testScope();

    panic( "scope.3" );
    s1 = new Date();
    s2 = new Date()
    testScope();

    panic( "scope.4" );
    s1 = new Array(1,2,3);
    s2 = new Array(4,5,6)
    testScope();

    panic( "scope.5" );
    s1 = new Boolean(true);
    s2 = new Boolean(false)
    testScope();

    panic( "scope.6" );
    s1 = new Function("return 1");
    s2 = new Function("return 2")
    testScope();

    panic( "scope.7" );
    s1 = new Object();
    s2 = new Object()
    testScope();

    panic( "scope.8" );
    function getArg(a) {
      return arguments;
    }

    s1 = new getArg(1);
    s2 = new getArg(2);
    testScope();
*/
    }

///////////////////////////////////////////////////////////////////////////////
// Test Number

function TestNumber ()
{
    test_name = "Number";

    b = Number( 2 );

    if ( !( b == 2.0 && typeof( b ) == "number" ) )
        panic( "number.1" );

    if ( !( Number() == 0 ) )
        panic( "number.2" );

    no = new Number( "12.5" );

    if ( !( no == 12.5 && typeof( no ) == "number" ) )
        panic( "number.3" );

    no = new Number();

    if ( !( no == 0 && typeof( no ) == "number" ) )
        panic( "number.4" );

    n = Number.MAX_VALUE;

    if ( !( 1.79769e308 < n && n < Infinity ) )
        panic( "number.5" );

    n = Number.MIN_VALUE;

    if ( !( 0 < n && n < 1e-307 ) )
        panic( "number.6" );

    if ( !( isNaN( Number.NaN ) ) )
        panic( "number.7" );

    if ( !( Number.POSITIVE_INFINITY == Infinity ) )
        panic( "number.8" );

    if ( !( Number.NEGATIVE_INFINITY == -Infinity ) )
        panic( "number.9" );
/*
    if ( !( Number.prototype == 0 ) )
        panic( "number.10" );

    if ( !( Number.prototype.constructor == Number ) )
        panic( "number.11" );
*/
    n = new Number( 15 );
    if ( !( n.toString() == "15" ) )
        panic( "number.12" );

    if ( !( n.toString( 16 ).toLowerCase() == "f" ) )
        panic( "number.13" );

    vn = n.valueOf();
    if ( !( vn == 15 && typeof( vn ) == "number" ) )
        panic( "number.14" );
    }

///////////////////////////////////////////////////////////////////////////////
// Test Math

function near( x,y )
{
    return Math.abs( x - y ) < 0.001;
    }

function TestMath ()
{
    test_name = "Math";

    // Test initial psudo random sequnce:
    // 1623, 3442, 1447, 1829, 1305, ...
    //

    if ( 1623 != int( 4096 * Math.random () ) )
        panic( "Math.random (): 1623 expected" );

    if ( 3442 != int( 4096 * Math.random () ) )
        panic( "Math.random (): 3442 expected" );

    if ( 1447 != int( 4096 * Math.random () ) )
        panic( "Math.random (): 1447 expected" );

    if ( 1829 != int( 4096 * Math.random () ) )
        panic( "Math.random (): 1829 expected" );

    if ( 1305 != int( 4096 * Math.random () ) )
        panic( "Math.random (): 1305 expected" );

    // 15.8
    // Just check that it is a likely value (that there was no cut-and-paste
    // error).

    if ( ! ( Math.E > 2.71 && Math.E < 2.72 ) )
        panic( "Math.1" );

    if ( !( Math.LN10 > 2.3 && Math.LN10 < 2.31 ) )
        panic( "Math.2" );

    if ( !( Math.LN2 > 0.69 && Math.LN2 < 0.70 ) )
        panic( "Math.3" );

    if ( !( Math.LOG2E > 1.44 && Math.LOG2E < 1.45 ) )
        panic( "Math.4" );

    if ( !( Math.LOG10E > 0.43 && Math.LOG10E < 0.44 ) )
        panic( "Math.5" );

    if ( !( Math.PI > 3.14 && Math.PI < 3.15 ) )
        panic( "Math.6" );

    if ( !( Math.SQRT1_2 > 0.70 && Math.SQRT1_2 < 0.71 ) )
        panic( "Math.6" );

    if ( !( Math.SQRT2 > 1.41 && Math.SQRT2 < 1.42 ) )
        panic( "Math.7" );

    // For all functions just test some border cases (they are normally
    // handled by the underlying Java engine), and a value to ensure that
    // there was no error in the cut and paste work of creating these
    // functions.  The values where tested on a TI-36X calculator.
    // The number 0.3 was selected to be defined but give different results for
    // all the tested functions. pow(0.3,0.4) is different from pow(0.4,0.3),
    // to test argument inversion

    if ( !( near( 12.1231456, 12.1231457 ) && ! near( 12.1,12.2 ) ) )
        panic( "near.1" );

    if ( !( isNaN( Math.abs( NaN ) ) ) )
        panic( "abs.1" );

    if ( !( 2 == Math.abs( -2 ) ) )
        panic( "abs.2" );

    if ( !( 2==Math.abs(2) ) )
        panic( "abs.3" );

    if ( !( isNaN( Math.acos( NaN ) ) ) )
        panic( "acos.1" );

    if ( !( isNaN( Math.acos( 3 ) ) ) )
        panic( "acos.2" );

    if ( !( isNaN( Math.acos( -3 ) ) ) )
        panic( "acos.3" );

    if ( !( 0 == Math.acos( 1 ) ) )
        panic( "acos.4" );

    if ( !( near( Math.acos( 0.3 ), 1.266103 ) ) )
        panic( "acos.5" );

    if ( !( isNaN( Math.asin( NaN ) ) ) )
        panic( "asin.1" );

    if ( !( isNaN( Math.asin( 3 ) ) ) )
        panic( "asin.2" );

    if ( !( isNaN( Math.asin(-3 ) ) ) )
        panic( "asin.3" );

    if ( !( 0==Math.asin( 0 ) ) )
        panic( "asin.4" );

    if ( !( near( Math.asin(.3 ),0.304692 ) ) )
        panic( "asin.4" );

    if ( !( 0==Math.atan( 0 ) ) )
        panic( "atan.1" );

    if ( !( near( Math.atan( 0.3 ),0.291456 ) ) )
        panic( "atan.2" );

    if ( !( 0==Math.atan2( 0,0 ) ) )
        panic( "atan2.1" );

    if ( !( isNaN( Math.ceil( NaN ) ) ) )
        panic( "ceil.1" );

    if ( !( Math.ceil( 0 )==0 ) )
        panic( "ceil.2" );

    if ( !( Math.ceil( Infinity )==Infinity ) )
        panic( "ceil.3" );

    if ( !( Math.ceil(-Infinity )==-Infinity ) )
        panic( "ceil.4" );

    if ( !( Math.ceil(-2.5 )==-2 ) )
        panic( "ceil.5" );

    if ( !( Math.ceil( 12.345 )==13 ) )
        panic( "ceil.6" );

    if ( !( Math.ceil( 12.3434 )-Math.floor(-12.3434 ) ) )
        panic( "ceil.7" );

    if ( !( 1==Math.cos( 0 ) ) )
        panic( "cos.1" );

    if ( !( near( Math.cos(.3 ),0.9553 ) ) )
        panic( "cos.2" );

    if ( !( 1==Math.exp( 0 ) ) )
        panic( "exp.1" );

    if ( !( near( Math.exp( 0.3 ),1.34985 ) ) )
        panic( "exp.2" );

    if ( !( isNaN( Math.floor( NaN ) ) ) )
        panic( "floor.1" );

    if ( !( Math.floor( 0 )==0 ) )
        panic( "floor.2" );

    if ( !( Math.floor( Infinity )==Infinity ) )
        panic( "floor.3" );

    if ( !( Math.floor(-Infinity )==-Infinity ) )
        panic( "floor.4" );

    if ( !( Math.floor(-2.5 )==-3 ) )
        panic( "floor.5" );

    if ( !( Math.floor( 12.345 ) == 12 ) )
        panic( "floor.6" );

    if ( !( 0==Math.log( 1 ) ) )
        panic( "log.1" );

    if ( !( near( Math.log( 0.3 ),-1.2039 ) ) )
        panic( "log.2" );

    if ( !( isNaN( Math.max( 3,NaN ) ) ) )
        panic( "max.1" );

    if ( !( 4 == Math.max( 2,4 ) ) )
        panic( "max.2" );

    if ( !( isNaN( Math.min( 3,NaN ) ) ) )
        panic( "min.1" );

    if ( !( 2 == Math.min( 2,4 ) ) )
        panic( "min.2" );

    if ( !( Math.pow( 2,3 ) == 8 ) )
        panic( "pow.1" );

    if ( !( 0==Math.pow(-0.5,Infinity ) ) )
        panic( "pow.2" );

    if ( !( near( Math.pow( 0.3,0.4 ),0.61780 ) ) )
        panic( "pow.3" );

    r = Math.random ();
    if ( !( r >= 0 && r < 1 ) )
        panic( "random" );

    if ( !( Math.round( 3.5 )==4 ) )
        panic( "round.1" );

    if ( !( Math.round(-3.5 )==-3 ) )
        panic( "round.2" );

    if ( !( 0==Math.sin( 0 ) ) )
        panic( "sin.1" );

    if ( !( near( Math.sin(.3 ),0.29552 ) ) )
        panic( "sin.2" );

    if ( !( Math.sqrt( 1 )==1 ) )
        panic( "sqrt.1" );

    if ( !( near( Math.sqrt( 0.3 ),0.54772 ) ) )
        panic( "sqrt.2" );

    if ( !( Math.tan( 0 ) == 0 ) )
        panic( "tan.1" );

    if ( !( near( Math.tan( 0.3 ),0.30933 ) ) )
        panic( "tan.2" );
    }

///////////////////////////////////////////////////////////////////////////////
// Test Array

function by_number( a, b )
{
    return a - b;
    }

function TestArray ()
{
    test_name = "Array";

    var a, b;

    // Constructors and the length property.

    a = new Array( 5 );

    if ( a.length != 5 )
        panic( "new Array( LENGTH )" );

    a = new Array( 1, 2, 3, 4, 5 );

    if ( a.length != 5 )
        panic( "new Array( ITEM...)" );

    // Methods

    a = new Array( 1, 2, 3 );
    b = a.concat( new Array( 4, 5 ) );

    if ( b.length != 5 )
        panic( "concat ()" );

    if ( b.join () != "1,2,3,4,5" )
        panic( "concat ()" );

    a = new Array( 1, 2, 3 );

    if ( a.join () != "1,2,3" )
        panic( "join ()" );

    if ( a.join( "*" ) != "1*2*3" )
        panic( "join( GLUE )" );

    a = new Array( 1, 2, 3 );

    if ( a.pop () != 3 )
        panic( "pop ()" );

    if ( a.pop () != 2 )
        panic( "pop ()" );

    if ( a.pop () != 1 )
        panic( "pop ()" );

    if ( typeof a.pop () != "undefined" )
        panic( "pop ()" );

    a = new Array( 1, 2 );

    if ( a.push ( 7 ) != 7 )
        panic( "push( ITEM )" );

    if ( a.push ( 7, 8, 9 ) != 9 )
        panic( "push( ITEM...)" );

    a = new Array( 1, 2, 3 );
    a.reverse ();

    if ( a.join ( "" ) != "321" )
        panic( "reverse ()" );

    a = new Array( 1, 2, 3 );

    if ( a.shift () != 1 )
        panic( "shift ()" );

    if ( a.shift () != 2 )
        panic( "shift ()" );

    if ( a.shift () != 3 )
        panic( "shift ()" );

    if ( typeof a.shift () != "undefined" )
        panic( "shift ()" );

    a = new Array( 1, 2, 3, 4, 5 );
    b = a.slice( 1, 4 );

    if ( b.join( "" ) != "234" )
        panic( "slice( START, END )" );

    b = a.slice( 1, -2 );
    if ( b.join( "" ) != "23" )
        panic( "slice( START, -END )" );

    b = a.slice( 2 );
    if ( b.join( "" ) != "345" )
        panic( "slice( START )" );

    a = new Array( 1, 2, 3 );
    b = a.splice( 1, 1 );

    if ( a.join( "" ) != "13" )
        panic( "splice( POS, DEL )" );

    if ( b.join ( "" ) != "2" )
        panic( "splice( POS, DEL )" );

    a = new Array( 1, 2, 3 );
    b = a.splice( 1, 0, "new item" );

    if ( a.join ( "" ) != "1new item23" )
        panic( "splice( POS, 0, ITEM )" );

    a = new Array( 1, 2, 3, 4 );
    b = a.splice( 1, 2, "new item" );

    if ( a.join ( "" ) != "1new item4" )
        panic( "splice( POS, DEL, ITEM" );

    if ( b.join ( "" ) != "23" )
        panic( "splice( POS, DEL )" );

    a = new Array ();
    {
        var i;

        for ( i = 0; i < 50; i++ )
        {
            a.push( int( Math.random () * 100 ) );
            }

        a.sort( by_number );

        var last = 0;
        for ( i = 0; i < a.length; i++ )
        {
            if ( a[ i ] < last )
                panic( "sort( by_number )" );
            last = a[ i ];
            }

        a.sort ();
        last = "";
        for ( i = 0; i < a.length; i++ )
        {
            if ( a[ i ].toString () < last )
                panic( "sort ()" );

            last = a[ i ].toString ();
            }
        }

    a = new Array( 1, 2, 3 );

    if ( a.toString () != "1,2,3" )
        panic( "toString ()" );

    a = new Array( 1, 2, 3 );

    if ( a.unshift( 7, 8, 9 ) != 6 )
        panic( "unshift ()" );
    }

///////////////////////////////////////////////////////////////////////////////
// Test String

function TestString ()
{
    test_name = "String";

    var s;

    // Constructor and the length property.

    s = new String ( "foo" );

    if ( s.length != 3 )
        panic( "new String( STRING )" );

    if ( s != "foo" )
        panic( "new String( STRING )" );

    // Methods.

    s.append ( "bar" );

    if ( s != "foobar" )
        panic( "append ()" );

    if ( s.charAt ( 3 ) != "b" )
        panic( "charAt ()" );

    if ( "foobar".charAt ( 3 ) != "b" )
        panic( "charAt ()" );

    if ( s.charCodeAt ( 5 ) != #'r')
        panic( "charCodeAt ()" );

    if ( "foobar".charCodeAt ( 5 ) != #'r')
        panic( "charCodeAt ()" );

    if ( s.concat ( "FOO" ) != "foobarFOO" )
        panic( "concat ()" );

    if ( "foobar".concat ( "FOO" ) != "foobarFOO" )
        panic( "concat ()" );

    if ( String.fromCharCode (#'f', #'o', #'o', #'b', #'a', #'r') != "foobar" )
        panic( "fromCharCode ()" );

    s = "foobar foo bar foo";

    if ( s.indexOf ( "foo" ) != 0 )
        panic( "indexOf ()" );

    if ( s.indexOf ( " foo" ) != 6 )
        panic( "indexOf ()" );

    if ( s.indexOf ( "foo", 1 ) != 7 )
        panic( "indexOf ()" );

    if ( s.indexOf ( "Foo" ) != -1 )
        panic( "indexOf ()" );

    s = "foobar foo bar foo";

    if ( s.lastIndexOf ( "foo" ) != 15 )
        panic( "lastIndexOf( 1 )" );

    if ( s.lastIndexOf ( "bar" ) != 11 )
        panic( "lastIndexOf( 2 )" );

    if ( s.lastIndexOf ( "foo", 14 ) != 7 )
        panic( "lastIndexOf( 3 )" );

    if ( s.lastIndexOf ( "Foo" ) != -1 )
        panic( "lastIndexOf( 4 )" );

    // TODO: match () 
    // TODO: pack () 
    // TODO: replace () 
    // TODO: search () 

    s = "Hello, world!";

    if ( s.slice( 7 ) != "world!" )
        panic( "slice( START )" );

    if ( s.slice( -5 ) != "orld!" )
        panic( "slice( -START )" );

    if ( s.slice( -500 ) != s )
        panic( "slice( -START )" );

    if ( s.slice( 500 ) != "" )
        panic( "slice( START )" );

    if ( s.slice( 7, 9 ) != "wo" )
        panic( "slice( START, END )" );

    if ( s.slice( 7, -2 ) != "worl" )
        panic( "slice( START, -END )" );

    if ( s.slice( 7, -20 ) != "" )
        panic( "slice( START, -END )" );

    if ( s.slice( 7, 200 ) != "world!" )
        panic( "slice( START, END )" );

    if ( s.slice( 700, 200 ) != "" )
        panic( "slice( START, END )" );

    // TODO: split ()

    s = "Hello, world!";

    if ( s.substr ( 7 ) != "world!" )
        panic( "substr( START )" );

    if ( s.substr( 7, 4 ) != "worl" )
        panic( "substr( START, LEN )" );

    if ( s.substr( 7, 400 ) != "world!" )
        panic( "substr( START, LEN )" );

    if ( s.substr( -6 ) != "world!" )
        panic( "substr( -START )" );

    if ( s.substr( -6, 4 ) != "worl" )
        panic( "substr( -START, LEN )" );

    if ( s.substr( -600, 5 ) != "Hello" )
        panic( "substr( -START, LEN )" );

    // TODO: substring ()

    s = "FoObAr";

    if ( s.toLowerCase () != "foobar" )
        panic( "toLowerCase ()" );

    if ( s.toUpperCase () != "FOOBAR" )
        panic( "toUpperCase ()" );

    // TODO: unpack ()
    }

///////////////////////////////////////////////////////////////////////////////
// Test Core Global Object Methods

function TestCoreGM ()
{
    test_name = "Core global object methods";

    var v;

    // TODO: callC ()
    // TODO: debug ()

    v = float( true );

    if ( v != 1.0 )
        panic( "float( true )" );

    v = float( false );

    if ( v != 0.0 )
        panic( "float( false )" );

    v = float( "3.14" );

    if ( v != 3.14 )
        panic( "float(\"3.14\" )" );

    v = float( 3.14 );

    if ( v != 3.14 )
        panic( "float( 3.14" );

    v = float( new Array( 1, 2, 3, 4 ) );

    if ( v != 4.0 )
        panic( "float( new Array(...) )" );

    // TODO: int ()
    // TODO: isNaN ()
    // TODO: isFloat ()
    // TODO: isInt ()
    // TODO: parseFloat ()
    // TODO: registerCFunction ()

    // 15.1
    if ( !( "NaN" == new String( NaN ) ) )
        panic( "global.1" );

    if ( !( "Infinity" == new String( Infinity ) ) )
        panic( "global.2" );

    n2 = parseInt( "" );
    if ( !( isNaN( n2 ) ) )
        panic( "parseInt.1" );

    n1 = parseInt( "albert" );
    if ( !( isNaN( n1 ) ) )
        panic( "parseInt.2" );

    if ( !( 12 == parseInt( "12" ) ) )
        panic( "parseInt.3" );

    if ( !( 12 == parseInt( "12.5" ) ) )
        panic( "parseInt.4" );

    if ( !( 12 == parseInt("12.5albert") ) )
        panic( "parseInt.5" );

    if ( !( 0xF0 == parseInt("0xF0") ) )
        panic( "parseInt.6" );

    if ( !( 0x12 == parseInt("12",16) ) )
        panic( "parseInt.7" );

    if ( !( 0xF0 == parseInt("0XF0",16) ) )
        panic( "parseInt.8" );

    f1 = parseFloat("");
    if ( !( isNaN( f1 ) ) )
        panic( "parseFloat.1" );

    f2 = parseFloat("albert");
    if ( !( isNaN(f2) ) )
        panic( "parseFloat.2" );

    if ( !( 12 == parseFloat("12") ) )
        panic( "parseFloat.3" );

    if ( !( 12.5 == parseFloat("12.5") ) )
        panic( "parseFloat.4" );

    if ( !( 12.5 == parseFloat("12.5albert") ) )
        panic( "parseFloat.5" );

    if ( !( 12.5 == parseFloat("1.25E1") ) )
        panic( "parseFloat.6" );

    if ( !( 125 == parseFloat("1.25e2albert") ) )
        panic( "parseFloat.7" );

    if ( !( "abc" == escape("abc") ) )
        panic( "escape.1" );

    if ( !( "a%3cb" == escape("a<b") ) )
        panic( "escape.2" );

    if ( !( "abc" == unescape("abc") ) )
        panic( "unescape.1" );

    if ( !( "a<b" == unescape("a%3cb") ) )
        panic( "unescape.2" );

    if ( !( isFinite( 12 ) ) )
        panic( "isfinite.1" );

    if ( !( !isFinite( Infinity ) ) )
        panic( "isfinite.2" );
    }

///////////////////////////////////////////////////////////////////////////////
// Test Miscellaneous

function l1( p1 )
{
    l2( 21, 22 );
    }

function l2( p2 )
{
    l3( 31, 32, 33 );
    }

function l3( p3 )
{
    l4( 41, 42, 43, 44 );
    }

function l4( p4 )
{
    VM.verboseStackTrace = false;
    VM.stackTrace ();
    }

function stack_overflow_without_locals( p1 )
{
    stack_overflow_without_locals( p1 );
    }

function stack_overflow_with_locals( p1 )
{
    var a = 1, b = 2, c = 3, d = 4, e = 5, f = 6, g = 7, h = 8,
        i = 9, j = 10, k = 11, l = 12, m = 13, n = 14, o = 15, p = 16,
        q = 17, r = 18, s = 19;

    stack_overflow_with_locals( p1 );
    }

function TestMisc ()
{
    test_name = "misc";

    //
    // Stack overflow detection in the `jsr' operand. If this test
    // fails, we will crash with a core dump.
    //
    try
    {
        stack_overflow_without_locals ();
        }
    catch ( e )
    {
        //System.stderr.writeln ( e );
        //VM.verboseStackTrace = true;
	    //VM.stackTrace ();
        }

    // Stack overflow detection in the `locals' operand
    //
    try
    {
        stack_overflow_with_locals ();
        }
    catch ( e )
    {
        //System.stderr.writeln ( e );
        //VM.verboseStackTrace = true;
	    //VM.stackTrace ();
        }

    l1( 11 );
    }

///////////////////////////////////////////////////////////////////////////////
// Test Object

//
// Create some classes.  This example is the one that can be found from
// the document "Object Hierarchy and Inheritance in JavaScript":
// http://developer.netscape.com/docs/manuals/communicator/jsobj/jsobj.pdf
//

function Employee ( name, dept )
{
    this.name = name || "";
    this.dept = dept || "general";
    }

function Manager ()
{
    this.reports = new Array ();
    }

Manager.prototype = new Employee;

function WorkerBee ( name, dept, projs )
{
    this.base = Employee;
    this.base ( name, dept );
    this.projects = projs || new Array ();
    }

WorkerBee.prototype = new Employee;

function SalesPerson ()
{
    this.dept = "sales";
    this.quota = 100;
    }

SalesPerson.prototype = new WorkerBee;

function Engineer ( name, projs, mach )
{
    this.base = WorkerBee;
    this.base ( name, "engineering", projs );
    this.machine = mach || "";
    }

Engineer.prototype = new WorkerBee;

function instanceOf ( object, constructor )
{
    while ( object != null )
    {
        if ( object == constructor.prototype )
            return true;

        object = object.__proto__;
        }

    return false;
    }

function TestObject ()
{
    test_name = "object";

    // The jane example.
    //

    var jane = new Engineer ( "Doe, Jane", new Array( "navigator", "javascript" ),
                              "belau" );

    if ( jane.machine != "belau" )
        panic( "jane.machine" );

    if ( jane.projects.toString () != "navigator,javascript" )
        panic( "jane.projects" );

    if ( jane.name != "Doe, Jane" )
        panic( "jane.name" );

    if ( jane.dept != "engineering" )
        panic( "jane.depth" );

    // The instanceOf () example.
    //

    if ( ! instanceOf ( jane, Engineer ) )
        panic( "instanceOf( jane, Engineer )" );

    if ( ! instanceOf ( jane, WorkerBee ) )
        panic( "instanceOf( jane, WorkerBee )" );

    if ( ! instanceOf ( jane, Employee ) )
        panic( "instanceOf( jane, Employee )" );

    if ( ! instanceOf ( jane, Object ) )
        panic( "instanceOf( jane, Object )" );

    // The explicit __proto__ chain.
    //

    if ( jane.__proto__ != Engineer.prototype )
        panic( "jane.__proto__ != Engineer.prototype" );

    if ( jane.__proto__.__proto__ != WorkerBee.prototype )
        panic( "jane.__proto__.__proto__ != WorkerBee.prototype" );

    if ( jane.__proto__.__proto__.__proto__ != Employee.prototype )
        panic( "jane.__proto__.__proto__.__proto__ != Employee.prototype" );

    if ( jane.__proto__.__proto__.__proto__.__proto__ != Object.prototype )
        panic( "jane.__proto__.__proto__.__proto__.__proto__ != Object.prototype" );

    if ( jane.__proto__.__proto__.__proto__.__proto__.__proto__ != null )
        panic( "jane.__proto__.__proto__.__proto__.__proto__.__proto__ != null" );
    }


///////////////////////////////////////////////////////////////////////////////
// Test MD5

function MDString( str )
{
    md5.init ();
    md5.update( str );
    return md5.final ();
    }

function TestMD5 ()
{
    test_name = "MD5";

    //
    // MD5 test drive assimilated from RFC
    //

    md5 = new MD5; // reusable md5 object

    if ( MDString( "" )
        != "D41D8CD98F00B204E9800998ECF8427E" )
        panic( "MD5.1" );

    if ( MDString( "a" )
        != "0CC175B9C0F1B6A831C399E269772661" )
        panic( "MD5.2" );

    if ( MDString( "abc" )
        != "900150983CD24FB0D6963F7D28E17F72" )
        panic( "MD5.3" );

    if ( MDString( "message digest" )
        != "F96B697D7CB7938D525A2F31AAF161D0" )
        panic( "MD5.4" );

    if ( MDString( "abcdefghijklmnopqrstuvwxyz" )
        != "C3FCD3D76192E4007DFB496CCA67E13B" )
        panic( "MD5.5" );

    if ( MDString( "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789" )
        != "D174AB98D277D9F5A5611C2C9F419D9F" )
        panic( "MD5.6" );

    if ( MDString( "12345678901234567890123456789012345678901234567890123456789012345678901234567890" )
        != "57EDF4A22BE3C955AC49DA2E2107B67A" )
        panic( "MD5.7" );
    }

///////////////////////////////////////////////////////////////////////////////
// Test Suite

System.stdout.writeln( "------------------- TestSuite started." );

TestBoolean ();
TestOpcodes ();
TestStatements ();
TestNumber ();
TestMath ();
TestArray ();
TestString ();
TestCoreGM ();
TestObject ();
TestMisc ();
TestMD5 ();

System.stdout.writeln( "------------------- TestSuite done." );

