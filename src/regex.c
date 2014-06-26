/***************************************************************************//**

  @file         regex.c

  @author       Stephen Brennan

  @date         Created Sunday, 18 May 2014

  @brief        Definitions for regular expression routines.

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

#include <stdbool.h>

#include "regex.h"      // the functions we are defining
#include "fsm.h"        // tools to implement regex
#include "libstephen.h" // for smb_al

/**
   @brief Copy the transitions and states from src into dest.

   All the states are added, and the transitions are modified with the offset
   determined from the initial number of states in the destination.
 */
static void fsm_copy_trans(fsm *dest, const fsm *src)
{
  int offset = al_length(&dest->transitions);
  int i,j;
  // Copy the transitions from the source one, altering their start and
  // destination to fit with the new state numbering.
  for (i = 0; i < al_length(&src->transitions); i++) {
    // Add new state corresponding to this one
    fsm_add_state(dest, false);
    // For each transition out of this state, add a copy to dest.
    const smb_al *list = al_get(&src->transitions, i).data_ptr;
    for (j = 0; j < al_length(list); j++) {
      const fsm_trans *old = al_get(list, j).data_ptr;
      fsm_trans *new = fsm_trans_copy(old);
      new->dest = new->dest + offset;
      fsm_add_trans(dest, i + offset, new);
    }
  }
}

/**
   @brief Concatenate two FSMs into the first FSM object.

   This function concatenates two FSMs.  This means it constructs a machine that
   will accept a string where the first part is accepted by the first machine,
   and the second part is accepted by the second machine.  It is done by hooking
   up the accepting states of the first machine to the start state of the second
   (via epsilon transition).  Then, only the second machines's accepting states
   are used for the final product.

   Note that the concatenation is done in-place into the first FSM.  If you
   don't wish for that to happen, use fsm_copy() to copy the first one, and then
   use fsm_concat() to concatenate into the copy.

   @param first The first FSM.  Is modified to contain the concatenation.
   @param second The second FSM.  It is not modified.
 */
void fsm_concat(fsm *first, const fsm *second)
{
  int i, offset = al_length(&first->transitions);
  DATA d;
  // Create a transition from each accepting state into the second machine's
  // start state.
  for (i = 0; i < al_length(&first->accepting); i++) {
    int start = (int)al_get(&first->accepting, i).data_llint;
    // This could be outside the loop, but we need a new one for each instance :(
    fsm_trans *ft = fsm_trans_create_single(EPSILON, EPSILON, FSM_TRANS_POSITIVE,
                                            second->start + offset);
    fsm_add_trans(first, start, ft);
  }

  fsm_copy_trans(first, second);

  // Replace the accepting states of the first with the second.
  al_destroy(&first->accepting);
  al_init(&first->accepting);
  for (i = 0; i < al_length(&second->accepting); i++) {
    d = al_get(&second->accepting, i);
    d.data_llint += offset;
    al_append(&first->accepting, d);
  }
}

/**
   @brief Construct the union of the two FSMs into the first one.

   This function creates an FSM that will accept any string accepted by the
   first OR the second one.  The FSM is constructed in place of the first
   machine, modifying the parameter.  If this is not what you want, make a copy
   of the first one, and pass the copy instead.

   @param first The first FSM to union.  Will be modified with the result.
   @param second The second FSM to union
 */
void fsm_union(fsm *first, const fsm *second)
{
  int newStart, i, offset = al_length(&first->transitions);
  fsm_trans *fsTrans, *ssTrans;
  DATA d;

  // Add the transitions from the second into the first.
  fsm_copy_trans(first, second);

  // Create a new start state!
  newStart = fsm_add_state(first, false);

  // Add epsilon-trans from new start to first start
  fsTrans = fsm_trans_create_single(EPSILON, EPSILON, FSM_TRANS_POSITIVE, first->start);
  fsm_add_trans(first, newStart, fsTrans);

  // Add epsilon-trans from new start to second start
  ssTrans = fsm_trans_create_single(EPSILON, EPSILON, FSM_TRANS_POSITIVE, second->start + offset);
  fsm_add_trans(first, newStart, ssTrans);

  // Accept from either the first or the second
  for (i = 0; i < al_length(&second->accepting); i++) {
    d = al_get(&second->accepting, i);
    d.data_llint += offset;
    al_append(&first->accepting, d);
  }

  // Change the start state
  first->start = newStart;
}

/**
   @brief Modify an FSM to accept 0 or more occurrences of it.

   This modifies the existing FSM to accept L*, where L is the language accepted
   by the machine.

   @param f The FSM to perform Kleene star on
 */
