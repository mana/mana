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

#include "actorspritemanager.h"
#include "animatedsprite.h"
#include "client.h"
#include "configuration.h"
#include "effectmanager.h"
#include "graphics.h"
#include "guild.h"
#include "localplayer.h"
#include "log.h"
#include "map.h"
#include "particle.h"
#include "party.h"
#include "simpleanimation.h"
#include "sound.h"
#include "sprite.h"
#include "text.h"
#include "statuseffect.h"

#include "gui/buy.h"
#include "gui/buysell.h"
#include "gui/gui.h"
#include "gui/npcdialog.h"
#include "gui/npcpostdialog.h"
#include "gui/sell.h"
#include "gui/socialwindow.h"
#include "gui/speechbubble.h"
#include "gui/theme.h"
#include "gui/userpalette.h"

#include "net/charhandler.h"
#include "net/gamehandler.h"
#include "net/net.h"
#include "net/npchandler.h"
#include "net/playerhandler.h"

#include "resources/beinginfo.h"
#include "resources/colordb.h"
#include "resources/emotedb.h"
#include "resources/image.h"
#include "resources/itemdb.h"
#include "resources/iteminfo.h"
#include "resources/monsterdb.h"
#include "resources/npcdb.h"
#include "resources/resourcemanager.h"

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
Being::Being(int id, Type type, int subtype, Map *map):
    ActorSprite(id),
    mInfo(BeingInfo::Unknown),
    mActionTime(0),
    mEmotion(0), mEmotionTime(0),
    mSpeechTime(0),
    mAttackType(1),
    mAttackSpeed(350),
    mAction(STAND),
    mSubType(0xFFFF),
    mDirection(DOWN),
    mSpriteDirection(DIRECTION_DOWN),
    mDispName(0),
    mShowName(false),
    mEquippedWeapon(NULL),
    mText(0),
    mGender(GENDER_UNSPECIFIED),
    mParty(NULL),
    mIsGM(false),
    mType(type),
    mX(0), mY(0),
    mDamageTaken(0)
{
    setMap(map);
    setSubtype(subtype);

    mSpeechBubble = new SpeechBubble;

    mWalkSpeed = Net::getPlayerHandler()->getDefaultWalkSpeed();

    if (getType() == PLAYER)
        mShowName = config.getBoolValue("visiblenames");

    config.addListener("visiblenames", this);

    if (getType() == PLAYER || getType() == NPC)
        setShowName(true);

    updateColors();
}

Being::~Being()
{
    config.removeListener("visiblenames", this);

    delete mSpeechBubble;
    delete mDispName;
    delete mText;
    mSpeechBubble = 0;
    mDispName = 0;
    mText = 0;
}

void Being::setSubtype(Uint16 subtype)
{
    if (subtype == mSubType)
        return;

    mSubType = subtype;

    if (getType() == MONSTER)
    {
        mInfo = MonsterDB::get(mSubType);
        setName(mInfo->getName());
        setupSpriteDisplay(mInfo->getDisplay());
    }
    else if (getType() == NPC)
    {
        mInfo = NPCDB::get(mSubType);
        setupSpriteDisplay(mInfo->getDisplay(), false);
    }
    else if (getType() == PLAYER)
    {
        int id = -100 - subtype;

        // Prevent showing errors when sprite doesn't exist
        if (!ItemDB::exists(id))
            id = -100;

        setSprite(Net::getCharHandler()->baseSprite(), id);
    }
}

ActorSprite::TargetCursorSize Being::getTargetCursorSize() const
{
    return mInfo->getTargetCursorSize();
}

unsigned char Being::getWalkMask() const
{
    return mInfo->getWalkMask();
}

