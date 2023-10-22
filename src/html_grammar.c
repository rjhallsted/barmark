#include "html_grammar.h"

#include <ctype.h>
#include <stdbool.h>
#include <string.h>

#include "util.h"

/*
Html grammar matches take a null-terminated string to match against, and a
pointer to a match length variable to set. They will return a boolean indicating
if the matcher matches the string, and if true, increase *match_len by the
number of characters consumed by the match.
*/

/*
Tag name:
ascii-letter, [ascii-letter, digit, '-'](0+),
*/
bool m_tag_name(char *line, size_t *match_len) {
  if (!isalpha(line[0])) {
    return false;
  }
  size_t match_len_ref = 1;
  while (isalpha(line[match_len_ref]) || isdigit(line[match_len_ref]) ||
         line[match_len_ref] == '-') {
    match_len_ref++;
  }
  *match_len += match_len_ref;
  return true;
}

bool is_whitespace(char c) { return c == ' ' || c == '\t'; }

/*
Whitespace:
[' ', '\t']
*/
bool m_whitespace(char line[static 1], size_t match_len[static 1]) {
  if (is_whitespace(line[0])) {
    *match_len += 1;
    return true;
  }
  return false;
}

/*
Whitespace_spread:
whitespae(0+)

Always returns true.
*/
bool m_whitespace_spread(char line[static 1], size_t match_len[static 1]) {
  size_t match_len_ref = 0;
  while (m_whitespace(line + match_len_ref, &match_len_ref)) {
    ;
  }
  *match_len += match_len_ref;
  return true;
}

/*
Line ending:
'\n'
*/
bool m_line_ending(char line[static 1], size_t match_len[static 1]) {
  if (line[0] == '\n') {
    *match_len += 1;
    return true;
  }
  return false;
}

/*
Whitespace with line ending:
whitespace-spread, line-ending(0-1), whitespace-spread

Always returns true.
*/
bool m_whitespace_with_opt_line_ending(char line[static 1],
                                       size_t match_len[static 1]) {
  size_t match_len_ref = 0;
  m_whitespace_spread(line, &match_len_ref);
  m_line_ending(line + match_len_ref, &match_len_ref);
  m_whitespace_spread(line + match_len_ref, &match_len_ref);
  *match_len += match_len_ref;
  return true;
}

/*
Attribute Name:
[ascii-letter, '_', ':'], [ascii-letter, '_', '.', ':', '-'](0+)
*/
bool m_attribute_name(char line[static 1], size_t match_len[static 1]) {
  if (!isalpha(line[0]) && line[0] != '_' && line[0] != ':') {
    return false;
  }
  size_t match_len_ref = 1;
  while (isalpha(line[match_len_ref]) || line[match_len_ref] == '_' ||
             line[match_len_ref] == '.',
         line[match_len_ref] == ':', line[match_len_ref] == '-') {
    match_len_ref++;
  }
  *match_len += match_len_ref;
  return true;
}

/*
Unquoted Attribute Value:
[.![whitespace, '\n', ''', '"', '=', '<', '>', '`']](1+)
*/
bool m_unquoted_attribute_value(char line[static 1],
                                size_t match_len[static 1]) {
  size_t match_len_ref = 0;
  while (line[match_len_ref] && !is_whitespace(line[match_len_ref]) &&
         line[match_len_ref] != '\n' && line[match_len_ref] != '\'' &&
         line[match_len_ref] != '"' && line[match_len_ref] != '=' &&
         line[match_len_ref] != '<' && line[match_len_ref] != '>' &&
         line[match_len_ref] != '`') {
    match_len_ref++;
  }
  if (match_len_ref > 0) {
    *match_len += match_len_ref;
    return true;
  }
  return false;
}

/*
Single Quoted Attribute Value:
''', [.!'''](0+), '''
*/
bool m_single_quoted_attribute_value(char line[static 1],
                                     size_t match_len[static 1]) {
  if (line[0] != '\'') {
    return false;
  }
  size_t match_len_ref = 1;
  while (line[match_len_ref] && line[match_len_ref] != '\'') {
    match_len_ref++;
  }
  if (line[match_len_ref] != '\'') {
    return false;
  }
  *match_len += match_len_ref;
  return true;
}

