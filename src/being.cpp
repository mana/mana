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
 *
 *  $Id$
 */
#include "being.h"

#include "equipment.h"
#include "game.h"
#include "graphics.h"
#include "log.h"
#include "map.h"

#include "resources/spriteset.h"

#include "gui/gui.h"

#include "utils/tostring.h"

extern Spriteset *emotionset;

PATH_NODE::PATH_NODE(Uint16 iX, Uint16 iY):
    x(iX), y(iY)
{
}

Being::Being(Uint32 id, Uint16 job, Map *map):
    mJob(job),
    mX(0), mY(0), mDirection(DOWN),
    mAction(0),
    mWalkTime(0),
    mEmotion(0), mEmotionTime(0),
    mAttackSpeed(350),
    mEquipment(new Equipment()),
    mId(id),
    mWeapon(0),
    mWalkSpeed(150),
    mMap(NULL),
    mHairStyle(0), mHairColor(0),
    mSex(2),
    mSpeechTime(0),
    mDamageTime(0),
    mShowSpeech(false), mShowDamage(false)
{
    setMap(map);
    mSprites.resize(VECTOREND_SPRITE, NULL);
}

Being::~Being()
{
    for (int i = 0; i < VECTOREND_SPRITE; i++)
    {
        if (mSprites[i] != NULL)
        {
            delete mSprites[i];
        }
    }

    clearPath();
    setMap(NULL);
}

void
Being::setDestination(Uint16 destX, Uint16 destY)
{
    if (mMap)
    {
        setPath(mMap->findPath(mX, mY, destX, destY));
    }
}

void
Being::clearPath()
{
    mPath.clear();
}

void
Being::setPath(const Path &path)
{
    mPath = path;

    if (mAction != WALK && mAction != DEAD)
    {
        nextStep();
        mWalkTime = tick_time;
    }
}

void
Being::setHairColor(Uint16 color)
{
    mHairColor = color;
    if (mHairColor < 1 || mHairColor > NR_HAIR_COLORS + 1)
    {
        mHairColor = 1;
    }
}

void
Being::setHairStyle(Uint16 style)
{
    mHairStyle = style;
    if (mHairStyle < 1 || mHairStyle > NR_HAIR_STYLES)
    {
        mHairStyle = 1;
    }
}

void
Being::setVisibleEquipment(Uint8 slot, Uint8 id)
{
}

void
Being::setSpeech(const std::string &text, Uint32 time)
{
    mSpeech = text;
    mSpeechTime = tick_time;
    mShowSpeech = true;
}

void
Being::setDamage(Sint16 amount, Uint32 time)
{
    mDamage = amount ? toString(amount) : "miss";
    mDamageTime = tick_time;
    mShowDamage = true;
}

void
Being::setMap(Map *map)
{
    // Remove sprite from potential previous map
    if (mMap != NULL)
    {
        mMap->removeSprite(mSpriteIterator);
    }

    mMap = map;

    // Add sprite to potential new map
    if (mMap != NULL)
    {
        mSpriteIterator = mMap->addSprite(this);
    }
}

void
Being::setAction(Action action)
{
    if (action != mAction)
    {
        std::string currentAction = "stand";
        switch (action)
        {
            case WALK:
                currentAction = "walk";
                break;
            case SIT:
                currentAction = "sit";
                break;
            case ATTACK:
                if (getType() == MONSTER)
                {
                    currentAction = "dead";
                }else{
                    switch (getWeapon())
                    {
                        case 2:
                            currentAction = "attack_bow";
                            break;
                        case 1:
                            currentAction = "attack_stab";
                            break;
                        case 0:
                            currentAction = "attack";
                            break;
                    }
                };
                break;
            case MONSTER_ATTACK:
                currentAction = "attack";
                break;
            case DEAD:
                currentAction = "dead";
                break;
            default:
                currentAction = "stand";
                break;
        }

        for (int i = 0; i < VECTOREND_SPRITE; i++)
        {
            if (mSprites[i] != NULL)
            {
                if (currentAction == "attack" ||
                    currentAction == "attack_stab" ||
                    currentAction == "attack_bow")
                {
                    mSprites[i]->play(currentAction, mAttackSpeed);
                }
                else
                {
                    mSprites[i]->play(currentAction);
                }
            }
        }
    }
    mAction = action;
}

