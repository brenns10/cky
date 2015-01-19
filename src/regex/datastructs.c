/***************************************************************************//**

  @file         regex/datastructs.c

  @author       Stephen Brennan

  @date         Created Saturday, 26 July 2014

  @brief        Regular expression data structure definitions.

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

#include "libstephen/base.h"
#include "regex.h"   // functions we're implementing

/**
   @brief Initialize a regex_hit.
   @param obj The (pre-allocated) regex_hit.
   @param start The start location of the hit.
   @param length The length of the hit.
 */
void regex_hit_init(regex_hit *obj, int start, int length)
{
  // Initialization logic
  obj->start = start;
  obj->length = length;
}

/**
   @brief Allocate a regex_hit.
   @param start The start location of the hit.
   @param length The end location of the hit.
   @return Pointer to the regex_hit.  Must be freed with regex_hit_delete().
 */
regex_hit *regex_hit_create(int start, int length)
{
  regex_hit *obj = smb_new(regex_hit, 1);
  regex_hit_init(obj, start, length);
  return obj;
}

/**
   @brief Clean up a regex_hit object.
   @param obj The regex_hit object to clean up.
 */
void regex_hit_destroy(regex_hit *obj)
{
  // Cleanup logic (none)
}

/**
   @brief Clean up and free a regex_hit object.
   @param obj The regex_hit to free.
 */
void regex_hit_delete(regex_hit *obj) {
  if (obj) {
    regex_hit_destroy(obj);
    smb_free(obj);
  } else {
    fprintf(stderr, "regex_hit_delete: called with null pointer.\n");
  }
}
