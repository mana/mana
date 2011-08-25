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
#include "event.h"
#include "game.h"
#include "graphics.h"
#include "guild.h"
#include "localplayer.h"
#include "log.h"
#include "map.h"
#include "particle.h"
#include "party.h"
#include "playerrelations.h"
#include "simpleanimation.h"
#include "sound.h"
#include "text.h"

#include "gui/gui.h"
#include "gui/socialwindow.h"
#include "gui/speechbubble.h"

#include "net/charhandler.h"
#include "net/gamehandler.h"
#include "net/net.h"
#include "net/playerhandler.h"
#include "net/npchandler.h"

#include "resources/beinginfo.h"
#include "resources/colordb.h"
#include "resources/emotedb.h"
#include "resources/image.h"
#include "resources/itemdb.h"
#include "resources/iteminfo.h"
#include "resources/monsterdb.h"
#include "resources/npcdb.h"
#include "resources/theme.h"
#include "resources/userpalette.h"

#include "utils/stringutils.h"

#include <cassert>
#include <cmath>

#define HAIR_FILE "hair.xml"

int Being::mNumberOfHairstyles = 1;

Being::Being(int id, Type type, int subtype, Map *map):
    ActorSprite(id),
    mInfo(BeingInfo::Unknown),
    mActionTime(0),
    mSpeechTime(0),
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
    mSpeedPixelsPerTick(Vector(0.0f, 0.0f, 0.0f)),
    mDamageTaken(0),
    mIp(0)
{
    setMap(map);
    setSubtype(subtype);

    mSpeechBubble = new SpeechBubble;

    mMoveSpeed = Net::getPlayerHandler()->getDefaultMoveSpeed();

    if (getType() == PLAYER)
        mShowName = config.getBoolValue("visiblenames");

    if (getType() == PLAYER || getType() == NPC)
        setShowName(true);

    updateColors();
    listen(Event::ConfigChannel);
    listen(Event::ChatChannel);
}

