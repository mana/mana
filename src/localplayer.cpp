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

#include "localplayer.h"

#include "configuration.h"
#include "event.h"
#include "flooritem.h"
#include "guild.h"
#include "item.h"
#include "map.h"
#include "particle.h"
#include "playerinfo.h"
#include "sound.h"

#include "gui/gui.h"
#include "gui/okdialog.h"

#include "gui/widgets/chattab.h"

#include "net/chathandler.h"
#include "net/gamehandler.h"
#include "net/guildhandler.h"
#include "net/net.h"
#include "net/partyhandler.h"
#include "net/playerhandler.h"

#include "resources/iteminfo.h"
#include "resources/userpalette.h"

#include "utils/gettext.h"
#include "utils/stringutils.h"

constexpr unsigned AWAY_MESSAGE_TIMEOUT = 60 * 1000;

// Actions are allowed at 5.5 per second
constexpr unsigned ACTION_TIMEOUT = 182;

LocalPlayer *local_player = nullptr;

LocalPlayer::LocalPlayer(int id, int subtype):
    Being(id, PLAYER, subtype, nullptr)
{
    listen(Event::AttributesChannel);

    mAwayListener = new AwayListener();

    setShowName(config.getValue("showownname", 1));

    listen(Event::ConfigChannel);
    listen(Event::ActorSpriteChannel);
}

LocalPlayer::~LocalPlayer()
{
    delete mAwayDialog;
    delete mAwayListener;
}

void LocalPlayer::logic()
{
    // Show XP messages
    if (!mMessages.empty())
    {
        if (mMessageTimer.passed())
        {
            const auto &[message, color] = mMessages.front();

            particleEngine->addTextRiseFadeOutEffect(
                    message,
                    getPixelX(),
                    getPixelY() - 32 - 16,
                    &userPalette->getColor(color),
                    gui->getInfoParticleFont(), true);

            mMessages.pop_front();
            mMessageTimer.set(300);
        }
    }

    PlayerInfo::logic();

    if (mTarget)
    {
        ActorSprite::Type targetType = mTarget->getType();
        switch (targetType)
        {
            case ActorSprite::NPC:
                mTarget->setTargetType(
                    withinRange(mTarget,
                                Net::getGameHandler()->getNpcTalkRange()) ?
                                    TCT_IN_RANGE : TCT_NORMAL);
                break;
            case ActorSprite::MONSTER:
            case ActorSprite::PLAYER:
            {
                // Dealing with attacks
                bool withinAttackRange = withinRange(mTarget, getAttackRange());
                mTarget->setTargetType(withinAttackRange ?
                                       TCT_IN_RANGE : TCT_NORMAL);

                if (!mTarget->isAlive())
                {
                    stopAttack();
                }
                else if (mGoingToTarget)
                {
                    if (!withinAttackRange && getPath().empty())
                    {
                        setDestination(mTarget->getPosition());
                    }
                    else if (withinAttackRange)
                    {
                        // Truncate the path to terminate at the next node.
                        // This permits to avoid a walking glitch in tile path
                        // mode.
                        if (!mPath.empty())
                        {
                            pathSetByMouse();
                            setDestination(mPath.front());
                        }

                        mKeepAttacking = true;
                        mGoingToTarget = false;
                    }
                }
                else if (mKeepAttacking)
                {
                    attack(mTarget, true);
                }
                break;
            }
            default:
                break;
        }
    }
    else if (mPickUpTarget
        && withinRange(mPickUpTarget, Net::getGameHandler()->getPickupRange()))
    {
        Net::getPlayerHandler()->pickUp(mPickUpTarget);
        mPickUpTarget = nullptr;
    }

    Being::logic();
}

void LocalPlayer::setAction(Action action, int attackId)
{
    if (action == DEAD)
    {
        mLastTargetTimer.reset();
        setTarget(nullptr);
    }

    Being::setAction(action, attackId);
}

void LocalPlayer::setGMLevel(int level)
{
    mGMLevel = level;

    if (level > 0)
        setGM(true);
}


