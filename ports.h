#include "stdint.h"

static inline uint8_t inb( int port ) {
    uint8_t result;
    asm volatile("inb %w1, %b0" : "=a" (result) : "Nd" (port) );
    return result;
}

static inline uint16_t inw( int port ) {
    uint16_t result;
    asm volatile("inw %w1, %w0" : "=a" (result) : "Nd" (port) );
    return result;
}

static inline uint32_t inl( int port ) {
    uint32_t result;
    asm volatile("inl %w1, %0" : "=a" (result) : "Nd" (port) );
    return result;
}

static inline void outb( uint8_t value, int port ) {
    asm volatile("outb %b0, %w1" : : "a" (value), "Nd" (port) );
}

static inline void outw( uint16_t value, int port ) {
    asm volatile("outw %w0, %w1" : : "a" (value), "Nd" (port) );
}

static inline void outl( uint32_t value, int port ) {
    asm volatile("outl %0, %w1" : : "a" (value), "Nd" (port) );
}
