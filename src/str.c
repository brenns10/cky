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