/*
Double Quoted Attribute Value:
'"', [.!'"'](0+), '"'
*/
bool m_double_quoted_attribute_value(char line[static 1],
                                     size_t match_len[static 1]) {
  if (line[0] != '"') {
    return false;
  }
  size_t match_len_ref = 1;
  while (line[match_len_ref] && line[match_len_ref] != '"') {
    match_len_ref++;
  }
  if (line[match_len_ref] != '"') {
    return false;
  }
  *match_len += match_len_ref;
  return true;
}

/*
Attribute Value:
[unquoted-attribute-value, single-quoted-attribute-value,
double-quoted-attribute-value]
*/
bool m_attribute_value(char line[static 1], size_t match_len[static 1]) {
  return m_unquoted_attribute_value(line, match_len) ||
         m_single_quoted_attribute_value(line, match_len) ||
         m_double_quoted_attribute_value(line, match_len);
}

/*
Attribute Value Specification:
m_whitespace_with_opt_line_ending, '=', m_whitespace_with_opt_line_ending,
attribute-value
*/
bool m_attribute_value_specification(char line[static 1],
                                     size_t match_len[static 1]) {
  size_t match_len_ref = 0;
  m_whitespace_with_opt_line_ending(line, &match_len_ref);
  if (line[match_len_ref] != '=') {
    return false;
  }
  match_len_ref++;
  m_whitespace_with_opt_line_ending(line, &match_len_ref);
  if (!m_attribute_value(line + match_len_ref, &match_len_ref)) {
    return false;
  }
  *match_len += match_len_ref;
  return true;
}

/*
Attribute:
m_whitespace_with_opt_line_ending, attribute-name,
attribute-value-specification(0-1)
*/
bool m_attribute(char line[static 1], size_t match_len[static 1]) {
  size_t match_len_ref = 0;
  m_whitespace_with_opt_line_ending(line, &match_len_ref);
  if (!m_attribute_name(line + match_len_ref, &match_len_ref)) {
    return false;
  }
  m_attribute_value_specification(line + match_len_ref, &match_len_ref);
  *match_len += match_len_ref;
  return true;
}

/*
Open Tag:
'<',tag-name, attributes(0+), m_whitespace_with_opt_line_ending, '/'(0-1), '>'

Also tags an optional array of forbidden tag names and will return false if the
tag name matches any of them.
*/
bool m_open_tag(char line[static 1], size_t match_len[static 1],
                char **forbidden_tags, int unsigned forbidden_tags_size) {
  if (line[0] != '<') {
    return false;
  }
  size_t match_len_ref = 1;
  if (!m_tag_name(line + match_len_ref, &match_len_ref)) {
    return false;
  }
  if (forbidden_tags_size > 0 && forbidden_tags) {
    char *tag_name = strndup(line + 1, match_len_ref - 1);
    for (int unsigned i = 0; i < forbidden_tags_size; i++) {
      if (strcasecmp(tag_name, forbidden_tags[i]) == 0) {
        free(tag_name);
        return false;
      }
    }
    free(tag_name);
  }
  while (m_attribute(line + match_len_ref, &match_len_ref)) {
    ;
  }
  m_whitespace_with_opt_line_ending(line + match_len_ref, &match_len_ref);
  if (line[match_len_ref] == '/') {
    match_len_ref++;
  }
  if (line[match_len_ref] != '>') {
    return false;
  }
  *match_len += match_len_ref;
  return true;
}

/*
Closing Tag:
'<', '/', tag-name, attributes(0+), m_whitespace_with_opt_line_ending, '>'
*/
bool m_closing_tag(char line[static 1], size_t match_len[static 1],
                   char **forbidden_tags, int unsigned forbidden_tags_size) {
  if (line[0] != '<' || line[1] != '/') {
    return false;
  }
  size_t match_len_ref = 2;
  if (!m_tag_name(line + match_len_ref, &match_len_ref)) {
    return false;
  }
  if (forbidden_tags_size > 0 && forbidden_tags) {
    char *tag_name = strndup(line + 1, match_len_ref - 1);
    for (int unsigned i = 0; i < forbidden_tags_size; i++) {
      if (strcasecmp(tag_name, forbidden_tags[i]) == 0) {
        free(tag_name);
        return false;
      }
    }
    free(tag_name);
  }
  while (m_attribute(line + match_len_ref, &match_len_ref)) {
    ;
  }
  m_whitespace_with_opt_line_ending(line + match_len_ref, &match_len_ref);
  if (line[match_len_ref] != '>') {
    return false;
  }
  *match_len += match_len_ref;
  return true;
}