Position LocalPlayer::getNextWalkPosition(unsigned char dir)
{
    // Compute where the next tile will be set.
    int dx = 0;
    int dy = 0;
    if (dir & Being::UP)
        dy--;
    if (dir & Being::DOWN)
        dy++;
    if (dir & Being::LEFT)
        dx--;
    if (dir & Being::RIGHT)
        dx++;

    Vector pos = getPosition();

    // If no map or no direction is given, give back the current player position
    if (!mMap || (!dx && !dy))
        return Position((int)pos.x, (int)pos.y);

    const int tileW = mMap->getTileWidth();
    const int tileH = mMap->getTileHeight();

    // Get the current tile pos and its offset
    const int tileX = (int)pos.x / tileW;
    const int tileY = (int)pos.y / tileH;
    int offsetX = (int)pos.x % tileW;
    int offsetY = (int)pos.y % tileH;

    // Get the walkability of every surrounding tiles.
    bool wTopLeft = mMap->getWalk(tileX - 1, tileY - 1, getWalkMask());
    bool wTop = mMap->getWalk(tileX, tileY - 1, getWalkMask());
    bool wTopRight = mMap->getWalk(tileX + 1, tileY - 1, getWalkMask());
    bool wLeft = mMap->getWalk(tileX - 1, tileY, getWalkMask());
    bool wRight = mMap->getWalk(tileX + 1, tileY, getWalkMask());
    bool wBottomLeft = mMap->getWalk(tileX - 1, tileY + 1, getWalkMask());
    bool wBottom = mMap->getWalk(tileX, tileY + 1, getWalkMask());
    bool wBottomRight = mMap->getWalk(tileX + 1, tileY + 1, getWalkMask());

    // Make diagonals unwalkable when both straight directions are blocking
    if (!wTop)
    {
        if (!wRight)
            wTopRight = false;
        if (!wLeft)
            wTopLeft = false;
    }
    if (!wBottom)
    {
        if (!wRight)
            wBottomRight = false;
        if (!wLeft)
            wBottomLeft = false;
    }

    // We'll make tests for each desired direction

    // Handle diagonal cases by setting the way back to a straight direction
    // when necessary.
    if (dx && dy)
    {
        // Going top-right
        if (dx > 0 && dy < 0)
        {
            // Choose a straight direction when diagonal target is blocked
            if (!wTop && wRight)
                dy = 0;
            else if (wTop && !wRight)
                dx = 0;
            else if (!wTop && !wRight)
                return Position(tileX * tileW + tileW
                                - getCollisionRadius(),
                                tileY * tileH + getCollisionRadius());
            else if (!wTopRight)
            {
                // Both straight direction are walkable
                // Go right when below the corner
                if (offsetY >=
                    (offsetX / tileH - (offsetX / tileW * tileH)))
                    dy = 0;
                else // Go up otherwise
                    dx = 0;
            }
            else // The top-right diagonal is walkable
            {
                return mMap->checkNodeOffsets(getCollisionRadius(),
                                                getWalkMask(),
                                                Position((int)pos.x + tileW,
                                                        (int)pos.y - tileH));
            }
        }

        // Going top-left
        if (dx < 0 && dy < 0)
        {
            // Choose a straight direction when diagonal target is blocked
            if (!wTop && wLeft)
                dy = 0;
            else if (wTop && !wLeft)
                dx = 0;
            else if (!wTop && !wLeft)
                return Position(tileX * tileW + getCollisionRadius(),
                                tileY * tileH + getCollisionRadius());
            else if (!wTopLeft)
            {
                // Go left when below the corner
                if (offsetY >= (offsetX / mMap->getTileWidth()
                    * mMap->getTileHeight()))
                    dy = 0;
                else // Go up otherwise
                    dx = 0;
            }
            else // The diagonal is walkable
                return mMap->checkNodeOffsets(getCollisionRadius(),
                                              getWalkMask(),
                                    Position((int)pos.x - tileW,
                                             (int)pos.y - tileH));
        }

        // Going bottom-left
        if (dx < 0 && dy > 0)
        {
            // Choose a straight direction when diagonal target is blocked
            if (!wBottom && wLeft)
                dy = 0;
            else if (wBottom && !wLeft)
                dx = 0;
            else if (!wBottom && !wLeft)
                return Position(tileX * tileW + getCollisionRadius(),
                                tileY * tileH + tileH - getCollisionRadius());
            else if (!wBottomLeft)
            {
                // Both straight direction are walkable
                // Go down when below the corner
                if (offsetY >= (offsetX / mMap->getTileHeight()
                    - (offsetX / mMap->getTileWidth()
                        * mMap->getTileHeight()) ))
                    dx = 0;
                else // Go left otherwise
                    dy = 0;
            }
            else // The diagonal is walkable
                return mMap->checkNodeOffsets(getCollisionRadius(),
                                              getWalkMask(),
                                              Position((int)pos.x - tileW,
                                                       (int)pos.y + tileH));
        }

        // Going bottom-right
        if (dx > 0 && dy > 0)
        {
            // Choose a straight direction when diagonal target is blocked
            if (!wBottom && wRight)
                dy = 0;
            else if (wBottom && !wRight)
                dx = 0;
            else if (!wBottom && !wRight)
                return Position(tileX * tileW + tileW - getCollisionRadius(),
                                tileY * tileH + tileH - getCollisionRadius());
            else if (!wBottomRight)
            {
                // Both straight direction are walkable
                // Go down when below the corner
                if (offsetY >= (offsetX / mMap->getTileWidth()
                    * mMap->getTileHeight()))
                    dx = 0;
                else // Go right otherwise
                    dy = 0;
            }
            else // The diagonal is walkable
                return mMap->checkNodeOffsets(getCollisionRadius(),
                                              getWalkMask(),
                                              Position((int)pos.x + tileW,
                                                       (int)pos.y + tileH));
        }

    } // End of diagonal cases

    // Straight directions
    // Right direction
    if (dx > 0 && !dy)
    {
        // If the straight destination is blocked,
        // Make the player go the closest possible.
        if (!wRight)
            return Position(tileX * tileW + tileW - getCollisionRadius(),
                            (int)pos.y);
        else
        {
            if (!wTopRight)
            {
                // If we're going to collide with the top-right corner
                if (offsetY - getCollisionRadius() < 0)
                {
                    // We make the player corrects its offset
                    // before going further
                    return Position(tileX * tileW
                                    + tileW - getCollisionRadius(),
                                    tileY * tileH + getCollisionRadius());

                }
            }

            if (!wBottomRight)
            {
                // If we're going to collide with the bottom-right corner
                if (offsetY + getCollisionRadius() > tileH)
                {
                    // We make the player corrects its offset
                    // before going further
                    return Position(tileX * tileW
                                    + tileW - getCollisionRadius(),
                                    tileY * tileH
                                    + tileH - getCollisionRadius());

                }
            }
            // If the way is clear, step up one checked tile ahead.
            return mMap->checkNodeOffsets(getCollisionRadius(), getWalkMask(),
                                       Position((int)pos.x + tileW,
                                                (int)pos.y));
        }
    }

    // Left direction
    if (dx < 0 && !dy)
    {
        // If the straight destination is blocked,
        // Make the player go the closest possible.
        if (!wLeft)
            return Position(tileX * tileW + getCollisionRadius(), (int)pos.y);
        else
        {
            if (!wTopLeft)
            {
                // If we're going to collide with the top-left corner
                if (offsetY - getCollisionRadius() < 0)
                {
                    // We make the player corrects its offset
                    // before going further
                    return Position(tileX * tileW + getCollisionRadius(),
                                    tileY * tileH + getCollisionRadius());

                }
            }

            if (!wBottomLeft)
            {
                // If we're going to collide with the bottom-left corner
                if (offsetY + getCollisionRadius() > tileH)
                {
                    // We make the player corrects its offset
                    // before going further
                    return Position(tileX * tileW + getCollisionRadius(),
                                    tileY * tileH
                                    + tileH - getCollisionRadius());

                }
            }
            // If the way is clear, step up one checked tile ahead.
            return mMap->checkNodeOffsets(getCollisionRadius(), getWalkMask(),
                                       Position((int)pos.x - tileW,
                                                (int)pos.y));
        }
    }

    // Up direction
    if (!dx && dy < 0)
    {
        // If the straight destination is blocked,
        // Make the player go the closest possible.
        if (!wTop)
            return Position((int)pos.x, tileY * tileH + getCollisionRadius());
        else
        {
            if (!wTopLeft)
            {
                // If we're going to collide with the top-left corner
                if (offsetX - getCollisionRadius() < 0)
                {
                    // We make the player corrects its offset
                    // before going further
                    return Position(tileX * tileW + getCollisionRadius(),
                                    tileY * tileH + getCollisionRadius());

                }
            }

            if (!wTopRight)
            {
                // If we're going to collide with the top-right corner
                if (offsetX + getCollisionRadius() > tileW)
                {
                    // We make the player corrects its offset
                    // before going further
                    return Position(tileX * tileW
                                    + tileW - getCollisionRadius(),
                                    tileY * tileH + getCollisionRadius());

                }
            }
            // If the way is clear, step up one checked tile ahead.
            return mMap->checkNodeOffsets(getCollisionRadius(), getWalkMask(),
                                          Position((int)pos.x,
                                                   (int)pos.y - tileH));
        }
    }

    // Down direction
    if (!dx && dy > 0)
    {
        // If the straight destination is blocked,
        // Make the player go the closest possible.
        if (!wBottom)
            return Position((int)pos.x, tileY * tileH
                            + tileH - getCollisionRadius());
        else
        {
            if (!wBottomLeft)
            {
                // If we're going to collide with the bottom-left corner
                if (offsetX - getCollisionRadius() < 0)
                {
                    // We make the player corrects its offset
                    // before going further
                    return Position(tileX * tileW + getCollisionRadius(),
                                    tileY * tileH
                                    + tileH - getCollisionRadius());
                }
            }

            if (!wBottomRight)
            {
                // If we're going to collide with the bottom-right corner
                if (offsetX + getCollisionRadius() > tileW)
                {
                    // We make the player corrects its offset
                    // before going further
                    return Position(tileX * tileW
                                    + tileW - getCollisionRadius(),
                                    tileY * tileH
                                    + tileH - getCollisionRadius());

                }
            }
            // If the way is clear, step up one checked tile ahead.
            return mMap->checkNodeOffsets(getCollisionRadius(), getWalkMask(),
                                          Position((int)pos.x,
                                                   (int)pos.y + tileH));
        }
    }

    // Return the current position if everything else has failed.
    return Position((int)pos.x, (int)pos.y);
}

