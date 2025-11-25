// Jacob
// string_utils.c (eller i en lämplig källfil)
#include <stddef.h> // För size_t

/**
 * En enkel implementering av memcpy för inbyggda system
 * (förutsätter att src och dest inte överlappar)
 */
void* memcpy(void* restrict dest, const void* restrict src, size_t n) {
    unsigned char* d = dest;
    const unsigned char* s = src;
    
    // Kopiera byte för byte
    while (n--) {
        *d++ = *s++;
    }
    return dest;
}

void *memset(void *s, int c, unsigned int n) {
    unsigned char *p = (unsigned char *)s;
    while (n--) {
        *p++ = (unsigned char)c;
    }
    return s;
}