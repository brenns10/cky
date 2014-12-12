/***************************************************************************//**

  @file         regex.h

  @author       Stephen Brennan

  @date         Created Sunday, 18 May 2014

  @brief        Declarations for regular expression routines.

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

#ifndef SMB_REGEX_H
#define SMB_REGEX_H

#include "libstephen/al.h"
#include "fsm.h"

/**
   @brief A struct to hold regular expression search hits.
 */
typedef struct {

  /**
     @brief The location the hit starts at.  This is inclusive.
   */
  int start;

  /**
     @brief Number of characters in the hit.
   */
  int length;

} regex_hit;

// datastructs.c
void regex_hit_init(regex_hit *obj, int start, int length);
regex_hit *regex_hit_create(int start, int length);
void regex_hit_destroy(regex_hit *obj);
void regex_hit_delete(regex_hit *obj);

// parse.c
void regex_parse_check_modifier(fsm *new, const wchar_t **regex);
fsm *regex_parse_create_whitespace_fsm(int type);
fsm *regex_parse_create_word_fsm(int type);
fsm *regex_parse_create_digit_fsm(int type);
fsm *regex_parse_outer_escape(const wchar_t **regex);
fsm *regex_parse_char_class(const wchar_t **regex);
fsm *regex_parse(const wchar_t *regex);

// search.c
smb_al *fsm_search(fsm *regex_fsm, const wchar_t *srchText, bool greedy,
                   bool overlap);
smb_al *regex_search(const wchar_t *regex, const wchar_t *srchText, bool greedy,
                     bool overlap);

#endif
