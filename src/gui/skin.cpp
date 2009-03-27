/*
 *  Aethyra
 *  Copyright (C) 2009  Aethyra Development Team
 *
 *  This file is part of Aethyra.
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

#include <algorithm>

#include "skin.h"

#include "../configuration.h"
#include "../configlistener.h"
#include "../log.h"

#include "../resources/image.h"
#include "../resources/resourcemanager.h"

#include "../utils/dtor.h"
#include "../utils/strprintf.h"
#include "../utils/xml.h"

SkinLoader *skinLoader = NULL;
ConfigListener *SkinLoader::skinConfigListener = NULL;

class SkinConfigListener : public ConfigListener
{
    void optionChanged(const std::string &)
    {
        if (skinLoader)
            skinLoader->updateAlpha();
    }
};

Skin::Skin(ImageRect skin, Image *close, std::string filePath, std::string name):
    instances(0),
    mFilePath(filePath),
    mName(name),
    border(skin),
    closeImage(close)
{
}

Skin::~Skin()
{
    // Clean up static resources
    for (int i = 0; i < 9; i++)
    {
        delete border.grid[i];
        border.grid[i] = NULL;
    }

    closeImage->decRef();
}

void Skin::updateAlpha()
{
    const float alpha = config.getValue("guialpha", 0.8);

    for_each(border.grid, border.grid + 9,
             std::bind2nd(std::mem_fun(&Image::setAlpha), alpha));
    closeImage->setAlpha(alpha);
}

int Skin::getMinWidth() const
{
    return (border.grid[0]->getWidth() + border.grid[1]->getWidth()) +
            border.grid[2]->getWidth();
}

int Skin::getMinHeight() const
{
    return (border.grid[0]->getHeight() + border.grid[3]->getHeight()) +
            border.grid[6]->getHeight();
}

Skin *SkinLoader::load(const std::string &filename,
                       const std::string &defaultPath)
{
    std::string filePath = filename;

    ResourceManager *resman = ResourceManager::getInstance();

    logger->log("Loading Skin '%s'.", filename.c_str());

    if (filename.empty() && defaultPath.empty())
        logger->error("SkinLoader::load(): Invalid File Name.");

    XML::Document *doc = new XML::Document(filePath);
    xmlNodePtr rootNode = doc->rootNode();

    if (!rootNode || !xmlStrEqual(rootNode->name, BAD_CAST "skinset"))
    {
        filePath = defaultPath;

        logger->log("Widget Skinning error. Loading '%s' instead.",
                    filePath.c_str());

        delete doc;

        doc = new XML::Document(filePath);
        rootNode = doc->rootNode();
        if (!rootNode || !xmlStrEqual(rootNode->name, BAD_CAST "skinset"))
        {
            logger->error(strprintf("Skinning failed. Check this skin file "
                                    "to make sure it's valid: %s",
                                    filePath.c_str()));
        }
    }

    SkinIterator skinIterator = mSkins.find(filePath);

    if (mSkins.end() != skinIterator)
    {
        skinIterator->second->instances++;
        return skinIterator->second;
    }

    std::string skinSetImage;
    skinSetImage = XML::getProperty(rootNode, "image", "");
    Image *dBorders = NULL;
    ImageRect border;

    if (!skinSetImage.empty())
    {
        logger->log("SkinLoader::load(): <skinset> defines "
                    "'%s' as a skin image.", skinSetImage.c_str());
        dBorders = resman->getImage("graphics/gui/" + skinSetImage);
    }
    else
    {
        logger->error("SkinLoader::load(): Skinset does not define an image!");
    }

    //iterate <widget>'s
    for_each_xml_child_node(widgetNode, rootNode)
    {
        if (!xmlStrEqual(widgetNode->name, BAD_CAST "widget"))
            continue;

        std::string widgetType;
        widgetType = XML::getProperty(widgetNode, "type", "unknown");
        if (widgetType == "Window")
        {
            // Iterate through <part>'s
            // LEEOR / TODO:
            // We need to make provisions to load in a CloseButton image. For 
            // now it can just be hard-coded.
            for_each_xml_child_node(partNode, widgetNode)
            {
                if (!xmlStrEqual(partNode->name, BAD_CAST "part"))
                    continue;

                std::string partType;
                partType = XML::getProperty(partNode, "type", "unknown");
                // TOP ROW
                const int xPos = XML::getProperty(partNode, "xpos", 0);
                const int yPos = XML::getProperty(partNode, "ypos", 0);
                const int width = XML::getProperty(partNode, "width", 1);
                const int height = XML::getProperty(partNode, "height", 1);

                if (partType == "top-left-corner")
                    border.grid[0] = dBorders->getSubImage(xPos, yPos, width, height);
                else if (partType == "top-edge")
                    border.grid[1] = dBorders->getSubImage(xPos, yPos, width, height);
                else if (partType == "top-right-corner")
                    border.grid[2] = dBorders->getSubImage(xPos, yPos, width, height);

                // MIDDLE ROW
                else if (partType == "left-edge")
                    border.grid[3] = dBorders->getSubImage(xPos, yPos, width, height);
                else if (partType == "bg-quad")
                    border.grid[4] = dBorders->getSubImage(xPos, yPos, width, height);
                else if (partType == "right-edge")
                    border.grid[5] = dBorders->getSubImage(xPos, yPos, width, height);

                // BOTTOM ROW
                else if (partType == "bottom-left-corner")
                    border.grid[6] = dBorders->getSubImage(xPos, yPos, width, height);
                else if (partType == "bottom-edge")
                    border.grid[7] = dBorders->getSubImage(xPos, yPos, width, height);
                else if (partType == "bottom-right-corner")
                    border.grid[8] = dBorders->getSubImage(xPos, yPos, width, height);

                // Part is of an uknown type.
                else
                    logger->log("SkinLoader::load(): Unknown Part Type '%s'", partType.c_str());
            }
        }
        // Widget is of an uknown type.
        else
        {
            logger->log("SkinLoader::load(): Unknown Widget Type '%s'", widgetType.c_str());
        }
    }
    dBorders->decRef();

    logger->log("Finished loading Skin.");

    delete doc;

    // Hard-coded for now until we update the above code to look for window buttons.
    Image *closeImage = resman->getImage("graphics/gui/close_button.png");

    Skin *skin = new Skin(border, closeImage, filename);

    mSkins[filename] = skin;

    updateAlpha();

    return skin;
}

SkinLoader::SkinLoader() :
    mSkins()
{
    skinConfigListener = new SkinConfigListener();
    // Send GUI alpha changed for initialization
    skinConfigListener->optionChanged("guialpha");
    config.addListener("guialpha", skinConfigListener);
}

SkinLoader::~SkinLoader()
{
    delete_all(mSkins);
    config.removeListener("guialpha", skinConfigListener);
    delete skinConfigListener;
    skinConfigListener = NULL;
}

void SkinLoader::updateAlpha()
{
    for (SkinIterator iter = mSkins.begin(); iter != mSkins.end(); ++iter)
    {
        iter->second->updateAlpha();
    }
}

