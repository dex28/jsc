

dir = new Directory( "." );
dir.open();

charcount = 100;

for( ;; )
{
    x = dir.read ();
    if ( x == false )
        break;

    if ( x == "." || x == ".." )
        continue;

    charcount += 1 + x.length;

    if ( charcount > 60 )
    {
		System.stdout.writeln( "" );
        System.stdout.write( x );
        charcount = 0;
        }
    else
    {
    	System.stdout.write( " " + x );
		}
    }

dir.close ();

System.stdout.writeln( "" );
System.stdout.flush ();
