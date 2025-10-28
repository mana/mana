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

#include "resources/spritedef.h"

#include "log.h"
#include "sprite.h"

#include "resources/animation.h"
#include "resources/dye.h"
#include "resources/image.h"
#include "resources/resourcemanager.h"

#include "configuration.h"

#include "utils/dtor.h"
#include "utils/xml.h"

#include <set>

static std::set<std::string> processedFiles;


Action::Action() = default;
Action::~Action()
{
    delete_all(mAnimations);
}

Animation *Action::getAnimation(int direction) const
{
    auto i = mAnimations.find(direction);

    // When the given direction is not available, return the first one.
    // (either DEFAULT, or more usually DOWN).
    if (i == mAnimations.end())
        i = mAnimations.begin();

    return (i == mAnimations.end()) ? nullptr : i->second;
}

void Action::setAnimation(int direction, Animation *animation)
{
    mAnimations[direction] = animation;
}


Action *SpriteDef::getAction(const std::string &action) const
{
    auto i = mActions.find(action);

    if (i == mActions.end())
    {
        Log::warn("No action \"%s\" defined!", action.c_str());
        return nullptr;
    }

    return i->second;
}

SpriteDef *SpriteDef::load(const std::string &animationFile, int variant)
{
    std::string::size_type pos = animationFile.find('|');
    std::string palettes;
    if (pos != std::string::npos)
        palettes = animationFile.substr(pos + 1);

    processedFiles.clear();
    processedFiles.insert(animationFile);

    XML::Document doc(animationFile.substr(0, pos));
    XML::Node rootNode = doc.rootNode();

    if (!rootNode || rootNode.name() != "sprite")
    {
        Log::info("Error, failed to parse %s", animationFile.c_str());

        std::string errorFile = paths.getStringValue("sprites")
                                + paths.getStringValue("spriteErrorFile");
        if (animationFile != errorFile)
        {
            return load(errorFile, 0);
        }

        return nullptr;
    }

    auto *def = new SpriteDef;
    def->loadSprite(rootNode, variant, palettes);
    def->substituteActions();
    return def;
}

void SpriteDef::substituteAction(std::string complete, std::string with)
{
    if (mActions.find(complete) == mActions.end())
    {
        auto i = mActions.find(with);
        if (i != mActions.end())
        {
            mActions[complete] = i->second;
        }
    }
}

void SpriteDef::substituteActions()
{
    substituteAction(SpriteAction::STAND, SpriteAction::DEFAULT);
    substituteAction(SpriteAction::MOVE, SpriteAction::STAND);
    substituteAction(SpriteAction::ATTACK, SpriteAction::STAND);
    substituteAction(SpriteAction::CAST_MAGIC, SpriteAction::ATTACK);
    substituteAction(SpriteAction::USE_ITEM, SpriteAction::CAST_MAGIC);
    substituteAction(SpriteAction::SIT, SpriteAction::STAND);
    substituteAction(SpriteAction::SLEEP, SpriteAction::SIT);
    substituteAction(SpriteAction::HURT, SpriteAction::STAND);
    substituteAction(SpriteAction::DEAD, SpriteAction::HURT);
}

void SpriteDef::loadSprite(XML::Node spriteNode, int variant,
                           const std::string &palettes)
{
    // Get the variant
    const int variantCount = spriteNode.getProperty("variants", 0);
    int variant_offset = 0;

    if (variantCount > 0 && variant < variantCount)
    {
        variant_offset =
            variant * spriteNode.getProperty("variant_offset", 0);
    }

    for (auto node : spriteNode.children())
    {
        if (node.name() == "imageset")
        {
            loadImageSet(node, palettes);
        }
        else if (node.name() == "action")
        {
            loadAction(node, variant_offset);
        }
        else if (node.name() == "include")
        {
            includeSprite(node);
        }
    }
}

void SpriteDef::loadImageSet(XML::Node node, const std::string &palettes)
{
    const std::string name = node.getProperty("name", "");

    // We don't allow redefining image sets. This way, an included sprite
    // definition will use the already loaded image set with the same name.
    if (mImageSets.find(name) != mImageSets.end())
        return;

    const int width = node.getProperty("width", 0);
    const int height = node.getProperty("height", 0);
    std::string imageSrc = node.getProperty("src", "");
    Dye::instantiate(imageSrc, palettes);

    ResourceManager *resman = ResourceManager::getInstance();
    auto imageSet = resman->getImageSet(imageSrc, width, height);
    if (!imageSet)
    {
        Log::critical(strprintf("Couldn't load imageset (%s)!",
                                imageSrc.c_str()));
    }

    imageSet->setOffsetX(node.getProperty("offsetX", 0));
    imageSet->setOffsetY(node.getProperty("offsetY", 0));
    mImageSets[name] = imageSet;
}

