/***************************************************************************//**

  @file         fsm/simulation.c

  @author       Stephen Brennan

  @date         Created Saturday, 26 July 2014

  @brief        Simulation functions for FSMs.

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

#include <stdio.h>      // for fprintf
#include <stdbool.h>    // for bool

#include "fsm.h"        // funtions we're defining
#include "libstephen/al.h" // al_*, SMB_DP

////////////////////////////////////////////////////////////////////////////////
// Helper Functions
////////////////////////////////////////////////////////////////////////////////

/**
   @brief Return the epsilon closure of the state on this FSM.

   The epsilon closure is the set of all states that can be reached from this
   state without consuming any input.

   @param f The FSM
   @param state The state to calculate from
   @return A list of states in the epsilon closure
 */
smb_al *fsm_sim_nondet_epsilon_closure(fsm *f, int state)
{
  smb_status status;
  smb_al *closure = al_create(&status);
  smb_al *trans_list;
  smb_al *visit_queue = al_create(&status);
  fsm_trans *ft;
  DATA d;
  int i;

  d.data_llint = state;
  al_push_back(visit_queue, d, &status);

  // Breadth first search here: while queue is not empty.
  while (al_length(visit_queue) > 0) {
    // Get next state to expand, mark it as visited (in the closure)
    d = al_pop_front(visit_queue, &status);
    al_append(closure, d, &status);
    trans_list = (smb_al *)al_get(&f->transitions, d.data_llint, &status).data_ptr;
    // For every transition out of it
    for (i = 0; i < al_length(trans_list); i++) {
      // If epsilon transitions out of it, and we haven't visited or put it in
      // the queue yet, add it to the visit queue.
      ft = (fsm_trans *) al_get(trans_list, i, &status).data_ptr;
      d.data_llint = ft->dest;
      if (fsm_trans_check(ft, EPSILON) &&
          al_index_of(visit_queue, d) == -1 &&
          al_index_of(closure, d) == -1){
        d.data_llint = ft->dest;
        al_push_back(visit_queue, d, &status);
      }
    }
  }

  al_delete(visit_queue);
  return closure;
}

/**
   @brief Combine the two lists (with no duplicates), and free the second.

   @param first The target list (which will be added to)
   @param second The list which will be combined into first and freed.
 */
void fsm_sim_nondet_union_and_delete(smb_al *first, smb_al *second)
{
  smb_status status;
  int i;
  DATA d;
  for (i = 0; i < al_length(second); i++) {
    d = al_get(second, i, &status);
    if (al_index_of(first, d) == -1) {
      al_append(first, d, &status);
    }
  }
  al_delete(second);
}

/**
   @brief Test if the intersection between the two lists is non-empty.
   @param first The first list
   @param second The second list
   @retval true when the intersection is NOT empty
   @retval false when the intersection IS empty
 */
bool fsm_sim_nondet_non_empty_intersection(smb_al *first, smb_al *second)
{
  smb_status status;
  int i;
  DATA d;
  for (i = 0; i < al_length(first); i++) {
    d = al_get(first, i, &status);
    if (al_index_of(second, d) != -1)
      return true;
  }
  return false;
}

/**
   @brief Copy all the values from one array list into another.
   @param dest The destination array list
   @param from The source array list
 */
void al_copy_all(smb_al *dest, const smb_al *from)
{
  smb_status status;
  int i;
  for (i = 0; i < al_length(from); i++) {
    al_append(dest, al_get(from, i, &status), &status);
  }
}

////////////////////////////////////////////////////////////////////////////////
// Public Functions
////////////////////////////////////////////////////////////////////////////////

/**
   @brief Run the finite state machine as a deterministic one.

   Simulates a DFSM.  Any possible input at any given state should have exactly
   one transition.  Zero transitions are allowed (and interpreted as an
   immediate reject).

   @param f The FSM to simulate
   @param input The string of input
 */
bool fsm_sim_det(fsm *f, const wchar_t *input)
{
  smb_status status;
  DATA d;
  int i, next, state;
  smb_al *list;
  fsm_trans *trans;
  state = f->start;

  // While we have not exhausted our input
  while (*input != '\0') {
    SMB_DP("State: %d, Input %Lc\n", state, *input);
    // Get the list of transitions from this state
    list = (smb_al *) al_get(&f->transitions, state, &status).data_ptr;
    next = -1;
    // Look for transitions that match this character
    for (i = 0; i < al_length(list); i++) {
      trans = (fsm_trans *) al_get(list, i, &status).data_ptr;
      if (fsm_trans_check(trans, *input)) {
        if (next == -1) {
          next = trans->dest;
        } else {
          // If there are two matching transitions, error.
          fprintf(stderr, "Error: non-deterministic FSM simulated as "
                  "deterministic.\n");
        }
      }
    }
    // If there are no transitions, fail.
    if (next == -1) {
      SMB_DP("No available transitions, reject.\n");
      return false;
    }
    state = next;
    input++;
  }

  SMB_DP("State: %d, Input finished.\n", state);

  // If the state is in the accepting states, we accept, else reject
  d.data_llint = state;
  return al_index_of(&f->accepting, d) != -1;
}

