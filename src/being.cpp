/*
 *  The Mana Client
 *  Copyright (C) 2004-2009  The Mana World Development Team
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

#include "being.h"

#include "animatedsprite.h"
#include "client.h"
#include "configuration.h"
#include "effectmanager.h"
#include "graphics.h"
#include "localplayer.h"
#include "log.h"
#include "map.h"
#include "particle.h"
#include "simpleanimation.h"
#include "sound.h"
#include "text.h"
#include "statuseffect.h"

#include "gui/speechbubble.h"

#include "resources/colordb.h"
#include "resources/emotedb.h"
#include "resources/image.h"
#include "resources/itemdb.h"
#include "resources/iteminfo.h"
#include "resources/resourcemanager.h"

#include "gui/gui.h"
#include "gui/palette.h"
#include "gui/speechbubble.h"

#include "utils/dtor.h"
#include "utils/stringutils.h"
#include "utils/xml.h"
#include "net/net.h"
#include "net/playerhandler.h"

#include <cassert>
#include <cmath>

#define BEING_EFFECTS_FILE "effects.xml"
#define HAIR_FILE "hair.xml"

static const int DEFAULT_BEING_WIDTH = 32;
static const int DEFAULT_BEING_HEIGHT = 32;


int Being::mNumberOfHairstyles = 1;

// TODO: mWalkTime used by eAthena only
Being::Being(int id, int job, Map *map):
    mFrame(0),
    mWalkTime(0),
    mEmotion(0), mEmotionTime(0),
    mSpeechTime(0),
    mAttackSpeed(350),
    mAction(STAND),
    mJob(job),
    mId(id),
    mDirection(DOWN),
    mSpriteDirection(DIRECTION_DOWN),
    mMap(NULL),
    mDispName(0),
    mShowName(false),
    mEquippedWeapon(NULL),
    mText(0),
    mStunMode(0),
    mAlpha(1.0f),
    mStatusParticleEffects(&mStunParticleEffects, false),
    mChildParticleEffects(&mStatusParticleEffects, false),
    mMustResetParticles(false),
    mX(0), mY(0),
    mDamageTaken(0),
    mUsedTargetCursor(NULL)
{
    setMap(map);

    mSpeechBubble = new SpeechBubble;

    mNameColor = &guiPalette->getColor(Palette::NPC);
    mTextColor = &guiPalette->getColor(Palette::CHAT);
    mWalkSpeed = Net::getPlayerHandler()->getDefaultWalkSpeed();
}

Being::~Being()
{
    mUsedTargetCursor = NULL;
    delete_all(mSprites);

    if (player_node && player_node->getTarget() == this)
        player_node->setTarget(NULL);

    setMap(NULL);

    delete mSpeechBubble;
    delete mDispName;
    delete mText;
}

void Being::setPosition(const Vector &pos)
{
    mPos = pos;

    updateCoords();

    if (mText)
        mText->adviseXY((int)pos.x,
                        (int)pos.y - getHeight() - mText->getHeight() - 6);
}

Position Being::checkNodeOffsets(const Position &position) const
{
    // Pre-computing character's position in tiles
    const int tx = position.x / 32;
    const int ty = position.y / 32;

    // Pre-computing character's position offsets.
    int fx = position.x % 32;
    int fy = position.y % 32;

    // Compute the being radius:
    // FIXME: the beings' radius should be obtained from xml values
    // and stored into the Being ojects.
    int radius = getWidth() / 2;
    // FIXME: Hande beings with more than 1/2 tile radius by not letting them
    // go or spawn in too narrow places. The server will have to be aware
    // of being's radius value (in tiles) to handle this gracefully.
    if (radius > 32 / 2) radius = 32 / 2;
    // set a default value if no value returned.
    if (radius < 1) radius = 32 / 3;

    // Fix coordinates so that the player does not seem to dig into walls.
    if (fx > (32 - radius) && !mMap->getWalk(tx + 1, ty, getWalkMask()))
        fx = 32 - radius;
    else if (fx < radius && !mMap->getWalk(tx - 1, ty, getWalkMask()))
        fx = radius;
    else if (fy > (32 - radius) && !mMap->getWalk(tx, ty + 1, getWalkMask()))
        fy = 32 - radius;
    else if (fy < radius && !mMap->getWalk(tx, ty - 1, getWalkMask()))
        fy = radius;

    // FIXME: Check also diagonal positions.

    // Test also the current character's position, to avoid the corner case
    // where a player can approach an obstacle by walking from slightly
    // under, diagonally. First part to the walk on water bug.
    //if (offsetY < 16 && !mMap->getWalk(posX, posY - 1, getWalkMask()))
    //  fy = 16;

    return Position(tx * 32 + fx, ty * 32 + fy);
}

void Being::setDestination(int dstX, int dstY)
{
    if (Net::getNetworkType() == ServerInfo::EATHENA)
    {
        if (mMap)
            setPath(mMap->findPath(mX, mY, dstX, dstY, getWalkMask()));
        return;
    }

    // If the destination is unwalkable, don't bother trying to get there
    if (!mMap->getWalk(dstX / 32, dstY / 32))
        return;

    Position dest = checkNodeOffsets(dstX, dstY);
    mDest.x = dest.x;
    mDest.y = dest.y;
    int srcX = mPos.x;
    int srcY = mPos.y;

    Path thisPath;

    if (mMap)
    {
        thisPath = mMap->findPath(mPos.x / 32, mPos.y / 32,
                                  mDest.x / 32, mDest.y / 32, getWalkMask());
    }

    if (thisPath.empty())
    {
        setPath(Path());
        return;
    }

    // Find the starting offset
    float startX = (srcX % 32);
    float startY = (srcY % 32);

    // Find the ending offset
    float endX = (dstX % 32);
    float endY = (dstY % 32);

    // Find the distance, and divide it by the number of steps
    int changeX = (int)((endX - startX) / thisPath.size());
    int changeY = (int)((endY - startY) / thisPath.size());

    // Convert the map path to pixels over tiles
    // And add interpolation between the starting and ending offsets
    Path::iterator it = thisPath.begin();
    int i = 0;
    while (it != thisPath.end())
    {
        // A position that is valid on the start and end tile is not
        // necessarily valid on all the tiles in between, so check the offsets.
        *it = checkNodeOffsets(it->x * 32 + startX + changeX * i,
                               it->y * 32 + startY + changeY * i);
        i++;
        it++;
    }

    // Remove the last path node, as it's more clever to go to mDest instead.
    // It also permit to avoid zigzag at the end of the path,
    // especially with mouse.
    thisPath.pop_back();
    thisPath.push_back(Position(mDest.x, mDest.y));

    setPath(thisPath);
}

void Being::clearPath()
{
    mPath.clear();
}

void Being::setPath(const Path &path)
{
    mPath = path;

    if ((Net::getNetworkType() == ServerInfo::EATHENA) &&
            mAction != WALK && mAction != DEAD)
    {
        nextTile();
        mWalkTime = tick_time;
    }
}

void Being::setSpeech(const std::string &text, int time)
{
    // Remove colors
    mSpeech = removeColors(text);

    // Trim whitespace
    trim(mSpeech);

    // Check for links
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
        if (mSpeech[start + 1] == '@' && mSpeech[start + 2] == '@')
        {
            mSpeech.erase(end, 1);
            mSpeech.erase(start, (position - start) + 1);
        }
        position = mSpeech.find('@');

        while (position != std::string::npos)
        {
            mSpeech.erase(position, 2);
            position = mSpeech.find('@');
        }

        start = mSpeech.find('[', start + 1);
        end = mSpeech.find(']', start);
    }

    if (!mSpeech.empty())
        mSpeechTime = time <= SPEECH_MAX_TIME ? time : SPEECH_MAX_TIME;

    const int speech = (int) config.getValue("speech", TEXT_OVERHEAD);
    if (speech == TEXT_OVERHEAD)
    {
        if (mText)
            delete mText;

        mText = new Text(mSpeech,
                         getPixelX(), getPixelY() - getHeight(),
                         gcn::Graphics::CENTER,
                         &guiPalette->getColor(Palette::PARTICLE),
                         true);
    }
}

void Being::takeDamage(Being *attacker, int amount, AttackType type)
{
    gcn::Font *font;
    std::string damage = amount ? toString(amount) : type == FLEE ?
            "dodge" : "miss";
    const gcn::Color *color;

    font = gui->getInfoParticleFont();

    // Selecting the right color
    if (type == CRITICAL || type == FLEE)
    {
        color = &guiPalette->getColor(Palette::HIT_CRITICAL);
    }
    else if (!amount)
    {
        if (attacker == player_node)
        {
            // This is intended to be the wrong direction to visually
            // differentiate between hits and misses
            color = &guiPalette->getColor(Palette::HIT_MONSTER_PLAYER);
        }
        else
        {
            color = &guiPalette->getColor(Palette::MISS);
        }
    }
    else if (getType() == MONSTER)
    {
        color = &guiPalette->getColor(Palette::HIT_PLAYER_MONSTER);
    }
    else
    {
        color = &guiPalette->getColor(Palette::HIT_MONSTER_PLAYER);
    }

    // Show damage number
    particleEngine->addTextSplashEffect(damage,
                                        getPixelX(), getPixelY() - 16,
                                        color, font, true);

    if (amount > 0)
    {
        if (getType() == MONSTER)
        {
            mDamageTaken += amount;
            updateName();
        }

        if (type != CRITICAL)
            effectManager->trigger(26, this);
        else
            effectManager->trigger(28, this);
    }
}

void Being::handleAttack(Being *victim, int damage, AttackType type)
{
    if (this != player_node)
        setAction(Being::ATTACK, 1);

    if (getType() == PLAYER && victim)
    {
        if (mEquippedWeapon)
        {
            fireMissile(victim, mEquippedWeapon->getMissileParticle());
        }
    }
    if (Net::getNetworkType() == ServerInfo::EATHENA)
    {
        mFrame = 0;
        mWalkTime = tick_time;
    }
}

void Being::setName(const std::string &name)
{
    mName = name;

    if (getShowName())
        showName();
}

void Being::setShowName(bool doShowName)
{
    bool oldShow = mShowName;
    mShowName = doShowName;

    if (doShowName != oldShow)
    {
        if (doShowName)
            showName();
        else
        {
            delete mDispName;
            mDispName = 0;
        }
    }
}

void Being::setGuildName(const std::string &name)
{
    logger->log("Got guild name \"%s\" for being %s(%i)", name.c_str(), mName.c_str(), mId);
}


void Being::setGuildPos(const std::string &pos)
{
    logger->log("Got guild position \"%s\" for being %s(%i)", pos.c_str(), mName.c_str(), mId);
}

void Being::setMap(Map *map)
{
    // Remove sprite from potential previous map
    if (mMap)
        mMap->removeSprite(mMapSprite);

    mMap = map;

    // Add sprite to potential new map
    if (mMap)
        mMapSprite = mMap->addSprite(this);

    // Clear particle effect list because child particles became invalid
    mChildParticleEffects.clear();
    mMustResetParticles = true; // Reset status particles on next redraw
}

void Being::controlParticle(Particle *particle)
{
    mChildParticleEffects.addLocally(particle);
}

void Being::fireMissile(Being *victim, const std::string &particle)
{
    if (!victim || particle.empty())
        return;

    Particle *target = particleEngine->createChild();
    Particle *missile = target->addEffect(particle, getPixelX(), getPixelY());

    if (missile)
    {
        target->setLifetime(2000);
        target->moveBy(Vector(0.0f, 0.0f, 32.0f));
        victim->controlParticle(target);

        missile->setDestination(target, 7, 0);
        missile->setDieDistance(8);
        missile->setLifetime(900);
    }
}

void Being::setAction(Action action, int attackType)
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
                currentAction = mEquippedWeapon->getAttackType();
            else
                currentAction = ACTION_ATTACK;

            for (SpriteIterator it = mSprites.begin(); it != mSprites.end(); it++)
                if (*it)
                    (*it)->reset();
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
        for (SpriteIterator it = mSprites.begin(); it != mSprites.end(); it++)
            if (*it)
                (*it)->play(currentAction);
        mAction = action;
    }
}

void Being::setDirection(Uint8 direction)
{
    if (mDirection == direction)
        return;

    mDirection = direction;

    // if the direction does not change much, keep the common component
    int mFaceDirection = mDirection & direction;
    if (!mFaceDirection)
        mFaceDirection = direction;

    SpriteDirection dir;
    if (mFaceDirection & UP)
        dir = DIRECTION_UP;
    else if (mFaceDirection & DOWN)
        dir = DIRECTION_DOWN;
    else if (mFaceDirection & RIGHT)
        dir = DIRECTION_RIGHT;
    else
        dir = DIRECTION_LEFT;
    mSpriteDirection = dir;

    for (SpriteIterator it = mSprites.begin(); it != mSprites.end(); it++)
        if (*it)
           (*it)->setDirection(dir);
}

/** TODO: Used by eAthena only */
void Being::nextTile()
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

    if (!mMap->getWalk(pos.x, pos.y, getWalkMask()))
    {
        setAction(STAND);
        return;
    }

    mX = pos.x;
    mY = pos.y;
    setAction(WALK);
    mWalkTime += (int)(mWalkSpeed.x / 10);
}

