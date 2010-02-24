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
#include "effectmanager.h"
#include "equipment.h"
#include "flooritem.h"
#include "graphics.h"
#include "guild.h"
#include "inventory.h"
#include "item.h"
#include "log.h"
#include "map.h"
#include "monster.h"
#include "particle.h"
#include "simpleanimation.h"
#include "sound.h"
#include "statuseffect.h"
#include "text.h"

#include "gui/gui.h"
#include "gui/ministatus.h"
#include "gui/palette.h"
#include "gui/skilldialog.h"
#include "gui/statuswindow.h"
#include "gui/storagewindow.h"

#include "gui/widgets/chattab.h"

#include "net/guildhandler.h"
#include "net/inventoryhandler.h"
#include "net/net.h"
#include "net/partyhandler.h"
#include "net/playerhandler.h"
#include "net/specialhandler.h"
#include "net/tradehandler.h"

#include "resources/animation.h"
#include "resources/imageset.h"
#include "resources/itemdb.h"
#include "resources/iteminfo.h"
#include "resources/resourcemanager.h"

#include "utils/gettext.h"
#include "utils/stringutils.h"

#include <cassert>

// This is the minimal delay between to permitted
// setDestination() calls using the keyboard.
// TODO: This can fine tuned later on when running is added...
const short walkingKeyboardDelay = 1000;

LocalPlayer *player_node = NULL;

LocalPlayer::LocalPlayer(int id, int job):
    Player(id, job, 0),
    mEquipment(new Equipment),
    mAttackRange(0),
    mTargetTime(-1),
    mLastTarget(-1),
    mCharacterPoints(0),
    mCorrectionPoints(0),
    mSpecialRechargeUpdateNeeded(0),
    mLevel(1),
    mExp(0), mExpNeeded(0),
    mMp(0), mMaxMp(0),
    mMoney(0),
    mTotalWeight(1), mMaxWeight(1),
    mHp(1), mMaxHp(1),
    mSkillPoints(0),
    mTarget(NULL),
    mPlayerFollowed(""),
    mPickUpTarget(NULL),
    mTrading(false), mGoingToTarget(false), mKeepAttacking(false),
    mLastAction(-1),
    mWalkingDir(0),
    mPathSetByMouse(false),
    mInventory(new Inventory(Net::getInventoryHandler()
                             ->getSize(Net::InventoryHandler::INVENTORY))),
    mLocalWalkTime(-1),
    mMessageTime(0)
{
    // Variable to keep the local player from doing certain actions before a map
    // is initialized. e.g. drawing a player's name using the TextManager, since
    // it appears to be dependant upon map coordinates for updating drawing.
    mMapInitialized = false;

    mUpdateName = true;

    mTextColor = &guiPalette->getColor(Palette::PLAYER);
    mNameColor = &guiPalette->getColor(Palette::SELF);

    initTargetCursor();

    config.addListener("showownname", this);
    setShowName(config.getValue("showownname", 1));
}

LocalPlayer::~LocalPlayer()
{
    delete mInventory;

    config.removeListener("showownname", this);

    for (int i = Being::TC_SMALL; i < Being::NUM_TC; i++)
    {
        delete mTargetCursor[0][i];
        delete mTargetCursor[1][i];
        mTargetCursorImages[0][i]->decRef();
        mTargetCursorImages[1][i]->decRef();
    }
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
                    &guiPalette->getColor(info.second),
                    gui->getInfoParticleFont(), true);

            mMessages.pop_front();
            mMessageTime = 30;
        }
        mMessageTime--;
    }

    if ((mSpecialRechargeUpdateNeeded%11) == 0)
    {
        mSpecialRechargeUpdateNeeded = 0;
        for (std::map<int, Special>::iterator i = mSpecials.begin();
             i != mSpecials.end();
             i++)
        {
            i->second.currentMana += i->second.recharge;
            if (i->second.currentMana > i->second.neededMana)
            {
                i->second.currentMana = i->second.neededMana;
            }
        }
    }
    mSpecialRechargeUpdateNeeded++;

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
        if (mTarget->getType() == Being::NPC)
        {
            // NPCs are always in range
            mTarget->setTargetAnimation(
                mTargetCursor[0][mTarget->getTargetCursorSize()]);
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
            const int inRange = rangeX > attackRange || rangeY > attackRange
                                                                    ? 1 : 0;
            mTarget->setTargetAnimation(
                mTargetCursor[inRange][mTarget->getTargetCursorSize()]);

            if (!mTarget->isAlive())
                stopAttack();

            if (mKeepAttacking && mTarget)
                attack(mTarget, true);
        }
    }

    Player::logic();
}

