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

#include <cstring>
#include <iostream>
#include <list>

#include <string.h>
#include <zlib.h>
#include <cassert>
#include <ctime>

#include "xmlutils.h"
#include "zlibutils.h"
#include "base64.h"
#include "tostring.h"

#include "map.hpp"

Map::Map(std::string filename):
    mMaxGid(0)
{
    std::cout<<"Loading map "<<filename<<std::endl;
    //load XML tree
    mXmlDoc = xmlReadFile(filename.c_str(), NULL, 0);

    if (!mXmlDoc)
    {
        std::cerr<<"Could not load "<<filename;
        throw 1;
    }

    xmlNodePtr rootNode = xmlDocGetRootElement(mXmlDoc);

    if (!rootNode || !xmlStrEqual(rootNode->name, BAD_CAST "map")) {
        std::cerr<<filename<<"is not a Tiled map file!";
        throw 1;
    }

    mWidth = XML::getProperty(rootNode, "width", 0);
    mHeight = XML::getProperty(rootNode, "height", 0);
    int layerNum = 0;
    for_each_xml_child_node(node, rootNode)
    {
        if (xmlStrEqual(node->name, BAD_CAST "tileset"))
        {
            //add tilesets to vector of used tilesets
            Tileset* tileset = new Tileset;
            tileset->name = XML::getProperty(node, "name", "Unnamed");
            tileset->tilewidth = XML::getProperty(node, "tilewidth", 0);
            tileset->tileheight = XML::getProperty(node, "tileheight", 0);
            tileset->firstgid = XML::getProperty(node, "firstgid", 0);
            for_each_xml_child_node(imageNode, node)
            {
                if (xmlStrEqual(imageNode->name, BAD_CAST "image"))
                tileset->imagefile = XML::getProperty(imageNode, "source", "");
            }

            //add tileset to tileset list
            Map::mTilesets.push_back(tileset);
        }
    }

    for_each_xml_child_node(node, rootNode)
    {
        if (xmlStrEqual(node->name, BAD_CAST "layer"))
        {
            //build layer information
            std::string name = XML::getProperty(node, "name", "");
            Layer* layer = new Layer(name, mWidth * mHeight);
            if (
                (mWidth  != XML::getProperty(node, "width" , 0)) ||
                (mHeight != XML::getProperty(node, "height", 0)) ||
                (0       != XML::getProperty(node, "x"     , 0)) ||
                (0       != XML::getProperty(node, "y"     , 0)))
            {
                std::cerr<<"Error: layer size does not match map size for layer \""<<name<<"\" in "<<filename<<std::endl;
                throw 1;
            }

            for_each_xml_child_node(dataNode, node)
            {
                if (!xmlStrEqual(dataNode->name, BAD_CAST "data")) continue;

                std::string encoding = XML::getProperty(dataNode, "encoding", "");
                std::string compression = XML::getProperty(dataNode, "compression", "");

                if (encoding != "base64")
                {
                    std::cerr<<"Layers in "<<filename<<" are not base64 encoded!";
                    return;
                }

                // Read base64 encoded map file
                xmlNodePtr dataChild = dataNode->xmlChildrenNode;
                if (!dataChild)
                    continue;

                int len = strlen((const char*)dataChild->content) + 1;
                unsigned char *charData = new unsigned char[len + 1];
                const char *charStart = (const char*)dataChild->content;
                unsigned char *charIndex = charData;

                while (*charStart) {
                    if (*charStart != ' ' && *charStart != '\t' &&
                            *charStart != '\n')
                    {
                        *charIndex = *charStart;
                        charIndex++;
                    }
                    charStart++;
                }
                *charIndex = '\0';

                int binLen;
                unsigned char *binData =
                    php3_base64_decode(charData, strlen((char*)charData), &binLen);

                delete[] charData;

                if (binData) {
                    if (compression == "gzip")
                    {
                        unsigned char *inflated;
                        unsigned int inflatedSize =
                            inflateMemory(binData, binLen, inflated);
                        free(binData);
                        binData = inflated;
                        binLen = inflatedSize;
                        if (inflated == NULL)
                        {
                            std::cerr<<"Error: while decompressing layer "<<layerNum<<" in "<<filename;
                            throw 1;
                        }
                    }

                    int c = 0;
                    for (int i = 0; i < binLen - 3; i += 4) {
                        int gid = binData[i] |
                            binData[i + 1] << 8 |
                            binData[i + 2] << 16 |
                            binData[i + 3] << 24;

                        if (gid == 0)
                        {
                            layer->at(c).tileset = -1;
                            layer->at(c).index = 0;
                        }
                        else
                        {
                            for (int s = mTilesets.size()-1; s >= 0; s--)
                            {
                                if (mTilesets.at(s)->firstgid <= gid)
                                {
                                    layer->at(c).tileset = s;
                                    layer->at(c).index = gid - mTilesets.at(s)->firstgid;
                                    if (mMaxGid < gid) mMaxGid = gid;
                                    break;
                                }
                            }
                        }

                        c++;
                    }
                    free(binData);
                } else {
                    std::cerr<<"error processing layer data in "<<filename<<std::endl;
                }
            }
            mLayers.push_back(layer);
            layerNum++;
        }
    }

    std::cout<<"tilesets: "<<mTilesets.size()<<std::endl;
    std::cout<<"layers:"<<mLayers.size()<<std::endl;
    std::cout<<"largest GID:"<<mMaxGid<<std::endl<<std::endl;
}

