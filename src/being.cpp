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

#include <algorithm>
#include <cassert>
#include <cmath>

#include "animatedsprite.h"
#include "equipment.h"
#include "game.h"
#include "graphics.h"
#include "log.h"
#include "map.h"
#include "particle.h"

#include "resources/resourcemanager.h"
#include "resources/imageset.h"
#include "resources/iteminfo.h"

#include "gui/gui.h"

#include "utils/dtor.h"
#include "utils/tostring.h"

int Being::instances = 0;
ImageSet *Being::emotionSet = NULL;

Being::Being(int id, int job, Map *map):
    mX(0), mY(0),
    mEmotion(0), mEmotionTime(0),
    mAttackSpeed(350),
    mWalkTime(0),
    mAction(STAND),
    mJob(job),
    mId(id),
    mWalkSpeed(150),
    mSpeedModifier(1024),
    mSpriteDirection(DIRECTION_DOWN), mDirection(DOWN),
    mMap(NULL),
    mEquippedWeapon(NULL),
    mSpeechTime(0),
    mPx(0), mPy(0),
    mSprites(VECTOREND_SPRITE, NULL),
    mSpriteIDs(VECTOREND_SPRITE, 0),
    mSpriteColors(VECTOREND_SPRITE, "")
{
    setMap(map);

    if (instances == 0)
    {
        // Load the emotion set
        ResourceManager *rm = ResourceManager::getInstance();
        emotionSet = rm->getImageSet("graphics/sprites/emotions.png", 30, 32);
        if (!emotionSet) logger->error("Unable to load emotions!");
    }

    instances++;
}

Being::~Being()
{
    std::for_each(mSprites.begin(), mSprites.end(), make_dtor(mSprites));
    clearPath();

    for (   std::list<Particle *>::iterator i = mChildParticleEffects.begin();
            i != mChildParticleEffects.end();
            i++)
    {
        (*i)->kill();
    }

    setMap(NULL);

    instances--;

    if (instances == 0)
    {
        emotionSet->decRef();
        emotionSet = NULL;
    }
}

void Being::setPositionInPixels(int x, int y)
{
    mMap->freeTile(mX / 32, mY / 32, getBlockType());
    mX = x;
    mY = y;
    mMap->blockTile(x / 32, y / 32, getBlockType());
}

void Being::adjustCourse(Uint16 srcX, Uint16 srcY, Uint16 dstX, Uint16 dstY)
{
    if (!mMap || (mX == dstX && mY == dstY))
    {
        setPath(Path());
        return;
    }

    if (mX / 32 == dstX / 32 && mY / 32 == dstY / 32)
    {
        // The being is already on the last tile of the path.
        Path p;
        p.push_back(PATH_NODE(dstX, dstY));
        setPath(p);
        return;
    }

    Path p1;
    int p1_size, p1_length;
    Uint16 *p1_dist;
    int onPath = -1;
    if (srcX / 32 == dstX / 32 && srcY / 32 == dstY / 32)
    {
        p1_dist = new Uint16[1];
        p1_size = 1;
        p1_dist[0] = 0;
        p1_length = 0;
    }
    else
    {
        p1 = mMap->findPath(srcX / 32, srcY / 32, dstX / 32, dstY / 32, getWalkMask());
        if (p1.empty())
        {
            // No path, but don't teleport since it could be user input.
            setPath(p1);
            return;
        }
        p1_size = p1.size();
        p1_dist = new Uint16[p1_size];
        int j = 0;
        // Remove last tile so that it can be replaced by the exact destination.
        p1.pop_back();
        for (Path::iterator i = p1.begin(), i_end = p1.end(); i != i_end; ++i)
        {
            // Get distance from source to tile i.
            p1_dist[j] = mMap->getMetaTile(i->x, i->y)->Gcost;
            // Check if the being is already walking on the path.
            if (i->x == mX / 32 && i->y == mY / 32)
            {
                onPath = j;
            }
            // Do not set any offset for intermediate steps.
            i->x = i->x * 32;
            i->y = i->y * 32;
            ++j;
        }
        p1_length = mMap->getMetaTile(dstX / 32, dstY / 32)->Gcost;
        p1_dist[p1_size - 1] = p1_length;
    }
    p1.push_back(PATH_NODE(dstX, dstY));

    if (mX / 32 == srcX / 32 && mY / 32 == srcY / 32)
    {
        // The being is at the start of the path.
        setPath(p1);
        delete[] p1_dist;
        return;
    }

    if (onPath >= 0)
    {
        // The being is already on the path, but it needs to be slowed down.
        for (int j = onPath; j >= 0; --j)
        {
            p1.pop_front();
        }
        int r = p1_length - p1_dist[onPath];  // remaining length
        assert(r > 0);
        setPath(p1, p1_length * 1024 / r);
        delete[] p1_dist;
        return;
    }

    Path bestPath;
    int bestRating = -1, bestStart = 0, bestLength = 0;
    int j = 0;

    for (Path::iterator i = p1.begin(), i_end = p1.end(); i != i_end; ++i)
    {
        // Look if it is worth passing by tile i.
        Path p2 = mMap->findPath(mX / 32, mY / 32, i->x / 32, i->y / 32, getWalkMask());
        if (!p2.empty())
        {
            int l1 = mMap->getMetaTile(i->x / 32, i->y / 32)->Gcost;
            int l2 = p1_length - p1_dist[j];
            int r = l1 + l2 / 2; // TODO: tune rating formula
            assert(r > 0);
            if (bestRating < 0 || r < bestRating)
            {
                bestPath.swap(p2);
                bestRating = r;
                bestStart = j;
                bestLength = l1 + l2;
            }
        }
        ++j;
    }

    if (bestRating < 0)
    {
        // Unable to reach the path? Still, don't teleport since it could be
        // user input instead of server command.
        setPath(p1);
        delete[] p1_dist;
        return;
    }

    bestPath.pop_back();
    for (Path::iterator i = bestPath.begin(), i_end = bestPath.end(); i != i_end; ++i)
    {
        i->x = i->x * 32;
        i->y = i->y * 32;
    }

    // Concatenate paths.
    for (int j = bestStart; j > 0; --j)
    {
        p1.pop_front();
    }
    p1.splice(p1.begin(), bestPath);

    assert(bestLength > 0);
    setPath(p1, p1_length * 1024 / bestLength);
    delete[] p1_dist;
}

