/*
 *  The Mana Client
 *  Copyright (C) 2010-2012  The Mana Developers
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

#include "actorsprite.h"

#include "animatedsprite.h"
#include "configuration.h"
#include "event.h"
#include "imagesprite.h"
#include "localplayer.h"
#include "log.h"
#include "particle.h"
#include "simpleanimation.h"

#include "resources/animation.h"
#include "resources/image.h"
#include "resources/imageset.h"
#include "resources/resourcemanager.h"
#include "resources/theme.h"

#include "utils/time.h"

#include <cassert>

#define EFFECTS_FILE "effects.xml"

ImageSet *ActorSprite::targetCursorImages[2][NUM_TC];
SimpleAnimation *ActorSprite::targetCursor[2][NUM_TC];
bool ActorSprite::loaded = false;

ActorSprite::ActorSprite(int id)
    : mId(id)
{}

ActorSprite::~ActorSprite()
{
    // Notify listeners of the destruction.
    Event event(Event::Destroyed);
    event.setActor("source", this);
    event.trigger(Event::ActorSpriteChannel);
}

int ActorSprite::getDrawOrder() const
{
    int drawOrder = Actor::getDrawOrder();

    // See note at ActorSprite::draw
    if (mMap)
        drawOrder += mMap->getTileHeight() / 2;

    return drawOrder;
}

bool ActorSprite::draw(Graphics *graphics, int offsetX, int offsetY) const
{
    int px = getPixelX() + offsetX;
    int py = getPixelY() + offsetY;

    if (mUsedTargetCursor)
        mUsedTargetCursor->draw(graphics, px, py);

    // This is makes sure that actors positioned on the center of a tile have
    // their sprite aligned to the bottom of that tile, mainly to maintain
    // compatibility with older clients.
    if (mMap)
        py += mMap->getTileHeight() / 2;

    return drawSpriteAt(graphics, px, py);
}

bool ActorSprite::drawSpriteAt(Graphics *graphics, int x, int y) const
{
    return mSprites.draw(graphics, x, y);
}

void ActorSprite::logic()
{
    // Update sprite animations
    mSprites.update(Time::deltaTimeMs());

    if (mUsedTargetCursor)
        mUsedTargetCursor->update(Time::deltaTimeMs());

    // See note at ActorSprite::draw
    float py = mPos.y;
    if (mMap)
        py += mMap->getTileHeight() / 2;

    // Update particle effects
    mChildParticleEffects.moveTo(mPos.x, py);
}

void ActorSprite::setMap(Map* map)
{
    Actor::setMap(map);

    // Clear particle effect list because child particles became invalid
    mChildParticleEffects.clear();
}

void ActorSprite::controlParticle(Particle *particle)
{
    mChildParticleEffects.addLocally(particle);
}

void ActorSprite::setTargetType(TargetCursorType type)
{
    if (type == TCT_NONE)
        untarget();
    else
        mUsedTargetCursor = targetCursor[type][getTargetCursorSize()];
}

void ActorSprite::setupSpriteDisplay(const SpriteDisplay &display,
                                     bool forceDisplay)
{
    mSprites.clear();

    for (const auto &sprite : display.sprites)
    {
        std::string file = paths.getStringValue("sprites") + sprite.sprite;
        mSprites.add(AnimatedSprite::load(file, sprite.variant));
    }

    // Ensure that something is shown, if desired
    if (mSprites.size() == 0 && forceDisplay)
    {
        if (display.image.empty())
        {
            mSprites.add(AnimatedSprite::load(paths.getStringValue("sprites")
                + paths.getStringValue("spriteErrorFile")));
        }
        else
        {
            ResourceManager *resman = ResourceManager::getInstance();
            std::string imagePath = paths.getStringValue("itemIcons")
                                    + display.image;
            Image *img = resman->getImage(imagePath);

            if (!img)
                img = Theme::getImageFromTheme(
                    paths.getStringValue("unknownItemFile"));

            mSprites.add(new ImageSprite(img));
        }
    }

    mChildParticleEffects.clear();

    //setup particle effects
    if (Particle::enabled)
    {
        for (const auto &particle : display.particles)
            controlParticle(particleEngine->addEffect(particle, 0, 0));
    }
}

void ActorSprite::load()
{
    if (loaded)
        unload();

    initTargetCursor();

    loaded = true;
}

void ActorSprite::unload()
{
    if (!loaded)
        return;

    cleanupTargetCursors();
    loaded = false;
}

void ActorSprite::initTargetCursor()
{
    static const std::string targetCursor = "graphics/target-cursor-%s-%s.png";
    static const char * const cursorTypeStr[NUM_TCT] = {
        "normal",
        "in-range"
    };
    static const int targetWidths[NUM_TC] = { 44, 62, 82 };
    static const int targetHeights[NUM_TC] = { 35, 44, 60 };
    static const char * const cursorSizeStr[NUM_TC] = { "s", "m", "l" };

    // Load target cursors
    for (int size = 0; size < NUM_TC; size++)
    {
        for (int type = 0; type < NUM_TCT; type++)
        {
            loadTargetCursor(strprintf(targetCursor.c_str(),
                                       cursorTypeStr[type],
                                       cursorSizeStr[size]),
                             targetWidths[size],
                             targetHeights[size], type, size);
        }
    }
}

void ActorSprite::cleanupTargetCursors()
{
    for (int size = 0; size < NUM_TC; size++)
    {
        for (int type = 0; type < NUM_TCT; type++)
        {
            delete targetCursor[type][size];
            if (targetCursorImages[type][size])
                targetCursorImages[type][size]->decRef();
        }
    }
}

void ActorSprite::loadTargetCursor(const std::string &filename,
                                   int width, int height, int type, int size)
{
    assert(size > -1);
    assert(size < 3);

    ResourceManager *resman = ResourceManager::getInstance();
    ImageSet *currentImageSet = resman->getImageSet(filename, width, height);

    if (!currentImageSet)
    {
        logger->log("Error loading target cursor: %s", filename.c_str());
        return;
    }

    Animation anim;

    for (unsigned int i = 0; i < currentImageSet->size(); ++i)
    {
        anim.addFrame(currentImageSet->get(i), DEFAULT_FRAME_DELAY,
                      -(currentImageSet->getWidth() / 2),
                      -(currentImageSet->getHeight() / 2));
    }

    auto *currentCursor = new SimpleAnimation(std::move(anim));

    targetCursorImages[type][size] = currentImageSet;
    targetCursor[type][size] = currentCursor;
}