/**
 * When copying tiles from another map, add new tilesets to this map, and return the translation table.
 */
std::map<int, int> Map::addAndTranslateTilesets(const Map* srcMap)
{
    std::map<int, int> translation;
    translation[-1] = -1;
    std::vector<Tileset*>* srcTilesets = const_cast<Map*>(srcMap)->getTilesets();

    for (std::vector<Tileset*>::size_type a = 0; a < srcTilesets->size(); a++)
    {
        std::vector<Tileset*>::size_type b;
        for (b = 0; b < mTilesets.size(); b++)
        {
            if (*srcTilesets->at(a) == *mTilesets.at(b))
            {
                break;
            }
        }
        if (b == mTilesets.size())
        {
            mMaxGid += srcTilesets->at(a)->firstgid;
            Tileset* destTileset = new Tileset(*srcTilesets->at(a));
            destTileset->firstgid = mMaxGid;//it is possible to get some holes in the gid index this way but who cares, we got 32bit.
            mTilesets.push_back(destTileset);
        }
        translation[a] = b;
    }
    return translation;
}

bool Map::overwrite(  Map* srcMap,
                    int srcX, int srcY, int srcWidth, int srcHeight,
                    int destX, int destY,
                    const ConfigurationOptions& config)
{
    //plausibility check of coordinates
    bool checkPassed = true;
    if (srcX + srcWidth > srcMap->getWidth()) {
        std::cerr<<"Error: Area exceeds right map border of source map!"<<std::endl;
        checkPassed = false;
    }
    if (srcY + srcHeight > srcMap->getHeight()) {
        std::cerr<<"Error: Area exceeds lower map border of source map!"<<std::endl;
        checkPassed = false;
    }
    if (destX + srcWidth > mWidth) {
        std::cerr<<"Error: Area exceeds right map border of target map!"<<std::endl;
        checkPassed = false;
    }
    if (destY + srcHeight > mHeight) {
        std::cerr<<"Error: Area exceeds lower map border of target map!"<<std::endl;
        checkPassed = false;
    }
    if (!config.createMissingLayers)
    {
        if (config.copyLayersByOrdinal)
        {
            if (srcMap->getNumberOfLayers() > mLayers.size()) {
                std::cerr<<"Error: Source has more layers than target map"<<std::endl
                         <<"(and the command-line \"create layers\" option was not used)"<<std::endl;
                checkPassed = false;
            }
        }
        else
        {
            for (int i = 0; i < srcMap->getNumberOfLayers(); i++)
            {
                Layer* srcLayer = srcMap->getLayer(i);
                Layer* destLayer = getLayer(srcLayer->getName());
                if (!destLayer)
                {
                    std::cerr<<"Error: target map has no layer named \""<<srcLayer->getName()<<"\""<<std::endl
                         <<"(and the command-line \"create layers\" option was not used)"<<std::endl;
                    checkPassed = false;
                }
            }
        }
    }

    if (!checkPassed) return false;

    std::map<int, int> translation = addAndTranslateTilesets(srcMap);


    //combining layer information
    for (int i = 0; i < srcMap->getNumberOfLayers(); i++)
    {
        Layer* srcLayer = srcMap->getLayer(i);
        Layer* destLayer = NULL;
        if (config.copyLayersByOrdinal)
        {
            if (i < mLayers.size())
            {
                destLayer = mLayers.at(i);
            }
        }
        else
        {
            destLayer = getLayer(srcLayer->getName());
        }

        if (!destLayer)
        {
            assert(config.createMissingLayers); /* Tested earlier, in the checkPassed section */
            /* Generate a name for the new layer, which must be
             * unique in the target map, and should be unique in
             * the source map (to avoid collisions later in the
             * copying process).
             * Start by trying the name of the source layer.
             */
            std::string name = srcLayer->getName();
            if (getLayer(name))
            {
                int k=0;
                do
                {
                    name = "Layer" + toString(k);
                    k++;
                } while (getLayer(name) || srcMap->getLayer(name));
            }

            destLayer = new Layer(name, mWidth * mHeight);
            mLayers.push_back(destLayer);
            std::cout<<"Created new layer "<<name<<std::endl;
        }

        for (int y=0; y<srcHeight; y++)
        {

            for (int x=0; x<srcWidth; x++)
            {
                int srcIndex = srcMap->getWidth() * (y + srcY) + (x + srcX);
                int tgtIndex = mWidth * (y + destY) + (x + destX);
                Tile tmpTile = srcLayer->at(srcIndex);
                tmpTile.tileset = translation[tmpTile.tileset];
                destLayer->at(tgtIndex) = tmpTile;
            }
        }
    }

    std::clog<<"copying successful!"<<std::endl;
    return true;
}