void LocalPlayer::setAction(Action action, int attackType)
{
    if (action == DEAD)
    {
        mLastTarget = -1;
        setTarget(NULL);
    }

    Player::setAction(action, attackType);
}

void LocalPlayer::setGM(bool gm)
{
    mIsGM = gm;
}

void LocalPlayer::setGMLevel(int level)
{
    mGMLevel = level;

    if (level > 0)
        setGM(true);
}


void LocalPlayer::nextTile(unsigned char dir = 0)
{
    if (Net::getNetworkType() == ServerInfo::EATHENA)
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


        Player::nextTile();
    }
    else
    {
        if (!mMap || !dir)
            return;

        const Vector &pos = getPosition();

        // Compute where the next step will be set.
        int dx = 0, dy = 0;
        if (dir & UP)
            dy--;
        if (dir & DOWN)
            dy++;
        if (dir & LEFT)
            dx--;
        if (dir & RIGHT)
            dx++;

        // Prevent skipping corners over colliding tiles
        if (dx && !mMap->getWalk(((int) pos.x + dx) / 32,
                                 (int) pos.y / 32, getWalkMask()))
            dx = 16 - (int) pos.x % 32;
        if (dy && !mMap->getWalk((int) pos.x / 32,
                                 ((int) pos.y + dy) / 32, getWalkMask()))
            dy = 16 - (int) pos.y % 32;

        // Choose a straight direction when diagonal target is blocked
        if (dx && dy && !mMap->getWalk((pos.x + dx) / 32,
                                       (pos.y + dy) / 32, getWalkMask()))
            dx = 16 - (int) pos.x % 32;

        int dScaler; // Distance to walk

        // Checks our path up to 1 tiles, if a blocking tile is found
        // We go to the last good tile, and break out of the loop
        for (dScaler = 1; dScaler <= 32; dScaler++)
        {
            if ( (dx || dy) &&
                 !mMap->getWalk( ((int) pos.x + (dx * dScaler)) / 32,
                                 ((int) pos.y + (dy * dScaler)) / 32, getWalkMask()) )
            {
                dScaler--;
                break;
            }
        }

        if (dScaler > 16)
        {
            //effectManager->trigger(15, (int) pos.x + (dx * dScaler), (int) pos.y + (dy * dScaler));
            setDestination((int) pos.x + (dx * dScaler), (int) pos.y + (dy * dScaler));
        }
        else if (dir)
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
    Player *player = static_cast<Player*>(being);

    // TODO: Allow user to choose which guild to invite being to
    // For now, just invite to the first guild you have permissions to invite with
    std::map<int, Guild*>::iterator itr = mGuilds.begin();
    std::map<int, Guild*>::iterator itr_end = mGuilds.end();
    for (; itr != itr_end; ++itr)
    {
        if (checkInviteRights(itr->second->getName()))
        {
            Net::getGuildHandler()->invite(itr->second->getId(), player);
            return;
        }
    }
}

void LocalPlayer::clearInventory()
{
    mEquipment->clear();
    mInventory->clear();
}

void LocalPlayer::setInvItem(int index, int id, int amount)
{
    bool equipment = false;
    int itemType = ItemDB::get(id).getType();
    if (itemType != ITEM_UNUSABLE && itemType != ITEM_USABLE)
        equipment = true;
    mInventory->setItem(index, id, amount, equipment);
}

