#ifndef _MD5_H_INCLUDED
#define _MD5_H_INCLUDED

// MD5 context
//
class __declspec(dllexport) MD5
{
public:
    // POINTER defines a generic pointer type
    //
    typedef void* POINTER;

    // UINT32 defines a four byte word
    //
    typedef unsigned long int UINT32;

private:
    UINT32 state[ 4 ];            // state (ABCD)
    UINT32 count[ 2 ];            // number of bits, modulo 2^64 (lsb first)
    unsigned char buffer[ 64 ];  // input buffer

    void Transform( const unsigned char block[ 64 ] );

public:
    void Init ();
    void Update( const unsigned char*, unsigned int );
    void Final( unsigned char [ 16 ] );
    void FinalHex( char [] );
    };

#endif // _MD5_H_INCLUDED