void Being::logic()
{
    // Reduce the time that speech is still displayed
    if (mSpeechTime > 0)
        mSpeechTime--;

    // Remove text and speechbubbles if speech boxes aren't being used
    if (mSpeechTime == 0 && mText)
    {
        delete mText;
        mText = 0;
    }

    if ((Net::getNetworkType() == ServerInfo::MANASERV) && (mAction != DEAD))
    {
        const Vector dest = (mPath.empty()) ?
            mDest : Vector(mPath.front().x,
                           mPath.front().y);

        // The Vector representing the difference between current position
        // and the next destination path node.
        Vector dir = dest - mPos;

        const float nominalLength = dir.length();

        // When we've not reached our destination, move to it.
        if (nominalLength > 1.0f && !mWalkSpeed.isNull())
        {
            // The deplacement of a point along a vector is calculated
            // using the Unit Vector (â) multiplied by the point speed.
            // â = a / ||a|| (||a|| is the a length.)
            // Then, diff = (dir/||dir||) * speed.
            const Vector normalizedDir = dir.normalized();
            Vector diff(normalizedDir.x * mWalkSpeed.x,
                        normalizedDir.y * mWalkSpeed.y);

            // Test if we don't miss the destination by a move too far:
            if (diff.length() > nominalLength)
            {
                setPosition(mPos + dir);

                // Also, if the destination is reached, try to get the next
                // path point, if existing.
                if (!mPath.empty())
                    mPath.pop_front();
            }
            // Otherwise, go to it using the nominal speed.
            else
                setPosition(mPos + diff);

            if (mAction != WALK)
                setAction(WALK);

            // Update the player sprite direction
            int direction = 0;
            const float dx = std::abs(dir.x);
            float dy = std::abs(dir.y);

            // When not using mouse for the player, we slightly prefer
            // UP and DOWN position, especially when walking diagonally.
            if (this == player_node && !player_node->isPathSetByMouse())
                dy = dy + 2;

            if (dx > dy)
                 direction |= (dir.x > 0) ? RIGHT : LEFT;
            else
                 direction |= (dir.y > 0) ? DOWN : UP;

            setDirection(direction);
        }
        else if (!mPath.empty())
        {
            // If the current path node has been reached,
            // remove it and go to the next one.
            mPath.pop_front();
        }
        else if (mAction == WALK)
        {
            setAction(STAND);
        }
    }
    else if (Net::getNetworkType() == ServerInfo::EATHENA)
    {
        // Update pixel coordinates
        setPosition(mX * 32 + 16 + getXOffset(),
                    mY * 32 + 32 + getYOffset());
    }

    if (mEmotion != 0)
    {
        mEmotionTime--;
        if (mEmotionTime == 0)
            mEmotion = 0;
    }

    // Update sprite animations
    if (mUsedTargetCursor)
        mUsedTargetCursor->update(tick_time * MILLISECONDS_IN_A_TICK);

    for (SpriteIterator it = mSprites.begin(); it != mSprites.end(); it++)
        if (*it)
            (*it)->update(tick_time * MILLISECONDS_IN_A_TICK);

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

void Being::draw(Graphics *graphics, int offsetX, int offsetY) const
{
    // TODO: Eventually, we probably should fix all sprite offsets so that
    //       these translations aren't necessary anymore. The sprites know
    //       best where their base point should be.
    const int px = getPixelX() + offsetX - 16;
    // Temporary fix to the Y offset.
    const int py = getPixelY() + offsetY -
        ((Net::getNetworkType() == ServerInfo::MANASERV) ? 15 : 32);

    if (mUsedTargetCursor)
        mUsedTargetCursor->draw(graphics, px, py);

    for (SpriteConstIterator it = mSprites.begin(); it != mSprites.end(); it++)
    {
        if (*it)
        {
            if ((*it)->getAlpha() != mAlpha)
                (*it)->setAlpha(mAlpha);
            (*it)->draw(graphics, px, py);
        }
    }
}

void Being::drawSpriteAt(Graphics *graphics, int x, int y) const
{
    const int px = x - 16;
    const int py = y - 32;

    for (SpriteConstIterator it = mSprites.begin(); it != mSprites.end(); it++)
    {
        if (*it)
        {
            if ((*it)->getAlpha() != mAlpha)
                (*it)->setAlpha(mAlpha);
            (*it)->draw(graphics, px, py);
        }
    }
}

void Being::drawEmotion(Graphics *graphics, int offsetX, int offsetY)
{
    if (!mEmotion)
        return;

    const int px = getPixelX() - offsetX - 16;
    const int py = getPixelY() - offsetY - 64 - 32;
    const int emotionIndex = mEmotion - 1;

    if (emotionIndex >= 0 && emotionIndex <= EmoteDB::getLast())
        EmoteDB::getAnimation(emotionIndex)->draw(graphics, px, py);
}

void Being::drawSpeech(int offsetX, int offsetY)
{
    const int px = getPixelX() - offsetX;
    const int py = getPixelY() - offsetY;
    const int speech = (int) config.getValue("speech", TEXT_OVERHEAD);

    // Draw speech above this being
    if (mSpeechTime == 0)
    {
        if (mSpeechBubble->isVisible())
            mSpeechBubble->setVisible(false);
    }
    else if (mSpeechTime > 0 && (speech == NAME_IN_BUBBLE ||
             speech == NO_NAME_IN_BUBBLE))
    {
        const bool showName = (speech == NAME_IN_BUBBLE);

        if (mText)
        {
            delete mText;
            mText = NULL;
        }

        mSpeechBubble->setCaption(showName ? mName : "", mTextColor);

        mSpeechBubble->setText(mSpeech, showName);
        mSpeechBubble->setPosition(px - (mSpeechBubble->getWidth() / 2),
                                   py - getHeight() - (mSpeechBubble->getHeight()));
        mSpeechBubble->setVisible(true);
    }
    else if (mSpeechTime > 0 && speech == TEXT_OVERHEAD)
    {
        mSpeechBubble->setVisible(false);

        if (! mText)
        {
            mText = new Text(mSpeech,
                             getPixelX(), getPixelY() - getHeight(),
                             gcn::Graphics::CENTER,
                             &guiPalette->getColor(Palette::PARTICLE),
                             true);
        }
    }
    else if (speech == NO_SPEECH)
    {
        mSpeechBubble->setVisible(false);

        if (mText)
            delete mText;

        mText = NULL;
    }
}

void Being::setStatusEffectBlock(int offset, Uint16 newEffects)
{
    for (int i = 0; i < STATUS_EFFECTS; i++)
    {
        int index = StatusEffect::blockEffectIndexToEffectIndex(offset + i);

        if (index != -1)
            setStatusEffect(index, (newEffects & (1 << i)) > 0);
    }
}

void Being::handleStatusEffect(StatusEffect *effect, int effectId)
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

    if (active != wasActive)
    {
        updateStatusEffect(index, active);
        if (active)
            mStatusEffects.insert(index);
        else
            mStatusEffects.erase(index);
    }
}

