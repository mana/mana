/*
 *  The Mana Client
 *  Copyright (C) 2004-2009  The Mana World Development Team
 *  Copyright (C) 2009-2012  The Mana Developers
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
#include "client.h"
#include "configuration.h"
#include "effectmanager.h"
#include "event.h"
#include "game.h"
#include "guild.h"
#include "localplayer.h"
#include "log.h"
#include "map.h"
#include "particle.h"
#include "party.h"
#include "playerrelations.h"
#include "sound.h"
#include "sprite.h"
#include "statuseffect.h"
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
#include "resources/itemdb.h"
#include "resources/iteminfo.h"
#include "resources/monsterdb.h"
#include "resources/npcdb.h"
#include "resources/statuseffectdb.h"
#include "resources/theme.h"
#include "resources/userpalette.h"

#include "utils/stringutils.h"

#include <cmath>

Being::Being(int id, Type type, int subtype, Map *map)
    : ActorSprite(id)
    , mInfo(BeingInfo::Unknown)
{
    setMap(map);
    setType(type, subtype);

    mSpeechBubble = new SpeechBubble;
    mSpeechBubble->addDeathListener(this);

    mMoveSpeed = Net::getPlayerHandler()->getDefaultMoveSpeed();

    listen(Event::ConfigChannel);
    listen(Event::ChatChannel);
}

Being::~Being()
{
    delete mSpeechBubble;
    delete mDispName;
    delete mText;
}

/**
 * Can be used to change the type of the being.
 *
 * Practical use: players (usually GMs) can change into monsters and back.
 */
void Being::setType(Type type, int subtype)
{
    if (mType == type && mSubType == subtype)
        return;

    mType = type;
    mSubType = subtype;

    for (auto &spriteState : mSpriteStates)
    {
        spriteState.visibleId = 0;
        spriteState.particles.clear();
    }

    switch (getType())
    {
    case MONSTER:
        mInfo = MonsterDB::get(mSubType);
        setName(mInfo->name);
        setupSpriteDisplay(mInfo->display);
        break;
    case NPC:
        mInfo = NPCDB::get(mSubType);
        setupSpriteDisplay(mInfo->display, false);
        mShowName = true;
        break;
    case PLAYER: {
        mSprites.clear();
        mChildParticleEffects.clear();

        int id = -100 - subtype;

        // Prevent showing errors when sprite doesn't exist
        if (!itemDb->exists(id))
            id = -100;

        setSprite(Net::getCharHandler()->baseSprite(), id);
        restoreAllSpriteParticles();
        mShowName = this == local_player ? config.showOwnName
                                         : config.visibleNames;
        break;
    }
    case FLOOR_ITEM:
    case PORTAL:
    case UNKNOWN:
        break;
    }

    mSprites.doRedraw();

    updateName();
    updateNamePosition();
    updateColors();
}

bool Being::isTargetSelection() const
{
    return mInfo->targetSelection;
}

ActorSprite::TargetCursorSize Being::getTargetCursorSize() const
{
    return mInfo->targetCursorSize;
}

Cursor Being::getHoverCursor() const
{
    return mInfo->hoverCursor;
}

unsigned char Being::getWalkMask() const
{
    return mInfo->walkMask;
}

Map::BlockType Being::getBlockType() const
{
    return mInfo->blockType;
}

void Being::setMoveSpeed(const Vector &speed)
{
    mMoveSpeed = speed;
    // If we already can, recalculate the system speed right away.
    if (mMap)
        mSpeedPixelsPerSecond =
                Net::getPlayerHandler()->getPixelsPerSecondMoveSpeed(speed);
}

int Being::getSpeechTextYPosition() const
{
    return getPixelY() - std::min(getHeight(), 64) - 6;
}

void Being::setPosition(const Vector &pos)
{
    Actor::setPosition(pos);

    updateNamePosition();

    if (mText)
        mText->adviseXY(getPixelX(), getSpeechTextYPosition());
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
    mSpeech = text;
    removeColors(mSpeech);
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
        mSpeechTimer.set(std::min(time, SPEECH_MAX_TIME));

    const int speech = config.speech;
    if (speech == TEXT_OVERHEAD)
    {
        delete mText;
        mText = new Text(mSpeech,
                         getPixelX(), getSpeechTextYPosition(),
                         gcn::Graphics::CENTER,
                         &Theme::getThemeColor(Theme::BUBBLE_TEXT),
                         true);
    }
}

