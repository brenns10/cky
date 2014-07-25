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

#include "libstephen.h"
#include "fsm.h"

typedef struct {
  int start;
  int length;
} regex_hit;

void regex_hit_init(regex_hit *obj, int start, int length);
regex_hit *regex_hit_create(int start, int length);
void regex_hit_destroy(regex_hit *obj);
void regex_hit_delete(regex_hit *obj);

void fsm_concat(fsm *first, const fsm *second);
void fsm_union(fsm *first, const fsm *second);
void fsm_kleene(fsm *f);
fsm *create_regex_fsm(const wchar_t *regex);

smb_al *fsm_search(fsm *regex_fsm, const wchar_t *srchText, bool greedy, bool overlap);
smb_al *regex_search(const wchar_t *regex, const wchar_t *srchText, bool greedy, bool overlap);

#endif