void Being::adjustCourse(Uint16 srcX, Uint16 srcY)
{
    if (!mPath.empty())
    {
        adjustCourse(srcX, srcY, mPath.back().x, mPath.back().y);
    }
}

void
Being::setDestination(Uint16 destX, Uint16 destY)
{
    adjustCourse(mX, mY, destX, destY);
}

void
Being::clearPath()
{
    mPath.clear();
}

void
Being::setPath(const Path &path, int mod)
{
    mPath = path;
    mSpeedModifier = mod >= 512 ? (mod <= 2048 ? mod : 2048) : 512; // TODO: tune bounds

    int sz = mPath.size();
    if (sz > 1)
    {
        // The path contains intermediate steps, so avoid going through tile
        // centers for them. Instead, interpolate the tile offset.
        int sx = mX & 31, sy = mY & 31;
        int dx = (mPath.back().x & 31) - sx;
        int dy = (mPath.back().y & 31) - sy;
        Path::iterator j = mPath.begin();
        for (int i = 0; i < sz - 1; ++i)
        {
            j->x |= sx + dx * (i + 1) / (sz - 1);
            j->y |= sy + dy * (i + 1) / (sz - 1);
            ++j;
        }
    }

    if (mAction != WALK && mAction != DEAD)
    {
        mWalkTime = tick_time;
        mStepTime = 0;
        nextStep();
    }
}

void
Being::setSprite(int slot, int id, const std::string &color)
{
    assert(slot >= BASE_SPRITE && slot < VECTOREND_SPRITE);
    mSpriteIDs[slot] = id;
    mSpriteColors[slot] = color;
}

void
Being::setSpeech(const std::string &text, Uint32 time)
{
    mSpeech = text;
    mSpeechTime = 500;
}

void
Being::takeDamage(int amount)
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

void
Being::handleAttack()
{
    setAction(Being::ATTACK);
}

void
Being::setMap(Map *map)
{

    // Remove sprite from potential previous map
    if (mMap)
    {
        mMap->freeTile(mX / 32, mY / 32, getBlockType());
        mMap->removeSprite(mSpriteIterator);
    }

    mMap = map;

    // Add sprite to potential new map
    if (mMap)
    {
        mSpriteIterator = mMap->addSprite(this);
        mMap->blockTile(mX / 32, mY / 32, getBlockType());
    }

    // Clear particle effect list because child particles became invalid
    mChildParticleEffects.clear();
}

void
Being::controlParticle(Particle *particle)
{
    if (particle)
    {
        // The effect may not die without the beings permission or we segfault
        particle->disableAutoDelete();
        mChildParticleEffects.push_back(particle);
    }
}

