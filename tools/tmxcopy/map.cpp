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

#include <iostream>
#include <map>
#include <list>

#include <string.h>
#include <zlib.h>

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
            Layer* layer = new Layer;
            layer->resize(mWidth * mHeight);
            //build layer information
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
                                if (mTilesets.at(s)->firstgid < gid)
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

bool Map::overwrite(  Map* srcMap,
                    int srcX, int srcY, int srcWidth, int srcHeight,
                    int destX, int destY)
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
    if (!checkPassed) return false;

    std::map<int, int> translation;
    translation[-1] = -1;
    std::vector<Tileset*>* srcTilesets = srcMap->getTilesets();

    //add new tilesets and add redundant tilesets to list of redundand tilesets
    for (int a = 0; a < srcTilesets->size(); a++)
    {
        int b;
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
            srcTilesets->at(a)->firstgid = mMaxGid;//it is possible to get some holes in the gid index this way but who cares, we got 32bit.
            mTilesets.push_back(srcTilesets->at(a));
        }
        translation[a] = b;
    }

    //combining layer information
    for (int i = 0; i < srcMap->getNumberOfLayers(); i++)
    {
        Layer* srcLayer = srcMap->getLayer(i);
        Layer* destLayer = mLayers.at(i);

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
        xmlNewProp(newNode, BAD_CAST "name", BAD_CAST ("Layer" + toString(i)).c_str());
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
