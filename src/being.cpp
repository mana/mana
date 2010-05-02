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
#include "sprite.h"
#include "text.h"
#include "statuseffect.h"

#include "gui/gui.h"
#include "gui/speechbubble.h"
#include "gui/theme.h"
#include "gui/userpalette.h"

#include "resources/colordb.h"
#include "resources/emotedb.h"
#include "resources/image.h"
#include "resources/itemdb.h"
#include "resources/iteminfo.h"
#include "resources/resourcemanager.h"

#include "net/net.h"
#include "net/playerhandler.h"

#include "utils/dtor.h"
#include "utils/stringutils.h"
#include "utils/xml.h"

#include <cassert>
#include <cmath>

#define HAIR_FILE "hair.xml"

static const int DEFAULT_BEING_WIDTH = 32;
static const int DEFAULT_BEING_HEIGHT = 32;


int Being::mNumberOfHairstyles = 1;

// TODO: mWalkTime used by eAthena only
Being::Being(int id, int subtype, Map *map):
    ActorSprite(id),
    mFrame(0),
    mWalkTime(0),
    mEmotion(0), mEmotionTime(0),
    mSpeechTime(0),
    mAttackSpeed(350),
    mAction(STAND),
    mSubType(subtype),
    mDirection(DOWN),
    mSpriteDirection(DIRECTION_DOWN),
    mDispName(0),
    mShowName(false),
    mEquippedWeapon(NULL),
    mText(0),
    mX(0), mY(0),
    mDamageTaken(0)
{
    setMap(map);

    mSpeechBubble = new SpeechBubble;

    mNameColor = &userPalette->getColor(UserPalette::NPC);
    mTextColor = &Theme::getThemeColor(Theme::CHAT);
    mWalkSpeed = Net::getPlayerHandler()->getDefaultWalkSpeed();
}

Being::~Being()
{
    delete mSpeechBubble;
    delete mDispName;
    delete mText;
}

void Being::setPosition(const Vector &pos)
{
    Actor::setPosition(pos);

    updateCoords();

    if (mText)
        mText->adviseXY((int)pos.x,
                        (int)pos.y - getHeight() - mText->getHeight() - 6);
}

void Being::setDestination(int dstX, int dstY)
{
    if (Net::getNetworkType() == ServerInfo::TMWATHENA)
    {
        if (mMap)
            setPath(mMap->findPath(mX, mY, dstX, dstY, getWalkMask()));
        return;
    }

    // Manaserv's part:

    // We can't calculate anything without a map anyway.
    if (!mMap)
        return;

    // Don't handle flawed destinations from server...
    if (dstX == 0 || dstY == 0)
        return;

    // If the destination is unwalkable, don't bother trying to get there
    if (!mMap->getWalk(dstX / 32, dstY / 32))
        return;

    Position dest = mMap->checkNodeOffsets(getCollisionRadius(), getWalkMask(),
                                           dstX, dstY);
    Path thisPath = mMap->findPixelPath(mPos.x, mPos.y, dest.x, dest.y,
                                   getCollisionRadius(), getWalkMask());

    if (thisPath.empty())
    {
        // If there is no path but the destination is on the same walkable tile,
        // we accept it.
        if ((int)mPos.x / 32 == dest.x / 32
            && (int)mPos.y / 32 == dest.y / 32)
        {
            mDest.x = dest.x;
            mDest.y = dest.y;
        }
        setPath(Path());
        return;
    }

    // The destination is valid, so we set it.
    mDest.x = dest.x;
    mDest.y = dest.y;

    setPath(thisPath);
}

void Being::clearPath()
{
    mPath.clear();
}

void Being::setPath(const Path &path)
{
    mPath = path;

    if ((Net::getNetworkType() == ServerInfo::TMWATHENA) &&
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
                         &userPalette->getColor(UserPalette::PARTICLE),
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
        color = &userPalette->getColor(UserPalette::HIT_CRITICAL);
    }
    else if (!amount)
    {
        if (attacker == player_node)
        {
            // This is intended to be the wrong direction to visually
            // differentiate between hits and misses
            color = &userPalette->getColor(UserPalette::HIT_MONSTER_PLAYER);
        }
        else
        {
            color = &userPalette->getColor(UserPalette::MISS);
        }
    }
    else if (getType() == MONSTER)
    {
        color = &userPalette->getColor(UserPalette::HIT_PLAYER_MONSTER);
    }
    else
    {
        color = &userPalette->getColor(UserPalette::HIT_MONSTER_PLAYER);
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
    if (Net::getNetworkType() == ServerInfo::TMWATHENA)
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

            reset();
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
        play(currentAction);
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

    CompoundSprite::setDirection(dir);
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

int Being::getCollisionRadius() const
{
    // FIXME: Get this from XML file
    return 16;
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

        // This is a hack that stops NPCs from running off the map...
        if (mDest.x <= 0 && mDest.y <= 0)
            return;

        // The Vector representing the difference between current position
        // and the next destination path node.
        Vector dir = dest - mPos;

        const float nominalLength = dir.length();

        // When we've not reached our destination, move to it.
        if (nominalLength > 0.0f && !mWalkSpeed.isNull())
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

            // Update the player sprite direction.
            // N.B.: We only change this if the distance is more than one pixel.
            if (nominalLength > 1.0f)
            {
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
    else if (Net::getNetworkType() == ServerInfo::TMWATHENA)
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

    ActorSprite::logic();
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
                             &userPalette->getColor(UserPalette::PARTICLE),
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
    return std::max(CompoundSprite::getWidth(), DEFAULT_BEING_WIDTH);
}

int Being::getHeight() const
{
    return std::max(CompoundSprite::getHeight(), DEFAULT_BEING_HEIGHT);
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
    return size();
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