bool Map::randomFill(Map* templateMap, const std::string& destLayerName,
            int destX, int destY, int destWidth, int destHeight,
            const ConfigurationOptions& config)
{
    //plausibility check of coordinates
    bool checkPassed = true;
    if (destX + destWidth > mWidth)
    {
        std::cerr<<"Error: Area exceeds right map border of target map!"<<std::endl;
        checkPassed = false;
    }
    if (destY + destHeight > mHeight)
    {
        std::cerr<<"Error: Area exceeds lower map border of target map!"<<std::endl;
        checkPassed = false;
    }
    if (destWidth < templateMap->getWidth())
    {
        std::cerr<<"Error: Template is wider than target area"<<std::endl;
        checkPassed = false;
    }
    if (destWidth < templateMap->getHeight())
    {
        std::cerr<<"Error: Template is higher than target area"<<std::endl;
        checkPassed = false;
    }
    if (templateMap->getNumberOfLayers() == 0)
    {
        std::cerr<<"Error: Template has no layers"<<std::endl;
        checkPassed = false;
    }
    if (!config.createMissingLayers && !getLayer(destLayerName))
    {
        std::cerr<<"Error: target map has no layer named \""<<destLayerName<<"\""<<std::endl
             <<"(and the command-line \"create layers\" option was not used)"<<std::endl;
        checkPassed = false;
    }
    if (!checkPassed) return false;

    std::map<int, int> translation = addAndTranslateTilesets(templateMap);


    Layer* destLayer = getLayer(destLayerName);
    if (!destLayer)
    {
        destLayer = new Layer(destLayerName, mWidth * mHeight);
        mLayers.push_back(destLayer);
        std::cout<<"Created new layer "<<destLayerName<<std::endl;
    }

    /* Now generate extra tiles.
     * TODO Need to configure this for desired density.  For 2x1 trees, dW*dH/10 is very sparse, dW*dH/2 is dense */
    srand(time(NULL));
    for (int i = destWidth*destHeight / 10; i > 0; i--)
    {
        /* Pick a random location, with enough tiles left and down from it to
         * fit the template in (the +1 is because it starts on tile (x,y))
         */
        int x = destX + (rand() % (destWidth  - templateMap->getWidth () + 1));
        int y = destY + (rand() % (destHeight - templateMap->getHeight() + 1));

        bool areaIsClear = true;

        for (int loop_y=0; loop_y<templateMap->getHeight(); loop_y++)
        {
            for (int loop_x=0; loop_x<templateMap->getWidth(); loop_x++)
            {
                if (! destLayer->getTile(x+loop_x, y+loop_y, mWidth).empty())
                {
                    areaIsClear = false;
                }
            }
        }

        if (areaIsClear)
        {
            int p = rand() % templateMap->getNumberOfLayers();
            std::cout <<"Copying pattern "<<p<<" to "<<x<<", "<<y<<std::endl;

            Layer* srcLayer = templateMap->getLayer(p);
            for (int loop_y=0; loop_y<templateMap->getHeight(); loop_y++)
            {
                for (int loop_x=0; loop_x<templateMap->getWidth(); loop_x++)
                {
                    Tile& srcTile  = srcLayer->getTile(loop_x, loop_y, templateMap->getWidth());
                    Tile& destTile = destLayer->getTile(x+loop_x, y+loop_y, mWidth);
                    destTile.tileset = translation[srcTile.tileset];
                    destTile.index = srcTile.index;
                }
            }
        }
        else
        {
            std::cout <<"Area occupied "<<x<<", "<<y<<std::endl;
        }
    }

    std::clog<<"copying successful!"<<std::endl;
    return true;
}

