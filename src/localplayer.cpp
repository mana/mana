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

// This is the minimal delay between to permitted
// setDestination() calls using the keyboard.
// TODO: This can fine tuned later on when running is added...
const short walkingKeyboardDelay = 1000;

#define AWAY_LIMIT_TIMER 60

LocalPlayer *player_node = NULL;

LocalPlayer::LocalPlayer(int id, int subtype):
    Being(id, PLAYER, subtype, 0),
    mAttackRange(0),
    mTargetTime(-1),
    mLastTarget(-1),
    mTarget(NULL),
    mPickUpTarget(NULL),
    mGoingToTarget(false), mKeepAttacking(false),
    mLastAction(-1),
    mWalkingDir(0),
    mPathSetByMouse(false),
    mLocalWalkTime(-1),
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
    // Actions are allowed once per second
    if (get_elapsed_time(mLastAction) >= 1000)
        mLastAction = -1;

    // Show XP messages
    if (!mMessages.empty())
    {
        if (mMessageTime == 0)
        {
            //const Vector &pos = getPosition();

            MessagePair info = mMessages.front();

            particleEngine->addTextRiseFadeOutEffect(
                    info.first,
                    /*(int) pos.x,
                    (int) pos.y - 48,*/
                    getPixelX(),
                    getPixelY() - 48,
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
            // TODO: Make this nicer, probably using getPosition() only
            const int rangeX =
                (Net::getNetworkType() == ServerInfo::MANASERV) ?
                abs(mTarget->getPosition().x - getPosition().x) :
                abs(mTarget->getTileX() - getTileX());
            const int rangeY =
                (Net::getNetworkType() == ServerInfo::MANASERV) ?
                abs(mTarget->getPosition().y - getPosition().y) :
                abs(mTarget->getTileY() - getTileY());

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

void LocalPlayer::setAction(Action action, int attackType)
{
    if (action == DEAD)
    {
        mLastTarget = -1;
        setTarget(NULL);
    }

    Being::setAction(action, attackType);
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

    // Get the current tile pos and its offset
    int tileX = (int)pos.x / mMap->getTileWidth();
    int tileY = (int)pos.y / mMap->getTileHeight();
    int offsetX = (int)pos.x % mMap->getTileWidth();
    int offsetY = (int)pos.y % mMap->getTileHeight();

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
                    return Position(tileX * 32 + 32 - getCollisionRadius(),
                                    tileY * 32 + getCollisionRadius());
                else // Both straight direction are walkable
                {
                    // Go right when below the corner
                    if (offsetY >= (offsetX / mMap->getTileHeight()
                        - (offsetX / mMap->getTileWidth()
                           * mMap->getTileHeight()) ))
                        dy = 0;
                    else // Go up otherwise
                        dx = 0;
                }
            }
            else // The diagonal is walkable
                return mMap->checkNodeOffsets(getCollisionRadius(),
                                              getWalkMask(),
                                    Position((int)pos.x + 32, (int)pos.y - 32));
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
                    return Position(tileX * 32 + getCollisionRadius(),
                                    tileY * 32 + getCollisionRadius());
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
                                    Position((int)pos.x - 32, (int)pos.y - 32));
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
                    return Position(tileX * 32 + getCollisionRadius(),
                                    tileY * 32 + 32 - getCollisionRadius());
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
                                    Position((int)pos.x - 32, (int)pos.y + 32));
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
                    return Position(tileX * 32 + 32 - getCollisionRadius(),
                                    tileY * 32 + 32 - getCollisionRadius());
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
                                    Position((int)pos.x + 32, (int)pos.y + 32));
        }

    } // End of diagonal cases

    // Straight directions
    // Right direction
    if (dx > 0 && !dy)
    {
        // If the straight destination is blocked,
        // Make the player go the closest possible.
        if (!wRight)
            return Position(tileX * 32 + 32 - getCollisionRadius(), (int)pos.y);
        else
        {
            if (!wTopRight)
            {
                // If we're going to collide with the top-right corner
                if (offsetY - getCollisionRadius() < 0)
                {
                    // We make the player corrects its offset
                    // before going further
                    return Position(tileX * 32 + 32 - getCollisionRadius(),
                                    tileY * 32 + getCollisionRadius());

                }
            }

            if (!wBottomRight)
            {
                // If we're going to collide with the bottom-right corner
                if (offsetY + getCollisionRadius() > 32)
                {
                    // We make the player corrects its offset
                    // before going further
                    return Position(tileX * 32 + 32 - getCollisionRadius(),
                                    tileY * 32 + 32 - getCollisionRadius());

                }
            }
            // If the way is clear, step up one checked tile ahead.
            return mMap->checkNodeOffsets(getCollisionRadius(), getWalkMask(),
                                       Position((int)pos.x + 32, (int)pos.y));
        }
    }

    // Left direction
    if (dx < 0 && !dy)
    {
        // If the straight destination is blocked,
        // Make the player go the closest possible.
        if (!wLeft)
            return Position(tileX * 32 + getCollisionRadius(), (int)pos.y);
        else
        {
            if (!wTopLeft)
            {
                // If we're going to collide with the top-left corner
                if (offsetY - getCollisionRadius() < 0)
                {
                    // We make the player corrects its offset
                    // before going further
                    return Position(tileX * 32 + getCollisionRadius(),
                                    tileY * 32 + getCollisionRadius());

                }
            }

            if (!wBottomLeft)
            {
                // If we're going to collide with the bottom-left corner
                if (offsetY + getCollisionRadius() > 32)
                {
                    // We make the player corrects its offset
                    // before going further
                    return Position(tileX * 32 + getCollisionRadius(),
                                    tileY * 32 + 32 - getCollisionRadius());

                }
            }
            // If the way is clear, step up one checked tile ahead.
            return mMap->checkNodeOffsets(getCollisionRadius(), getWalkMask(),
                                       Position((int)pos.x - 32, (int)pos.y));
        }
    }

    // Up direction
    if (!dx && dy < 0)
    {
        // If the straight destination is blocked,
        // Make the player go the closest possible.
        if (!wTop)
            return Position((int)pos.x, tileY * 32 + getCollisionRadius());
        else
        {
            if (!wTopLeft)
            {
                // If we're going to collide with the top-left corner
                if (offsetX - getCollisionRadius() < 0)
                {
                    // We make the player corrects its offset
                    // before going further
                    return Position(tileX * 32 + getCollisionRadius(),
                                    tileY * 32 + getCollisionRadius());

                }
            }

            if (!wTopRight)
            {
                // If we're going to collide with the top-right corner
                if (offsetX + getCollisionRadius() > 32)
                {
                    // We make the player corrects its offset
                    // before going further
                    return Position(tileX * 32 + 32 - getCollisionRadius(),
                                    tileY * 32 + getCollisionRadius());

                }
            }
            // If the way is clear, step up one checked tile ahead.
            return mMap->checkNodeOffsets(getCollisionRadius(), getWalkMask(),
                                       Position((int)pos.x, (int)pos.y - 32));
        }
    }

    // Down direction
    if (!dx && dy > 0)
    {
        // If the straight destination is blocked,
        // Make the player go the closest possible.
        if (!wBottom)
            return Position((int)pos.x, tileY * 32 + 32 - getCollisionRadius());
        else
        {
            if (!wBottomLeft)
            {
                // If we're going to collide with the bottom-left corner
                if (offsetX - getCollisionRadius() < 0)
                {
                    // We make the player corrects its offset
                    // before going further
                    return Position(tileX * 32 + getCollisionRadius(),
                                    tileY * 32 + 32 - getCollisionRadius());

                }
            }

            if (!wBottomRight)
            {
                // If we're going to collide with the bottom-right corner
                if (offsetX + getCollisionRadius() > 32)
                {
                    // We make the player corrects its offset
                    // before going further
                    return Position(tileX * 32 + 32 - getCollisionRadius(),
                                    tileY * 32 + 32 - getCollisionRadius());

                }
            }
            // If the way is clear, step up one checked tile ahead.
            return mMap->checkNodeOffsets(getCollisionRadius(), getWalkMask(),
                                       Position((int)pos.x, (int)pos.y + 32));
        }
    }

    // Return the current position if everything else has failed.
    return Position((int)pos.x, (int)pos.y);
}

