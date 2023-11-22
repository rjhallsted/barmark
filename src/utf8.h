#ifndef UTF8_H
#define UTF8_H

typedef int unsigned codepoint;

int unsigned utf8_character_len(unsigned char *str);
codepoint utf8_char(unsigned char *str, int unsigned *len);

#endif  // UTF8_H