void Being::takeDamage(Being *attacker, int amount,
                       AttackType type, int attackId)
{
    gcn::Font *font;
    std::string damage = amount ? toString(amount)
                                : (type == FLEE ? "dodge" : "miss");
    const gcn::Color *color;

    font = gui->getInfoParticleFont();

    // Selecting the right color
    if (type == CRITICAL || type == FLEE)
    {
        if (attacker == local_player)
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
        if (attacker == local_player)
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
        if (attacker == local_player)
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
        auto &hurtSfx = mInfo->getSound(SoundEvent::Hurt);
        if (attacker)
            sound.playSfx(hurtSfx, attacker->getPixelX(), attacker->getPixelY());
        else
            sound.playSfx(hurtSfx);

        if (getType() == MONSTER)
        {
            mDamageTaken += amount;
            updateName();
        }

        // Init the particle effect path based on current weapon or default.
        int hitEffectId = 0;
        const ItemInfo *attackerWeapon = attacker ?
            attacker->getEquippedWeapon() : nullptr;

        if (attackerWeapon && attacker->getType() == PLAYER)
        {
            if (type != CRITICAL)
                hitEffectId = attackerWeapon->hitEffectId;
            else
                hitEffectId = attackerWeapon->criticalHitEffectId;
        }
        else if (attacker && attacker->getType() == MONSTER)
        {
            const Attack &attack = attacker->getInfo().getAttack(attackId);

            if (type != CRITICAL)
                hitEffectId = attack.hitEffectId;
            else
                hitEffectId = attack.criticalHitEffectId;
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
    if (this != local_player)
        setAction(Being::ATTACK, attackId);

    if (victim)
    {
        lookAt(victim->getPosition());

        if (getType() == PLAYER && mEquippedWeapon)
            fireMissile(victim, mEquippedWeapon->missileParticleFile);
        else
            fireMissile(victim, mInfo->getAttack(attackId).missileParticleFilename);
    }

    if (getType() == PLAYER)
    {
        auto itemInfo = mEquippedWeapon;

        // Fall back to racesprite item
        if (!itemInfo)
            itemInfo = &itemDb->get(-100 - mSubType);

        const auto event = damage > 0 ? EquipmentSoundEvent::Hit
                                      : EquipmentSoundEvent::Strike;
        const auto &soundFile = itemInfo->getSound(event);
        sound.playSfx(soundFile, getPixelX(), getPixelY());
    }
    else
    {
        const auto event = damage > 0 ? SoundEvent::Hit : SoundEvent::Miss;
        const auto &soundFile = mInfo->getSound(event);
        sound.playSfx(soundFile, getPixelX(), getPixelY());
    }
}

void Being::setName(const std::string &name)
{
    if (getType() == NPC)
        mName = name.substr(0, name.find('#', 0));
    else
        mName = name;

    updateName();
}

void Being::setShowName(bool doShowName)
{
    if (mShowName == doShowName)
        return;

    mShowName = doShowName;
    updateName();
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

    if (this == local_player && socialWindow)
        socialWindow->addTab(guild);
}

void Being::removeGuild(int id)
{
    const auto it = mGuilds.find(id);
    assert(it != mGuilds.end());

    auto [_, guild] = *it;

    if (this == local_player && socialWindow)
        socialWindow->removeTab(guild);

    guild->removeMember(mId);
    mGuilds.erase(it);
}

Guild *Being::getGuild(const std::string &guildName) const
{
    for (auto &[_, guild] : mGuilds)
        if (guild->getName() == guildName)
            return guild;

    return nullptr;
}

Guild *Being::getGuild(int id) const
{
    auto itr = mGuilds.find(id);
    if (itr != mGuilds.end())
        return itr->second;

    return nullptr;
}

void Being::clearGuilds()
{
    for (auto &[_, guild] : mGuilds)
    {
        if (this == local_player && socialWindow)
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

    if (this == local_player && socialWindow)
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

void Being::setStatusEffect(int id, bool active)
{
    const auto it = mStatusEffects.find(id);
    const bool wasActive = it != mStatusEffects.end();

    if (active != wasActive)
    {
        if (active)
            mStatusEffects.insert(id);
        else
            mStatusEffects.erase(it);

        updateStatusEffect(id, active);
    }
}

void Being::updateStatusEffect(int id, bool newStatus)
{
    auto effect = StatusEffectDB::getStatusEffect(id);
    if (!effect)
        return;

    if (Particle *particle = effect->getParticle(newStatus))
        mStatusParticleEffects[id] = ParticleHandle(particle);
    else
        mStatusParticleEffects.erase(id);
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
                currentAction = mEquippedWeapon->attackAction;
                mSprites.reset();
            }
            else
            {
                currentAction = mInfo->getAttack(attackId).action;
                mSprites.reset();

                // Attack particle effect
                if (Particle::enabled)
                {
                    int effectId = mInfo->getAttack(attackId).effectId;
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
            sound.playSfx(mInfo->getSound(SoundEvent::Die),
                          getPixelX(), getPixelY());
            break;
        case STAND:
            currentAction = SpriteAction::STAND;
            break;
    }

    if (currentAction != SpriteAction::INVALID)
    {
        mSprites.play(currentAction);
        mAction = action;
    }

    if (currentAction != SpriteAction::MOVE)
        mActionTimer.set();
}

void Being::setAction(const std::string &action)
{
    // Actions are triggered by strings from abilities when using manaserv,
    // it's not necessarily an attack, but it seems the most appropriate value.
    mAction = ATTACK;
    mSprites.play(action);
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

void Being::setDirection(uint8_t direction)
{
    if (!direction || mDirection == direction)
        return;

    mDirection = direction;

    SpriteDirection dir = DIRECTION_DEFAULT;
    if (mDirection & UP)
        dir = DIRECTION_UP;
    else if (mDirection & DOWN)
        dir = DIRECTION_DOWN;
    else if (mDirection & RIGHT)
        dir = DIRECTION_RIGHT;
    else
        dir = DIRECTION_LEFT;
    mSpriteDirection = dir;

    updatePlayerSprites();
    mSprites.setDirection(dir);
}

int Being::getCollisionRadius() const
{
    // FIXME: Get this from XML file once a better pathfinding algorithm is up.
    return 16;
}

void Being::logic()
{
    // Remove text and speechbubbles if speech boxes aren't being used
    if (mText && mSpeechTimer.passed())
    {
        delete mText;
        mText = nullptr;
    }

    if (mRestoreParticlesOnLogic)
    {
        mRestoreParticlesOnLogic = false;

        restoreAllSpriteParticles();

        // Restart status/particle effects, if needed
        for (int id : mStatusEffects)
        {
            const StatusEffect *effect = StatusEffectDB::getStatusEffect(id);
            if (effect && effect->persistentParticleEffect)
                updateStatusEffect(id, true);
        }
    }

    if (mAction != DEAD && !mSpeedPixelsPerSecond.isNull())
    {
        updateMovement();

        // Update particle effects
        const float py = mPos.y + paths.getIntValue("spriteOffsetY");

        for (auto &spriteState : mSpriteStates)
            for (auto &particle : spriteState.particles)
                particle->moveTo(mPos.x, py);

        for (auto &[_, p] : mStatusParticleEffects)
            p->moveTo(mPos.x, py);
    }

    ActorSprite::logic();

    // Remove it after 1.5 secs if the dead animation isn't long enough,
    // or simply play it until it's finished.
    if (!isAlive() && Net::getGameHandler()->removeDeadBeings() && getType() != PLAYER)
        if (mActionTimer.elapsed() > std::max(mSprites.getMaxDuration(), 1500))
            actorSpriteManager->scheduleDelete(this);
}

void Being::updateMovement()
{
    float dt = Time::deltaTime();

    while (dt > 0.f)
    {
        const Vector dest = mPath.empty() ? mDest
                                          : Vector(mPath.front().x,
                                                   mPath.front().y);

        // Avoid going to flawed destinations
        // We make the being stop move in that case.
        if (dest.x <= 0 || dest.y <= 0)
        {
            mDest = mPos;
            mPath.clear();
            break;
        }

        // The Vector representing the difference between current position
        // and the next destination path node.
        const Vector dir = dest - mPos;

        // When we've not reached our destination, move to it.
        if (!dir.isNull())
        {
            const float distanceToDest = dir.length();

            // The deplacement of a point along a vector is calculated
            // using the Unit Vector (â) multiplied by the point speed.
            // â = a / ||a|| (||a|| is the a length.)
            // Then, diff = (dir/||dir||) * speed.
            const Vector normalizedDir = dir.normalized();
            Vector diff(normalizedDir.x * mSpeedPixelsPerSecond.x * dt,
                        normalizedDir.y * mSpeedPixelsPerSecond.y * dt);
            const float distanceToMove = diff.length();

            // Test if we don't miss the destination by a move too far:
            if (distanceToMove > distanceToDest)
            {
                setPosition(dest);

                // Also, if the destination is reached, try to get the next
                // path point, if existing.
                if (!mPath.empty())
                {
                    mPath.pop_front();

                    if (mPath.empty())
                        pathFinished();
                }

                // Set dt to the time left after performing this move.
                dt -= dt * (distanceToDest / distanceToMove);
            }
            else
            {
                // Otherwise, go to it using the nominal speed.
                setPosition(mPos + diff);
                // And set the remaining time to 0.
                dt = 0.f;
            }

            if (mAction != MOVE)
                setAction(MOVE);

            // The player direction is handled for keyboard
            // by LocalPlayer::startWalking(), we shouldn't get
            // in the way here for other cases.
            // Hence, we set the direction in Being::logic() only when:
            // 1. It is not the local_player
            // 2. When it is the local_player but only by mouse
            // (because in that case, the path can have more than one tile.)
            if (local_player != this || local_player->isPathSetByMouse())
            {
                int direction = 0;
                const float dx = std::abs(dir.x);
                const float dy = std::abs(dir.y);

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

            if (mPath.empty())
                pathFinished();
        }
        else
        {
            if (mAction == MOVE)
                setAction(STAND);
            break;
        }
    }
}

void Being::drawSpeech(int offsetX, int offsetY)
{
    const int px = getPixelX() - offsetX;
    const int speech = config.speech;

    // Draw speech above this being
    if (mSpeechTimer.passed())
    {
        if (mSpeechBubble->isVisible())
            mSpeechBubble->setVisible(false);
    }
    else if (speech == NAME_IN_BUBBLE || speech == NO_NAME_IN_BUBBLE)
    {
        const bool showName = (speech == NAME_IN_BUBBLE);

        delete mText;
        mText = nullptr;

        mSpeechBubble->setCaption(showName ? mName : std::string(), mNameColor);

        mSpeechBubble->setText(mSpeech, showName);
        mSpeechBubble->setPosition(px - (mSpeechBubble->getWidth() / 2),
                                   getSpeechTextYPosition()
                                   - mSpeechBubble->getHeight() - offsetY);
        mSpeechBubble->setVisible(true);
    }
    else if (speech == TEXT_OVERHEAD)
    {
        mSpeechBubble->setVisible(false);

        if (!mText)
        {
            mText = new Text(mSpeech,
                             getPixelX(), getPixelY() - getHeight(),
                             gcn::Graphics::CENTER,
                             &Theme::getThemeColor(Theme::BUBBLE_TEXT),
                             true);
        }
    }
    else if (speech == NO_SPEECH)
    {
        mSpeechBubble->setVisible(false);

        delete mText;
        mText = nullptr;
    }
}

void Being::updateNamePosition()
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

void Being::updateName()
{
    delete mDispName;
    mDispName = nullptr;

    if (!mShowName)
        return;

    std::string mDisplayName(mName);

    if (getType() == PLAYER)
    {
        if (config.showGender)
        {
            if (getGender() == Gender::Female)
                mDisplayName += " \u2640";
            else if (getGender() == Gender::Male)
                mDisplayName += " \u2642";
        }

        // Display the IP when under tmw-Athena (GM only).
        if (Net::getNetworkType() == ServerType::TmwAthena && local_player
                && local_player->getShowIp() && getIp())
        {
            mDisplayName += strprintf(" %s", ipToString(getIp()));
        }
    }

    if (getType() == MONSTER)
    {
        if (config.showMonstersTakedDamage)
        {
            mDisplayName += ", " + toString(getDamageTaken());
        }
    }

    gcn::Font *font = nullptr;
    if (local_player && local_player->getTarget() == this
        && getType() != MONSTER)
    {
        font = boldFont;
    }

    mDispName = new FlashText(mDisplayName, getPixelX(), getPixelY(),
                              gcn::Graphics::CENTER, mNameColor, font);

    updateNamePosition();
}

void Being::addSpriteParticles(SpriteState &spriteState, const SpriteDisplay &display)
{
    if (!particleEngine)    // happens in CharSelectDialog, for example
        return;
    if (!spriteState.particles.empty())
        return;

    for (const auto &particle : display.particles)
    {
        Particle *p = particleEngine->addEffect(particle, 0, 0, 0);
        spriteState.particles.emplace_back(p);
    }
}

void Being::restoreAllSpriteParticles()
{
    if (mType != PLAYER)
        return;

    for (auto &spriteState : mSpriteStates)
    {
        if (spriteState.id)
        {
            auto &itemInfo = itemDb->get(spriteState.id);
            addSpriteParticles(spriteState, itemInfo.display);
        }
    }
}

void Being::updateColors()
{
    switch (getType()) {
    case ActorSprite::UNKNOWN:
        return;
    case ActorSprite::PLAYER:
        if (this == local_player)
        {
            mNameColor = &userPalette->getColor(UserPalette::SELF);
        }
        else if (mIsGM)
        {
            mNameColor = &userPalette->getColor(UserPalette::GM);
        }
        else if (mParty && mParty == local_player->getParty())
        {
            mNameColor = &userPalette->getColor(UserPalette::PARTY);
        }
        else
        {
            mNameColor = &userPalette->getColor(UserPalette::PC);
        }
        break;
    case ActorSprite::NPC:
        mNameColor = &userPalette->getColor(UserPalette::NPC);
        break;
    case ActorSprite::MONSTER:
        mNameColor = &userPalette->getColor(UserPalette::MONSTER);
        break;
    case ActorSprite::FLOOR_ITEM:
    case ActorSprite::PORTAL:
        return;
    }

    if (mDispName)
        mDispName->setColor(mNameColor);
}

/**
 * Updates the visible sprite IDs of the player, taking into account the item
 * replacements.
 */
void Being::updatePlayerSprites()
{
    if (mType != PLAYER)
        return;

    // hack for allow different logic in dead player
    const int direction = mAction == DEAD ? DIRECTION_DEAD : mSpriteDirection;

    // Get the current item IDs
    std::vector<int> itemIDs(mSpriteStates.size());
    for (size_t i = 0; i < mSpriteStates.size(); i++)
        itemIDs[i] = mSpriteStates[i].id;

    // Apply the replacements
    for (auto &spriteState : mSpriteStates)
    {
        if (!spriteState.id)
            continue;

        auto &itemInfo = itemDb->get(spriteState.id);
        for (const auto &replacement : itemInfo.replacements)
        {
            if (replacement.direction != DIRECTION_ALL && replacement.direction != direction)
                continue;

            if (replacement.sprite == SPRITE_ALL)
            {
                if (replacement.items.empty())
                {
                    itemIDs.assign(itemIDs.size(), 0);
                }
                else
                {
                    for (int &id : itemIDs)
                    {
                        for (auto &item : replacement.items)
                            if (!item.from || id == item.from)
                                id = item.to;
                    }
                }
            }
            else if (replacement.sprite < itemIDs.size())
            {
                int &id = itemIDs[replacement.sprite];

                if (replacement.items.empty())
                {
                    id = 0;
                }
                else
                {
                    for (auto &item : replacement.items)
                        if (!item.from || id == item.from)
                            id = item.to;
                }
            }
        }
    }

    // Set the new sprites
    bool newSpriteSet = false;

    mSprites.ensureSize(mSpriteStates.size());

    for (size_t i = 0; i < mSpriteStates.size(); i++)
    {
        auto &spriteState = mSpriteStates[i];
        if (spriteState.visibleId == itemIDs[i])
            continue;

        spriteState.visibleId = itemIDs[i];

        if (spriteState.visibleId == 0)
        {
            mSprites.set(i, nullptr);
        }
        else
        {
            newSpriteSet = true;

            auto &itemInfo = itemDb->get(spriteState.visibleId);
            std::string filename = itemInfo.getSprite(mGender, mSubType);
            Sprite *equipmentSprite = nullptr;

            if (!filename.empty())
            {
                if (!spriteState.color.empty())
                    filename += "|" + spriteState.color;

                equipmentSprite = Sprite::load(
                    paths.getStringValue("sprites") + filename);

                if (equipmentSprite)
                    equipmentSprite->setDirection(getSpriteDirection());
            }

            mSprites.set(i, equipmentSprite);
        }
    }

    // Make sure any new sprites are set to the correct action
    if (newSpriteSet)
        setAction(mAction);
}

void Being::setSprite(unsigned slot, int id, const std::string &color,
                      bool isWeapon)
{
    if (slot >= mSpriteStates.size())
        mSpriteStates.resize(slot + 1);

    auto &spriteState = mSpriteStates[slot];

    // Clear current particles when the ID changes
    if (spriteState.id != id)
        spriteState.particles.clear();

    // Clear the current sprite when the color changes
    if (spriteState.color != color && spriteState.visibleId)
    {
        spriteState.visibleId = 0;
        mSprites.set(slot, nullptr);
    }

    spriteState.id = id;
    spriteState.color = color;

    if (id == 0)        // id = 0 means unequip
    {
        if (isWeapon)
            mEquippedWeapon = nullptr;
    }
    else
    {
        auto &itemInfo = itemDb->get(id);

        if (mType == PLAYER)
            addSpriteParticles(spriteState, itemInfo.display);

        if (isWeapon)
            mEquippedWeapon = &itemInfo;
    }

    updatePlayerSprites();
}

void Being::setSpriteID(unsigned slot, int id)
{
    assert(slot < mSpriteStates.size());
    setSprite(slot, id, mSpriteStates[slot].color);
}

void Being::setSpriteColor(unsigned slot, const std::string &color)
{
    assert(slot < mSpriteStates.size());
    setSprite(slot, mSpriteStates[slot].id, color);
}

bool Being::drawnWhenBehind() const
{
    // For now, just draw actors with only one layer when obscured
    return mSprites.getNumberOfLayers() == 1;
}

void Being::setGender(Gender gender)
{
    if (gender != mGender)
    {
        mGender = gender;

        // Reset all sprites to force reload with the correct gender
        for (size_t i = 0; i < mSpriteStates.size(); i++)
        {
            auto &spriteState = mSpriteStates[i];
            if (spriteState.visibleId)
            {
                mSprites.set(i, nullptr);
                spriteState.visibleId = 0;
            }
        }

        updatePlayerSprites();

        if (config.showGender)
            updateName();
    }
}

void Being::setGM(bool gm)
{
    mIsGM = gm;

    updateColors();
}

void Being::setIp(int ip)
{
    if (mIp == ip)
        return;

    mIp = ip;

    if (local_player && local_player->getShowIp())
        updateName();
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
            event.getInt("permissions") & PlayerPermissions::SPEECH_FLOAT)
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
        if (getType() == PLAYER && event.hasValue(&Config::visibleNames))
        {
            setShowName(config.visibleNames);
        }
    }
}

void Being::death(const gcn::Event &event)
{
    if (event.getSource() == mSpeechBubble)
        mSpeechBubble = nullptr;
}

void Being::setMap(Map *map)
{
    for (auto &spriteState : mSpriteStates)
        spriteState.particles.clear();

    mStatusParticleEffects.clear();

    mRestoreParticlesOnLogic = true;

    ActorSprite::setMap(map);

    // Recalculate pixel/tick speed
    if (map && !mMoveSpeed.isNull())
    {
        mSpeedPixelsPerSecond =
            Net::getPlayerHandler()->getPixelsPerSecondMoveSpeed(mMoveSpeed, map);
    }
}