void LocalPlayer::nextTile(unsigned char dir = 0)
{
    if (!mMap || !dir)
        return;

    const Vector &pos = getPosition();
    Position destination = getNextWalkPosition(dir);

    if ((int)pos.x != destination.x
        || (int)pos.y != destination.y)
    {
        lookAt(destination);
        setDestination(destination.x, destination.y);
    }
    else if (dir != mDirection)
    {
        // If the being can't move, just change direction
        Net::getPlayerHandler()->setDirection(dir);
        setDirection(dir);
    }
}

bool LocalPlayer::checkInviteRights(const std::string &guildName)
{
    if (Guild *guild = getGuild(guildName))
        return guild->getInviteRights();

    return false;
}

void LocalPlayer::inviteToGuild(Being *being)
{
    if (being->getType() != PLAYER)
        return;

    // TODO: Allow user to choose which guild to invite being to
    // For now, just invite to the first guild you have permissions to invite with
    auto itr = mGuilds.begin();
    auto itr_end = mGuilds.end();
    for (; itr != itr_end; ++itr)
    {
        if (checkInviteRights(itr->second->getName()))
        {
            Net::getGuildHandler()->invite(itr->second->getId(), being);
            return;
        }
    }
}

void LocalPlayer::pickUp(FloorItem *item)
{
    if (!item)
        return;

    cancelGoToTarget();

    if (withinRange(item, Net::getGameHandler()->getPickupRange()))
    {
        Net::getPlayerHandler()->pickUp(item);
        // We found it, so set the player direction to it
        // if the player does not move
        if (getDestination() == getPosition())
            lookAt(item->getPosition());
        mPickUpTarget = nullptr;
    }
    else
    {
        pathSetByMouse();
        setDestination(item->getPixelX(), item->getPixelY());
        mPickUpTarget = item;
    }
}

