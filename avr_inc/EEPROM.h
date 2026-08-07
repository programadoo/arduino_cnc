#ifndef EEPROM_h
#define EEPROM_h

#include <inttypes.h>
#include <avr/eeprom.h>

struct EERef{
    EERef( const int index ) : index( index ) {}
    
    int index;

    uint8_t operator*() const            { return eeprom_read_byte( (uint8_t*) index ); }
    operator uint8_t() const             { return **this; }

    EERef &operator=( const EERef &ref ) { return *this = *ref; }
    EERef &operator=( uint8_t in )       { eeprom_update_byte( (uint8_t*) index, in ); return *this; }
    EERef &operator+=( uint8_t in )      { return *this = **this + in; }
    EERef &operator-=( uint8_t in )      { return *this = **this - in; }
    EERef &operator*=( uint8_t in )      { return *this = **this * in; }
    EERef &operator/=( uint8_t in )      { return *this = **this / in; }

    EERef &operator++()                  { return *this += 1; }
    EERef &operator--()                  { return *this -= 1; }
};

struct EEPROMClass{
    EERef operator[]( const int idx )    { return idx; }
    uint8_t read( int idx )              { return eeprom_read_byte( (const uint8_t*) idx ); }
    void write( int idx, uint8_t val )   { eeprom_write_byte( (uint8_t*) idx, val ); }
    void update( int idx, uint8_t val )  { eeprom_update_byte( (uint8_t*) idx, val ); }

    template< typename T > T &get( int idx, T &t ){
        uint8_t *ptr = (uint8_t*) &t;
        for( int count = sizeof(T) ; count -- ; )
            *ptr++ = eeprom_read_byte( (const uint8_t*) idx++ );
        return t;
    }

    template< typename T > const T &put( int idx, const T &t ){
        const uint8_t *ptr = (const uint8_t*) &t;
        for( int count = sizeof(T) ; count -- ; )
            eeprom_update_byte( (uint8_t*) idx++, *ptr++ );
        return t;
    }
};

static EEPROMClass EEPROM;

#endif
