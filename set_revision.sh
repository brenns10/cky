#-------------------------------------------------------------------------------
# 
# File:         set_revision.sh
#
# Author:       Stephen Brennan
#
# Date Created: Tuesday, 24 June 2014
#
# Description:  Set the revision of the libstephen subrepo.
#
# Since updating your subrepository does not change the .hgsubstate file in the
# parent repository, this script will set .hgsubstate to the current revision of
# the subrepo.
#
# You must run this script from the repository root directory.
#
#-------------------------------------------------------------------------------

REPO=cky
SUBREPO=libstephen

cd $SUBREPO
echo `hg --debug id -i` $SUBREPO > ../.hgsubstate
