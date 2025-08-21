#!/bin/bash
if [ "$#" -eq "0" ]; then
    echo "Usage: make-dist.sh <version>"
    exit 1
fi
name="mana-$1"

git ls-files --recurse-submodules -z \
    | grep -z --invert-match \
        -e '^\.' \
        -e '^Xcode/' \
    | tar caf $name-source.tar.gz --xform s:^:$name/: --null -T-

echo "Release ready as $name.tar.gz"