Being *LocalPlayer::getTarget() const
{
    return mTarget;
}

void LocalPlayer::setTarget(Being *target)
{
    if ((!mLastTargetTimer.passed() || target == this) && target)
        return;

    // Targeting allowed 4 times a second
    if (target)
        mLastTargetTimer.set(250);

    if (target == mTarget)
        return;

    if (!target && mAction != ATTACK)
        mKeepAttacking = false;

    Being *oldTarget = nullptr;
    if (mTarget)
    {
        mTarget->untarget();
        oldTarget = mTarget;
    }

    if (mTarget && mTarget->getType() == ActorSprite::MONSTER)
        mTarget->setShowName(false);

    mTarget = target;

    if (oldTarget)
        oldTarget->updateName();
    if (mTarget)
        mTarget->updateName();

    if (target && target->getType() == ActorSprite::MONSTER)
        target->setShowName(true);
}

void LocalPlayer::setDestination(int x, int y)
{
    if (!mMap)
        return;

    int srcX = x;
    int srcY = y;
    int dstX = (int)mDest.x;
    int dstY = (int)mDest.y;
    int tileWidth = mMap->getTileWidth();
    int tileHeight = mMap->getTileHeight();
    if (!Net::getPlayerHandler()->usePixelPrecision())
    {
        // For tile-based clients, we accept positions on the same tile.
        srcX = srcX / tileWidth;
        srcY = srcY / tileHeight;
        dstX = dstX / tileWidth;
        dstY = dstY / tileHeight;
    }

    // Only send a new message to the server when destination changes
    if (srcX != dstX || srcY != dstY)
    {
        Being::setDestination(x, y);
        // Note: Being::setDestination() updates mDest, so we get the new
        // destination.
        dstX = (int)mDest.x;
        dstY = (int)mDest.y;

        if (!Net::getPlayerHandler()->usePixelPrecision())
        {
            dstX = dstX / tileWidth;
            dstY = dstY / tileHeight;
        }

        // If the destination given to being class is accepted,
        // we inform the Server.
        if (srcX == dstX && srcY == dstY)
            Net::getPlayerHandler()->setDestination(x, y, mDirection);
    }
}

