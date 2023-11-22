#ifndef UTF8_H
#define UTF8_H

typedef int unsigned codepoint;

int unsigned utf8_char_len(char *str);
codepoint utf8_char(char *str, int unsigned *len);

#endif  // UTF8_H