bool Map::translateAllLayers(Map* templateMap, const std::string& destLayerName,
            const ConfigurationOptions& config)
{
    bool checkPassed = true;
    if (templateMap->getNumberOfLayers() != 2)
    {
        std::cerr<<"Error: template should have exactly 2 layers"<<std::endl;
        checkPassed = false;
    }
    if (!config.createMissingLayers && !getLayer(destLayerName))
    {
        std::cerr<<"Error: target map has no layer named \""<<destLayerName<<"\""<<std::endl
             <<"(and the command-line \"create layers\" option was not used)"<<std::endl;
        checkPassed = false;
    }
    if (!checkPassed) return false;

    //FIXME - as is, this will add tilesets that are in the template but
    //not used in the main map
    std::map<int, int> tilesetTranslation = addAndTranslateTilesets(templateMap);

    //Lacking a better name, we'll say this is translating visible layers to collision
    std::map<Tile, Tile> collisionTranslation;

    Layer* fromLayer = templateMap->getLayer(0);
    Layer* toLayer = templateMap->getLayer(1);
    for (int xy = (templateMap->getWidth() * templateMap->getHeight() -1);
        xy >= 0; xy--)
    {
        Tile& fromTile = fromLayer->at(xy);
        Tile& toTile = toLayer->at(xy);
        if (!fromTile.empty())
        {
            collisionTranslation[fromTile] = toTile;  //FIXME - just getting it compiling so far
        }
    }

    /* Now apply that template to the game map */
    Layer* destLayer = getLayer(destLayerName);
    if (!destLayer)
    {
        destLayer = new Layer(destLayerName, mWidth * mHeight);
        mLayers.push_back(destLayer);
        std::cout<<"Created new layer "<<destLayerName<<std::endl;
    }

    for (std::vector<Layer*>::iterator layer = mLayers.begin();
         layer != mLayers.end();
         layer++)
    {
        if ((*layer)->getName() == destLayerName)
            continue;

        for (int xy = mWidth * mHeight -1; xy >= 0; xy--)
        {
            Tile& fromTile = (*layer)->at(xy);
            Tile& toTile = destLayer->at(xy);
            std::map<Tile,Tile>::iterator iteratedTile = collisionTranslation.find(fromTile);

            if (iteratedTile != collisionTranslation.end())
            {
                toTile = (*iteratedTile).second;
            }
        }

    }

    return true;
}