Map::BlockType Being::getBlockType() const
{
    return mInfo->getBlockType();
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
    Path thisPath = mMap->findPixelPath((int) mPos.x, (int) mPos.y,
                                        dest.x, dest.y,
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
            mAction != MOVE && mAction != DEAD)
    {
        nextTile();
        mActionTime = tick_time;
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

    const int speech = config.getIntValue("speech");
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
        sound.playSfx(mInfo->getSound(SOUND_EVENT_HURT));

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

    if (getType() == PLAYER && victim && mEquippedWeapon)
        fireMissile(victim, mEquippedWeapon->getMissileParticle());
    else
        fireMissile(victim, mInfo->getAttack(mAttackType)->missileParticle);

    if (Net::getNetworkType() == ServerInfo::TMWATHENA)
    {
        reset();
        mActionTime = tick_time;
    }

    sound.playSfx(mInfo->getSound((damage > 0) ?
                  SOUND_EVENT_HIT : SOUND_EVENT_MISS));
}

void Being::setName(const std::string &name)
{
    if (getType() == NPC)
    {
        mName = name.substr(0, name.find('#', 0));
        showName();
    }
    else
    {
        mName = name;

        if (getType() == PLAYER && getShowName())
            showName();
    }
}

void Being::setShowName(bool doShowName)
{
    if (mShowName == doShowName)
        return;

    mShowName = doShowName;

    if (doShowName)
        showName();
    else
    {
        delete mDispName;
        mDispName = 0;
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

void Being::addGuild(Guild *guild)
{
    mGuilds[guild->getId()] = guild;
    guild->addMember(mId, mName);

    if (this == player_node && socialWindow)
    {
        socialWindow->addTab(guild);
    }
}

void Being::removeGuild(int id)
{
    if (this == player_node && socialWindow)
    {
        socialWindow->removeTab(mGuilds[id]);
    }

    mGuilds[id]->removeMember(mId);
    mGuilds.erase(id);
}

Guild *Being::getGuild(const std::string &guildName) const
{
    std::map<int, Guild*>::const_iterator itr, itr_end = mGuilds.end();
    for (itr = mGuilds.begin(); itr != itr_end; ++itr)
    {
        Guild *guild = itr->second;
        if (guild->getName() == guildName)
        {
            return guild;
        }
    }

    return NULL;
}

Guild *Being::getGuild(int id) const
{
    std::map<int, Guild*>::const_iterator itr;
    itr = mGuilds.find(id);
    if (itr != mGuilds.end())
    {
        return itr->second;
    }

    return NULL;
}

void Being::clearGuilds()
{
    std::map<int, Guild*>::const_iterator itr, itr_end = mGuilds.end();
    for (itr = mGuilds.begin(); itr != itr_end; ++itr)
    {
        Guild *guild = itr->second;

        if (this == player_node && socialWindow)
            socialWindow->removeTab(guild);

        guild->removeMember(mId);
    }

    mGuilds.clear();
}

void Being::setParty(Party *party)
{
    if (party == mParty)
        return;

    Party *old = mParty;
    mParty = party;

    if (old)
    {
        old->removeMember(mId);
    }

    if (party)
    {
        party->addMember(mId, mName);
    }

    updateColors();

    if (this == player_node && socialWindow)
    {
        if (old)
            socialWindow->removeTab(old);

        if (party)
            socialWindow->addTab(party);
    }
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
    std::string currentAction = SpriteAction::INVALID;

    switch (action)
    {
        case MOVE:
            currentAction = SpriteAction::MOVE;
            // Note: When adding a run action,
            // Differentiate walk and run with action name,
            // while using only the ACTION_MOVE.
            break;
        case SIT:
            currentAction = SpriteAction::SIT;
            break;
        case ATTACK:
            if (mEquippedWeapon)
            {
                currentAction = mEquippedWeapon->getAttackAction();
                reset();
            }
            else
            {
                mAttackType = attackType;
                currentAction = mInfo->getAttack(attackType)->action;
                reset();

                if (Net::getNetworkType() == ServerInfo::MANASERV)
                {
                    int rotation = 0;
                    //attack particle effect
                    std::string particleEffect = mInfo->getAttack(attackType)
                                                 ->particleEffect;
                    if (!particleEffect.empty() && Particle::enabled)
                    {
                        switch (mSpriteDirection)
                        {
                           case DIRECTION_DOWN: rotation = 0; break;
                            case DIRECTION_LEFT: rotation = 90; break;
                            case DIRECTION_UP: rotation = 180; break;
                            case DIRECTION_RIGHT: rotation = 270; break;
                            default: break;
                        }
                        Particle *p;
                        p = particleEngine->addEffect(particleEffect, 0, 0,
                                                      rotation);
                        controlParticle(p);
                    }
                }
            }

            break;
        case HURT:
            //currentAction = SpriteAction::HURT;// Buggy: makes the player stop
                                            // attacking and unable to attack
                                            // again until he moves.
                                            // TODO: fix this!
            break;
        case DEAD:
            currentAction = SpriteAction::DEAD;
            sound.playSfx(mInfo->getSound(SOUND_EVENT_DIE));
            break;
        case STAND:
            currentAction = SpriteAction::STAND;
            break;
    }

    if (currentAction != SpriteAction::INVALID)
    {
        play(currentAction);
        mAction = action;
    }

    if (currentAction != SpriteAction::MOVE)
        mActionTime = tick_time;
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
    setAction(MOVE);
    mActionTime += (int)(mWalkSpeed.x / 10);
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

            if (mAction != MOVE)
                setAction(MOVE);

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
        else if (mAction == MOVE)
        {
            setAction(STAND);
        }
    }
    else if (Net::getNetworkType() == ServerInfo::TMWATHENA)
    {
        int frameCount = getFrameCount();

        switch (mAction)
        {
            case STAND:
            case SIT:
            case DEAD:
            case HURT:
               break;

            case MOVE:
                if ((int) ((get_elapsed_time(mActionTime) * frameCount)
                        / getWalkSpeed().x) >= frameCount)
                    nextTile();
                break;

            case ATTACK:
                int rotation = 0;
                std::string particleEffect = "";

                int curFrame = (get_elapsed_time(mActionTime) * frameCount)
                               / mAttackSpeed;

                //attack particle effect
                if (mEquippedWeapon)
                {
                    particleEffect = mEquippedWeapon->getParticleEffect();

                    if (!particleEffect.empty() &&
                        findSameSubstring(particleEffect,
                                     paths.getStringValue("particles")).empty())
                        particleEffect = paths.getStringValue("particles")
                                         + particleEffect;
                }
                else
                {
                    particleEffect = mInfo->getAttack(mAttackType)
                                             ->particleEffect;
                }

                if (!particleEffect.empty() && Particle::enabled
                    && curFrame == 1)
                {
                    switch (mDirection)
                    {
                        case DOWN: rotation = 0; break;
                        case LEFT: rotation = 90; break;
                        case UP: rotation = 180; break;
                        case RIGHT: rotation = 270; break;
                        default: break;
                    }
                    Particle *p;
                    p = particleEngine->addEffect(particleEffect, 0, 0,
                                                  rotation);
                    controlParticle(p);
                }

                if (curFrame >= frameCount)
                    nextTile();

                break;
        }

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

    int frameCount = getFrameCount();
    if (frameCount < 10)
        frameCount = 10;

    if (!isAlive() && Net::getGameHandler()->removeDeadBeings() &&
        (int) ((get_elapsed_time(mActionTime)
                / getWalkSpeed().x) >= frameCount))
    {
        if (getType() != PLAYER)
            actorSpriteManager->destroy(this);
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
    const int speech = config.getIntValue("speech");

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
    if (mAction != MOVE ||  !(mDirection & (pos | neg)))
        return 0;

    int offset = 0;

    if (mMap)
    {
        offset = (pos == LEFT && neg == RIGHT) ?
                 (int)((get_elapsed_time(mActionTime)
                        * mMap->getTileWidth()) / mWalkSpeed.x) :
                 (int)((get_elapsed_time(mActionTime)
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
    if (!mDispName)
        return;

    // Monster names show above the sprite instead of below it
    if (getType() == MONSTER)
        mDispName->adviseXY(getPixelX(),
                getPixelY() - getHeight() - mDispName->getHeight());
    else
        mDispName->adviseXY(getPixelX(), getPixelY());
}

void Being::optionChanged(const std::string &value)
{
    if (getType() == PLAYER && value == "visiblenames")
    {
        setShowName(config.getBoolValue("visiblenames"));
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

    if (config.getBoolValue("showgender"))
    {
        if (getGender() == GENDER_FEMALE)
            mDisplayName += " \u2640";
        else if (getGender() == GENDER_MALE)
            mDisplayName += " \u2642";
    }

    if (getType() == MONSTER)
    {
        if (config.getBoolValue("showMonstersTakedDamage"))
        {
            mDisplayName += ", " + toString(getDamageTaken());
        }
    }

    gcn::Font *font = 0;
    if (player_node && player_node->getTarget() == this
        && getType() != MONSTER)
    {
        font = boldFont;
    }

    mDispName = new FlashText(mDisplayName, getPixelX(), getPixelY(),
                              gcn::Graphics::CENTER, mNameColor, font);

    updateCoords();
}

void Being::updateColors()
{
    if (getType() == MONSTER)
    {
        mNameColor = &userPalette->getColor(UserPalette::MONSTER);
        mTextColor = &userPalette->getColor(UserPalette::MONSTER);
    }
    else if (getType() == NPC)
    {
        mNameColor = &userPalette->getColor(UserPalette::NPC);
        mTextColor = &userPalette->getColor(UserPalette::NPC);
    }
    else if (this == player_node)
    {
        mNameColor = &userPalette->getColor(UserPalette::SELF);
        mTextColor = &Theme::getThemeColor(Theme::PLAYER);
    }
    else
    {
        mTextColor = &userPalette->getColor(Theme::PLAYER);

        if (mIsGM)
        {
            mTextColor = &userPalette->getColor(UserPalette::GM);
            mNameColor = &userPalette->getColor(UserPalette::GM);
        }
        else if (mParty && mParty == player_node->getParty())
        {
            mNameColor = &userPalette->getColor(UserPalette::PARTY);
        }
        else
        {
            mNameColor = &userPalette->getColor(UserPalette::PC);
        }
    }

    if (mDispName)
    {
        mDispName->setColor(mNameColor);
    }
}

void Being::setSprite(unsigned int slot, int id, const std::string &color,
                      bool isWeapon)
{
    assert(slot < Net::getCharHandler()->maxSprite());

    if (slot >= size())
        ensureSize(slot + 1);

    if (slot >= mSpriteIDs.size())
        mSpriteIDs.resize(slot + 1, 0);

    if (slot >= mSpriteColors.size())
        mSpriteColors.resize(slot + 1, "");

    // id = 0 means unequip
    if (id == 0)
    {
        removeSprite(slot);

        if (isWeapon)
            mEquippedWeapon = NULL;
    }
    else
    {
        std::string filename = ItemDB::get(id).getSprite(mGender);
        AnimatedSprite *equipmentSprite = NULL;

        if (!filename.empty())
        {
            if (!color.empty())
                filename += "|" + color;

            equipmentSprite = AnimatedSprite::load(
                                    paths.getStringValue("sprites") + filename);
        }

        if (equipmentSprite)
            equipmentSprite->setDirection(getSpriteDirection());

        CompoundSprite::setSprite(slot, equipmentSprite);

        if (isWeapon)
            mEquippedWeapon = &ItemDB::get(id);

        setAction(mAction);
    }

    mSpriteIDs[slot] = id;
    mSpriteColors[slot] = color;
}

void Being::setSpriteID(unsigned int slot, int id)
{
    setSprite(slot, id, mSpriteColors[slot]);
}

void Being::setSpriteColor(unsigned int slot, const std::string &color)
{
    setSprite(slot, mSpriteIDs[slot], color);
}

int Being::getNumberOfLayers() const
{
    return CompoundSprite::getNumberOfLayers();
}

void Being::load()
{
    // Hairstyles are encoded as negative numbers. Count how far negative
    // we can go.
    int hairstyles = 1;

    while (ItemDB::get(-hairstyles).getSprite(GENDER_MALE) !=
                                        paths.getStringValue("spriteErrorFile"))
        hairstyles++;

    mNumberOfHairstyles = hairstyles;
}

void Being::updateName()
{
    if (mShowName)
        showName();
}

void Being::setGender(Gender gender)
{
    if (gender != mGender)
    {
        mGender = gender;

        // Reload all subsprites
        for (unsigned int i = 0; i < mSpriteIDs.size(); i++)
        {
            if (mSpriteIDs.at(i) != 0)
                setSprite(i, mSpriteIDs.at(i), mSpriteColors.at(i));
        }

        updateName();
    }
}

void Being::setGM(bool gm)
{
    mIsGM = gm;

    updateColors();
}

bool Being::canTalk()
{
    return mType == NPC;
}

void Being::talkTo()
{
    Net::getNpcHandler()->talk(mId);
}

bool Being::isTalking()
{
    return NpcDialog::isActive() || BuyDialog::isActive() ||
           SellDialog::isActive() || BuySellDialog::isActive() ||
           NpcPostDialog::isActive();
}
