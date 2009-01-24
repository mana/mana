/*
 *  The Mana World
 *  Copyright (C) 2004  The Mana World Development Team
 *
 *  This file is part of The Mana World.
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

#include <cassert>
#include <cmath>

#include "animatedsprite.h"
#include "being.h"
#include "configuration.h"
#include "equipment.h"
#include "game.h"
#include "graphics.h"
#include "localplayer.h"
#include "log.h"
#include "map.h"
#include "particle.h"
#include "sound.h"
#include "text.h"

#include "resources/emotedb.h"
#include "resources/imageset.h"
#include "resources/itemdb.h"
#include "resources/iteminfo.h"
#include "resources/resourcemanager.h"

#include "gui/gui.h"
#include "gui/speechbubble.h"

#include "utils/dtor.h"
#include "utils/gettext.h"  
#include "utils/tostring.h"
#include "utils/xml.h"

int Being::instances = 0;
int Being::mNumberOfHairstyles = 1;
std::vector<AnimatedSprite*> Being::emotionSet;

static const int X_SPEECH_OFFSET = 18;
static const int Y_SPEECH_OFFSET = 60;

Being::Being(int id, int job, Map *map):
    mJob(job),
    mX(0), mY(0),
    mAction(STAND),
    mWalkTime(0),
    mEmotion(0), mEmotionTime(0),
    mAttackSpeed(350),
    mEquipment(new Equipment()),
    mId(id),
    mWalkSpeed(150),
    mDirection(DOWN),
    mMap(NULL),
    mName(""),
    mIsGM(false),
    mParticleEffects(config.getValue("particleeffects", 1)),
    mEquippedWeapon(NULL),
    mHairStyle(1), mHairColor(0),
    mGender(GENDER_UNSPECIFIED),
    mSpeechTime(0),
    mPx(0), mPy(0),
    mSprites(VECTOREND_SPRITE, NULL),
    mSpriteIDs(VECTOREND_SPRITE, 0),
    mSpriteColors(VECTOREND_SPRITE, ""),
    mStatusParticleEffects(&mStunParticleEffects, false),
    mChildParticleEffects(&mStatusParticleEffects, false)
{
    setMap(map);

    mSpeechBubble = new SpeechBubble();

    if (instances == 0)
    {
        // Setup emote sprites
        for (int i = 0; i <= EmoteDB::getLast(); i++)
        {
            EmoteInfo info = EmoteDB::get(i);

            std::string file = "graphics/sprites/" + info.sprites.front()->sprite;
            int variant = info.sprites.front()->variant;
            emotionSet.push_back(AnimatedSprite::load(file, variant));
        }

        // Hairstyles are encoded as negative numbers.  Count how far negative we can go.
        int hairstyles = 1;
        while (ItemDB::get(-hairstyles).getSprite(GENDER_MALE) != "error.xml")
        {
            hairstyles++;
        }
        mNumberOfHairstyles = hairstyles;
    }

    instances++;
    mSpeech = "";
    mText = 0;
}

Being::~Being()
{
    delete_all(mSprites);
    clearPath();

    setMap(NULL);

    instances--;

    if (instances == 0)
    {
        delete_all(emotionSet);
    }

    delete mSpeechBubble;
    delete mText;
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
    mHairStyle = style < 0 ? mHairStyle : style % mNumberOfHairstyles;
    mHairColor = color < 0 ? mHairColor : color % ColorDB::size();
}

void Being::setSprite(int slot, int id, std::string color)
{
    assert(slot >= BASE_SPRITE && slot < VECTOREND_SPRITE);
    mSpriteIDs[slot] = id;
    mSpriteColors[slot] = color;
}

void Being::setSpeech(const std::string &text, Uint32 time)
{
    mSpeech = text;

    // Trim whitespace
    while (mSpeech[0] == ' ')
    {
        mSpeech = mSpeech.substr(1, mSpeech.size());
    }
    while (mSpeech[mSpeech.size()] == ' ')
    {
        mSpeech = mSpeech.substr(0, mSpeech.size() - 1);
    }

    // check for links
    std::string::size_type start = mSpeech.find('[');
    std::string::size_type end = mSpeech.find(']', start);

    while (start != std::string::npos && end != std::string::npos)
    {
        // Catch multiple embeds and ignore them so it doesn't crash the client.
        while ((mSpeech.find('[', start + 1) != std::string::npos) && 
               (mSpeech.find('[', start + 1) < end))
        {
            start = mSpeech.find('[', start + 1);
        }

        std::string::size_type position = mSpeech.find('|');
        mSpeech.erase(end, 1);
        mSpeech.erase(start, (position - start) + 1);
        position = mSpeech.find('@');

        while (position != std::string::npos)
        {
            mSpeech.erase(position, 2);
            position = mSpeech.find('@');
        }

        start = mSpeech.find('[', start + 1);
        end = mSpeech.find(']', start);
    }

    if (mSpeech != "")
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

void Being::showCrit()
{
    gcn::Font *font;
    std::string text = "crit!";

    // Selecting the right color
    if (getType() == MONSTER)
    {
        font = hitBlueFont;
    }
    else
    {
        font = hitRedFont;
    }

    // Show crit notice
    particleEngine->addTextSplashEffect(text, 255, 255, 255, font,
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
}

void Being::controlParticle(Particle *particle)
{
    mChildParticleEffects.addLocally(particle);
}

void Being::setAction(Action action)
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
            else 
            {
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
    if (mDirection == direction)
        return;

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
    else 
    {
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
    if (mSpeechTime > 0)
         mSpeechTime--;

    // Remove text if speech boxes aren't being used
    if (mSpeechTime == 0 && mText)
    {
        delete mText;
        mText = 0;
    }

    int oldPx = mPx;
    int oldPy = mPy;

    // Update pixel coordinates
    mPx = mX * 32 + getXOffset();
    mPy = mY * 32 + getYOffset();

    if (mPx != oldPx || mPy != oldPy)
    {
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

    if (emotionIndex >= 0 && emotionIndex <= EmoteDB::getLast())
        emotionSet[emotionIndex]->draw(graphics, px, py);
}

void Being::drawSpeech(Graphics *graphics, int offsetX, int offsetY)
{
    int px = mPx + offsetX;
    int py = mPy + offsetY;

    // Draw speech above this being
    if (mSpeechTime > 0 && config.getValue("speechbubble", 1))
    {
        if (mText)
        {
            delete mText;
            mText = 0;
        }

        if (mName != mSpeechBubble->getCaption())
            mSpeechBubble->setName(mName);

        // Not quite centered, but close enough. However, it's not too important to get 
        // it right right now, as it doesn't take bubble collision into account yet. 
        mSpeechBubble->setText(mSpeech);
        mSpeechBubble->setPosition(px - (mSpeechBubble->getWidth() * 4 / 11), py - 70 - 
                                        (mSpeechBubble->getNumRows()*14));
        mSpeechBubble->setVisible(true);
    }
    else if (mSpeechTime > 0 && !config.getValue("speechbubble", 1))
    {
        mSpeechBubble->setVisible(false);
        // don't introduce a memory leak
        if (mText)
            delete mText;

        mText = new Text(mSpeech, mPx + X_SPEECH_OFFSET, mPy - Y_SPEECH_OFFSET,
                         gcn::Graphics::CENTER, gcn::Color(255, 255, 255));
    }
    else if (mSpeechTime == 0)
    {
        mSpeechBubble->setVisible(false);
    }
}

Being::Type Being::getType() const
{
    return UNKNOWN;
}

int Being::getOffset(char pos, char neg) const
{
    // Check whether we're walking in the requested direction
    if (mAction != WALK ||  !(mDirection & (pos | neg))) 
    {
        return 0;
    }

    int offset = (get_elapsed_time(mWalkTime) * 32) / mWalkSpeed;

    // We calculate the offset _from_ the _target_ location
    offset -= 32;
    if (offset > 0) 
    {
        offset = 0;
    }

    // Going into negative direction? Invert the offset.
    if (mDirection & pos) 
    {
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
    else 
    {
        return 0;
    }
}


int Being::getHeight() const
{
    if (mSprites[BASE_SPRITE])
    {
        return mSprites[BASE_SPRITE]->getHeight();
    }
    else 
    {
        return 0;
    }
}

