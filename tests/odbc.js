
{
    var x = new ODBC;
    x.connect( "UFODB", "admin", "" );

    x.getAutoCommit ();
    x.setAutoCommit( false );
    x.getAutoCommit ();

    x.commit ();

    z = x.execSql( "select szTitle as NASLOV from Waves where cUsageCounter = 3" );

    print( z.retired, z.pera );
    z.retired = null;
    print( z.retired, z.pera );
    }


