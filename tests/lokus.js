
{
    var x = new SQL;
    x.connect( "LOKUS", "admin", "" );

    x.getAutoCommit ();
    x.setAutoCommit( false );
    x.getAutoCommit ();

    x.commit ();

    z = x.execSql( "select * from qTimetable", "2000-04-01", "2" );

    while( z.fetch () )
    {
        print( z.Aomr, z.Turnr )
        }

    }