void LocalPlayer::pickUp(FloorItem *item)
{
    int dx = item->getX() - (int) getPosition().x / 32;
    int dy = item->getY() - (int) getPosition().y / 32;

    if (dx * dx + dy * dy < 4)
    {
        Net::getPlayerHandler()->pickUp(item);
        mPickUpTarget = NULL;
    }
    else
    {
        if (Net::getNetworkType() == ServerInfo::MANASERV)
        {
            setDestination(item->getX() * 32 + 16, item->getY() * 32 + 16);
            mPickUpTarget = item;
        }
        else
        {
            setDestination(item->getX(), item->getY());
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
    if (mLastTarget != -1 || target == this)
        return;

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

    if (mTarget)
        mTarget->untarget();

    if (mTarget && mTarget->getType() == Being::MONSTER)
        static_cast<Monster *>(mTarget)->setShowName(false);

    mTarget = target;

    if (target && target->getType() == Being::MONSTER)
        static_cast<Monster *>(target)->setShowName(true);
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
            || Net::getNetworkType() == ServerInfo::EATHENA)
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
        // then, stop (sending to server).
        else if (!dir)
        {
            if (!mPathSetByMouse)
                player_node->stopWalking(true);
            return;
        }

        // If the delay to send another walk message to the server hasn't expired,
        // don't do anything or we could get disconnected for spamming the server
        if (get_elapsed_time(mLocalWalkTime) < walkingKeyboardDelay)
            return;
    }

    mWalkingDir = dir;

    // If we're not already walking, start walking.
    if (mAction != WALK && dir)
    {
        startWalking(dir);
    }
    else if (mAction == WALK && (Net::getNetworkType() == ServerInfo::MANASERV))
    {
        nextTile(dir);
    }
}

