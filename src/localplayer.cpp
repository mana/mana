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

#include "localplayer.h"

#include "client.h"
#include "configuration.h"
#include "event.h"
#include "flooritem.h"
#include "graphics.h"
#include "guild.h"
#include "item.h"
#include "map.h"
#include "particle.h"
#include "playerinfo.h"
#include "simpleanimation.h"
#include "sound.h"

#include "gui/gui.h"
#include "gui/okdialog.h"

#include "gui/widgets/chattab.h"

#include "net/chathandler.h"
#include "net/guildhandler.h"
#include "net/inventoryhandler.h"
#include "net/net.h"
#include "net/partyhandler.h"
#include "net/playerhandler.h"
#include "net/specialhandler.h"
#include "net/tradehandler.h"

#include "resources/animation.h"
#include "resources/imageset.h"
#include "resources/iteminfo.h"
#include "resources/userpalette.h"

#include "utils/gettext.h"
#include "utils/stringutils.h"

#include <cassert>

#define AWAY_LIMIT_TIMER 60

LocalPlayer *player_node = NULL;

LocalPlayer::LocalPlayer(int id, int subtype):
    Being(id, PLAYER, subtype, 0),
    mAttackRange(ATTACK_RANGE_NOT_SET),
    mTargetTime(-1),
    mLastTarget(-1),
    mTarget(NULL),
    mPickUpTarget(NULL),
    mGoingToTarget(false), mKeepAttacking(false),
    mLastAction(-1),
    mWalkingDir(0),
    mPathSetByMouse(false),
    mLocalWalkTime(-1),
    mKeyboardMoveDelay(500),
    mMessageTime(0),
    mShowIp(false),
    mAwayDialog(0),
    mAfkTime(0),
    mAwayMode(false)
{
    listen(Event::AttributesChannel);

    mAwayListener = new AwayListener();

    mUpdateName = true;

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
    // Actions are allowed at 5.5 per second
    if (get_elapsed_time(mLastAction) >= 182)
        mLastAction = -1;

    // Show XP messages
    if (!mMessages.empty())
    {
        if (mMessageTime == 0)
        {
            MessagePair info = mMessages.front();

            particleEngine->addTextRiseFadeOutEffect(
                    info.first,
                    getPixelX(),
                    getPixelY() - 32 - 16,
                    &userPalette->getColor(info.second),
                    gui->getInfoParticleFont(), true);

            mMessages.pop_front();
            mMessageTime = 30;
        }
        mMessageTime--;
    }

    PlayerInfo::logic();

    // Targeting allowed 4 times a second
    if (get_elapsed_time(mLastTarget) >= 250)
        mLastTarget = -1;

    // Remove target if its been on a being for more than a minute
    if (get_elapsed_time(mTargetTime) >= 60000)
    {
        mTargetTime = -1;
        setTarget(NULL);
        mLastTarget = -1;
    }

    if (mTarget)
    {
        if (mTarget->getType() == ActorSprite::NPC)
        {
            // NPCs are always in range
            mTarget->setTargetType(TCT_IN_RANGE);
        }
        else
        {
            // Find whether target is in range
            const int rangeX = abs(mTarget->getPosition().x - getPosition().x);
            const int rangeY = abs(mTarget->getPosition().y - getPosition().y);

            const int attackRange = getAttackRange();
            const TargetCursorType targetType = rangeX > attackRange ||
                                                rangeY > attackRange ?
                                                TCT_NORMAL : TCT_IN_RANGE;
            mTarget->setTargetType(targetType);

            if (!mTarget->isAlive())
                stopAttack();

            if (mKeepAttacking && mTarget)
                attack(mTarget, true);
        }
    }

    Being::logic();
}

