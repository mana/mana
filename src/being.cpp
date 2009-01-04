/*
 *  The Mana World
 *  Copyright 2004 The Mana World Development Team
 *
 *  This file is part of The Mana World.
 *
 *  The Mana World is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  any later version.
 *
 *  The Mana World is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with The Mana World; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#include "being.h"

#include "animatedsprite.h"
#include "equipment.h"
#include "game.h"
#include "graphics.h"
#include "log.h"
#include "map.h"
#include "particle.h"
#include "sound.h"
#include "localplayer.h"
#include "text.h"
#include "statuseffect.h"

#include "resources/itemdb.h"
#include "resources/resourcemanager.h"
#include "resources/imageset.h"
#include "resources/iteminfo.h"

#include "gui/gui.h"

#include "utils/dtor.h"
#include "utils/tostring.h"

#include "utils/xml.h"

#include <cassert>

#define BEING_EFFECTS_FILE "effects.xml"
#define HAIR_FILE "hair.xml"

int Being::instances = 0;
ImageSet *Being::emotionSet = NULL;

static const int X_SPEECH_OFFSET = 18;
static const int Y_SPEECH_OFFSET = 60;

Being::Being(int id, int job, Map *map):
    mJob(job),
    mX(0), mY(0),
    mAction(0),
    mWalkTime(0),
    mEmotion(0), mEmotionTime(0),
    mAttackSpeed(350),
    mEquipment(new Equipment()),
    mId(id),
    mWalkSpeed(150),
    mDirection(DOWN),
    mMap(NULL),
    mEquippedWeapon(NULL),
    mHairStyle(1), mHairColor(0),
    mGender(2),
    mSpeechTime(0),
    mPx(0), mPy(0),
    mStunMode(0),
    mSprites(VECTOREND_SPRITE, NULL),
    mSpriteIDs(VECTOREND_SPRITE, 0),
    mSpriteColors(VECTOREND_SPRITE, ""),
    mStatusParticleEffects(&mStunParticleEffects, false),
    mChildParticleEffects(&mStatusParticleEffects, false),
    mMustResetParticles(false)
{
    setMap(map);

    if (instances == 0)
    {
        // Load the emotion set
        ResourceManager *rm = ResourceManager::getInstance();
        emotionSet = rm->getImageSet("graphics/gui/emotions.png", 30, 32);
        if (!emotionSet) logger->error("Unable to load emotions!");
    }

    instances++;
    mSpeech = 0;
}

Being::~Being()
{
    delete_all(mSprites);
    clearPath();

    setMap(NULL);

    instances--;

    if (instances == 0)
    {
        emotionSet->decRef();
        emotionSet = NULL;
    }

    delete mSpeech;
}

void Being::setDestination(Uint16 destX, Uint16 destY)
{
    if (mMap)
    {
        setPath(mMap->findPath(mX, mY, destX, destY));
    }
}

void Being::clearPath()
{
    mPath.clear();
}

void Being::setPath(const Path &path)
{
    mPath = path;

    if (mAction != WALK && mAction != DEAD)
    {
        nextStep();
        mWalkTime = tick_time;
    }
}

void Being::setHairStyle(int style, int color)
{
    mHairStyle = style < 0 ? mHairStyle : style % getHairStylesNr();
    mHairColor = color < 0 ? mHairColor : color % getHairColorsNr();
}

void Being::setSprite(int slot, int id, std::string color)
{
    assert(slot >= BASE_SPRITE && slot < VECTOREND_SPRITE);
    mSpriteIDs[slot] = id;
    mSpriteColors[slot] = color;
}

void Being::setSpeech(const std::string &text, Uint32 time)
{
    // don't introduce a memory leak
    delete mSpeech;

    mSpeech = new Text(text, mPx + X_SPEECH_OFFSET, mPy - Y_SPEECH_OFFSET,
                       gcn::Graphics::CENTER, speechFont,
                       gcn::Color(255, 255, 255));
    mSpeechTime = 500;
}

void Being::takeDamage(int amount)
{
    gcn::Font *font;
    std::string damage = amount ? toString(amount) : "miss";

    // Selecting the right color
    if (damage == "miss")
    {
        font = hitYellowFont;
    }
    else
    {
        // Hit particle effect
        controlParticle(particleEngine->addEffect(
                    "graphics/particles/hit.particle.xml", 0, 0));

        if (getType() == MONSTER)
        {
            font = hitBlueFont;
        }
        else
        {
            font = hitRedFont;
        }
    }

    // Show damage number
    particleEngine->addTextSplashEffect(damage, 255, 255, 255, font,
                                        mPx + 16, mPy + 16);
}

void Being::handleAttack(Being *victim, int damage)
{
    setAction(Being::ATTACK);
    mFrame = 0;
    mWalkTime = tick_time;
}

void Being::setMap(Map *map)
{
    // Remove sprite from potential previous map
    if (mMap)
    {
        mMap->removeSprite(mSpriteIterator);
    }

    mMap = map;

    // Add sprite to potential new map
    if (mMap)
    {
        mSpriteIterator = mMap->addSprite(this);
    }

    // Clear particle effect list because child particles became invalid
    mChildParticleEffects.clear();
    mMustResetParticles = true; // Reset status particles on next redraw
}

void Being::controlParticle(Particle *particle)
{
    mChildParticleEffects.addLocally(particle);
}

void Being::setAction(Uint8 action)
{
    SpriteAction currentAction = ACTION_INVALID;
    switch (action)
    {
        case WALK:
            currentAction = ACTION_WALK;
            break;
        case SIT:
            currentAction = ACTION_SIT;
            break;
        case ATTACK:
            if (mEquippedWeapon)
            {
                currentAction = mEquippedWeapon->getAttackType();
            }
            else {
                currentAction = ACTION_ATTACK;
            }
            for (int i = 0; i < VECTOREND_SPRITE; i++)
            {
                if (mSprites[i])
                {
                    mSprites[i]->reset();
                }
            }
            break;
        case HURT:
            //currentAction = ACTION_HURT;  // Buggy: makes the player stop
                                            // attacking and unable to attack
                                            // again until he moves
            break;
        case DEAD:
            currentAction = ACTION_DEAD;
            break;
        case STAND:
            currentAction = ACTION_STAND;
            break;
    }

    if (currentAction != ACTION_INVALID)
    {
        for (int i = 0; i < VECTOREND_SPRITE; i++)
        {
            if (mSprites[i])
            {
                mSprites[i]->play(currentAction);
            }
        }
        mAction = action;
    }
}


void Being::setDirection(Uint8 direction)
{
    mDirection = direction;
    SpriteDirection dir = getSpriteDirection();

    for (int i = 0; i < VECTOREND_SPRITE; i++)
    {
       if (mSprites[i] != NULL)
            mSprites[i]->setDirection(dir);
    }
}

SpriteDirection Being::getSpriteDirection() const
{
    SpriteDirection dir;

    if (mDirection & UP)
    {
        dir = DIRECTION_UP;
    }
    else if (mDirection & DOWN)
    {
        dir = DIRECTION_DOWN;
    }
    else if (mDirection & RIGHT)
    {
        dir = DIRECTION_RIGHT;
    }
    else {
        dir = DIRECTION_LEFT;
    }

    return dir;
}

void Being::nextStep()
{
    if (mPath.empty())
    {
        setAction(STAND);
        return;
    }

    Position pos = mPath.front();
    mPath.pop_front();

    int dir = 0;
    if (pos.x > mX)
        dir |= RIGHT;
    else if (pos.x < mX)
        dir |= LEFT;
    if (pos.y > mY)
        dir |= DOWN;
    else if (pos.y < mY)
        dir |= UP;

    setDirection(dir);

    if (mMap->tileCollides(pos.x, pos.y))
    {
        setAction(STAND);
        return;
    }

    mX = pos.x;
    mY = pos.y;
    setAction(WALK);
    mWalkTime += mWalkSpeed / 10;
}

void Being::logic()
{
    // Reduce the time that speech is still displayed
    if (mSpeechTime > 0 && mSpeech)
    {
        if (--mSpeechTime == 0)
        {
            delete mSpeech;
            mSpeech = 0;
        }
    }

    int oldPx = mPx;
    int oldPy = mPy;
    // Update pixel coordinates
    mPx = mX * 32 + getXOffset();
    mPy = mY * 32 + getYOffset();
    if (mPx != oldPx || mPy != oldPy)
    {
        if (mSpeech)
        {
            mSpeech->adviseXY(mPx + X_SPEECH_OFFSET, mPy - Y_SPEECH_OFFSET);
        }
        updateCoords();
    }
    if (mEmotion != 0)
    {
        mEmotionTime--;
        if (mEmotionTime == 0) {
            mEmotion = 0;
        }
    }

    // Update sprite animations
    for (int i = 0; i < VECTOREND_SPRITE; i++)
    {
        if (mSprites[i] != NULL)
        {
            mSprites[i]->update(tick_time * 10);
        }
    }

    // Restart status/particle effects, if needed
    if (mMustResetParticles) {
        mMustResetParticles = false;
        for (std::set<int>::iterator it = mStatusEffects.begin();
             it != mStatusEffects.end(); it++) {
            const StatusEffect *effect = StatusEffect::getStatusEffect(*it, true);
            if (effect && effect->particleEffectIsPersistent())
                updateStatusEffect(*it, true);
        }
    }

    // Update particle effects
    mChildParticleEffects.moveTo((float) mPx + 16.0f,
                                 (float) mPy + 32.0f);

}

void Being::draw(Graphics *graphics, int offsetX, int offsetY) const
{
    int px = mPx + offsetX;
    int py = mPy + offsetY;

    for (int i = 0; i < VECTOREND_SPRITE; i++)
    {
        if (mSprites[i] != NULL)
        {
            mSprites[i]->draw(graphics, px, py);
        }
    }
}

void Being::drawEmotion(Graphics *graphics, int offsetX, int offsetY)
{
    if (!mEmotion)
        return;

    const int px = mPx + offsetX + 3;
    const int py = mPy + offsetY - 60;
    const int emotionIndex = mEmotion - 1;

    if (emotionIndex >= 0 && emotionIndex < (int) emotionSet->size())
        graphics->drawImage(emotionSet->get(emotionIndex), px, py);
}

Being::Type Being::getType() const
{
    return UNKNOWN;
}

void Being::setStatusEffectBlock(int offset, Uint16 newEffects)
{
    for (int i = 0; i < STATUS_EFFECTS; i++) {
        int index = StatusEffect::blockEffectIndexToEffectIndex(offset + i);

        if (index != -1)
            setStatusEffect(index, (newEffects & (1 << i)) > 0);
    }
}

void Being::handleStatusEffect(StatusEffect *effect, int effectId)
{
    if (!effect)
        return;

    SpriteAction action = effect->getAction();
    if (action != ACTION_INVALID)
        setAction(action);

    Particle *particle = effect->getParticle();

    if (effectId >= 0)
        mStatusParticleEffects.setLocally(effectId, particle);
    else {
        mStunParticleEffects.clearLocally();
        if (particle)
            mStunParticleEffects.addLocally(particle);
    }
}

void Being::updateStunMode(int oldMode, int newMode)
{
    handleStatusEffect(StatusEffect::getStatusEffect(oldMode, false), -1);
    handleStatusEffect(StatusEffect::getStatusEffect(newMode, true), -1);
}

void Being::updateStatusEffect(int index, bool newStatus)
{
    handleStatusEffect(StatusEffect::getStatusEffect(index, newStatus), index);
}

void Being::setStatusEffect(int index, bool active)
{
    const bool wasActive = mStatusEffects.find(index) != mStatusEffects.end();

    if (active != wasActive) {
        updateStatusEffect(index, active);
        if (active)
            mStatusEffects.insert(index);
        else
            mStatusEffects.erase(index);
    }
}

int Being::getOffset(char pos, char neg) const
{
    // Check whether we're walking in the requested direction
    if (mAction != WALK || !(mDirection & (pos | neg))) {
        return 0;
    }

    int offset = (get_elapsed_time(mWalkTime) * 32) / mWalkSpeed;

    // We calculate the offset _from_ the _target_ location
    offset -= 32;
    if (offset > 0) {
        offset = 0;
    }

    // Going into negative direction? Invert the offset.
    if (mDirection & pos) {
        offset = -offset;
    }

    return offset;
}


int Being::getWidth() const
{
    if (mSprites[BASE_SPRITE])
    {
        return mSprites[BASE_SPRITE]->getWidth();
    }
    else {
        return 0;
    }
}


int Being::getHeight() const
{
    if (mSprites[BASE_SPRITE])
    {
        return mSprites[BASE_SPRITE]->getHeight();
    }
    else {
        return 0;
    }
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
        XML::Document doc(BEING_EFFECTS_FILE);
        xmlNodePtr root = doc.rootNode();

        if (!root || !xmlStrEqual(root->name, BAD_CAST "being-effects"))
        {
            logger->log("Error loading being effects file: "
                    BEING_EFFECTS_FILE);
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
            } else if (xmlStrEqual(node->name, BAD_CAST "default"))
            {
                EffectDescription *EffectDescription =
                    getEffectDescription(node, &id);

                if (default_effect)
                    delete default_effect;

                default_effect = EffectDescription;
            }
        }

        effects_initialized = true;
    } // done initializing

    EffectDescription *ed = effects[effectId];

    if (!ed)
        return default_effect;
    else
        return ed;
}

void Being::internalTriggerEffect(int effectId, bool sfx, bool gfx)
{
    logger->log("Special effect #%d on %s", effectId,
                getId() == player_node->getId() ? "self" : "other");

    EffectDescription *ed = getEffectDescription(effectId);

    if (!ed) {
        logger->log("Unknown special effect and no default recorded");
        return;
    }

    if (gfx && ed->mGFXEffect != "") {
        Particle *selfFX;

        selfFX = particleEngine->addEffect(ed->mGFXEffect, 0, 0);
        controlParticle(selfFX);
    }

    if (sfx && ed->mSFXEffect != "") {
        sound.playSfx(ed->mSFXEffect);
    }
}




static int hairStylesNr;
static int hairColorsNr;
static std::vector<std::string> hairColors;

static void initializeHair();

int Being::getHairStylesNr()
{
    initializeHair();
    return hairStylesNr;
}

int Being::getHairColorsNr()
{
    initializeHair();
    return hairColorsNr;
}

std::string Being::getHairColor(int index)
{
    initializeHair();
    if (index < 0 || index >= hairColorsNr)
        return "#000000";

    return hairColors[index];
}

static bool hairInitialized = false;

static void initializeHair()
{
    if (hairInitialized)
        return;

    // Hairstyles are encoded as negative numbers.  Count how far negative we can go.
    int hairstylesCtr = -1;
    while (ItemDB::get(hairstylesCtr).getSprite(0) != "error.xml")
        --hairstylesCtr;

    hairStylesNr = -hairstylesCtr; // done.
    if (hairStylesNr == 0)
        hairStylesNr = 1; // No hair style -> no hair

    hairColorsNr = 0;

    XML::Document doc(HAIR_FILE);
    xmlNodePtr root = doc.rootNode();

    if (!root || !xmlStrEqual(root->name, BAD_CAST "colors"))
    {
        logger->log("Error loading being hair configuration file");
    } else {
        for_each_xml_child_node(node, root)
        {
            if (xmlStrEqual(node->name, BAD_CAST "color"))
            {
                int index = atoi(XML::getProperty(node, "id", "-1").c_str());
                std::string value = XML::getProperty(node, "value", "");

                if (index >= 0 && value != "") {
                    if (index >= hairColorsNr) {
                        hairColorsNr = index + 1;
                        hairColors.resize(hairColorsNr, "#000000");
                    }
                    hairColors[index] = value;
                }
            }
        }
    } // done initializing

    if (hairColorsNr == 0) { // No colours -> black only
        hairColorsNr = 1;
        hairColors.resize(hairColorsNr, "#000000");
    }

    hairInitialized = 1;
}
