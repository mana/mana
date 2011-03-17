/*
 *  The Mana Client
 *  Copyright (C) 2010  The Mana Developers
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

#include "client.h"
#include "event.h"
#include "imagesprite.h"
#include "localplayer.h"
#include "log.h"
#include "simpleanimation.h"
#include "sound.h"
#include "statuseffect.h"

#include "net/net.h"

#include "resources/image.h"
#include "resources/imageset.h"
#include "resources/resourcemanager.h"
#include "resources/theme.h"

#include <cassert>

#define EFFECTS_FILE "effects.xml"

ImageSet *ActorSprite::targetCursorImages[2][NUM_TC];
SimpleAnimation *ActorSprite::targetCursor[2][NUM_TC];
bool ActorSprite::loaded = false;

ActorSprite::ActorSprite(int id):
    mId(id),
    mStunMode(0),
    mStatusParticleEffects(&mStunParticleEffects, false),
    mChildParticleEffects(&mStatusParticleEffects, false),
    mMustResetParticles(false),
    mUsedTargetCursor(NULL)
{}

ActorSprite::~ActorSprite()
{
    setMap(NULL);

    mUsedTargetCursor = NULL;

    // Notify listeners of the destruction.
    Mana::Event event(EVENT_DESTROYED);
    event.setActor("source", this);
    event.trigger(CHANNEL_ACTORSPRITE);
}

bool ActorSprite::draw(Graphics *graphics, int offsetX, int offsetY) const
{
    // TODO: Eventually, we probably should fix all sprite offsets so that
    //       these translations aren't necessary anymore. The sprites know
    //       best where their base point should be.
    const int px = getPixelX() + offsetX - 16;
    // Temporary fix to the Y offset.
    const int py = getPixelY() + offsetY - 16;

    if (mUsedTargetCursor)
    {
        mUsedTargetCursor->reset();
        mUsedTargetCursor->update(tick_time * MILLISECONDS_IN_A_TICK);
        mUsedTargetCursor->draw(graphics, px, py);
    }

    return drawSpriteAt(graphics, px, py);
}

bool ActorSprite::drawSpriteAt(Graphics *graphics, int x, int y) const
{
    return CompoundSprite::draw(graphics, x, y);
}

void ActorSprite::logic()
{
    // Update sprite animations
    update(tick_time * MILLISECONDS_IN_A_TICK);

    // Restart status/particle effects, if needed
    if (mMustResetParticles)
    {
        mMustResetParticles = false;
        for (std::set<int>::iterator it = mStatusEffects.begin();
             it != mStatusEffects.end(); it++)
        {
            const StatusEffect *effect = StatusEffect::getStatusEffect(*it, true);
            if (effect && effect->particleEffectIsPersistent())
                updateStatusEffect(*it, true);
        }
    }

    // Update particle effects
    mChildParticleEffects.moveTo(mPos.x, mPos.y);
}

void ActorSprite::actorLogic()
{
}

void ActorSprite::setMap(Map* map)
{
    Actor::setMap(map);

    // Clear particle effect list because child particles became invalid
    mChildParticleEffects.clear();
    mMustResetParticles = true; // Reset status particles on next redraw
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

struct EffectDescription {
    std::string mGFXEffect;
    std::string mSFXEffect;
};

static EffectDescription *default_effect = NULL;
static std::map<int, EffectDescription *> effects;
static bool effects_initialized = false;

static EffectDescription *getEffectDescription(xmlNodePtr node, int *id)
{
    EffectDescription *ed = new EffectDescription;

    *id = atoi(XML::getProperty(node, "id", "-1").c_str());
    ed->mSFXEffect = XML::getProperty(node, "audio", "");
    ed->mGFXEffect = XML::getProperty(node, "particle", "");

    return ed;
}

static EffectDescription *getEffectDescription(int effectId)
{
    if (!effects_initialized)
    {
        XML::Document doc(EFFECTS_FILE);
        xmlNodePtr root = doc.rootNode();

        if (!root || !xmlStrEqual(root->name, BAD_CAST "being-effects"))
        {
            logger->log("Error loading being effects file: "
                    EFFECTS_FILE);
            return NULL;
        }

        for_each_xml_child_node(node, root)
        {
            int id;

            if (xmlStrEqual(node->name, BAD_CAST "effect"))
            {
                EffectDescription *EffectDescription =
                    getEffectDescription(node, &id);
                effects[id] = EffectDescription;
            }
            else if (xmlStrEqual(node->name, BAD_CAST "default"))
            {
                EffectDescription *effectDescription =
                    getEffectDescription(node, &id);

                if (default_effect)
                    delete default_effect;

                default_effect = effectDescription;
            }
        }

        effects_initialized = true;
    } // done initializing

    EffectDescription *ed = effects[effectId];

    return ed ? ed : default_effect;
}

void ActorSprite::setStatusEffect(int index, bool active)
{
    const bool wasActive = mStatusEffects.find(index) != mStatusEffects.end();

    if (active != wasActive)
    {
        updateStatusEffect(index, active);
        if (active)
            mStatusEffects.insert(index);
        else
            mStatusEffects.erase(index);
    }
}

void ActorSprite::setStatusEffectBlock(int offset, Uint16 newEffects)
{
    for (int i = 0; i < STATUS_EFFECTS; i++)
    {
        int index = StatusEffect::blockEffectIndexToEffectIndex(offset + i);

        if (index != -1)
            setStatusEffect(index, (newEffects & (1 << i)) > 0);
    }
}

void ActorSprite::internalTriggerEffect(int effectId, bool sfx, bool gfx)
{
    logger->log("Special effect #%d on %s", effectId,
                getId() == player_node->getId() ? "self" : "other");

    EffectDescription *ed = getEffectDescription(effectId);

    if (!ed)
    {
        logger->log("Unknown special effect and no default recorded");
        return;
    }

    if (gfx && !ed->mGFXEffect.empty())
    {
        Particle *selfFX;

        selfFX = particleEngine->addEffect(ed->mGFXEffect, 0, 0);
        controlParticle(selfFX);
    }

    if (sfx && !ed->mSFXEffect.empty())
        sound.playSfx(ed->mSFXEffect);
}

void ActorSprite::updateStunMode(int oldMode, int newMode)
{
    if (this == player_node)
    {
        Mana::Event event(EVENT_STUN);
        event.setInt("oldMode", oldMode);
        event.setInt("newMode", newMode);
        event.trigger(CHANNEL_ACTORSPRITE);
    }

    handleStatusEffect(StatusEffect::getStatusEffect(oldMode, false), -1);
    handleStatusEffect(StatusEffect::getStatusEffect(newMode, true), -1);
}

void ActorSprite::updateStatusEffect(int index, bool newStatus)
{
    if (this == player_node)
    {
        Mana::Event event(EVENT_UPDATESTATUSEFFECT);
        event.setInt("index", index);
        event.setBool("newStatus", newStatus);
        event.trigger(CHANNEL_ACTORSPRITE);
    }

    handleStatusEffect(StatusEffect::getStatusEffect(index, newStatus), index);
}

void ActorSprite::handleStatusEffect(StatusEffect *effect, int effectId)
{
    if (!effect)
        return;

    // TODO: Find out how this is meant to be used
    // (SpriteAction != Being::Action)
    //SpriteAction action = effect->getAction();
    //if (action != ACTION_INVALID)
    //    setAction(action);

    Particle *particle = effect->getParticle();

    if (effectId >= 0)
    {
        mStatusParticleEffects.setLocally(effectId, particle);
    }
    else
    {
        mStunParticleEffects.clearLocally();
        if (particle)
            mStunParticleEffects.addLocally(particle);
    }
}

void ActorSprite::setupSpriteDisplay(const SpriteDisplay &display,
                                     bool forceDisplay)
{
    clear();

    SpriteRefs it, it_end;

    for (it = display.sprites.begin(), it_end = display.sprites.end();
         it != it_end; it++)
    {
        std::string file = "graphics/sprites/" + (*it)->sprite;
        int variant = (*it)->variant;
        addSprite(AnimatedSprite::load(file, variant));
    }

    // Ensure that something is shown, if desired
    if (size() == 0 && forceDisplay)
    {
        if (display.image.empty())
            addSprite(AnimatedSprite::load("graphics/sprites/error.xml"));
        else
        {
            ResourceManager *resman = ResourceManager::getInstance();
            std::string imagePath = "graphics/items/" + display.image;
            Image *img = resman->getImage(imagePath);

            if (!img)
                img = Theme::getImageFromTheme("unknown-item.png");

            addSprite(new ImageSprite(img));
        }
    }

    mChildParticleEffects.clear();

    //setup particle effects
    if (Particle::enabled)
    {
        std::list<std::string>::const_iterator it, it_end;
        for (it = display.particles.begin(), it_end = display.particles.end();
             it != it_end; it++)
        {
            Particle *p = particleEngine->addEffect(*it, 0, 0);
            controlParticle(p);
        }
    }

    mMustResetParticles = true;
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

static const char *cursorType(int type)
{
    switch (type)
    {
    case ActorSprite::TCT_IN_RANGE:
        return "in-range";
    case ActorSprite::TCT_NORMAL:
        return "normal";
    default:
        assert(false);
    }
}

static const char *cursorSize(int size)
{
    switch (size)
    {
    case ActorSprite::TC_LARGE:
        return "l";
    case ActorSprite::TC_MEDIUM:
        return "m";
    case ActorSprite::TC_SMALL:
        return "s";
    default:
        assert(false);
    }
}

void ActorSprite::initTargetCursor()
{
    static std::string targetCursor = "graphics/target-cursor-%s-%s.png";
    static int targetWidths[NUM_TC] = {44, 62, 82};
    static int targetHeights[NUM_TC] = {35, 44, 60};

    // Load target cursors
    for (int size = TC_SMALL; size < NUM_TC; size++)
    {
        for (int type = TCT_NORMAL; type < NUM_TCT; type++)
        {
            loadTargetCursor(strprintf(targetCursor.c_str(), cursorType(type),
                                       cursorSize(size)), targetWidths[size],
                             targetHeights[size], type, size);
        }
    }
}

void ActorSprite::cleanupTargetCursors()
{
    for (int size = TC_SMALL; size < NUM_TC; size++)
    {
        for (int type = TCT_NORMAL; type < NUM_TCT; type++)
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

    Animation *anim = new Animation;

    for (unsigned int i = 0; i < currentImageSet->size(); ++i)
    {
        anim->addFrame(currentImageSet->get(i), 75,
                      (16 - (currentImageSet->getWidth() / 2)),
                      (16 - (currentImageSet->getHeight() / 2)));
    }

    SimpleAnimation *currentCursor = new SimpleAnimation(anim);

    targetCursorImages[type][size] = currentImageSet;
    targetCursor[type][size] = currentCursor;
}