/** TODO: eAthena only */
int Being::getOffset(char pos, char neg) const
{
    // Check whether we're walking in the requested direction
    if (mAction != WALK ||  !(mDirection & (pos | neg)))
        return 0;

    int offset = 0;

    if (mMap)
    {
        offset = (pos == LEFT && neg == RIGHT) ?
                 (int)((get_elapsed_time(mWalkTime)
                        * mMap->getTileWidth()) / mWalkSpeed.x) :
                 (int)((get_elapsed_time(mWalkTime)
                        * mMap->getTileHeight()) / mWalkSpeed.y);
    }

    // We calculate the offset _from_ the _target_ location
    offset -= 32;
    if (offset > 0)
        offset = 0;

    // Going into negative direction? Invert the offset.
    if (mDirection & pos)
        offset = -offset;

    return offset;
}

int Being::getWidth() const
{
    AnimatedSprite *base = NULL;

    for (SpriteConstIterator it = mSprites.begin(); it != mSprites.end(); it++)
        if ((base = (*it)))
            break;

    if (base)
        return std::max(base->getWidth(), DEFAULT_BEING_WIDTH);

    return DEFAULT_BEING_WIDTH;
}

int Being::getHeight() const
{
    AnimatedSprite *base = NULL;

    for (SpriteConstIterator it = mSprites.begin(); it != mSprites.end(); it++)
        if ((base = (*it)))
            break;

    if (base)
        return std::max(base->getHeight(), DEFAULT_BEING_HEIGHT);

    return DEFAULT_BEING_HEIGHT;
}

