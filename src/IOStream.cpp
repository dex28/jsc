//
// I/O streams.
//

#include "JS.h"

const int DEFAULT_BUFFER_SIZE = 4096;

///////////////////////////////////////////////////////////////////////////////
// JSIOStream implementation
//

JSIOStream:: JSIOStream( void )
{
    //printf( "%p IOStream construct.\n", this );

    error = 0;

    buflen = DEFAULT_BUFFER_SIZE;
    buffer = NULL; // Must be reallocatable with jsRealloc()

    data_in_buf = 0;
    bufpos = 0;

    writep = 0;
    at_eof = 0;
    autoflush = 0;

    fCanRead = 0;
    fCanWrite = 0;
    }

JSIOStream:: ~JSIOStream( void )
{
    //printf( "%p IOStream destruct.\n", this );

    if ( buffer != NULL )
        delete buffer;

    buffer = NULL;
    }

void
JSIOStream:: SetBufferSize( size_t new_buflen )
{
    Flush ();

    uint8* new_buffer = (uint8*)jsRealloc( buffer, new_buflen );

    if ( new_buffer == NULL )
    {
        error = errno;
        return;
        }

    buflen = new_buflen;
    buffer = new_buffer;
    }

int
JSIOStream:: ReadByte( void )
{
    if ( bufpos < data_in_buf )
    {
        return buffer[ bufpos++ ];
        }
    else if ( at_eof )
    {
        return -1;
        }

    FillBuffer ();

    if ( bufpos < data_in_buf )
    {
        return buffer[ bufpos++ ];
        }

    return -1;
    }

size_t
JSIOStream:: Read( void *ptr, size_t size )
{
    if ( writep )
    {
        // We have buffered output data
        //
        if ( Flush () == EOF )
            return 0;

        assert( writep == 0 );
        }

    size_t total = 0;

    while ( size > 0 )
    {
        // First, take everything from the buffer
        //
        if ( bufpos < data_in_buf )
        {
            size_t got = data_in_buf - bufpos;

            if ( size < got )
                got = size;

            memcpy( ptr, buffer + bufpos, got );

            bufpos += got;
            size -= got;
            ptr = (uint8* ) ptr + got;
            total += got;
            }
        else
        {
            if ( at_eof ) // EOF seen, can't read more
                break;

            FillBuffer ();
            }
        }

    return total;
    }

size_t
JSIOStream:: Write( const void* ptr, size_t size )
{
    if ( ! fCanWrite )
    {
        error = EBADF;
        return 0;
        }

    if ( ! writep && bufpos < data_in_buf )
    {
        // We have some buffered data in the stream => the actual stream
        // position in context is not in sync with bufpos.
        // Seek back.
        //

        if ( OnSeek( SEEK_CUR, bufpos - data_in_buf ) < 0 )
        {
            error = errno;
            return 0;
            }

        bufpos = 0;
        data_in_buf = 0;
        }

    size_t total = 0;

    while ( size > 0 )
    {
        size_t space = buflen - data_in_buf;
        if ( size < space )
            space = size;

        // Append data to the buffer
        //
        memcpy( buffer + data_in_buf, ptr, space );
        data_in_buf += space;
        total += space;
        size -= space;
        ptr = (uint8*) ptr + space;

        // Now the buffer contains buffered write data
        //
        writep = 1;

        if ( size > 0 )
        {
            // Still some data left.  Must flush
            //
            if ( Flush () == EOF )
                return total;
            }
        }

    // Autoflush
    //
    if ( autoflush && writep )
    {
        if ( Flush () == EOF )
        {
            // Failed.  Just return something smaller than <size>
            //
            return total - data_in_buf;
            }
        }

    return total;
    }

void
JSIOStream:: Printf( char* fmt ... )
{
    char buf[ 1024 ];

    va_list marker;
    va_start( marker, fmt );
    int actlen = vsprintf( buf, fmt, marker );
    va_end( marker );

    Write( buf, actlen );
    }

void
JSIOStream:: PrintfLn( char* fmt ... )
{
    if ( fmt != NULL )
    {
        char buf[ 1024 ];

        va_list marker;
        va_start( marker, fmt );
        int actlen = vsprintf( buf, fmt, marker );
        va_end( marker );

        Write( buf, actlen );
        }

    Write( JS_ENDLINE, JS_ENDLINE_LEN );
    }

int
JSIOStream:: Flush ()
{
    if ( ! fCanWrite || ! writep )
        return 0;

    writep = 0;
    assert( bufpos == 0 );

    if ( data_in_buf > 0 )
    {
        size_t to_write = data_in_buf;

        data_in_buf = 0;
        if ( OnWrite( buffer, to_write) < to_write )
        {
            error = errno;
            return EOF;
            }
        }

    return 0;
    }

int
JSIOStream:: Unget( int byte)
{
    if ( byte < 0 )
        return byte;

    if ( writep )
    {
        // We have buffered output data
        //
        if ( Flush () == EOF )
            return EOF;

        assert( writep == 0 );
        }

    assert( bufpos > 0 );

    buffer[ --bufpos ] = byte;

    // Upon successful completion, we must return the byte
    //
    return byte;
    }

int
JSIOStream:: Seek( long offset, int whence )
{
    // Flush the possible buffered output
    //
    if ( Flush () == EOF )
        return -1;

    int result = OnSeek( offset, whence );

    if ( result == 0 ) // Successful.  Clear the eof flag
        at_eof = 0;

    return result;
    }

