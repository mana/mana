# Dye system

Images can be recolored at load time by appending a dye specification to
the file name, separated by `|`:

    image.png|R:#ff0000,ff8080;W:#000000,808080,ffffff

A dye specification is a `;` separated list of channel mappings. Each
mapping is a channel letter, a colon and a palette.

The dye system was added by Guillaume Melquiond in 2007, based on ideas
by fungos.

## Channels

The intensity-based channels recolor pixels based on which color
components they are made of. Only "pure" pixels are affected: pixels
where either all components are equal (gray, channel W) or where the
unused components are exactly 0. The intensity of a pixel is its largest
component value.

| Letter | Affected pixels              |
|--------|------------------------------|
| R      | red (r > 0, g = 0, b = 0)    |
| G      | green (g > 0, r = 0, b = 0)  |
| Y      | yellow (r = g > 0, b = 0)    |
| B      | blue (b > 0, r = 0, g = 0)   |
| M      | magenta (r = b > 0, g = 0)   |
| C      | cyan (g = b > 0, r = 0)      |
| W      | gray (r = g = b > 0)         |
| S      | exact color pairs            |
| A      | exact color pairs incl alpha |

For R, G, B, Y, M, C and W the palette is a `#` followed by
comma-separated RGB colors (`#rrggbb,rrggbb,...`). The palette describes
an intensity ramp from black through the listed colors. For example
`W:#000000,808080,ffffff` maps gray pixels by intensity onto the ramp
black, gray, white. Pixels that are not pure (e.g. skin tones where all
components differ) are never recolored by these channels.

For S ("simple") the palette is a list of color pairs instead of a ramp:
each pixel exactly matching an odd entry's RGB color is replaced by the
even entry that follows it. Alpha is preserved. For A the same applies
but colors have an alpha byte (`#rrggbbaa`) which is both compared and
replaced. When an S or A mapping is present it takes precedence and the
intensity channels are ignored.

The S and A channels were added by Andrei Karas in ManaPlus in 2012 and
are used by existing game data.

## Placeholders

An imageset can declare which channels it supports without providing
palettes by listing bare channel letters:

    <imageset name="base" src="graphics/sprites/x.png|W;B;R" .../>

These are placeholders, filled with palettes from the path used to
reference the sprite or particle definition:

    <sprite>equipment/chest/chainmail.xml|#aabbcc;#112233;#ddeeff</sprite>

The first palette fills the first placeholder, and so on. Placeholders
left unfilled (because the reference supplied no or too few palettes)
simply leave that channel undyed. Segments of a `|` suffix may also be
pre-filled mappings (`X:palette`), which are passed through unchanged.

## Implementation

- `Dye::instantiate` performs the placeholder substitution when sprite
  definitions, animations and particle effects are loaded.
- `ResourceManager::getImage` splits the `|`, constructs `Dye` from the
  suffix and applies it per pixel in `Image::load`. The suffix is part
  of the resource key, so differently dyed variants are cached
  separately.
- `DyePalette::getColor` implements the intensity ramp interpolation;
  `DyePalette::replaceColor` implements the S/A pair replacement.
- The `dyecmd` tool (built by default, `-DBUILD_DYECMD=OFF` to disable)
  applies dyes to images and can report how many pixels a spec affects.