void fsm_kleene(fsm *f)
{
  int i, newStart = fsm_add_state(f, false); // not accepting yet
  fsm_trans *newTrans;
  DATA d;

  // Add epsilon-trans from new start to first start
  newTrans = fsm_trans_create_single(EPSILON, EPSILON, FSM_TRANS_POSITIVE, f->start);
  fsm_add_trans(f, newStart, newTrans);

  // For each accepting state, add a epsilon-trans to the new start
  for (i = 0; i < al_length(&f->accepting); i++) {
    newTrans = fsm_trans_create_single(EPSILON, EPSILON, FSM_TRANS_POSITIVE, newStart);
    fsm_add_trans(f, (int) al_get(&f->accepting, i).data_llint, newTrans);
  }

  // Make the new start accepting
  d.data_llint = newStart;
  al_append(&f->accepting, d);

  // Change the start state (officially)
  f->start = newStart;
}

/**
   @brief Adjust the FSM according to its modifier, if any.

   When a character, character class, or parenthesized regex is read in, it
   could be followed by the modifiers `*`, `+`, or `?`.  This function adjusts
   the FSM for those modifiers, and adjusts the location pointer if one was
   present.

   @param new The newly read in FSM.
   @param regex The pointer to the pointer to the location in the regex.
 */
void check_modifier(fsm *new, const wchar_t **regex)
{
  fsm *f;

  switch ((*regex)[1]) {

  case L'*':
    fsm_kleene(new);
    (*regex)++;
    break;

  case L'+':
    f = fsm_copy(new);
    fsm_kleene(f);
    fsm_concat(new, f);
    fsm_delete(f, true);
    (*regex)++;
    break;

  case L'?':
    // Create the machine that accepts the empty string
    f = fsm_create();
    f->start = fsm_add_state(f, true);
    fsm_union(new, f);
    fsm_delete(f, true);
    (*regex)++;
    break;
  }
}

/**
   @brief Creates a FSM for a single character.
   @param character The character to make a transition for.
   @returns FSM for the character.
 */
fsm *create_single_char(wchar_t character)
{
  fsm *f = fsm_create();
  int s0 = fsm_add_state(f, false);
  int s1 = fsm_add_state(f, true);
  fsm_add_single(f, s0, s1, character, character, FSM_TRANS_POSITIVE);
  f->start= s0;
  return f;
}

/**
   @brief Create a FSM for a character class.

   Reads a character class (pointed by `*regex`), which it then converts to a
   single transition FSM.

   @param regex Pointer to pointer to location in string!
   @returns FSM for character class.
 */
fsm *create_char_class(const wchar_t **regex)
{
  // TODO: Implement character classes.
  while (**regex != L']') {
    (*regex)++;
  }

  return create_single_char(L'x');
}

/**
   @brief The workhorse recursive helper to create_regex_fsm().

   Crawls through a regex, holding a FSM of the expression so far.  Single
   characters are concatenated as simple 0 -> 1 machines.  Parenthesis initiate
   a recursive call.  Pipes union the current regex with everything after.

   Needs to be called with a non-NULL value for both, which is why there is a
   top level function.

   @param regex Local pointer to the location in the regular expression.
   @param final Pointer to the parent's location.
   @return An FSM for the regular expression.
 */
fsm *create_regex_fsm_recursive(const wchar_t *regex, const wchar_t **final)
{
  // Initial FSM is a machine that accepts the empty string.
  int i = 0;
  fsm *curr = fsm_create();
  fsm *new;
  curr->start = fsm_add_state(curr, true);

  // ASSUME THAT ALL PARENS, ETC ARE BALANCED!

  for ( ; *regex; regex++) {
    switch (*regex) {

    case L'(':
      new = create_regex_fsm_recursive(regex + 1, &regex);
      check_modifier(new, &regex);
      fsm_concat(curr, new);
      fsm_delete(new, true);
      break;

    case L')':
      *final = regex;
      return curr;
      break;

    case L'|':
      // Need to pass control back up without attempting to check modifiers.
      new = create_regex_fsm_recursive(regex + 1, &regex);
      fsm_union(curr, new);
      fsm_delete(new, true);
      *final = regex;
      return curr;
      break;

    case L']':
      new = create_char_class(&regex);
      check_modifier(new, &regex);
      fsm_concat(curr, new);
      fsm_delete(new, true);
      break;

    default:
      // A regular letter
      new = create_single_char(*regex);
      check_modifier(new, &regex);
      fsm_concat(curr, new);
      fsm_delete(new, true);
      break;

      // TODO: Add escape character support.
      
    }
  }
  *final = regex;
  return curr;
}

/**
   @brief Construct a FSM to accept the given regex.
   @param regex Regular expression string.
   @return A FSM that can be used to decide the language of the regex.
 */
fsm *create_regex_fsm(const wchar_t *regex){
  return create_regex_fsm_recursive(regex, &regex);
}
