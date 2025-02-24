/*
 *  The Mana Client
 *  Copyright (C) 2004-2009  The Mana World Development Team
 *  Copyright (C) 2009-2012  The Mana Developers
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

#include "resources/animation.h"

#include "dye.h"
#include "game.h"
#include "log.h"
#include "resourcemanager.h"

void Animation::addFrame(Image *image, int delay, int offsetX, int offsetY)
{
    auto &frame = mFrames.emplace_back();
    frame.image = image;
    frame.delay = delay;
    frame.offsetX = offsetX;
    frame.offsetY = offsetY;

    mDuration += delay;
}

void Animation::addTerminator()
{
    addFrame(nullptr, 0, 0, 0);
}

bool Animation::isTerminator(const Frame &candidate)
{
    return candidate.image == nullptr;
}

Animation Animation::fromXML(XML::Node node, const std::string &dyePalettes)
{
    Animation animation;

    std::string imagePath = node.getProperty("imageset", std::string());

    // Instanciate the dye coloration.
    Dye::instantiate(imagePath, dyePalettes);

    auto imageSet = ResourceManager::getInstance()->getImageSet(
        imagePath,
        node.getProperty("width", 0),
        node.getProperty("height", 0)
    );

    if (!imageSet)
        return animation;

    // Get animation frames
    for (auto frameNode : node.children())
    {
        int delay = frameNode.getProperty("delay", 0);
        int offsetX = frameNode.getProperty("offsetX", 0);
        int offsetY = frameNode.getProperty("offsetY", 0);
        Game *game = Game::instance();
        if (game)
        {
            offsetX -= imageSet->getWidth() / 2 - game->getCurrentTileWidth() / 2;
            offsetY -= imageSet->getHeight() - game->getCurrentTileHeight();
        }

        if (frameNode.name() == "frame")
        {
            int index = frameNode.getProperty("index", -1);

            if (index < 0)
            {
                logger->log("No valid value for 'index'");
                continue;
            }

            Image *img = imageSet->get(index);

            if (!img)
            {
                logger->log("No image at index %d", index);
                continue;
            }

            animation.addFrame(img, delay, offsetX, offsetY);
        }
        else if (frameNode.name() == "sequence")
        {
            int start = frameNode.getProperty("start", -1);
            int end = frameNode.getProperty("end", -1);

            if (start < 0 || end < 0)
            {
                logger->log("No valid value for 'start' or 'end'");
                continue;
            }

            while (end >= start)
            {
                Image *img = imageSet->get(start);

                if (!img)
                {
                    logger->log("No image at index %d", start);
                    continue;
                }

                animation.addFrame(img, delay, offsetX, offsetY);
                start++;
            }
        }
        else if (frameNode.name() == "end")
        {
            animation.addTerminator();
        }
    }

    animation.mImageSet = imageSet;

    return animation;
}
