#!/usr/bin/env bash
# Build GitHub Pages branch automatically.
## STARTING STATE: Must be in the branch master, with no uncommitted changes.
## You've run the tests on release mode, and they run successfully!

PROJECT=cky

# Exit script on first error:
set -e

# Get the current commit
COMMIT=$(git rev-parse HEAD)

# Generate coverage information.
make CFG=coverage
bin/coverage/main -t
make gcov

# Generate documentation.
make docs

# In a new directory, clone the gh-pages branch.
cd ..
git clone -b gh-pages "https://$GH_TOKEN@github.com/brenns10/$PROJECT.git" gh-pages
cd gh-pages

# Update git configuration so I can push.
if [ "$1" != "dry" ]; then
    # Update git config.
    git config user.name "Travis Builder"
    git config user.email "smb196@case.edu"
fi

# Copy the docs and coverage results.
cp -R ../$PROJECT/doc .
cp -R ../$PROJECT/cov .

# Add and commit changes.
git add -A .
git commit -m "[ci skip] Autodoc commit for $COMMIT."
if [ "$1" != "dry" ]; then
    git push -q origin gh-pages
fi
