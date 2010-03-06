/*
 *  Gui Skinning
 *  Copyright (C) 2008  The Legend of Mazzeroth Development Team
 *  Copyright (C) 2009  Aethyra Development Team
 *  Copyright (C) 2009  The Mana World Development Team
 *  Copyright (C) 2009-2010  The Mana Developers
 *
 *  This file is part of The Mana Client.
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
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "gui/theme.h"

#include "client.h"
#include "configuration.h"
#include "log.h"

#include "resources/image.h"
#include "resources/imageset.h"
#include "resources/resourcemanager.h"

#include "utils/dtor.h"
#include "utils/stringutils.h"
#include "utils/xml.h"

#include <physfs.h>

#include <algorithm>

std::string Theme::mThemePath;
Theme *Theme::mInstance = 0;

Skin::Skin(ImageRect skin, Image *close, Image *stickyUp, Image *stickyDown,
           const std::string &filePath,
           const std::string &name):
    instances(0),
    mFilePath(filePath),
    mName(name),
    mBorder(skin),
    mCloseImage(close),
    mStickyImageUp(stickyUp),
    mStickyImageDown(stickyDown)
{
}

Skin::~Skin()
{
    // Clean up static resources
    for (int i = 0; i < 9; i++)
        delete mBorder.grid[i];

    mCloseImage->decRef();
    delete mStickyImageUp;
    delete mStickyImageDown;
}

void Skin::updateAlpha(float minimumOpacityAllowed)
{
    const float alpha = std::max((double)minimumOpacityAllowed,
                                 config.getValue("guialpha", 0.8f));

    for_each(mBorder.grid, mBorder.grid + 9,
             std::bind2nd(std::mem_fun(&Image::setAlpha), alpha));

    mCloseImage->setAlpha(alpha);
    mStickyImageUp->setAlpha(alpha);
    mStickyImageDown->setAlpha(alpha);
}

int Skin::getMinWidth() const
{
    return mBorder.grid[ImageRect::UPPER_LEFT]->getWidth() +
           mBorder.grid[ImageRect::UPPER_RIGHT]->getWidth();
}

int Skin::getMinHeight() const
{
    return mBorder.grid[ImageRect::UPPER_LEFT]->getHeight() +
           mBorder.grid[ImageRect::LOWER_LEFT]->getHeight();
}

Theme::Theme():
    mMinimumOpacity(-1.0f)
{
    config.addListener("guialpha", this);
}

Theme::~Theme()
{
    delete_all(mSkins);
    config.removeListener("guialpha", this);
}

Theme *Theme::instance()
{
    if (!mInstance)
        mInstance = new Theme;

    return mInstance;
}

void Theme::deleteInstance()
{
    delete mInstance;
    mInstance = 0;
}

Skin *Theme::load(const std::string &filename, const std::string &defaultPath)
{
    // Check if this skin was already loaded
    SkinIterator skinIterator = mSkins.find(filename);
    if (mSkins.end() != skinIterator)
    {
        skinIterator->second->instances++;
        return skinIterator->second;
    }

    Skin *skin = readSkin(filename);

    if (!skin)
    {
        // Try falling back on the defaultPath if this makes sense
        if (filename != defaultPath)
        {
            logger->log("Error loading skin '%s', falling back on default.",
                        filename.c_str());

            skin = readSkin(defaultPath);
        }

        if (!skin)
        {
            logger->error(strprintf("Error: Loading default skin '%s' failed. "
                                    "Make sure the skin file is valid.",
                                    defaultPath.c_str()));
        }
    }

    // Add the skin to the loaded skins
    mSkins[filename] = skin;

    return skin;
}

void Theme::setMinimumOpacity(float minimumOpacity)
{
    if (minimumOpacity > 1.0f) return;

    mMinimumOpacity = minimumOpacity;
    updateAlpha();
}

void Theme::updateAlpha()
{
    for (SkinIterator iter = mSkins.begin(); iter != mSkins.end(); ++iter)
        iter->second->updateAlpha(mMinimumOpacity);
}

void Theme::optionChanged(const std::string &)
{
    updateAlpha();
}

Skin *Theme::readSkin(const std::string &filename)
{
    if (filename.empty())
        return 0;

    logger->log("Loading skin '%s'.", filename.c_str());

    XML::Document doc(resolveThemePath(filename));
    xmlNodePtr rootNode = doc.rootNode();

    if (!rootNode || !xmlStrEqual(rootNode->name, BAD_CAST "skinset"))
        return 0;

    const std::string skinSetImage = XML::getProperty(rootNode, "image", "");

    if (skinSetImage.empty())
    {
        logger->log("Theme::readSkin(): Skinset does not define an image!");
        return 0;
    }

    logger->log("Theme::load(): <skinset> defines '%s' as a skin image.",
                skinSetImage.c_str());

    Image *dBorders = Theme::getImageFromTheme(skinSetImage);
    ImageRect border;

    // iterate <widget>'s
    for_each_xml_child_node(widgetNode, rootNode)
    {
        if (!xmlStrEqual(widgetNode->name, BAD_CAST "widget"))
            continue;

        const std::string widgetType =
                XML::getProperty(widgetNode, "type", "unknown");
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

                const std::string partType =
                        XML::getProperty(partNode, "type", "unknown");
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

                else
                    logger->log("Theme::readSkin(): Unknown part type '%s'",
                                partType.c_str());
            }
        }
        else
        {
            logger->log("Theme::readSkin(): Unknown widget type '%s'",
                        widgetType.c_str());
        }
    }

    dBorders->decRef();

    logger->log("Finished loading skin.");

    // Hard-coded for now until we update the above code to look for window buttons
    Image *closeImage = Theme::getImageFromTheme("close_button.png");
    Image *sticky = Theme::getImageFromTheme("sticky_button.png");
    Image *stickyImageUp = sticky->getSubImage(0, 0, 15, 15);
    Image *stickyImageDown = sticky->getSubImage(15, 0, 15, 15);
    sticky->decRef();

    Skin *skin = new Skin(border, closeImage, stickyImageUp, stickyImageDown,
                          filename);
    skin->updateAlpha(mMinimumOpacity);
    return skin;
}

bool Theme::tryThemePath(std::string themePath)
{
    if (!themePath.empty())
    {
        themePath = "graphics/gui/" + themePath;
        if (PHYSFS_exists(themePath.c_str()))
        {
            mThemePath = themePath;
            return true;
        }
    }

    return false;
}

void Theme::prepareThemePath()
{
    // Try theme from settings
    if (tryThemePath(config.getValue("theme", "")))
        return;

    // Try theme from branding
    if (tryThemePath(branding.getValue("theme", "")))
        return;

    // Use default
    mThemePath = "graphics/gui";
}

std::string Theme::resolveThemePath(const std::string &path)
{
    // Need to strip off any dye info for the existence tests
    int pos = path.find('|');
    std::string file;
    if (pos > 0)
        file = path.substr(0, pos);
    else
        file = path;

    // Might be a valid path already
    if (PHYSFS_exists(file.c_str()))
        return path;

    // Try the theme
    file = getThemePath() + "/" + file;
    if (PHYSFS_exists(file.c_str()))
        return getThemePath() + "/" + path;

    // Backup
    return "graphics/gui/" + path;
}

Image *Theme::getImageFromTheme(const std::string &path)
{
    ResourceManager *resman = ResourceManager::getInstance();
    return resman->getImage(resolveThemePath(path));
}

ImageSet *Theme::getImageSetFromTheme(const std::string &path,
                                        int w, int h)
{
    ResourceManager *resman = ResourceManager::getInstance();
    return resman->getImageSet(resolveThemePath(path), w, h);
}
