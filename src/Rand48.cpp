//
// Pseudo-random number generator
//

#include "JS.h"

// This implementation assumes unsigned short integers of at least
// 16 bits, long integers of at least 32 bits, and ignores
// overflows on adding or multiplying two unsigned integers.
// Two's-complement representation is assumed in a few places.
//

// The Linear Congruential Method: X[n+1] = ( a * X[n] + c ) mod m
// X[n] : linear congruential sequence of random numbers
// X[0] : the starting value
// a    : multiplier
// c    : increment
// m    : modulus

const unsigned A0 = 0xE66D;
const unsigned A1 = 0xDEEC;
const unsigned A2 = 0x5;
const unsigned C  = 0xB;

const double two16m = 1.0 / ( 1L << 16 );

#define LOW(x)   ( unsigned(x) & 0xFFFF )
#define HIGH(x)  LOW( (x) >> 16 )

RAND48:: RAND48( void )
{
    x[0] = 0x330E;
    x[1] = 0xABCD;
    x[2] = 0x1234;
    a[0] = A0;
    a[1] = A1;
    a[2] = A2;
    c = C;
    }
     
void
RAND48:: Seed( unsigned long seedval )
{
    x[0] = 0x330E;
    x[1] = LOW( seedval );
    x[2] = HIGH( seedval );
    a[0] = A0;
    a[1] = A1;
    a[2] = A2;
    c = C;
    }

inline void MUL( long x, long y, unsigned z[] )
{
    long n = x * y;
    z[0] = LOW( n );
    z[1] = HIGH( n );
    }

inline void ADDEQU( unsigned& x, unsigned y, unsigned& z )
{
    z = x + y > 0xFFFF;
    x = LOW( x + y );
    }

double
RAND48:: Random( void )
{
    unsigned p[ 2 ], q[ 2 ], r[ 2 ], carry0, carry1;

    MUL( a[0], x[0], p );
    ADDEQU( p[0], c, carry0 );
    ADDEQU( p[1], carry0, carry1 );

    MUL( a[0], x[1], q );
    ADDEQU( p[1], q[0], carry0 );

    MUL( a[1], x[0], r );

    x[2] = LOW(
        carry0 + carry1 + ( p[1] + r[0] > 0xFFFF )
        + q[1] + r[1] +
	    a[0] * x[2] + a[1] * x[1] + a[2] * x[0]
        );

    x[1] = LOW( p[1] + r[0] );
    x[0] = LOW( p[0] );

    return two16m * ( two16m * ( two16m * x[0] + x[1] ) + x[2] );
    }