int Map::save(std::string filename)
{
    //remove old tileset and layer information in XML tree
    xmlNodePtr rootNode = xmlDocGetRootElement(mXmlDoc);
    std::list<xmlNodePtr> toRemove;
    for_each_xml_child_node(node, rootNode)
    {
        if (    xmlStrEqual(node->name, BAD_CAST "tileset")
            ||  xmlStrEqual(node->name, BAD_CAST "layer"))
        {
            toRemove.push_back(node);
        }
    }

    while (!toRemove.empty())
    {
        xmlUnlinkNode(toRemove.back());
        xmlFreeNode(toRemove.back());
        toRemove.pop_back();
    }

    //TODO: reorganize GIDs

    //add new tileset information to XML tree
    for (int i = 0; i< mTilesets.size(); i++)
    {
        xmlNodePtr newNode;

        xmlAddChild(rootNode, xmlNewDocText(mXmlDoc, BAD_CAST " "));
        newNode = xmlNewNode(NULL, BAD_CAST "tileset");
        xmlNewProp(newNode, BAD_CAST "name", BAD_CAST mTilesets.at(i)->name.c_str());
        xmlNewProp(newNode, BAD_CAST "firstgid", BAD_CAST toString(mTilesets.at(i)->firstgid).c_str());
        xmlNewProp(newNode, BAD_CAST "tilewidth", BAD_CAST toString(mTilesets.at(i)->tilewidth).c_str());
        xmlNewProp(newNode, BAD_CAST "tileheight", BAD_CAST toString(mTilesets.at(i)->tileheight).c_str());
        xmlAddChild(newNode, xmlNewDocText(mXmlDoc, BAD_CAST "\n  "));
        xmlNodePtr imageNode = xmlNewNode(NULL, BAD_CAST "image");
        xmlNewProp(imageNode, BAD_CAST "source", BAD_CAST mTilesets.at(i)->imagefile.c_str());
        xmlAddChild(newNode, imageNode);
        xmlAddChild(newNode, xmlNewDocText(mXmlDoc, BAD_CAST "\n "));
        xmlAddChild(rootNode, newNode);
        xmlAddChild(rootNode, xmlNewDocText(mXmlDoc, BAD_CAST "\n"));
    }

    //add new layer information to XML tree
    for (int i = 0; i < mLayers.size(); i++)
    {
        //lay out layer information in binary
        unsigned char* binData = (unsigned char*)malloc(mWidth * mHeight * 4);

        for (int a=0; a < mWidth * mHeight; a++)
        {
            Tile tile = mLayers.at(i)->at(a);
            int ldata;
            if (tile.tileset != -1)
            {
                ldata = tile.index + mTilesets.at(tile.tileset)->firstgid;
            } else {
                ldata = 0;
            }

            binData[a * 4 + 0] = (ldata & 0x000000ff);
            binData[a * 4 + 1] = (ldata & 0x0000ff00) >> 8;
            binData[a * 4 + 2] = (ldata & 0x00ff0000) >> 16;
            binData[a * 4 + 3] = (ldata & 0xff000000) >> 24;
        }


        //GZIP layer information
        /*
        unsigned char* gzipData = (unsigned char*)malloc((mWidth * mHeight * 4) + 128);
        unsigned int gzipLen;
        compressMemory (binData, (mWidth * mHeight * 4) + 128, gzipData, gzipLen);
        free (binData);
        std::cout<<"GZIP length: "<<gzipLen<<std::endl;
        */

        //encode layer information in base64
        unsigned char* base64Data;
        int base64len;
        //base64Data = php3_base64_encode(gzipData, gzipLen, &base64len);
        base64Data = php3_base64_encode(binData, (mWidth * mHeight * 4), &base64len);
        //free(gzipData);

        xmlNodePtr newNode;
        xmlAddChild(rootNode, xmlNewDocText(mXmlDoc, BAD_CAST " "));
        newNode = xmlNewNode(NULL, BAD_CAST "layer");
        xmlNewProp(newNode, BAD_CAST "name", BAD_CAST (mLayers.at(i)->getName()).c_str());
        xmlNewProp(newNode, BAD_CAST "width", BAD_CAST toString(mWidth).c_str());
        xmlNewProp(newNode, BAD_CAST "height", BAD_CAST toString(mHeight).c_str());
        xmlAddChild(newNode, xmlNewDocText(mXmlDoc, BAD_CAST "\n  "));
        xmlNodePtr dataNode = xmlNewNode(NULL, BAD_CAST "data");
        xmlNewProp(dataNode, BAD_CAST "encoding", BAD_CAST "base64");
        //xmlNewProp(dataNode, BAD_CAST "compression", BAD_CAST "gzip");
        xmlAddChild(dataNode, xmlNewDocText(mXmlDoc, BAD_CAST "\n   "));
        xmlAddChild(dataNode, xmlNewDocText(mXmlDoc, BAD_CAST base64Data));
        xmlAddChild(dataNode, xmlNewDocText(mXmlDoc, BAD_CAST "\n  "));
        xmlAddChild(newNode, dataNode);
        xmlAddChild(newNode, xmlNewDocText(mXmlDoc, BAD_CAST "\n "));
        xmlAddChild(rootNode, newNode);
        xmlAddChild(rootNode, xmlNewDocText(mXmlDoc, BAD_CAST "\n"));

        free(base64Data);
        free(binData);
    }

    //save XML tree
    int retval = xmlSaveFile(filename.c_str(), mXmlDoc);

    if (retval == -1)
    {
        std::cerr<<"Could not write outfile "<<filename<<std::endl;
        return false;
    }
    else
    {
        std::cout<<"File saved successfully to "<<filename<<std::endl;
        return true;
    }
}

Layer* Map::getLayer(std::string name)
{
    for (std::vector<Layer*>::iterator layer = mLayers.begin();
         layer != mLayers.end();
         layer++)
    {
        if ((*layer)->getName() == name)
            return *layer;
    }
    return NULL;
}

Map::~Map()
{
    for (std::vector<Layer*>::iterator layer = mLayers.begin();
         layer != mLayers.end();
         layer++)
    {
         delete *layer;
    }

    for (std::vector<Tileset*>::iterator tileset = mTilesets.begin();
         tileset != mTilesets.end();
         tileset++)
    {
         delete *tileset;
    }

    xmlFreeDoc(mXmlDoc);
}