long
JSIOStream:: GetPosition ()
{
    // Flush the possible buffered output
    //
    if ( Flush () == EOF )
        return -1;

    long pos = OnGetPosition ();
    if ( pos < 0 )
        return pos;

    // The logical position if at <bufpos>, the context's idea is at
    // <data_in_buf>.  Adjust.
    //
    return pos - ( data_in_buf - bufpos );
    }


long
JSIOStream:: GetLength ()
{
    // Flush the possible buffered output
    //
    if ( Flush () == EOF )
        return -1;

    return OnGetLength ();
    }


void
JSIOStream:: FillBuffer ()
{
    if ( ! fCanRead )
    {
        at_eof = 1;
        return;
        }

    data_in_buf = OnRead( buffer, buflen );
    bufpos = 0;
    if ( data_in_buf == 0 )
        at_eof = 1;
    }

///////////////////////////////////////////////////////////////////////////////
// JSIOStreamFile implementation
//

JSIOStreamFile:: JSIOStreamFile( FILE* fp, int fRead, int fWrite, int fClose )
{
    pFile = fp;

    if ( pFile == NULL )
    {
        error = -1;
        return;
        }

    fCanRead = fRead;
    fCanWrite = fWrite;
    fDoClose = fClose;

    buffer = NEW uint8[ buflen ];

    if ( buffer == NULL )
    {
        error = -1;
        }
    }

JSIOStreamFile:: JSIOStreamFile( const char* name, int fRead, int fWrite )
{
    pFile = fopen( name, "rt" );

    if ( pFile == NULL )
    {
        error = -1;
        return;
        }

    fCanRead = fRead;
    fCanWrite = fWrite;
    fDoClose = 1;

    buffer = NEW uint8[ buflen ];

    if ( buffer == NULL )
    {
        error = -1;
        }
    }

size_t
JSIOStreamFile:: OnRead( void* pBuf, size_t nSize )
{
    errno = 0;
    size_t rc = fread( pBuf, 1, nSize, pFile );
    error = errno;
    return rc;
    }


size_t
JSIOStreamFile:: OnWrite( const void* pBuf, size_t nSize )
{
    errno = 0;
    size_t rc = fwrite( pBuf, 1, nSize, pFile );
    error = errno;
    return rc;
    }

int
JSIOStreamFile:: OnSeek( long nOffset, int nWhence )
{
    return fseek( pFile, nOffset, nWhence );
    }

long
JSIOStreamFile:: OnGetPosition( void )
{
    return ftell( pFile );
    }

long
JSIOStreamFile:: OnGetLength( void )
{
    // Save current position
    //
    long cpos = ftell( pFile );
    if ( cpos < 0 )
        return -1;

    // Seek to the end of the file
    //
    if ( fseek( pFile, 0L, SEEK_END ) < 0 )
        return -1;

    // Fetch result
    //
    long result = ftell( pFile );

    // Seek back
    //
    if ( fseek( pFile, cpos, SEEK_SET ) < 0 )
    {
        // Couldn't revert the fp to the original position
        //
        return -1;
        }

    return result;
    }

JSIOStreamFile:: ~JSIOStreamFile( void )
{
    //printf( "%p IOStreamFile destruct.\n", this );

    Flush ();

    if ( fDoClose )
        fclose( pFile );
    }

///////////////////////////////////////////////////////////////////////////////
// JSIOStreamPipe implementation
//

JSIOStreamPipe:: JSIOStreamPipe( FILE *fp, int readp, int do_close )
    : JSIOStreamFile( fp, readp, 1, do_close )
{
    }

JSIOStreamPipe:: ~JSIOStreamPipe( void )
{
    //printf( "%p IOStreamPipe destruct.\n", this );

    Flush ();
    
    if ( fDoClose )
        pclose( pFile );
    }

///////////////////////////////////////////////////////////////////////////////
// JSIOStreamIOFunc implementation

JSIOStreamIOFunc:: JSIOStreamIOFunc
(
    JSIOFunc foo, void* context,
    int readp, int writep
    )
{
    assert( readp != writep ); // only R or W can be done

    func = foo;
    pContext = context;
    position = 0;

    fCanRead = readp;
    fCanWrite = writep;

    buffer = NEW uint8[ buflen ];

    if ( buffer == NULL )
    {
        error = -1;
        }
    }

JSIOStreamIOFunc:: ~JSIOStreamIOFunc( void )
{
    //printf( "%p IOStreamIOFunc destruct.\n", this );

    Flush ();
    }

size_t
JSIOStreamIOFunc::OnRead( void* pBuf, size_t nSize )
{
    error = 0;

    size_t moved = func( pContext, pBuf, nSize );

    if ( moved >= 0 )
    {
        position += moved;
        }

    return moved;
    }

size_t
JSIOStreamIOFunc::OnWrite( const void* pBuf, size_t nSize )
{
    error = 0;

    size_t moved = func( pContext, (void*)pBuf, nSize );

    if ( moved >= 0 )
    {
        position += moved;
        }

    return moved;
    }

int
JSIOStreamIOFunc:: OnSeek( long offset, int whence )
{
    return -1;
    }

long
JSIOStreamIOFunc:: OnGetPosition( void )
{
    return position;
    }

long
JSIOStreamIOFunc:: OnGetLength( void )
{
    return -1;
    }

