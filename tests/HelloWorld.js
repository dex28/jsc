
// test VM
//

System.print( VM.versionMajor, ".", VM.versionMinor, " (", VM.versionPatch, ") \n" );

function recursive2( n )
{
    if ( n > 4 )
        VM.stackTrace ();
    else
        recursive2( n + 1 );
    }

function recursive( n )
{
    if ( n > 2 )
        recursive2( n )
    else
        recursive( n + 1 );
    }

recursive( 0 );

// test function redefinition,
// undefined arguments and local variable scope
//

x = -256 >>> 5;

function fooMy( a )
{
    print( "fooMy1", a );
    }

function fooMy( a, b )
{
    var x = 2;
    print( "fooMy(", x, ")", a, b );
    }

fooMy( 1 );
fooMy( "1", "2" );
fooMy( "1", 2, "3" );
print( "x =", x );

// eval global object method
//

try
{
    eval( "print( \"evaluated Hello, World!\" )" );
    }
catch( e )
{
    print( "Detektovana greska:", e );
    }

// user defined extensions
//

loadClass( "e:\\mbk\\tisab\\CSUser.dll:InitModule" );

print( "add( 3.4, 5, 2.1, 3.3, 6.7 ) =", add( 3.4, 5, 2.1, 3.3, 6.7 ) );

Hello ();

Hello.show ();

try { Hello.fail (); }
catch( msg ) { print( msg ); }

try { Hello.msg = "foo"; }
catch( msg ) { print( msg ); }

var h1 = new Hello( "Hello, h1!" );
h1.show ();

Hello.investigate( h1 );

var h2 = h1.copy ();
h2.show ();

Hello.investigate( h2 );

print( Hello.msg );

print( Hello( "pear" ) );
print( Hello.mikicone );

Math.seed ();

var S = 0;
for( var i = 1 ; i <= 10000 ; i++ )
{
    var T = -1 * Math.log( 1 - Math.random () );
    S += T;
    }
print( S / 10000 );

{
    var x = new ODBC;
    x.connect( "tplis", "admin", "" );

    x.setAutoCommit( false );

    x.commit ();

    var y = new ODBC( x );
    y.domain_name = "admin";

    y.disconnect ();

    x.connect( "tplis", "admin", "" );
    x.setAutoCommit( false );

    z = y.execSql( "select retired from domains where domain_name = ?", "pera" );
    print( z.retired, z.pera );
    z.retired = null;
    print( z.retired, z.pera );
    }

System.sleep( 20 );

