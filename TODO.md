CKY To-Do List
==============

Currently, I'm in the midst of porting CKY over to my latest Libstephen version.
Now that I have a sane policy on error handling, the first thing will be to go
through CKY and update its error handling mechanisms to match Libstephen's.

Then, I will want to go back to Libstephen and figure out whether I want to make
any more API changes.  The only other thing I would change right now is the
memory allocation counting portion.  Once I have everything in a good place,
I'll designate the API version 1.0, and only additions will be allowed to
Libstephen after that.

Next, I'll be able to come back to CKY, and really hit this code.  I'll want to
do some heavy-duty test writing.  Also, I'll want to examine the high-level
interface for regular expressions, and define an API for that.  And finally,
I'll definitely want to see whether I can reduce the complexity of some of my
awful functions.

When that's done, I think I'll pull the regular expression portion of CKY, along
with its test, into Libstephen.  Regular expressions are very general purpose.

Then, finally, I'll be able to start work on implementing the grammar objects,
and the CKY parsing algorithm.  Depending on how that works, I might be able to
pull it into Libstephen, but that's too far ahead for me to plan.
