/*
 *  TMXCopy
 *  Copyright (C) 2007  Philipp Sehmisch
 *  Copyright (C) 2009  Steve Cotton
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#include <string>
#include <vector>
#include <set>
#include <libxml/parser.h>

struct ConfigurationOptions
{
    /* When copying map layers, how to match source layer to
     * destination layer.
     *
     * True: Pair the first layer to the first layer, the second
     * to the second, etc.
     *
     * False: Pair up layers with matching names.
     */
    bool copyLayersByOrdinal;

    /* Create extra layers in the target as necessary. */
    bool createMissingLayers;
};

struct Tileset
{
    std::string imagefile;
    int firstgid;
    std::string name;
    int tilewidth;
    int tileheight;

    bool operator== (Tileset const& a)
    {
        return (imagefile == a.imagefile &&
                tilewidth == a.tilewidth &&
                tileheight == a.tileheight
                );
    }
};

struct Tile
{
    int tileset; // number of tileset
    size_t index; // index in said tileset
};

typedef std::vector<Tile> LayerTiles;

/* This represents an empty tile in the layer.
 * Note that {0,0} would be the first tile in the first tileset.
 */
const Tile defaultTile = {-1, 0};

class Layer
{
    public:
        /* name - the name of the layer, as shown in Tiled
         * tileCount - total number of tiles (width*height)
         */
        Layer(std::string name, LayerTiles::size_type tileCount)
            : mTiles(tileCount, defaultTile),
            mName (name)
        {
        }

        std::string getName() { return mName; }
        Tile& at(LayerTiles::size_type c) { return mTiles.at(c); }

    private:
        LayerTiles mTiles;
        std::string mName;
};

class Map
{
    public:
        Map(std::string filename);
        ~Map();

        bool overwrite(Map* srcMap,
                    int srcX, int srcY, int srcWidth, int srcHeight,
                    int destX, int destY,
                    const ConfigurationOptions& config);

        int save(std::string filename);

        int getNumberOfLayers() { return mLayers.size(); }

        Layer* getLayer(size_t num) { return mLayers.at(num); }
        Layer* getLayer(std::string name);

        std::vector<Tileset*>* getTilesets() { return &mTilesets; }

        int getWidth() { return mWidth; }
        int getHeight() { return mHeight; }

    private:
        std::vector<Layer*> mLayers;

        int mWidth;
        int mHeight;
        int mMaxGid;

        std::vector<Tileset*> mTilesets;

        xmlDocPtr mXmlDoc;
};