void LocalPlayer::setAction(Action action, int attackId)
{
    if (action == DEAD)
    {
        mLastTarget = -1;
        setTarget(NULL);
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
    int dx = 0, dy = 0;
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
            if (!wTopRight)
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
                else // Both straight direction are walkable
                {
                    // Go right when below the corner
                    if (offsetY >=
                        (offsetX / tileH - (offsetX / tileW * tileH)))
                        dy = 0;
                    else // Go up otherwise
                        dx = 0;
                }
            }
            else // The diagonal is walkable
                return mMap->checkNodeOffsets(getCollisionRadius(),
                                              getWalkMask(),
                                    Position((int)pos.x + tileW,
                                             (int)pos.y - tileH));
        }

        // Going top-left
        if (dx < 0 && dy < 0)
        {
            if (!wTopLeft)
            {
                // Choose a straight direction when diagonal target is blocked
                if (!wTop && wLeft)
                    dy = 0;
                else if (wTop && !wLeft)
                    dx = 0;
                else if (!wTop && !wLeft)
                    return Position(tileX * tileW + getCollisionRadius(),
                                    tileY * tileH + getCollisionRadius());
                else // Both straight direction are walkable
                {
                    // Go left when below the corner
                    if (offsetY >= (offsetX / mMap->getTileWidth()
                        * mMap->getTileHeight()))
                        dy = 0;
                    else // Go up otherwise
                        dx = 0;
                }
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
            if (!wBottomLeft)
            {
                // Choose a straight direction when diagonal target is blocked
                if (!wBottom && wLeft)
                    dy = 0;
                else if (wBottom && !wLeft)
                    dx = 0;
                else if (!wBottom && !wLeft)
                    return Position(tileX * tileW + getCollisionRadius(),
                                    tileY * tileH + tileH - getCollisionRadius());
                else // Both straight direction are walkable
                {
                    // Go down when below the corner
                    if (offsetY >= (offsetX / mMap->getTileHeight()
                        - (offsetX / mMap->getTileWidth()
                           * mMap->getTileHeight()) ))
                        dx = 0;
                    else // Go left otherwise
                        dy = 0;
                }
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
            if (!wBottomRight)
            {
                // Choose a straight direction when diagonal target is blocked
                if (!wBottom && wRight)
                    dy = 0;
                else if (wBottom && !wRight)
                    dx = 0;
                else if (!wBottom && !wRight)
                    return Position(tileX * tileW + tileW - getCollisionRadius(),
                                    tileY * tileH + tileH - getCollisionRadius());
                else // Both straight direction are walkable
                {
                    // Go down when below the corner
                    if (offsetY >= (offsetX / mMap->getTileWidth()
                        * mMap->getTileHeight()))
                        dx = 0;
                    else // Go right otherwise
                        dy = 0;
                }
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
    Guild *guild = getGuild(guildName);
    if (guild)
    {
        return guild->getInviteRights();
    }

    return false;
}

void LocalPlayer::inviteToGuild(Being *being)
{
    if (being->getType() != PLAYER)
        return;

    // TODO: Allow user to choose which guild to invite being to
    // For now, just invite to the first guild you have permissions to invite with
    std::map<int, Guild*>::iterator itr = mGuilds.begin();
    std::map<int, Guild*>::iterator itr_end = mGuilds.end();
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

    int tileWidth = mMap->getTileWidth();
    int tileHeight = mMap->getTileHeight();
    int dx = item->getTileX() - (int) getPosition().x / tileWidth;
    int dy = item->getTileY() - ((int) getPosition().y - 1) / tileHeight;

    if (dx * dx + dy * dy < 4)
    {
        Net::getPlayerHandler()->pickUp(item);
        mPickUpTarget = NULL;
    }
    else
    {
        setDestination(item->getTileX() * tileWidth + tileWidth / 2,
                       item->getTileY() * tileHeight + tileHeight / 2);
        mPickUpTarget = item;
    }
}

Being *LocalPlayer::getTarget() const
{
    return mTarget;
}

void LocalPlayer::setTarget(Being *target)
{
    if ((mLastTarget != -1 || target == this) && target)
        return;

    if (target)
        mLastTarget = tick_time;

    if (target == mTarget)
        return;

    if (target || mAction == ATTACK)
    {
        mTargetTime = tick_time;
    }
    else
    {
        mKeepAttacking = false;
        mTargetTime = -1;
    }

    Being *oldTarget = 0;
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

    mPickUpTarget = NULL;
    mKeepAttacking = false;
}

void LocalPlayer::setWalkingDir(int dir)
{
    // This function is called by Game::handleInput()

    // First if player is pressing key for the direction he is already
    // going, do nothing more...

    // Else if he is pressing a key, and its different from what he has
    // been pressing, stop (do not send this stop to the server) and
    // start in the new direction
    if (dir && (dir != getWalkingDir()))
        player_node->stopWalking(false);

    // Else, he is not pressing a key,
    // and the current path hasn't been sent by mouse,
    // then let the path die (1/2 tile after that.)
    // This permit to avoid desyncs with other clients.
    else if (!dir)
        return;

    // If the delay to send another walk message to the server hasn't expired,
    // don't do anything or we could get disconnected for spamming the server
    if (get_elapsed_time(mLocalWalkTime) < mKeyboardMoveDelay)
        return;
    mLocalWalkTime = tick_time;

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
    // This function is called by setWalkingDir(),
    // but also by nextTile() for TMW-Athena...
    if (!mMap || !dir)
        return;

    if (mAction == MOVE && !mPath.empty())
    {
        // Just finish the current action, otherwise we get out of sync
        const Vector &pos = getPosition();
        Being::setDestination(pos.x, pos.y);
        return;
    }

    int dx = 0, dy = 0;
    if (dir & UP)
        dy--;
    if (dir & DOWN)
        dy++;
    if (dir & LEFT)
        dx--;
    if (dir & RIGHT)
        dx++;

    // Update the direction when the walk just start
    setDirection(dir);

    nextTile(dir);
}

void LocalPlayer::stopWalking(bool sendToServer)
{
    if (mAction == MOVE && mWalkingDir)
    {
        mWalkingDir = 0;
        mLocalWalkTime = 0;

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

void LocalPlayer::setMoveSpeed(const Vector& speed)
{
    Being::setMoveSpeed(speed);
    mKeyboardMoveDelay = Net::getPlayerHandler()->getKeyboardMoveDelay(speed);
}

void LocalPlayer::toggleSit()
{
    if (mLastAction != -1)
        return;
    mLastAction = tick_time;

    Being::Action newAction;
    switch (mAction)
    {
        case STAND: newAction = SIT; break;
        case SIT: newAction = STAND; break;
        default: return;
    }

    Net::getPlayerHandler()->changeAction(newAction);
}

void LocalPlayer::emote(Uint8 emotion)
{
    if (mLastAction != -1)
        return;
    mLastAction = tick_time;

    Net::getPlayerHandler()->emote(emotion);
}

void LocalPlayer::attack(Being *target, bool keep)
{
    if (mLastAction != -1)
        return;

    // Can only attack when standing still
    if (mAction != STAND && mAction != ATTACK)
        return;

    if (!target || target->getType() == ActorSprite::NPC)
        return;

    // Can't attack more times than its attack speed
    static int lastAttackTime = 0;
    if (get_elapsed_time(lastAttackTime) < mAttackSpeed)
        return;

    lastAttackTime = tick_time;

    mKeepAttacking = keep;

    if (mTarget != target || !mTarget)
    {
        mLastTarget = -1;
        setTarget(target);
    }

    Vector plaPos = this->getPosition();
    Vector tarPos = mTarget->getPosition();
    int dist_x = plaPos.x - tarPos.x;
    int dist_y = plaPos.y - tarPos.y;

    if (abs(dist_y) >= abs(dist_x))
    {
        if (dist_y < 0)
            setDirection(DOWN);
        else
            setDirection(UP);
    }
    else
    {
        if (dist_x < 0)
            setDirection(RIGHT);
        else
            setDirection(LEFT);
    }

    mLastAction = tick_time;

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
        setTarget(NULL);
    }
    mLastTarget = -1;
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
                    amount, itemInfo.getId(), itemInfo.getName().c_str()))
        }

        if (mMap && config.getBoolValue("showpickupparticle"))
        {
            // Show pickup notification
            std::string msg = "";
            if (amount > 1)
                msg = strprintf("%i ", amount);
            msg += itemInfo.getName();
            addMessageToQueue(msg, UserPalette::PICKUP_INFO);
        }
    }
}