void LocalPlayer::setWalkingDir(int dir)
{
    // This function is called by Game::handleInput()

    // Don't compute a new path before the last one set by keyboard is finished.
    // This permits to avoid movement glitches and server spamming.
    const Vector &pos = getPosition();
    const Vector &dest = getDestination();
    if (!isPathSetByMouse() && (pos.x != dest.x || pos.y != dest.y))
        return;

    // If the player is pressing a key, and its different from what he has
    // been pressing, stop (do not send this stop to the server) and
    // start in the new direction
    if (dir && (dir != getWalkingDir()))
        local_player->stopWalking(false);

    // Else, he is not pressing a key,
    // and the current path hasn't been sent by mouse,
    // then let the path die (1/2 tile after that.)
    // This permit to avoid desyncs with other clients.
    else if (!dir)
        return;

    cancelGoToTarget();

    mWalkingDir = dir;

    // If we're not already walking, start walking.
    if (mAction != MOVE && dir)
    {
        startWalking(dir);
    }
    else if (mAction == MOVE)
    {
        nextTile(dir);
    }
}

void LocalPlayer::startWalking(unsigned char dir)
{
    if (!mMap || !dir)
        return;

    if (mAction == MOVE && !mPath.empty())
    {
        // Just finish the current action, otherwise we get out of sync
        const Vector &pos = getPosition();
        Being::setDestination(pos.x, pos.y);
        return;
    }

    nextTile(dir);
}

void LocalPlayer::stopWalking(bool sendToServer)
{
    if (mAction == MOVE)
    {
        mWalkingDir = 0;

        setDestination((int) getPosition().x, (int) getPosition().y);
        if (sendToServer)
             Net::getPlayerHandler()->setDestination((int) getPosition().x,
                                                     (int) getPosition().y);
        setAction(STAND);
    }

    // No path set anymore, so we reset the path by mouse flag
    mPathSetByMouse = false;

    clearPath();
}