void
Being::setAction(Action action, int attackType)
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


void
Being::setDirection(Uint8 direction)
{
    if (mDirection == direction)
        return;

    // if the direction does not change much, keep the common component
    int mFaceDirection = mDirection & direction;
    if (!mFaceDirection)
        mFaceDirection = direction;
    mDirection = direction;

    SpriteDirection dir;
    if (mFaceDirection & UP)
    {
        dir = DIRECTION_UP;
    }
    else if (mFaceDirection & RIGHT)
    {
        dir = DIRECTION_RIGHT;
    }
    else if (mFaceDirection & DOWN)
    {
        dir = DIRECTION_DOWN;
    }
    else
    {
        dir = DIRECTION_LEFT;
    }
    mSpriteDirection = dir;

    for (int i = 0; i < VECTOREND_SPRITE; i++)
    {
        if (mSprites[i] != NULL)
            mSprites[i]->setDirection(dir);
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

    mStepX = node.x - mX;
    mStepY = node.y - mY;

    int dir = 0, dx = std::abs(mStepX), dy = std::abs(mStepY);
    if (dx * 2 > dy)
        dir |= mStepX > 0 ? RIGHT : LEFT;
    if (dy * 2 > dx)
        dir |= mStepY > 0 ? DOWN : UP;

    setDirection(dir);

    if (!mMap->getWalk(node.x / 32, node.y / 32))
    {
        setAction(STAND);
        return;
    }

    setPositionInPixels(node.x, node.y);
    setAction(WALK);
    mWalkTime += mStepTime / 10;
    mStepTime = mWalkSpeed * (int)std::sqrt((double)mStepX * mStepX + (double)mStepY * mStepY) *
                mSpeedModifier / (32 * 1024);
}

void
Being::logic()
{
    // Determine whether the being should take another step
    if (mAction == WALK && get_elapsed_time(mWalkTime) >= mStepTime)
    {
        nextStep();
    }

    // Reduce the time that speech is still displayed
    if (mSpeechTime > 0)
        mSpeechTime--;

    // Update pixel coordinates
    mPx = mX - 16 + getXOffset();
    mPy = mY - 16 + getYOffset();

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
    for (std::list<Particle *>::iterator i = mChildParticleEffects.begin();
         i != mChildParticleEffects.end();)
    {
        (*i)->setPosition((float)mPx + 16.0f, (float)mPy + 32.0f);
        if (!(*i)->isAlive())
        {
            (*i)->kill();
            i = mChildParticleEffects.erase(i);
        }
        else {
            i++;
        }
    }
}

void
Being::draw(Graphics *graphics, int offsetX, int offsetY) const
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

void
Being::drawEmotion(Graphics *graphics, int offsetX, int offsetY)
{
    if (!mEmotion)
        return;

    const int px = mPx + offsetX + 3;
    const int py = mPy + offsetY - 60;
    const int emotionIndex = mEmotion - 1;

    if (emotionIndex >= 0 && emotionIndex < (int) emotionSet->size())
        graphics->drawImage(emotionSet->get(emotionIndex), px, py);
}

void
Being::drawSpeech(Graphics *graphics, int offsetX, int offsetY)
{
    int px = mPx + offsetX;
    int py = mPy + offsetY;

    // Draw speech above this being
    if (mSpeechTime > 0)
    {
        graphics->setFont(speechFont);
        graphics->setColor(gcn::Color(255, 255, 255));
        graphics->drawText(mSpeech, px + 18, py - 60, gcn::Graphics::CENTER);
    }
}

Being::Type
Being::getType() const
{
    return UNKNOWN;
}

int Being::getOffset(int step) const
{
    // Check whether we're walking in the requested direction
    if (mAction != WALK || step == 0) {
        return 0;
    }

    int offset = (get_elapsed_time(mWalkTime) * std::abs(step)) / mStepTime;

    // We calculate the offset _from_ the _target_ location
    offset -= std::abs(step);
    if (offset > 0) {
        offset = 0;
    }

    // Going into negative direction? Invert the offset.
    if (step < 0) {
        offset = -offset;
    }

    return offset;
}


int
Being::getWidth() const
{
    if (mSprites[BASE_SPRITE])
    {
        return mSprites[BASE_SPRITE]->getWidth();
    }
    else {
        return 0;
    }
}


int
Being::getHeight() const
{
    if (mSprites[BASE_SPRITE])
    {
        return mSprites[BASE_SPRITE]->getHeight();
    }
    else {
        return 0;
    }
}