void LocalPlayer::setAttackRange(int range)
{
    // When the range is more than the minimal, we accept it
    if (range > ATTACK_RANGE_NOT_SET)
    {
        mAttackRange = range;
    }
    else if (Net::getNetworkType() == ServerInfo::TMWATHENA)
    {
        // TODO: Fix this to be more generic
        Item *weapon = PlayerInfo::getEquipment(TmwAthena::EQUIP_FIGHT1_SLOT);
        if (weapon)
        {
            const ItemInfo info = weapon->getInfo();
            if (info.getAttackRange() > ATTACK_RANGE_NOT_SET)
                mAttackRange = info.getAttackRange();
        }
    }
}

bool LocalPlayer::withinAttackRange(Being *target)
{
    const Vector &targetPos = target->getPosition();
    const Vector &pos = getPosition();
    const int dx = abs(targetPos.x - pos.x);
    const int dy = abs(targetPos.y - pos.y);
    const int range = getAttackRange();

    return !(dx > range || dy > range);
}

void LocalPlayer::setGotoTarget(Being *target)
{
    mLastTarget = -1;

    mTarget = target;
    mGoingToTarget = true;
    const Vector &targetPos = target->getPosition();
    setDestination(targetPos.x, targetPos.y);
}

void LocalPlayer::addMessageToQueue(const std::string &message, int color)
{
    mMessages.push_back(MessagePair(message, color));
}

void LocalPlayer::event(Event::Channel channel, const Event &event)
{
    if (channel == Event::ActorSpriteChannel)
    {
        if (event.getType() == Event::Destroyed)
        {
            ActorSprite *actor = event.getActor("source");

            if (mPickUpTarget == actor)
                mPickUpTarget = 0;

            if (mTarget == actor)
                mTarget = 0;
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
    mAfkTime = 0;
    if (mAwayMode)
    {
        mAwayDialog = new OkDialog(_("Away"),
                config.getValue("afkMessage", "I am away from keyboard"));
        mAwayDialog->addActionListener(mAwayListener);
    }

    mAwayDialog = 0;
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
        if (mAfkTime == 0
            || cur_time < mAfkTime
            || cur_time - mAfkTime > AWAY_LIMIT_TIMER)
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
            mAfkTime = cur_time;
        }
    }
}

void AwayListener::action(const gcn::ActionEvent &event)
{
    if (event.getId() == "ok" && player_node->getAwayMode())
    {
        player_node->changeAwayMode();
    }
}
