/*******************************************************************************

  File:         fsm.c

  Author:       Stephen Brennan

  Date Created: Wednesday, 14 May 2014

  Description:  Finite state machine implementations.

  Copyright (c) 2014, Stephen Brennan
  All rights reserved.

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

#include <stdlib.h>
#include <stdio.h>
#include <wchar.h>
#include <stdbool.h>
#include "libstephen.h" // already in fsm.h, but used here too
#include "fsm.h"

/**
   @brief Initialize an already-allocated FSM transition unit.

   An FSM transition unit defines what characters can and can't be accepted for
   a transition, similar to a regular expression's character class.  It can
   either be a positive transition (any character between start and end), or a
   negative transition (any character not between start and end).  It can have
   more than one transition criteria (i.e character range), but they all have to
   be either positive or negative.

   @param ft The pre-allocated transition
   @param n The number of transition criteria
   @param type The type of transition (positive or negative)
 */
void fsm_trans_init(fsm_trans *ft, int n, int type, int dest)
{
  ft->type = type;

  // Allocate space for the start range, plus null terminator
  ft->start = (wchar_t *) malloc((n + 1) * sizeof(wchar_t));
  if (!ft->start) {
    RAISE(ALLOCATION_ERROR);
    fprintf(stderr, "libstephen: fsm_trans_init(): Memory allocation failed.\n");
    return;
  }

  // Allocate space for the end range, plus null terminator
  ft->end = (wchar_t *) malloc((n + 1) * sizeof(wchar_t));
  if (!ft->end) {
    RAISE(ALLOCATION_ERROR);
    fprintf(stderr, "libstephen: fsm_trans_init(): Memory allocation failed.\n");
    free(ft->start);
    return;
  }

  ft->dest = dest;

  // Count the allocated space.
  SMB_INCREMENT_MALLOC_COUNTER(2 * (n + 1) * sizeof(wchar_t));
}

/**
   @brief Allocate and initialize a fsm_trans object.

   @param n The number of ranges
   @param type The type of transition
 */
fsm_trans *fsm_trans_create(int n, int type, int dest)
{
  // Allocate the space
  fsm_trans *ft = (fsm_trans *) malloc(sizeof(fsm_trans));
  CLEAR_ALL_ERRORS;

  // Check for allocation error
  if (!ft) {
    RAISE(ALLOCATION_ERROR);
    return NULL;
  }

  // Run initialization
  fsm_trans_init(ft, n, type, dest);

  // If the initialization failed, free and return
  if (CHECK(ALLOCATION_ERROR)) {
    free(ft);
    return NULL;
  }

  // Otherwise, count the memory and return it.
  SMB_INCREMENT_MALLOC_COUNTER(sizeof(fsm_trans));
  return ft;
}

/**
   @brief Clean up the fsm_trans object.  Do not free it.

   @param ft The object to clean up
 */
void fsm_trans_destroy(fsm_trans *ft)
{
  if (ft && ft->start && ft->end) {
    int len = wcslen(ft->start); // assume len(start) == len(end)
    free(ft->start);
    free(ft->end);
  } else {
    fprintf(stderr, "libstephen: fsm_trans_destroy() called with null pointers.\n");
  }
}

/**
   @brief Free the fsm_trans object.

   @param ft The object to free
 */
void fsm_trans_delete(fsm_trans *ft)
{
  if (ft) {
    fsm_trans_destroy(ft);
    free(ft);
    SMB_DECREMENT_MALLOC_COUNTER(sizeof(fsm_trans));
  } else {
    fprintf(stderr, "libstephen: fsm_trans_delete() called with null pointer.\n");
  }
}

/**
   @brief Initialize a fsm_trans object with a single range.

   @param ft The object to initialize
   @param start The beginning of the range
   @param end The end of the range
   @param type The type of the object
 */
void fsm_trans_init_single(fsm_trans *ft, wchar_t start, wchar_t end, int type, int dest)
{
  fsm_trans_init(ft, 1, type, dest);
  if (end < start)
    fprintf(stderr, "Error: initialization of invalid range in FSM.\n");
  ft->start[0] = start;
  ft->end[0] = end;
}

/**
   @brief Allocate and initialize a fsm_trans object with a single range.

   @param start The beginning of the range
   @param end The end of the range
   @param type The type of the object
 */
fsm_trans *fsm_trans_create_single(wchar_t start, wchar_t end, int type, int dest)
{
  fsm_trans *ft = fsm_trans_create(1, type, dest);
  if (end < start)
    fprintf(stderr, "Error: initialization of invalid range in FSM.\n");
  ft->start[0] = start;
  ft->end[0] = end;
  return ft;
}

/**
   Check whether the character is accepted by the transition.

   @param ft The transition to check
   @param c The character to check
 */
bool fsm_trans_check(const fsm_trans *ft, wchar_t c)
{
  wchar_t *start = ft->start, *end = ft->end;

  while (start && end && *start != '\0' && *end != '\0') {
    if (c >= *start && c <= *end) {
      if (ft->type == FSM_TRANS_POSITIVE)
        return true;
      else
        return false;
    }
    start++; end++;
  }

  if (ft->type == FSM_TRANS_POSITIVE)
    return false;
  else
    return true;
}

/**
   @brief Initialize a FSM.

   @param f The FSM to Initialize
 */
void fsm_init(fsm *f)
{
  CLEAR_ALL_ERRORS;
  f->nStates = 0;
  f->start = -1;
  al_init(&f->transitions);
  if (CHECK(ALLOCATION_ERROR))
    return;
  al_init(&f->accepting);
  if (CHECK(ALLOCATION_ERROR)) {
    al_destroy(&f->transitions);
    return;
  }
}

/**
   @brief Allocate and initialize an FSM.
 */
fsm *fsm_create(void)
{
  fsm *f = (fsm *) malloc(sizeof(fsm));
  CLEAR_ALL_ERRORS;

  if (!f) {
    RAISE(ALLOCATION_ERROR);
    return NULL;
  }

  fsm_init(f);
  if (CHECK(ALLOCATION_ERROR)) {
    free(f);
  }

  SMB_INCREMENT_MALLOC_COUNTER(sizeof(fsm));
  return f;
}

/**
   @brief Clean up a FSM, but do not free it.

   @param f The FSM to clean up
   @param free_transitions Do we free the transitions too?
 */
void fsm_destroy(fsm *f, bool free_transitions)
{
  int i;
  if (free_transitions) {
    for (i = 0; i < al_length(&f->transitions); i++) {
      fsm_trans_delete((fsm_trans *) al_get(&f->transitions, i).data_ptr);
    }
  }
  al_destroy(&f->transitions);
  al_destroy(&f->accepting);
}

/**
   @brief Clean up and free an FSM.

   @param f The FSM to free
   @param free_transitions Do we free the transitions too?
 */
void fsm_delete(fsm *f, bool free_transitions)
{
  fsm_destroy(f, free_transitions);
  free(f);
  SMB_DECREMENT_MALLOC_COUNTER(sizeof(fsm));
}