void Being::setTargetAnimation(SimpleAnimation *animation)
{
    mUsedTargetCursor = animation;
    mUsedTargetCursor->reset();
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

void Being::internalTriggerEffect(int effectId, bool sfx, bool gfx)
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

void Being::updateCoords()
{
    if (mDispName)
    {
        mDispName->adviseXY(getPixelX(), getPixelY());
    }
}

void Being::flashName(int time)
{
    if (mDispName)
        mDispName->flash(time);
}

void Being::showName()
{
    delete mDispName;
    mDispName = 0;
    std::string mDisplayName(mName);

    if (getType() == PLAYER)
    {
        if (config.getValue("showgender", false))
        {
            Player* player =  static_cast<Player*>(this);
            if (player)
            {
                if (player->getGender() == GENDER_FEMALE)
                    mDisplayName += " \u2640";
                else
                    mDisplayName += " \u2642";
            }
        }
    }
    else if (getType() == MONSTER)
    {
        if (config.getValue("showMonstersTakedDamage", false))
        {
            mDisplayName += ", " + toString(getDamageTaken());
        }
    }

    mDispName = new FlashText(mDisplayName, getPixelX(), getPixelY(),
                             gcn::Graphics::CENTER, mNameColor);
}

int Being::getNumberOfLayers() const
{
    return mSprites.size();
}

void Being::load()
{
    // Hairstyles are encoded as negative numbers. Count how far negative
    // we can go.
    int hairstyles = 1;

    while (ItemDB::get(-hairstyles).getSprite(GENDER_MALE) != "error.xml")
        hairstyles++;

    mNumberOfHairstyles = hairstyles;
}

void Being::updateName()
{
    if (mShowName)
        showName();
}
