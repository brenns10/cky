/***************************************************************************//**

  @file         fsm.h

  @author       Stephen Brennan

  @date         Created Wednesday, 14 May 2014

  @brief        Finite state machine definitions.

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

#ifndef SMB_FSM_H
#define SMB_FSM_H

#include <stdbool.h>
#include <wchar.h>
#include "libstephen.h"

#define FSM_TRANS_POSITIVE 0
#define FSM_TRANS_NEGATIVE 1

#define FSM_SIM_ACCEPTING 0
#define FSM_SIM_NOT_ACCEPTING 1
#define FSM_SIM_REJECTED 2
#define FSM_SIM_ACCEPTED 3

// TODO: determine a safe value for EPSILON
#define EPSILON ((wchar_t)-2)

typedef struct {

  int type;
  wchar_t *start;
  wchar_t *end;
  int dest;

} fsm_trans;

typedef struct {

  int start;
  smb_al transitions; // List of lists, indexed by state D:
  smb_al accepting;

} fsm;

typedef struct {

  fsm *f;
  smb_al *curr;
  const wchar_t *input;

} fsm_sim;

void fsm_trans_init(fsm_trans *ft, int n, int type, int dest);
fsm_trans *fsm_trans_create(int n, int type, int dest);
void fsm_trans_destroy(fsm_trans *ft);
void fsm_trans_delete(fsm_trans *ft);
fsm_trans *fsm_trans_copy(const fsm_trans *ft);

void fsm_trans_init_single(fsm_trans *ft, wchar_t start, wchar_t end, int type, int dest);
fsm_trans *fsm_trans_create_single(wchar_t start, wchar_t end, int type, int dest);

bool fsm_trans_check(const fsm_trans *ft, wchar_t c);

void fsm_init(fsm *f);
fsm *fsm_create(void);
void fsm_destroy(fsm *f, bool free_transitions);
void fsm_delete(fsm *f, bool free_transitions);
fsm *fsm_copy(const fsm *f);

int fsm_add_state(fsm *f, bool accepting);
void fsm_add_trans(fsm *f, int state, const fsm_trans *ft);
fsm_trans *fsm_add_single(fsm *f, int from, int to, wchar_t start, wchar_t end, int type);
bool fsm_sim_det(fsm *f, const wchar_t *input);
fsm *fsm_read(const wchar_t *source);
void fsm_print(fsm *f, FILE *dest);

void fsm_sim_delete(fsm_sim *fs, bool free_curr);
int fsm_sim_nondet_state(const fsm_sim *s);
fsm_sim *fsm_sim_nondet_begin(fsm *f, const wchar_t *input);
void fsm_sim_nondet_step(fsm_sim *s);
bool fsm_sim_nondet(fsm *f, const wchar_t *input);

#endif
