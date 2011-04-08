# Run this script to update the various submodules linked to
# from this project

git submodule init

perl -pi -e 's/git\@github.com:fletcher/git:\/\/github.com\/fletcher/gi' .git/config

git submodule update