void
Being::setDirection(Uint8 direction)
{
    mDirection |= direction;
    std::string dir;

    if (direction & UP)
    {
        dir = "up";
    }
    else if (direction & RIGHT)
    {
        dir = "right";
    }
    else if (direction & DOWN)
    {
        dir = "down";
    }
    else
    {
        dir = "left";
    }

    for (int i = 0; i < VECTOREND_SPRITE; i++)
    {
        if (mSprites[i] != NULL) mSprites[i]->setDirection(dir);
    }
}

void
Being::nextStep()
{
    if (mPath.empty())
    {
        setAction(STAND);
        return;
    }

    PATH_NODE node = mPath.front();
    mPath.pop_front();

    mDirection = 0;
    if (node.x > mX)
        setDirection(RIGHT);
    else if (node.x < mX)
        setDirection(LEFT);
    if (node.y > mY)
        setDirection(DOWN);
    else if (node.y < mY)
        setDirection(UP);

    mX = node.x;
    mY = node.y;
    setAction(WALK);
    mWalkTime += mWalkSpeed / 10;
}

void
Being::logic()
{
    // Determine whether speech should still be displayed
    if (get_elapsed_time(mSpeechTime) > 5000)
    {
        mShowSpeech = false;
    }

    // Determine whether damage should still be displayed
    if (get_elapsed_time(mDamageTime) > 3000)
    {
        mShowDamage = false;
    }

    // Update pixel coordinates
    mPx = mX * 32 + getXOffset();
    mPy = mY * 32 + getYOffset();

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
}

void
Being::draw(Graphics *graphics, int offsetX, int offsetY)
{
    int px = mPx + offsetX;
    int py = mPy + offsetY;

    //what are these two lines good for? please add a comment.
    unsigned char dir = 0;
    while (!(mDirection & (1 << dir))) dir++;

    for (int i = 0; i < VECTOREND_SPRITE; i++)
    {
        if (mSprites[i] != NULL)
        {
            mSprites[i]->draw(graphics, px, py);
        }
    }
}

void
Being::drawEmotion(Graphics *graphics, Sint32 offsetX, Sint32 offsetY)
{
    int px = mPx + offsetX;
    int py = mPy + offsetY;

    if (mEmotion)
    {
        graphics->drawImage(emotionset->get(mEmotion - 1),
                px + 3, py - 60);
    }
}

void
Being::drawSpeech(Graphics *graphics, Sint32 offsetX, Sint32 offsetY)
{
    int px = mPx + offsetX;
    int py = mPy + offsetY;

    // Draw speech above this being
    if (mShowSpeech)
    {
        graphics->setFont(speechFont);
        graphics->setColor(gcn::Color(255, 255, 255));
        graphics->drawText(mSpeech, px + 18, py - 60, gcn::Graphics::CENTER);
    }

    // Draw damage above this being
    if (mShowDamage)
    {
        // Selecting the right color
        if (mDamage == "miss")
        {
            graphics->setFont(hitYellowFont);
        }
        else if (getType() == MONSTER)
        {
            graphics->setFont(hitBlueFont);
        }
        else
        {
            graphics->setFont(hitRedFont);
        }

        int textY = (getType() == MONSTER) ? 32 : 70;
        int ft = get_elapsed_time(mDamageTime) - 1500;
        float a = (ft > 0) ? 1.0 - ft / 1500.0 : 1.0;

        graphics->setColor(gcn::Color(255, 255, 255, (int)(255 * a)));
        graphics->drawText(mDamage,
                           px + 16,
                           py - textY - get_elapsed_time(mDamageTime) / 100,
                           gcn::Graphics::CENTER);

        // Reset alpha value
        graphics->setColor(gcn::Color(255, 255, 255));
    }
}

Being::Type
Being::getType() const
{
    return UNKNOWN;
}

void
Being::setWeaponById(Uint16 weapon)
{
    switch (weapon)
    {
    case 529: // iron arrows
    case 1199: // arrows
        break;

    case 1200: // bow
    case 530: // short bow
    case 545: // forest bow
        setWeapon(2);
        break;

    case 521: // sharp knife
    case 522: // dagger
    case 536: // short sword
    case 1201: // knife
        setWeapon(1);
        break;

    case 0: // unequip
        setWeapon(0);
        break;

    default:
        logger->log("Not a weapon: %d", weapon);
    }
}

int
Being::getOffset(char pos, char neg) const
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