void LocalPlayer::toggleSit()
{
    if (!mLastActionTimer.passed())
        return;
    mLastActionTimer.set(ACTION_TIMEOUT);

    Being::Action newAction;
    switch (mAction)
    {
        case STAND: newAction = SIT; break;
        case SIT: newAction = STAND; break;
        default: return;
    }

    Net::getPlayerHandler()->changeAction(newAction);
}

void LocalPlayer::emote(int emoteId)
{
    if (!mLastActionTimer.passed())
        return;
    mLastActionTimer.set(ACTION_TIMEOUT);

    Net::getPlayerHandler()->emote(emoteId);
}

void LocalPlayer::attack(Being *target, bool keep)
{
    if (!mLastActionTimer.passed())
        return;

    // Can only attack when standing still
    if (mAction != STAND && mAction != ATTACK)
        return;

    if (!target || target->getType() == ActorSprite::NPC)
        return;

    // Can't attack more times than its attack speed
    static Timer lastAttackTimer;
    if (!lastAttackTimer.passed())
        return;

    lastAttackTimer.set(mAttackSpeed);

    mKeepAttacking = keep;

    if (mTarget != target || !mTarget)
    {
        mLastTargetTimer.reset();
        setTarget(target);
    }

    lookAt(mTarget->getPosition());

    mLastActionTimer.set(ACTION_TIMEOUT);

    setAction(ATTACK);

    if (mEquippedWeapon)
    {
        std::string soundFile = mEquippedWeapon->getSound(EQUIP_EVENT_STRIKE);
        if (!soundFile.empty())
            sound.playSfx(soundFile);
    }
    else
    {
        sound.playSfx(paths.getValue("attackSfxFile", "fist-swish.ogg"));
    }

    Net::getPlayerHandler()->attack(target->getId());
}

void LocalPlayer::stopAttack()
{
    if (mTarget)
    {
        if (mAction == ATTACK)
            setAction(STAND);
        setTarget(nullptr);
    }
    mLastTargetTimer.reset();
    cancelGoToTarget();
}

void LocalPlayer::pickedUp(const ItemInfo &itemInfo, int amount,
                           unsigned char fail)
{
    if (fail)
    {
        const char* msg;
        switch (fail)
        {
            case PICKUP_BAD_ITEM:
                msg = N_("Tried to pick up nonexistent item."); break;
            case PICKUP_TOO_HEAVY: msg = N_("Item is too heavy."); break;
            case PICKUP_TOO_FAR: msg = N_("Item is too far away"); break;
            case PICKUP_INV_FULL: msg = N_("Inventory is full."); break;
            case PICKUP_STACK_FULL: msg = N_("Stack is too big."); break;
            case PICKUP_DROP_STEAL:
                msg = N_("Item belongs to someone else."); break;
            default: msg = N_("Unknown problem picking up item."); break;
        }
        if (config.getValue("showpickupchat", 1))
        {
            SERVER_NOTICE(_(msg))
        }
        if (mMap && config.getBoolValue("showpickupparticle"))
        {
            // Show pickup notification
            addMessageToQueue(_(msg), UserPalette::PICKUP_INFO);
        }
    }
    else
    {
        if (config.getBoolValue("showpickupchat"))
        {
            // TRANSLATORS: This sentence may be translated differently
            // for different grammatical numbers (singular, plural, ...)
            SERVER_NOTICE(strprintf(ngettext("You picked up %d "
                    "[@@%d|%s@@].", "You picked up %d [@@%d|%s@@].", amount),
                    amount, itemInfo.id, itemInfo.name.c_str()))
        }

        if (mMap && config.getBoolValue("showpickupparticle"))
        {
            // Show pickup notification
            std::string msg;
            if (amount > 1)
                msg = strprintf("%i ", amount);
            msg += itemInfo.name;
            addMessageToQueue(msg, UserPalette::PICKUP_INFO);
        }
    }
}