void SpriteDef::loadAction(XML::Node node, int variant_offset)
{
    const std::string actionName = node.getProperty("name", "");
    const std::string imageSetName = node.getProperty("imageset", "");

    auto si = mImageSets.find(imageSetName);
    if (si == mImageSets.end())
    {
        Log::warn("imageset \"%s\" not defined in %s",
                  imageSetName.c_str(), getIdPath().c_str());
        return;
    }
    ImageSet *imageSet = si->second;

    if (actionName == SpriteAction::INVALID)
    {
        Log::warn("Unknown action \"%s\" defined in %s",
                  actionName.c_str(), getIdPath().c_str());
        return;
    }
    auto *action = new Action;
    mActions[actionName] = action;

    // When first action set it as default direction
    if (mActions.size() == 1)
    {
        mActions[SpriteAction::DEFAULT] = action;
    }

    // Load animations
    for (auto animationNode : node.children())
    {
        if (animationNode.name() == "animation")
        {
            loadAnimation(animationNode, action, imageSet, variant_offset);
        }
    }
}

void SpriteDef::loadAnimation(XML::Node animationNode,
                              Action *action, ImageSet *imageSet,
                              int variant_offset)
{
    const std::string directionName =
        animationNode.getProperty("direction", "");
    const SpriteDirection directionType = makeSpriteDirection(directionName);

    if (directionType == DIRECTION_INVALID)
    {
        Log::warn("Unknown direction \"%s\" used in %s",
                  directionName.c_str(), getIdPath().c_str());
        return;
    }

    auto *animation = new Animation;
    action->setAnimation(directionType, animation);

    // Get animation frames
    for (auto frameNode : animationNode.children())
    {
        const int delay = frameNode.getProperty("delay",
                                           DEFAULT_FRAME_DELAY);
        int offsetX = frameNode.getProperty("offsetX", 0) +
                imageSet->getOffsetX();
        int offsetY = frameNode.getProperty("offsetY", 0) +
                imageSet->getOffsetY();

        if (frameNode.name() == "frame")
        {
            const int index = frameNode.getProperty("index", -1);

            if (index < 0)
            {
                Log::info("No valid value for 'index'");
                continue;
            }

            Image *img = imageSet->get(index + variant_offset);

            if (!img)
            {
                Log::info("No image at index %d", index + variant_offset);
                continue;
            }

            animation->addFrame(img, delay, offsetX, offsetY);
        }
        else if (frameNode.name() == "sequence")
        {
            int start = frameNode.getProperty("start", -1);
            const int end = frameNode.getProperty("end", -1);

            if (start < 0 || end < 0)
            {
                Log::info("No valid value for 'start' or 'end'");
                continue;
            }

            while (end >= start)
            {
                Image *img = imageSet->get(start + variant_offset);

                if (!img)
                {
                    Log::info("No image at index %d", start + variant_offset);
                    break;
                }

                animation->addFrame(img, delay, offsetX, offsetY);
                start++;
            }
        }
        else if (frameNode.name() == "end")
        {
            animation->addTerminator();
        }
    } // for frameNode
}

void SpriteDef::includeSprite(XML::Node includeNode)
{
    std::string filename = includeNode.getProperty("file", "");

    if (filename.empty())
        return;
    filename = paths.getStringValue("sprites") + filename;

    if (processedFiles.find(filename) != processedFiles.end())
    {
        Log::info("Error, Tried to include %s which already is included.",
                  filename.c_str());
        return;
    }
    processedFiles.insert(filename);

    XML::Document doc(filename);
    XML::Node rootNode = doc.rootNode();

    if (!rootNode || rootNode.name() != "sprite")
    {
        Log::info("Error, no sprite root node in %s", filename.c_str());
        return;
    }

    loadSprite(rootNode, 0);
}

SpriteDef::~SpriteDef()
{
    // Actions are shared, so ensure they are deleted only once.
    std::set<Action*> actions;
    for (const auto &action : mActions)
    {
        actions.insert(action.second);
    }

    for (auto action : actions)
    {
        delete action;
    }
}

SpriteDirection SpriteDef::makeSpriteDirection(const std::string &direction)
{
    if (direction.empty() || direction == "default")
        return DIRECTION_DEFAULT;
    if (direction == "up")
        return DIRECTION_UP;
    if (direction == "left")
        return DIRECTION_LEFT;
    if (direction == "right")
        return DIRECTION_RIGHT;
    if (direction == "down")
        return DIRECTION_DOWN;

    return DIRECTION_INVALID;
}
