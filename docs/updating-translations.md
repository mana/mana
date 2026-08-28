# Updating translations

Translations are done on Weblate at https://hosted.weblate.org/projects/mana/mana/.
Weblate follows `master` of the upstream repository, uses `po/mana.pot` as
the template and exposes every `po/*.po` file. Which catalogs actually get
built and shipped is decided by `po/LINGUAS`.

Before a release, the steps are:

1. Import pending translations from Weblate.
2. Regenerate the template and merge it into the catalogs.
3. Push, so Weblate picks up the new strings, and give translators time.
4. Import the final translations and mention them in `NEWS`.

Always import from Weblate before regenerating the template. `msgmerge`
rewrites every catalog, so Weblate commits made against the old template
conflict with everything once the template has been rebuilt.

## 1. Import from Weblate

Merge the Weblate branch. A plain merge keeps every translator's commit under
their own name, which is the point.

```sh
git remote add weblate https://hosted.weblate.org/git/mana/mana/   # once
git fetch weblate
git log --oneline master..weblate/master
git merge weblate/master
```

Resolve any conflict inside a `.po` file in favour of the Weblate side.

## 2. Regenerate the template

The `update-po` target rebuilds `po/mana.pot` from the sources and merges it
into the catalogs listed in `LINGUAS`. It resolves paths relative to the
build directory, so it only does the right thing for an in-source build:

```sh
cmake -S . -B . && cmake --build . --target update-po
```

To update all catalogs, including the ones not shipped, or from a regular
out-of-source build, run the same steps by hand from the `po` directory:

```sh
cd po
grep '_(' -Irl ../src | sort > translatable-files
xgettext --files-from=translatable-files --directory=. \
         --output=mana.pot -d mana --keyword=_ --keyword=N_
for f in *.po; do msgmerge --quiet --update --backup=none -s "$f" mana.pot; done
```

Check the result and commit the template and catalogs together:

```sh
for f in po/*.po; do msgfmt --check --statistics -o /dev/null "$f"; done
git add po/mana.pot po/*.po
git commit -m "Updated translation files"
```

## 3. Push and announce

Weblate reads the template from upstream `master`, so push and confirm that
the new strings show up as untranslated on Weblate. Then announce the string
freeze and the deadline, for instance with a Weblate announcement, so
translators know how long they have.

## 4. Final import

Repeat step 1 once the deadline has passed. See which languages changed
since the previous release and credit them in `NEWS`:

```sh
git diff --stat v0.7.0..HEAD -- 'po/*.po'
```

```
- Updated German, French and Russian translations
```

## Enabling a language

Add its code to `po/LINGUAS` when a translation is reasonably complete and
the bundled fonts can render it. Languages using scripts without glyphs in
DejaVu Sans (Tamil, Telugu, Thai, Chinese, ...) have to stay disabled until
the client gains font fallback.

## Verifying a build

Catalogs are only built when gettext is found and `ENABLE_NLS` is on. In the
build directory they end up at `po/locale/<lang>/LC_MESSAGES/mana.mo`, so
counting them is a quick check that nothing was dropped:

```sh
find build/po/locale -name mana.mo | wc -l
```