void LocalPlayer::setAttackRange(int range)
{
    // When the range is more than the minimal, we accept it
    if (range > -1)
    {
        mAttackRange = range;
    }
    else if (Net::getNetworkType() == ServerType::TMWATHENA)
    {
        // TODO: Fix this to be more generic
        Item *weapon = PlayerInfo::getEquipment(TmwAthena::EQUIP_FIGHT1_SLOT);
        if (weapon)
        {
            const ItemInfo &info = weapon->getInfo();
            if (info.attackRange > -1)
                mAttackRange = info.attackRange;
        }
    }
}

bool LocalPlayer::withinRange(Actor *target, int range) const
{
    if (!target || range < 0)
        return false;

    const Vector &targetPos = target->getPosition();
    const Vector &pos = getPosition();
    const int dx = abs(targetPos.x - pos.x);
    const int dy = abs(targetPos.y - pos.y);
    return !(dx > range || dy > range);
}

void LocalPlayer::setGotoTarget(Being *target)
{
    if (!target)
        return;

    mLastTargetTimer.reset();

    setTarget(target);
    mGoingToTarget = true;
    mKeepAttacking = true;

    pathSetByMouse();
    setDestination(target->getPosition());
}

void LocalPlayer::addMessageToQueue(const std::string &message, int color)
{
    mMessages.emplace_back(message, color);
}

void LocalPlayer::event(Event::Channel channel, const Event &event)
{
    if (channel == Event::ActorSpriteChannel)
    {
        if (event.getType() == Event::Destroyed)
        {
            ActorSprite *actor = event.getActor("source");

            if (mPickUpTarget == actor)
                mPickUpTarget = nullptr;

            if (mTarget == actor)
                mTarget = nullptr;
        }
    }
    else if (channel == Event::AttributesChannel)
    {
        if (event.getType() == Event::UpdateAttribute)
        {
            if (event.getInt("id") == EXP)
            {
                int change = 0;
                int oldXp = event.getInt("oldValue");
                int newXp = event.getInt("newValue");

                // When the new XP is lower than the old one,
                // it means that a new level has been reached.
                // Thus, the xp difference can only be obtained
                // with the exp needed for the next level.
                // The new XP value is then the XP obtained for the new level.
                if (newXp < oldXp)
                {
                    change = PlayerInfo::getAttribute(EXP_NEEDED)
                        - oldXp + newXp;
                }
                else
                {
                    change = newXp - oldXp;
                }

                if (change > 0)
                    addMessageToQueue(toString(change) + " xp");
            }
        }
    }
    else if (channel == Event::ConfigChannel)
    {
        if (event.getType() == Event::ConfigOptionChanged &&
            event.getString("option") == "showownname")
        {
            setShowName(config.getValue("showownname", 1));
        }
    }

    Being::event(channel, event);
}

void LocalPlayer::changeAwayMode()
{
    mAwayMode = !mAwayMode;
    mAfkTimer.reset();

    if (mAwayMode)
    {
        mAwayDialog = new OkDialog(_("Away"),
                config.getValue("afkMessage", "I am away from keyboard"));
        mAwayDialog->addActionListener(mAwayListener);
    }

    mAwayDialog = nullptr;
}

void LocalPlayer::setAway(const std::string &message)
{
    if (!message.empty())
        config.setValue("afkMessage", message);
    changeAwayMode();
}

void LocalPlayer::afkRespond(ChatTab *tab, const std::string &nick)
{
    if (mAwayMode)
    {
        if (mAfkTimer.passed())
        {
            std::string msg = "*AFK*: "
                    + config.getValue("afkMessage", "I am away from keyboard");

            Net::getChatHandler()->privateMessage(nick, msg);
            if (!tab)
            {
                localChatTab->chatLog(getName() + " : " + msg,
                                      ACT_WHISPER, false);
            }
            else
            {
                tab->chatLog(getName(), msg);
            }

            mAfkTimer.set(AWAY_MESSAGE_TIMEOUT);
        }
    }
}

void AwayListener::action(const gcn::ActionEvent &event)
{
    if (event.getId() == "ok" && local_player->getAwayMode())
    {
        local_player->changeAwayMode();
    }
}
