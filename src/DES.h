#ifndef _DES_H_INCLUDED
#define _DES_H_INCLUDED

//
// DES implementation
//

typedef unsigned long DES_UINT32;
typedef long DES_INT32;

// Single des
//

class DES
{
    bool for_encryption;
    DES_UINT32 key_schedule[ 32 ];

public:

    // Initializes an already allocated des key context
    //
    bool init( const unsigned char* key, size_t keylen,
               bool for_encryption );

    // Initializes an already allocated des key context
    //
    bool init_with_key_check( const unsigned char* key, size_t keylen,
                              bool for_encryption );

    // Encrypt in ecb/cbc/cfb/ofb modes
    //
    void ecb( unsigned char* dest,
              const unsigned char* src, size_t len );

    void cbc( unsigned char* dest,
              const unsigned char* src, size_t len,
              unsigned char* iv );

    void cfb( unsigned char* dest,
              const unsigned char* src, size_t len,
              unsigned char* iv );

    void ofb( unsigned char* dest,
              const unsigned char* src, size_t len,
              unsigned char* iv );
    };

/// Triple des
//

class TripleDES
{
    bool for_encryption;
    DES_UINT32 key_schedule[ 96 ];

public:

    // Sets an already allocated 3des context
    //
    bool init( const unsigned char* key, size_t keylen,
               bool for_encryption );

    // Destroy any sensitive data in the context
    //
    void free( void );

    // Encrypt using ecb/cbc/cfb/ofb modes
    //
    void ecb( unsigned char* dest,
              const unsigned char* src, size_t len );

    void cbc( unsigned char* dest,
              const unsigned char* src, size_t len,
              unsigned char* iv );

    void cfb( unsigned char* dest,
              const unsigned char* src, size_t len,
              unsigned char* iv );

    void ofb( unsigned char* dest,
              const unsigned char* src, size_t len,
              unsigned char* iv );
    };

extern void
DES3_encryptBodyData
(
    const char* key,
    unsigned char* data,
    long len
    );

extern void
DES3_decryptBodyData
(
    const char* key,
    unsigned char* data,
    long len
    );

#endif // _DES_H_INCLUDED
