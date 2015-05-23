/***************************************************************************//**

  @file         str.c

  @author       Stephen Brennan

  @date         Created Sunday, 29 June 2014

  @brief        Common functions for working with strings.

  @copyright    Copyright (c) 2014, Stephen Brennan.
  All rights reserved.

  @copyright
  Redistribution and use in source and binary forms, with or without
  modification, are permitted provided that the following conditions are met:
    * Redistributions of source code must retain the above copyright notice,
      this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice,
      this list of conditions and the following disclaimer in the documentation
      and/or other materials provided with the distribution.
    * Neither the name of Stephen Brennan nor the names of his contributors may
      be used to endorse or promote products derived from this software without
      specific prior written permission.

  @copyright
  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
  AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
  IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
  DISCLAIMED. IN NO EVENT SHALL STEPHEN BRENNAN BE LIABLE FOR ANY DIRECT,
  INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
  BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
  DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
  OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
  NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
  EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

*******************************************************************************/

#include <wchar.h>
#include <wctype.h>

#include "libstephen/ll.h"
#include "str.h"
#include "fsm.h"

/**
   @brief Get the value of a hexadecimal digit.
   @param digit The digit
   @return The value of the digit in hexadecimal.
 */
int hexit_val(wchar_t digit)
{
  if (iswdigit(digit)) {
    return digit - L'0';
  } else if (digit == L'a' || digit == L'b' || digit == L'c' || digit == L'd'
             || digit == L'e' || digit == L'f' || digit == L'A' || digit == L'B'
             || digit == L'C' || digit == L'D' || digit == L'E' || digit == L'F'){
    return 10 + towupper(digit) - L'A';
  }
  return -1;
}

/**
   @brief Get an escaped character from the string source.

   This function will advance the source pointer to after the escape sequence.
   It can get escapes `abfnrtv\xu`, which includes hexadecimal and unicode
   escapes.

   @brief source The source pointer
   @return The character that was escaped
 */
wchar_t get_escape(const wchar_t **source, wchar_t epsilon)
{
  wchar_t value = 0;
  wchar_t specifier = **source;
  (*source)++;
  switch (specifier) {
  case L'a':
    return L'\a';
  case L'b':
    return L'\b';
  case L'e':
    return epsilon;
  case L'f':
    return L'\f';
  case L'n':
    return L'\n';
  case L'r':
    return L'\r';
  case L't':
    return L'\t';
  case L'v':
    return L'\v';
  case L'\\':
    return L'\\';
  case L'x':
    value += 16 * hexit_val(**source);
    (*source)++;
    value += hexit_val(**source);
    (*source)++;
    return value;
  case L'u':
    value += 16 * 16 * 16 * hexit_val(**source);
    (*source)++;
    value += 16 * 16 * hexit_val(**source);
    (*source)++;
    value += 16 * hexit_val(**source);
    (*source)++;
    value += hexit_val(**source);
    (*source)++;
    return value;
  default:
    return specifier;
  }
}

/**
   @brief Place the escaped wchar in source into out.

   Source should be a string "\..." containing an escape sequence.  It should
   include the backslash.  This function will read the escape sequence, convert
   it to a wchar_t, store that in out, and return the number of characters read.
   @param source The string escape sequence to translate.
   @param out Where to store the output character.
   @return The number of characters read.
 */
int read_escape(const wchar_t *source, wchar_t *out)
{
  wchar_t specifier = source[1];
  *out = 0;
  switch (specifier) {
  case L'a':
    *out = L'\a';
    return 2;
  case L'b':
    *out = L'\b';
    return 2;
  case L'e':
    *out = EPSILON;
    return 2;
  case L'f':
    *out = L'\f';
    return 2;
  case L'n':
    *out = L'\n';
    return 2;
  case L'r':
    *out = L'\r';
    return 2;
  case L't':
    *out = L'\t';
    return 2;
  case L'v':
    *out = L'\v';
    return 2;
  case L'\\':
    *out = L'\\';
    return 2;
  case L'x':
    *out += 16 * hexit_val(source[2]);
    *out += hexit_val(source[3]);
    return 4;
  case L'u':
    *out += 16 * 16 * 16 * hexit_val(source[2]);
    *out += 16 * 16 * hexit_val(source[3]);
    *out += 16 * hexit_val(source[4]);
    *out += hexit_val(source[5]);
    return 6;
  default:
    *out = specifier;
    return 2;
  }
}

/**
   @brief Read a single character from the string, accepting escape sequences.
   @param source The string to read from.
   @param out Place to store the resulting character.
   @return Number of characters read from source.
 */
int read_wchar(const wchar_t *source, wchar_t *out)
{
  if (source[0] == L'\\') {
    return read_escape(source, out);
  } else {
    *out = source[0];
    return 1;
  }
}

/**
   @brief Return a list of lines from the given string.

   The string is modified!  You should make a copy before doing this.
   @param source The string to split into lines.
   @return A linked list containing wchar_t* to each line.
 */
smb_ll *split_lines(wchar_t *source)
{
  wchar_t *start;
  smb_ll *list;
  DATA d;

  /*
    We go through `source` looking for every newline, replace it with NUL, and
    add the beginnig of the line to the list.
   */
  start = source;
  list = ll_create();
  while (*source != L'\0') {
    if (*source == L'\n') {
      // Add line to list.
      d.data_ptr = start;
      ll_append(list, d);
      // Null-terminate the line.
      *source = L'\0';
      // Next string starts at the next character.
      start = source + 1;
    }
    source++;
  }
  if (start != source) {
    d.data_ptr = start;
    ll_append(list, d);
  }
  return list;
}
