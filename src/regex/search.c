/***************************************************************************//**

  @file         search.c

  @author       Stephen Brennan

  @date         Created Saturday, 26 July 2014

  @brief        Regular expression search functions.

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

#include "regex.h"      // functions we're implementing
#include "fsm.h"        // for FSM's
#include "libstephen.h" // for array lists

/**
   @brief Perform a regex-style search with an FSM on a search text.

   The "regex-style search" runs the FSM starting at each character in the text.
   If the FSM ever enters an accepting state, then that is a potential match.
   The last potential match (if any) for every character in the text makes up
   the list of hits.

   @param regex_fsm The FSM to search with.  Not necessarily generated from a
   regular expression.
   @param srchText The text to search the FSM on.
   @param greedy When set to true, only returns one hit.  Otherwise, returns all
   hits.
   @param overlap When set to true, allows overlapping hits.  For instance, a
   search for "\w+" on "blah" would return blah, lah, ah, and h.  Since this is
   usually undesirable, the recommended value is false.  When false, the search
   will skip to the end of each hit and resume searching there.
   @return An array list (smb_al) of `regex_hit *` full of hits.  Each hit must
   be individually freed when you're finished, as well as the array list itself.
 */
smb_al *fsm_search(fsm *regex_fsm, const wchar_t *srchText, bool greedy,
                   bool overlap)
{
  fsm_sim *curr_sim;
  int start = 0, length, last_length, res;
  smb_al *results = al_create();
  DATA d;

  SMB_DP("STARTING FSM SEARCH\n");

  // OUTER WHILE: iterate over starting characters.
  while (srchText[start] != L'\0') {
    // Simulate the FSM at each character.
    curr_sim = fsm_sim_nondet_begin(regex_fsm, srchText + start);
    length = 0;
    last_length = -1;
    res = -1;

    SMB_DP("=> Begin simulation at index %d: '%lc'.\n", start, srchText[start]);

    // INNER WHILE: iterate over lengths.
    while (srchText[start + length] != L'\0' && res != FSM_SIM_REJECTED) {
      // Step through the FSM simulation until we hit the end of the string, are
      // rejected, or are accepted.
      fsm_sim_nondet_step(curr_sim);
      res = fsm_sim_nondet_state(curr_sim);
      length++;
      SMB_DP("   => On step (length %d), res=%d.\n", length, res);

      // When we encounter a possible match, mark it.  We will only return the
      // largest (last) match for each simulation.
      if (res == FSM_SIM_ACCEPTING || res == FSM_SIM_ACCEPTED) {
        last_length = length;
        SMB_DP("      Currently accepting!\n");
      }
      // If the search reports input exhausted, we leave the loop.
      if (res == FSM_SIM_ACCEPTED || res == FSM_SIM_REJECTED) {
        break;
      }
    } // END INNER WHILE

    // Clean up the simulation.
    fsm_sim_delete(curr_sim, true);

    if (last_length != -1) {
      // If we encounter a match during this simulation, record it.
      SMB_DP("=> Found match of length %d.\n", last_length);
      d.data_ptr = (void *) regex_hit_create(start, last_length);
      al_append(results, d);

      if (greedy) {
        SMB_DP("=> Greedy return.\n");
        return results; // return after first hit
      }

      if (overlap) {
        // If we allow overlapping hits, we only need to move one character
        // over.
        start++;
      } else {
        // If we don't allow overlapping hits, the next search has to start
        // after the end of this hit.
        start += last_length;
      }
    } else {
      start++;
    }
  } // END OUTER WHILE
  return results;
}

/**
   @brief Searches for a regular expression on a search text.
   @see fsm_search() For details on the other parameters and return value.
   @param regex The regular expression to search for.
 */
smb_al *regex_search(const wchar_t *regex, const wchar_t *srchText, bool greedy,
                     bool overlap)
{
  fsm *regex_fsm = regex_parse(regex);
  smb_al *return_value = fsm_search(regex_fsm, srchText, greedy, overlap);
  fsm_delete(regex_fsm, true);
  return return_value;
}