void LocalPlayer::startWalking(unsigned char dir)
{
    // This function is called by setWalkingDir(),
    // but also by nextStep() for eAthena...

    // TODO: Evaluate the implementation of this method for Manaserv
    if (!mMap || !dir)
        return;

    if (mAction == WALK && !mPath.empty())
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

    if (Net::getNetworkType() == ServerInfo::EATHENA)
    {
        // Prevent skipping corners over colliding tiles
        if (dx && !mMap->getWalk(getTileX() + dx, getTileY(), getWalkMask()))
            dx = 0;
        if (dy && !mMap->getWalk(getTileX(), getTileY() + dy, getWalkMask()))
            dy = 0;

        // Choose a straight direction when diagonal target is blocked
        if (dx && dy && !mMap->getWalk(getTileX() + dx, getTileY() + dy, getWalkMask()))
            dx = 0;

        // Walk to where the player can actually go
        if ((dx || dy) && mMap->getWalk(getTileX() + dx, getTileY() + dy, getWalkMask()))
        {
            setDestination(getTileX() + dx, getTileY() + dy);
        }
        else if (dir)
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
    if (mAction == WALK && mWalkingDir)
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

void LocalPlayer::useSpecial(int special)
{
    Net::getSpecialHandler()->use(special);
}

void LocalPlayer::setSpecialStatus(int id, int current, int max, int recharge)
{
    logger->log("SpecialUpdate Skill #%d -- (%d/%d) -> %d", id, current, max, recharge);
    mSpecials[id].currentMana = current;
    mSpecials[id].neededMana = max;
    mSpecials[id].recharge = recharge;
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

    if (!target || target->getType() == Being::NPC)
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

        if (abs(dist_y) >= abs(dist_x))
        {
            if (dist_y > 0)
                setDirection(DOWN);
            else
                setDirection(UP);
        }
        else
        {
            if (dist_x > 0)
                setDirection(RIGHT);
            else
                setDirection(LEFT);
        }

        mWalkTime = tick_time;
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
    {
        sound.playSfx("sfx/fist-swish.ogg");
    }

    Net::getPlayerHandler()->attack(target->getId());
    if ((Net::getNetworkType() == ServerInfo::EATHENA) && !keep)
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

void LocalPlayer::raiseAttribute(int attr)
{
    // we assume that the server allows the change. When not we will undo it later.
    mCharacterPoints--;
    IntMap::iterator it = mAttributeBase.find(attr);
    if (it != mAttributeBase.end())
        (*it).second++;
    Net::getPlayerHandler()->increaseAttribute(attr);
}

void LocalPlayer::lowerAttribute(int attr)
{
    // we assume that the server allows the change. When not we will undo it later.
    mCorrectionPoints--;
    mCharacterPoints++;
    IntMap::iterator it = mAttributeBase.find(attr);
    if (it != mAttributeBase.end())
        (*it).second--;
    Net::getPlayerHandler()->decreaseAttribute(attr);
}

void LocalPlayer::setAttributeBase(int num, int value, bool notify)
{
    int old = mAttributeBase[num];

    mAttributeBase[num] = value;
    if (skillDialog)
    {
        if (skillDialog->update(num).empty() || !(value > old))
            return;

        if (old != 0 && notify)
            effectManager->trigger(1, this);
    }

    if (statusWindow)
        statusWindow->update(num);
}

void LocalPlayer::setAttributeEffective(int num, int value)
{
    mAttributeEffective[num] = value;
    if (skillDialog)
        skillDialog->update(num);

    if (statusWindow)
        statusWindow->update(num);
}

void LocalPlayer::setCharacterPoints(int n)
{
    mCharacterPoints = n;

    if (statusWindow)
        statusWindow->update(StatusWindow::CHAR_POINTS);
}

void LocalPlayer::setCorrectionPoints(int n)
{
    mCorrectionPoints = n;

    if (statusWindow)
        statusWindow->update(StatusWindow::CHAR_POINTS);
}

void LocalPlayer::setSkillPoints(int points)
{
    mSkillPoints = points;
    if (skillDialog)
        skillDialog->update();
}

void LocalPlayer::setExperience(int skill, int current, int next, bool notify)
{
    std::pair<int, int> cur = getExperience(skill);
    int diff = current - cur.first;

    cur = std::pair<int, int>(current, next);

    mSkillExp[skill] = cur;

    std::string name;
    if (skillDialog)
        name = skillDialog->update(skill);

    if (mMap && notify && cur.first != -1 && diff > 0 && !name.empty())
    {
        addMessageToQueue(strprintf("%d %s xp", diff, name.c_str()));
    }

    if (statusWindow)
        statusWindow->update(skill);
}

std::pair<int, int> LocalPlayer::getExperience(int skill)
{
    return mSkillExp[skill];
}

void LocalPlayer::setHp(int value)
{
    mHp = value;

    if (statusWindow)
        statusWindow->update(StatusWindow::HP);
}

void LocalPlayer::setMaxHp(int value)
{
    mMaxHp = value;

    if (statusWindow)
        statusWindow->update(StatusWindow::HP);
}

void LocalPlayer::setLevel(int value)
{
    mLevel = value;

    if (statusWindow)
        statusWindow->update(StatusWindow::LEVEL);
}

void LocalPlayer::setExp(int value, bool notify)
{
    if (mMap && notify && value > mExp)
    {
        addMessageToQueue(toString(value - mExp) + " xp");
    }
    mExp = value;

    if (statusWindow)
        statusWindow->update(StatusWindow::EXP);
}

void LocalPlayer::setExpNeeded(int value)
{
    mExpNeeded = value;

    if (statusWindow)
        statusWindow->update(StatusWindow::EXP);
}

void LocalPlayer::setMP(int value)
{
    mMp = value;

    if (statusWindow)
        statusWindow->update(StatusWindow::MP);
}

void LocalPlayer::setMaxMP(int value)
{
    mMaxMp = value;

    if (statusWindow)
        statusWindow->update(StatusWindow::MP);
}

void LocalPlayer::setMoney(int value)
{
    mMoney = value;

    if (statusWindow)
        statusWindow->update(StatusWindow::MONEY);
}

void LocalPlayer::pickedUp(const ItemInfo &itemInfo, int amount)
{
    if (!amount)
    {
        if (config.getValue("showpickupchat", 1))
        {
            localChatTab->chatLog(_("Unable to pick up item."), BY_SERVER);
        }
    }
    else
    {
        if (config.getValue("showpickupchat", 1))
        {
            // TRANSLATORS: This sentence may be translated differently
            // for different grammatical numbers (singular, plural, ...)
            localChatTab->chatLog(strprintf(ngettext("You picked up %d "
                    "[@@%d|%s@@].", "You picked up %d [@@%d|%s@@].", amount),
                    amount, itemInfo.getId(), itemInfo.getName().c_str()),
                    BY_SERVER);
        }

        if (mMap && config.getValue("showpickupparticle", 0))
        {
            // Show pickup notification
            addMessageToQueue(itemInfo.getName(), Palette::PICKUP_INFO);
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
        Item *weapon = mEquipment->getEquipment(EQUIP_FIGHT1_SLOT);
        if (weapon)
        {
            const ItemInfo info = weapon->getInfo();
            return info.getAttackRange();
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
        int dist_x = abs(target->getTileX() - getTileY());
        int dist_y = abs(target->getTileY() - getTileX());

        if (dist_x > getAttackRange() || dist_y > getAttackRange())
        {
            return false;
        }

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

extern MiniStatusWindow *miniStatusWindow;

void LocalPlayer::handleStatusEffect(StatusEffect *effect, int effectId)
{
    Being::handleStatusEffect(effect, effectId);

    if (effect)
    {
        effect->deliverMessage();
        effect->playSFX();

        AnimatedSprite *sprite = effect->getIcon();

        if (!sprite)
        {
            // delete sprite, if necessary
            for (unsigned int i = 0; i < mStatusEffectIcons.size();)
                if (mStatusEffectIcons[i] == effectId)
                {
                    mStatusEffectIcons.erase(mStatusEffectIcons.begin() + i);
                    miniStatusWindow->eraseIcon(i);
                }
                else
                    i++;
        }
        else
        {
            // replace sprite or append
            bool found = false;

            for (unsigned int i = 0; i < mStatusEffectIcons.size(); i++)
                if (mStatusEffectIcons[i] == effectId)
                {
                    miniStatusWindow->setIcon(i, sprite);
                    found = true;
                    break;
                }

            if (!found)
            { // add new
                int offset = mStatusEffectIcons.size();
                miniStatusWindow->setIcon(offset, sprite);
                mStatusEffectIcons.push_back(effectId);
            }
        }
    }
}

void LocalPlayer::initTargetCursor()
{
    // Load target cursors
    loadTargetCursor("graphics/gui/target-cursor-blue-s.png", 44, 35,
                     false, TC_SMALL);
    loadTargetCursor("graphics/gui/target-cursor-red-s.png", 44, 35,
                     true, TC_SMALL);
    loadTargetCursor("graphics/gui/target-cursor-blue-m.png", 62, 44,
                     false, TC_MEDIUM);
    loadTargetCursor("graphics/gui/target-cursor-red-m.png", 62, 44,
                     true, TC_MEDIUM);
    loadTargetCursor("graphics/gui/target-cursor-blue-l.png", 82, 60,
                     false, TC_LARGE);
    loadTargetCursor("graphics/gui/target-cursor-red-l.png", 82, 60,
                     true, TC_LARGE);
}

void LocalPlayer::loadTargetCursor(const std::string &filename,
                                   int width, int height,
                                   bool outRange, TargetCursorSize size)
{
    assert(size > -1);
    assert(size < 3);

    ResourceManager *resman = ResourceManager::getInstance();

    ImageSet *currentImageSet = resman->getImageSet(filename, width, height);
    Animation *anim = new Animation;

    for (unsigned int i = 0; i < currentImageSet->size(); ++i)
    {
        anim->addFrame(currentImageSet->get(i), 75,
                      (16 - (currentImageSet->getWidth() / 2)),
                      (16 - (currentImageSet->getHeight() / 2)));
    }

    SimpleAnimation *currentCursor = new SimpleAnimation(anim);

    const int index = outRange ? 1 : 0;

    mTargetCursorImages[index][size] = currentImageSet;
    mTargetCursor[index][size] = currentCursor;
}

void LocalPlayer::addMessageToQueue(const std::string &message,
                                    Palette::ColorType color)
{
    mMessages.push_back(MessagePair(message, color));
}

void LocalPlayer::optionChanged(const std::string &value)
{
    if (value == "showownname")
    {
        setShowName(config.getValue("showownname", 1));
    }
}