/**
   @brief Begin a non-deterministic simulation of this finite state machine.

   This simulation is designed to be run in steps.  First, call this function.
   Then, loop through many calls to fsm_sim_nondet_step().  When
   fsm_sim_nondet_state() returns a value indicating that the simulation is
   complete, you will have your answer.  Note that you should also call
   fsm_sim_nondet_state() after a call to this function, because the input may
   be empty.

   @param f The FSM to simulate
   @param input The input to run on
 */
fsm_sim *fsm_sim_nondet_begin(fsm *f, const wchar_t *input)
{
  smb_al *curr = fsm_sim_nondet_epsilon_closure(f, f->start);
  fsm_sim *fs = fsm_sim_create(f, curr, input);
  return fs;
}

/**
   @brief Check the state of the simulation.
   @param s The simulation to check.
   @retval FSM_SIM_ACCEPTING when the simulation has not ended, but is accepting
   @retval FSM_SIM_NOT_ACCEPTING when the simulation has not ended, and is not
   accepting
   @retval FSM_SIM_REJECTED when the simulation has ended and rejected
   @retval FSM_SIM_ACCEPTED when the simulation has ended and accepted
   @see FSM_SIM_ACCEPTING
   @see FSM_SIM_NOT_ACCEPTING
   @see FSM_SIM_REJECTED
   @see FSM_SIM_ACCEPTED
 */
int fsm_sim_nondet_state(const fsm_sim *s)
{
  // If the current state is empty, REJECT
  if (al_length(s->curr) == 0) {
    return FSM_SIM_REJECTED;
  }
  if (fsm_sim_nondet_non_empty_intersection(&s->f->accepting, s->curr)) {
    // If one of our current states is accepting...
    if (*s->input == L'\0') {
      // ... and input is exhausted, ACCEPT
      return FSM_SIM_ACCEPTED;
    } else {
      // ... and input remains, we are accepting, but not done
      return FSM_SIM_ACCEPTING;
    }
  } else {
    // If no current state is accepting ...
    if (*s->input == L'\0') {
      // ... and the input is exhausted, REJECT
      return FSM_SIM_REJECTED;
    } else {
      // ... and input remains, we are not accepting, but not done
      return FSM_SIM_NOT_ACCEPTING;
    }
  }
}

/**
   @brief Perform a single step in the non-deterministic simulation.

   This function performs a single iteration of the simulation algorithm.  It
   takes the current states, finds next states with the input character, and
   then adds in the epsilon closures of all the states.  You should use
   fsm_sim_nondet_state() to determine the state of the simulation after a step.

   @param s The simulation state struct
 */
void fsm_sim_nondet_step(fsm_sim *s)
{
  smb_status status;
  int i, j, state, original;
  DATA d;
  fsm_trans *t;
  smb_al *next = al_create(&status);
  smb_al *trans;

  // For diagnostics, print out the entire current state set
  SMB_DP("Current state: ");
  for (i = 0; i < al_length(s->curr); i++) {
    SMB_DP("%Ld ", al_get(s->curr, i, &status).data_llint);
  }
  SMB_DP("\n");

  // For each current state:
  for (i = 0; i < al_length(s->curr); i++) {
    state = (int) al_get(s->curr, i, &status).data_llint;
    trans = (smb_al *) al_get(&s->f->transitions, state, &status).data_ptr;

    // For each transition out:
    for (j = 0; j < al_length(trans); j++) {
      t = (fsm_trans *)al_get(trans, j, &status).data_ptr;
      d.data_llint = t->dest;

      // If the transition contains the current input, and it's not already in
      // the next state list, add it to the next state list.
      if (fsm_trans_check(t, *s->input) &&
          al_index_of(next, d) == -1) {
        al_append(next, d, &status);
      }
    }
  }

  // For each state in the original next state list, union in all their epsilon
  // closures.
  original = al_length(next);
  for (i = 0; i < original; i++) {
    state = (int) al_get(next, i, &status).data_llint;
    fsm_sim_nondet_union_and_delete(next,
                                    fsm_sim_nondet_epsilon_closure(s->f,
                                                                   state));
  }

  // Delete the old state, set the new one, and advance the input
  al_delete(s->curr);
  s->curr = next;
  s->input++;

  // For diagnostics, print the new state
  SMB_DP("New state: ");
  for (i = 0; i < al_length(s->curr); i++) {
    SMB_DP("%Ld ", al_get(s->curr, i, &status).data_llint);
  }
  SMB_DP("\n");
}

/**
   @brief Simulate the FSM as a non-deterministic state machine.

   This function is a convenience function that executes the NDFSM to
   completion.  The other functions, begin, step, and delete, can be used to
   perform a more customizable simulation.

   @param f The FSM to simulate
   @param input The input string to run with
   @retval true if the machine accepts
   @retval false if the machine rejects
 */
bool fsm_sim_nondet(fsm *f, const wchar_t *input)
{
  fsm_sim *sim = fsm_sim_nondet_begin(f, input);
  int res = fsm_sim_nondet_state(sim);

  while (res != FSM_SIM_REJECTED && res != FSM_SIM_ACCEPTED) {
    fsm_sim_nondet_step(sim);
    res = fsm_sim_nondet_state(sim);
    SMB_DP("Current result: %d\n", res);
  }

  fsm_sim_delete(sim, true);
  if (res == FSM_SIM_REJECTED)
    return false;
  else
    return true;
}