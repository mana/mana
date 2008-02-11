/*
 *  TMXCopy
 *  Copyright 2007 Philipp Sehmisch
 *
 *
 *  TMXCopy is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  any later version.
 *
 *  TMXCopy is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with TMXCopy; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#include <string>
#include <vector>
#include <set>
#include <libxml/parser.h>

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

typedef std::vector<Tile> Layer;

class Map
{
    public:
        Map(std::string filename);

        bool overwrite(  Map* srcMap,
                    int srcX, int srcY, int srcWidth, int srcHeight,
                    int destX, int destY);

        int save(std::string filename);

        int getNumberOfLayers() { return mLayers.size(); }

        Layer* getLayer(size_t num) { return mLayers.at(num); }

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
