
function isValidPNum( pnum )
{
    if ( pnum.length != 10 && pnum.length != 12 )
    {
        print( pnum, "- invalid length" );
        return false;
        }

    var s = 0;

    for ( var i = pnum.length == 10 ? 0 : 2; i < pnum.length; i++ )
    {
        var d = parseInt( pnum.charAt( i ), 10 ) * ( i % 2 == 0 ? 2 : 1 );
        s += ( d >= 10 ? 1 + d - 10 : d );
        }

    var OK = ( s % 10 ) == 0;

    if ( ! OK )
        print( pnum, "- invalid number; residuum =", s % 10 );

    return OK;
    }

{
    var x = new SQL;
    x.connect( "TA", "sa", "sans" );

    x.getAutoCommit ();
    x.setAutoCommit( false );
    x.getAutoCommit ();

    x.commit ();

    z = x.execSql( "select PNum from personal_info" );

    for ( var count = 0; z.fetch (); count++ )
    {
        isValidPNum( z.PNum );
	if ( count % 10000 == 0 ) print( count );
        }

    print( "count is", z.PNum );
    }