void LocalPlayer::nextTile(unsigned char dir = 0)
{
    if (Net::getNetworkType() == ServerInfo::TMWATHENA)
    {
        // TODO: Fix picking up when reaching target (this method is obsolete)
        // TODO: Fix holding walking button to keep walking smoothly
        if (mPath.empty())
        {
            if (mPickUpTarget)
                pickUp(mPickUpTarget);

            if (mWalkingDir)
                startWalking(mWalkingDir);
        }

        // TODO: Fix automatically walking within range of target, when wanted
        if (mGoingToTarget && mTarget && withinAttackRange(mTarget))
        {
            mAction = Being::STAND;
            attack(mTarget, true);
            mGoingToTarget = false;
            mPath.clear();
            return;
        }
        else if (mGoingToTarget && !mTarget)
        {
            mGoingToTarget = false;
            mPath.clear();
        }


        Being::nextTile();
    }
    else
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

    int dx = item->getTileX() - (int) getPosition().x / mMap->getTileWidth();
    int dy = item->getTileY() - ((int) getPosition().y - 1)
             / mMap->getTileHeight();

    if (dx * dx + dy * dy < 4)
    {
        Net::getPlayerHandler()->pickUp(item);
        mPickUpTarget = NULL;
    }
    else
    {
        if (Net::getNetworkType() == ServerInfo::MANASERV)
        {
            setDestination(item->getPixelX() + 16, item->getPixelY() + 16);
            mPickUpTarget = item;
        }
        else
        {
            setDestination(item->getTileX(), item->getTileY());
            mPickUpTarget = item;
            stopAttack();
        }
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
    // Only send a new message to the server when destination changes
    if (x != mDest.x || y != mDest.y)
    {
        Being::setDestination(x, y);

        // Manaserv:
        // If the destination given to being class is accepted,
        // we inform the Server.
        if ((x == mDest.x && y == mDest.y)
            || Net::getNetworkType() == ServerInfo::TMWATHENA)
                Net::getPlayerHandler()->setDestination(x, y, mDirection);
    }

    mPickUpTarget = NULL;
    mKeepAttacking = false;
}

void LocalPlayer::setWalkingDir(int dir)
{
    // This function is called by Game::handleInput()

    if (Net::getNetworkType() == ServerInfo::MANASERV)
    {
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
        if (get_elapsed_time(mLocalWalkTime) < walkingKeyboardDelay)
            return;
    }

    mWalkingDir = dir;

    // If we're not already walking, start walking.
    if (mAction != MOVE && dir)
    {
        startWalking(dir);
    }
    else if (mAction == MOVE && (Net::getNetworkType() == ServerInfo::MANASERV))
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
        if (Net::getNetworkType() == ServerInfo::MANASERV)
        {
            const Vector &pos = getPosition();
            Being::setDestination(pos.x, pos.y);
        }
        else
            Being::setDestination(getTileX(), getTileY());
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
    if (Net::getNetworkType() == ServerInfo::MANASERV)
        setDirection(dir);

    if (Net::getNetworkType() == ServerInfo::TMWATHENA)
    {
        // Prevent skipping corners over colliding tiles
        if (dx && !mMap->getWalk(getTileX() + dx, getTileY(), getWalkMask()))
            dx = 0;
        if (dy && !mMap->getWalk(getTileX(), getTileY() + dy, getWalkMask()))
            dy = 0;

        // Choose a straight direction when diagonal target is blocked
        if (dx && dy && !mMap->getWalk(getTileX() + dx, getTileY() + dy,
                                       getWalkMask()))
            dx = 0;

        // Walk to where the player can actually go
        if ((dx || dy) && mMap->getWalk(getTileX() + dx, getTileY() + dy,
                                        getWalkMask()))
        {
            setDestination(getTileX() + dx, getTileY() + dy);
        }
        else if (dir != mDirection)
        {
            // If the being can't move, just change direction
            Net::getPlayerHandler()->setDirection(dir);
            setDirection(dir);
        }
    }
    else
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
    if (Net::getNetworkType() == ServerInfo::MANASERV)
    {
        if (mLastAction != -1)
            return;

        // Can only attack when standing still
        if (mAction != STAND && mAction != ATTACK)
            return;
    }

    mKeepAttacking = keep;

    if (!target || target->getType() == ActorSprite::NPC)
        return;

    if (mTarget != target || !mTarget)
    {
        mLastTarget = -1;
        setTarget(target);
    }

    if (Net::getNetworkType() == ServerInfo::MANASERV)
    {
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
    }
    else
    {
        int dist_x = target->getTileX() - getTileX();
        int dist_y = target->getTileY() - getTileY();

        // Must be standing to attack
        if (mAction != STAND)
            return;

        Uint8 direction = 0;
        if (abs(dist_y) >= abs(dist_x))
        {
            if (dist_y > 0)
                direction = DOWN;
            else
                direction = UP;
        }
        else
        {
            if (dist_x > 0)
                direction = RIGHT;
            else
                direction = LEFT;
        }
        Net::getPlayerHandler()->setDirection(direction);
        setDirection(direction);

        mActionTime = tick_time;
        mTargetTime = tick_time;
    }

    setAction(ATTACK);

    if (mEquippedWeapon)
    {
        std::string soundFile = mEquippedWeapon->getSound(EQUIP_EVENT_STRIKE);
        if (!soundFile.empty())
            sound.playSfx(soundFile);
    }
    else
        sound.playSfx(paths.getValue("attackSfxFile", "fist-swish.ogg"));

    Net::getPlayerHandler()->attack(target->getId());
    if ((Net::getNetworkType() == ServerInfo::TMWATHENA) && !keep)
        stopAttack();
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
        if (mMap && config.getValue("showpickupparticle", 0))
        {
            // Show pickup notification
            addMessageToQueue(_(msg), UserPalette::PICKUP_INFO);
        }
    }
    else
    {
        if (config.getValue("showpickupchat", 1))
        {
            // TRANSLATORS: This sentence may be translated differently
            // for different grammatical numbers (singular, plural, ...)
            SERVER_NOTICE(strprintf(ngettext("You picked up %d "
                    "[@@%d|%s@@].", "You picked up %d [@@%d|%s@@].", amount),
                    amount, itemInfo.getId(), itemInfo.getName().c_str()))
        }

        if (mMap && config.getValue("showpickupparticle", 0))
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

int LocalPlayer::getAttackRange()
{
    if (mAttackRange > -1)
    {
        return mAttackRange;
    }
    else
    {
        if (Net::getNetworkType() == ServerInfo::TMWATHENA)
        {
            // TODO: Fix this to be more generic
            Item *weapon = PlayerInfo::getEquipment(
                                                  TmwAthena::EQUIP_FIGHT1_SLOT);
            if (weapon)
            {
                const ItemInfo info = weapon->getInfo();
                return info.getAttackRange();
            }
        }
        return 48; // unarmed range
    }
}

bool LocalPlayer::withinAttackRange(Being *target)
{
    if (Net::getNetworkType() == ServerInfo::MANASERV)
    {
        const Vector &targetPos = target->getPosition();
        const Vector &pos = getPosition();
        const int dx = abs(targetPos.x - pos.x);
        const int dy = abs(targetPos.y - pos.y);
        const int range = getAttackRange();

        return !(dx > range || dy > range);
    }
    else
    {
        int dist_x = abs(target->getTileX() - getTileX());
        int dist_y = abs(target->getTileY() - getTileY());

        if (dist_x > getAttackRange() || dist_y > getAttackRange())
            return false;

        return true;
    }
}

void LocalPlayer::setGotoTarget(Being *target)
{
    mLastTarget = -1;
    if (Net::getNetworkType() == ServerInfo::MANASERV)
    {
        mTarget = target;
        mGoingToTarget = true;
        const Vector &targetPos = target->getPosition();
        setDestination(targetPos.x, targetPos.y);
    }
    else
    {
        setTarget(target);
        mGoingToTarget = true;
        setDestination(target->getTileX(), target->getTileY());
    }
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
                int change = event.getInt("newValue")
                        - event.getInt("oldValue");

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