Being::~Being()
{
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
        if (!itemDb->exists(id))
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

void Being::setMoveSpeed(const Vector &speed)
{
    mMoveSpeed = speed;
    // If we already can, recalculate the system speed right away.
    if (mMap)
        mSpeedPixelsPerTick =
                      Net::getPlayerHandler()->getPixelsPerTickMoveSpeed(speed);
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
    // We can't calculate anything without a map anyway.
    if (!mMap)
        return;

    // Don't handle flawed destinations from server...
    if (dstX <= 0 || dstY <= 0)
        return;

    // If the destination is unwalkable, don't bother trying to get there
    const int tileWidth = mMap->getTileWidth();
    const int tileHeight = mMap->getTileHeight();
    if (!mMap->getWalk(dstX / tileWidth, dstY / tileHeight))
        return;

    Position dest(0, 0);
    Path thisPath;
    if (Net::getPlayerHandler()->usePixelPrecision())
    {
        dest = mMap->checkNodeOffsets(getCollisionRadius(), getWalkMask(),
                                      dstX, dstY);
        thisPath = mMap->findPixelPath((int) mPos.x, (int) mPos.y,
                                       dest.x, dest.y,
                                       getCollisionRadius(), getWalkMask());
    }
    else
    {
        // We center the destination.
        dest.x = (dstX / tileWidth) * tileWidth + tileWidth / 2;
        dest.y = (dstY / tileHeight) * tileHeight + tileHeight / 2;
        // and find a tile centered pixel path
        thisPath = mMap->findTilePath((int) mPos.x, (int) mPos.y,
                                       dest.x, dest.y, getWalkMask());
    }

    if (thisPath.empty())
    {
        // If there is no path but the destination is on the same walkable tile,
        // we accept it.
        if ((int)mPos.x / tileWidth == dest.x / tileWidth
            && (int)mPos.y / tileHeight == dest.y / tileHeight)
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

void Being::takeDamage(Being *attacker, int amount,
                       AttackType type, int attackId)
{
    gcn::Font *font;
    std::string damage = amount ? toString(amount) : type == FLEE ?
            "dodge" : "miss";
    const gcn::Color *color;

    font = gui->getInfoParticleFont();

    // Selecting the right color
    if (type == CRITICAL || type == FLEE)
    {
        if (attacker == player_node)
        {
            color = &userPalette->getColor(
                UserPalette::HIT_LOCAL_PLAYER_CRITICAL);
        }
        else
        {
            color = &userPalette->getColor(UserPalette::HIT_CRITICAL);
        }
    }
    else if (!amount)
    {
        if (attacker == player_node)
        {
            // This is intended to be the wrong direction to visually
            // differentiate between hits and misses
            color = &userPalette->getColor(UserPalette::HIT_LOCAL_PLAYER_MISS);
        }
        else
        {
            color = &userPalette->getColor(UserPalette::MISS);
        }
    }
    else if (getType() == MONSTER)
    {
        if (attacker == player_node)
        {
            color = &userPalette->getColor(
                UserPalette::HIT_LOCAL_PLAYER_MONSTER);
        }
        else
        {
            color = &userPalette->getColor(
                UserPalette::HIT_PLAYER_MONSTER);
        }
    }
    else
    {
        color = &userPalette->getColor(UserPalette::HIT_MONSTER_PLAYER);
    }

    // Show damage number
    particleEngine->addTextSplashEffect(damage,
                                        getPixelX(), getPixelY() - getHeight(),
                                        color, font, true);

    if (amount > 0)
    {
        if (mInfo)
        {
            if (attacker)
            {
                sound.playSfx(mInfo->getSound(SOUND_EVENT_HURT),
                              attacker->getTileX(), attacker->getTileY());
            }
            else
            {
                sound.playSfx(mInfo->getSound(SOUND_EVENT_HURT));
            }
        }

        if (getType() == MONSTER)
        {
            mDamageTaken += amount;
            updateName();
        }

        // Init the particle effect path based on current weapon or default.
        int hitEffectId = 0;
        const ItemInfo *attackerWeapon = attacker ?
            attacker->getEquippedWeapon() : 0;

        if (attackerWeapon && attacker->getType() == PLAYER)
        {
            if (type != CRITICAL)
                hitEffectId = attackerWeapon->getHitEffectId();
            else
                hitEffectId = attackerWeapon->getCriticalHitEffectId();
        }
        else if (attacker && attacker->getType() == MONSTER)
        {
            const Attack *attack = attacker->getInfo()->getAttack(attackId);

            if (type != CRITICAL)
                hitEffectId = attack->mHitEffectId;
            else
                hitEffectId = attack->mCriticalHitEffectId;
        }
        else
        {
            if (type != CRITICAL)
                hitEffectId = paths.getIntValue("hitEffectId");
            else
                hitEffectId = paths.getIntValue("criticalHitEffectId");
        }
        effectManager->trigger(hitEffectId, this);
    }
}

void Being::handleAttack(Being *victim, int damage, int attackId)
{
    // Monsters, NPCs and remote players handle the first attack (id="1")
    // per default.
    // TODO: Fix this for Manaserv by sending the attack id.
    // TODO: Add attack type handling, see Attack struct and AttackType
    // and make use of it by grouping attacks per attack type and add random
    // attack use on tA, based on normal and critical attack types.
    if (this != player_node)
        setAction(Being::ATTACK, attackId);

    if (victim)
        lookAt(victim->getPosition());

    if (getType() == PLAYER && victim && mEquippedWeapon)
        fireMissile(victim, mEquippedWeapon->getMissileParticleFile());
    else
        fireMissile(victim,
                    mInfo->getAttack(attackId)->mMissileParticleFilename);

    sound.playSfx(mInfo->getSound((damage > 0) ?
                  SOUND_EVENT_HIT : SOUND_EVENT_MISS),
                  getPixelX(), getPixelY());
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
    logger->log("Got guild name \"%s\" for being %s(%i)", name.c_str(),
                mName.c_str(), mId);
}


void Being::setGuildPos(const std::string &pos)
{
    logger->log("Got guild position \"%s\" for being %s(%i)", pos.c_str(),
                mName.c_str(), mId);
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

    Particle *missile = particleEngine->addEffect(particle,
                                                  getPixelX(), getPixelY());

    if (missile)
    {
        Particle *target = particleEngine->createChild();
        target->moveBy(Vector(0.0f, 0.0f,
                       Game::instance()->getCurrentTileWidth()));
        target->setLifetime(1000);
        victim->controlParticle(target);

        missile->setDestination(target, 7, 0);
        missile->setDieDistance(8);
        missile->setLifetime(900);
    }

}

void Being::setAction(Action action, int attackId)
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
                currentAction = mInfo->getAttack(attackId)->mAction;
                reset();

                // Attack particle effect
                if (Particle::enabled)
                {
                    int effectId = mInfo->getAttack(attackId)->mEffectId;
                    int rotation = 0;
                    switch (mSpriteDirection)
                    {
                        case DIRECTION_DOWN: rotation = 0; break;
                        case DIRECTION_LEFT: rotation = 90; break;
                        case DIRECTION_UP: rotation = 180; break;
                        case DIRECTION_RIGHT: rotation = 270; break;
                        default: break;
                    }
                    effectManager->trigger(effectId, this, rotation);
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
            sound.playSfx(mInfo->getSound(SOUND_EVENT_DIE),
                          getPixelX(), getPixelY());
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

void Being::lookAt(const Vector &destPos)
{
    // We first handle simple cases

    // If the two positions are the same,
    // don't update the direction since it's only a matter of keeping
    // the previous one.
    if (mPos.x == destPos.x && mPos.y == destPos.y)
        return;

    if (mPos.x == destPos.x)
    {
        if (mPos.y > destPos.y)
            setDirection(UP);
        else
            setDirection(DOWN);
        return;
    }

    if (mPos.y == destPos.y)
    {
        if (mPos.x > destPos.x)
            setDirection(LEFT);
        else
            setDirection(RIGHT);
        return;
    }

    // Now let's handle diagonal cases
    // First, find the lower angle:
    if (mPos.x < destPos.x)
    {
        // Up-right direction
        if (mPos.y > destPos.y)
        {
            // Compute tan of the angle
            if ((mPos.y - destPos.y) / (destPos.x - mPos.x) < 1)
                // The angle is less than 45°, we look to the right
                setDirection(RIGHT);
            else
                setDirection(UP);
            return;
        }
        else // Down-right
        {
            // Compute tan of the angle
            if ((destPos.y - mPos.y) / (destPos.x - mPos.x) < 1)
                // The angle is less than 45°, we look to the right
                setDirection(RIGHT);
            else
                setDirection(DOWN);
            return;
        }
    }
    else
    {
        // Up-left direction
        if (mPos.y > destPos.y)
        {
            // Compute tan of the angle
            if ((mPos.y - destPos.y) / (mPos.x - destPos.x) < 1)
                // The angle is less than 45°, we look to the left
                setDirection(LEFT);
            else
                setDirection(UP);
            return;
        }
        else // Down-left
        {
            // Compute tan of the angle
            if ((destPos.y - mPos.y) / (mPos.x - destPos.x) < 1)
                // The angle is less than 45°, we look to the left
                setDirection(LEFT);
            else
                setDirection(DOWN);
            return;
        }
    }
}

void Being::setDirection(Uint8 direction)
{
    if (!direction || mDirection == direction)
        return;

    mDirection = direction;

    SpriteDirection dir;
    if (mDirection & UP)
        dir = DIRECTION_UP;
    else if (mDirection & DOWN)
        dir = DIRECTION_DOWN;
    else if (mDirection & RIGHT)
        dir = DIRECTION_RIGHT;
    else
        dir = DIRECTION_LEFT;
    mSpriteDirection = dir;

    CompoundSprite::setDirection(dir);
}

int Being::getCollisionRadius() const
{
    // FIXME: Get this from XML file once a better pathfinding algorithm is up.
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

    if ((mAction != DEAD) && !mSpeedPixelsPerTick.isNull())
    {
        const Vector dest = (mPath.empty()) ?
            mDest : Vector(mPath.front().x,
                           mPath.front().y);

        // Avoid going to flawed destinations
        if (dest.x <= 0 || dest.y <= 0)
        {
            // We make the being stop move in that case.
            mDest = mPos;
            mPath.clear();
            // By returning now, we're losing one tick for the rest of the logic
            // but as we have reset the destination, the next tick will be fine.
            return;
        }

        // The Vector representing the difference between current position
        // and the next destination path node.
        Vector dir = dest - mPos;

        float distance = dir.length();

        // When we've not reached our destination, move to it.
        if (distance > 0.0f)
        {
            // The deplacement of a point along a vector is calculated
            // using the Unit Vector (â) multiplied by the point speed.
            // â = a / ||a|| (||a|| is the a length.)
            // Then, diff = (dir/||dir||) * speed.
            const Vector normalizedDir = dir.normalized();
            Vector diff(normalizedDir.x * mSpeedPixelsPerTick.x,
                        normalizedDir.y * mSpeedPixelsPerTick.y);

            // Test if we don't miss the destination by a move too far:
            if (diff.length() > distance)
            {
                setPosition(mPos + dir);

                // Also, if the destination is reached, try to get the next
                // path point, if existing.
                if (!mPath.empty())
                    mPath.pop_front();
            }
            else
            {
                // Otherwise, go to it using the nominal speed.
                setPosition(mPos + diff);
                // And reset the nominalLength to the actual move length
                distance = diff.length();
            }

            if (mAction != MOVE)
                setAction(MOVE);

            // Update the player sprite direction.
            // N.B.: We only change this if the distance is more than one pixel
            // to avoid flawing the ending direction for players,
            // but always for very slow beings.
            float maxDistance = mSpeedPixelsPerTick.length();
            if (distance > ((maxDistance > 1.0f) ? 1.0f : 0.0f))
            {
                // The player direction is handled for keyboard
                // by LocalPlayer::startWalking(), we shouldn't get
                // in the way here for other cases.
                // Hence, we set the direction in Being::logic() only when:
                // 1. It is not the localPlayer
                // 2. When it is the localPlayer but only by mouse
                // (because in that case, the path can have more than one tile.)
                if ((player_node == this && player_node->isPathSetByMouse())
                    || player_node != this)
                {
                    int direction = 0;
                    const float dx = std::abs(dir.x);
                    float dy = std::abs(dir.y);

                    if (dx > dy)
                        direction |= (dir.x > 0) ? RIGHT : LEFT;
                    else
                        direction |= (dir.y > 0) ? DOWN : UP;

                    setDirection(direction);
                }
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

    ActorSprite::logic();

    // Remove it after 1.5 secs if the dead animation isn't long enough,
    // or simply play it until it's finished.
    if (!isAlive() && Net::getGameHandler()->removeDeadBeings() &&
        get_elapsed_time(mActionTime) > std::max(getDuration(), 1500))
    {

        if (getType() != PLAYER)
            actorSpriteManager->destroy(this);
    }
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

void Being::updateCoords()
{
    if (!mDispName)
        return;

    // Monster names show above the sprite instead of below it
    if (getType() == MONSTER)
        mDispName->adviseXY(getPixelX(), getPixelY() - getHeight());
    else
        mDispName->adviseXY(getPixelX(), getPixelY() + mDispName->getHeight());
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

    Being* player =  static_cast<Being*>(this);
    if (player)
    {
        if (config.getBoolValue("showgender"))
        {
            if (getGender() == GENDER_FEMALE)
                mDisplayName += " \u2640";
            else if (getGender() == GENDER_MALE)
                mDisplayName += " \u2642";
        }

        // Display the IP when under tmw-Athena (GM only).
        if (Net::getNetworkType() == ServerInfo::TMWATHENA && player_node
        && player_node->getShowIp() && player->getIp())
        {
            mDisplayName += strprintf(" %s", ipToString(player->getIp()));
        }
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
    if (slot >= size())
        ensureSize(slot + 1);

    if (slot >= mSpriteIDs.size())
        mSpriteIDs.resize(slot + 1);

    if (slot >= mSpriteColors.size())
        mSpriteColors.resize(slot + 1);

    // id = 0 means unequip
    if (id == 0)
    {
        removeSprite(slot);

        if (isWeapon)
            mEquippedWeapon = 0;
    }
    else
    {
        std::string filename = itemDb->get(id).getSprite(mGender);
        AnimatedSprite *equipmentSprite = 0;

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
            mEquippedWeapon = &itemDb->get(id);

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

    while (itemDb->get(-hairstyles).getSprite(GENDER_MALE) !=
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

void Being::event(Event::Channel channel, const Event &event)
{
    if (channel == Event::ChatChannel &&
            (event.getType() == Event::Being
             || event.getType() == Event::Player) &&
            event.getInt("permissions") & PlayerRelation::SPEECH_FLOAT)
    {
        try
        {
            if (mId == event.getInt("beingId"))
            {
                setSpeech(event.getString("text"));
            }
        }
        catch (BadEvent badEvent)
        {}
    }
    else if (channel == Event::ConfigChannel &&
             event.getType() == Event::ConfigOptionChanged)
    {
        if (getType() == PLAYER && event.getString("option") == "visiblenames")
        {
            setShowName(config.getBoolValue("visiblenames"));
        }
    }

}

void Being::setMap(Map *map)
{
    ActorSprite::setMap(map);

    // Recalculate pixel/tick speed
    if (map && !mMoveSpeed.isNull())
    {
        mSpeedPixelsPerTick =
            Net::getPlayerHandler()->getPixelsPerTickMoveSpeed(mMoveSpeed, map);
    }
}
