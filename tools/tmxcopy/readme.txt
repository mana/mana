=== TMXCopy ===

Tmxcopy is a little tool that allows to copy parts of one TMX map to another map. This will make it much easier to match the border areas of maps. The program is command line based. The usage is:

 tmxcopy [-c] [-n] srcFile x y width height tgtFile x y [outfile]

Here an example: 
When you want to copy the lower right corner (20x20 tiles) of mapA.tmx to the upper left corner of mapB.txt you would open map A with tiled and check at which coordinates the area you want to copy begins. Let's say mapA is 120x130 tiles. Then the area you want to copy would begin at 100:110 and would be 20x20 tiles large. So the first part of the command is:

 tmxcopy mapA.tmx 100 110 20 20

Then you open the target map to check the coordinates where you want to put the copied map part. We want the upper left corner, so the coordinates are 0:0. That means the next part of the command would be:
 
 mapB.tmx 0 0

The command is now complete:

 tmxcopy mapA.tmx 100 110 20 20 mapB.tmx 0 0

But when you enter this command the mapB will be overwritten. This could be a problem when you made an error in the command. So it is saver to write the output to a new map file so we can look at the result in Tiled before we replace the original map:

 tmxcopy mapA.tmx 100 110 20 20 mapB.tmx 0 0 temp.tmx

Now we can check temp.tmx to see if the copying worked correctly. 


Which layer gets copied to which:
By default layers are copied to layers of the same name.  The -n option will make it copy by layer number instead.

  mapA: Ground, Fringe, Over, Collision, Object
  mapB: Ground, Fencing, Fringe, Over, Collision, Object
  The default copies Ground->Ground, Fringe->Fringe, Over->Over, Collision->Collision (the object layer is not affected)
  -n copies Ground->Ground, Fringe->Fencing, Over->Fringe, Collision->Over (mapB's collision and object layers are not affected)

  mapA: Ground, Fringe, Over, Collision, Object
  mapC: Ground, Fringe, Overhead, Collision, Object
  The default quits with an error
  -n copies Over->Overhead

The -c option creates layers as needed.  Using it to copy mapB to mapA will add a Fencing layer to mapA.


=== TMX Random Fill ===

This is for generating big areas of woodland (or other things that want lots of randomly-placed patterns).

Usage: tmx_random_fill mapFile destLayer x y width height templateFile [-o outfile]
    -c create layers, if they don't already exist in the target
    -o save results to outfile, instead of overwriting the original

Fill a rectangular area of mapFile's layer 'destLayer' with a random selection from the templateFile.

The template is a map where each layer is a pattern.  For example, to make a woodland:
  Create a new 2x1 tile map (yes, this is tiny, and only the base of the tree will be visible).
  Add the Woodland_x3 tileset, using the correct height (96 pixels).
  Make a layer, add a tree.
  Make a layer, add the second tree.
  Make a layer, add the third tree.
  Save this as template_trees.tmx
  Run tmx_random_fill with the appropriate options (destLayer will be "Fringe")

It will then randomly place trees, but only in places where they won't overlap with other things on that layer.  The size of the template map is the size of the area which must be empty in the destination layer.


=== TMX Translate ===

This tool is still to be written.  I guarantee it will be written before I get a woodland map finished.

A big woodland with lots of randomly-placed trees needs a complex collision layer, most of which can be generated from the visible layers.
This tool takes a two-layer template map - one layer contains visible tiles, and one layer contains collision tiles.


=== Bugs (for all these programs) ===

The program works so far but there are still some minor problems: 

-Only tested for TMW-compilant maps. I don't guarantee that it works with Tiled maps that are made for other games and thus use different features. It is assumed that the target map and the source maps have the same number of layers, for example. 
-Compressed maps (tmx.gz) can not be handled yet (but compressed or uncompressed layers work properly) 
-When the target map has an object layer it is moved to the bottom of the layer list of the map (no problem for the game but inconvenient for editing). Objects on the source map are ignored. 
-Layer data of output file isn't gzip-compressed yet 
-Created TMX file is a bit malformated (but working properly) 

The last 2 problems can be solved easily by opening and saving the map in Tiled.
