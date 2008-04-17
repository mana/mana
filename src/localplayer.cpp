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

#include "localplayer.h"

#include "equipment.h"
#include "floor_item.h"
#include "game.h"
#include "guild.h"
#include "inventory.h"
#include "item.h"
#include "main.h"
#include "particle.h"
#include "sound.h"
#include "log.h"

#include "net/gameserver/player.h"
#include "net/chatserver/guild.h"

#include "gui/gui.h"

#include "net/messageout.h"
#include "net/protocol.h"

#include "utils/tostring.h"
#include "utils/gettext.h"

LocalPlayer *player_node = NULL;

LocalPlayer::LocalPlayer():
    Player(65535, 0, NULL),
    mAttackRange(0),
    mInventory(new Inventory),
    mEquipment(new Equipment),
    mAttributeBase(NB_CHARACTER_ATTRIBUTES, -1),
    mAttributeEffective(NB_CHARACTER_ATTRIBUTES, -1),
    mExpCurrent(CHAR_SKILL_NB, -1),
    mExpNext(CHAR_SKILL_NB, -1),
    mCharacterPoints(-1),
    mCorrectionPoints(-1),
    mLevel(1), mLevelProgress(0),
    mMoney(0),
    mTotalWeight(1), mMaxWeight(1),
    mHP(1), mMaxHP(1),
    mTarget(NULL), mPickUpTarget(NULL),
    mTrading(false),
    mLastAction(-1), mWalkingDir(0),
    mDestX(0), mDestY(0),
    mLocalWalkTime(-1),
    mExpMessageTime(0)
{
}

LocalPlayer::~LocalPlayer()
{
    delete mInventory;
}

void LocalPlayer::logic()
{
    // Actions are allowed once per second
    if (get_elapsed_time(mLastAction) >= 1000) {
        mLastAction = -1;
    }

    // Show XP messages
    if(!mExpMessages.empty())
    {
        if (mExpMessageTime == 0)
        {
            particleEngine->addTextRiseFadeOutEffect(mExpMessages.front(),
                                                     0, 128, 255,
                                                     speechFont,
                                                     mPx + 16, mPy - 16);
            mExpMessages.pop_front();
            mExpMessageTime = 30;
        }
        mExpMessageTime--;
    }

    Being::logic();
}

void LocalPlayer::nextStep()
{
    if (mPath.empty())
    {
        if (mPickUpTarget)
        {
            pickUp(mPickUpTarget);
        }

        if (mWalkingDir)
        {
            walk(mWalkingDir);
        }
    }

    Player::nextStep();
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

void LocalPlayer::invite(Being *being)
{
    // TODO: Allow user to choose which guild to invite being to
    // For now, just invite to the first guild you have permissions to invite with
    std::map<int, Guild*>::iterator itr = mGuilds.begin();
    std::map<int, Guild*>::iterator itr_end = mGuilds.end();
    for (; itr != itr_end; ++itr)
    {
        if (checkInviteRights(itr->second->getName()))
        {
            Net::ChatServer::Guild::invitePlayer(being->getName(), itr->second->getId());
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
    mInventory->setItem(index, id, amount);
}

void
LocalPlayer::moveInvItem(Item *item, int newIndex)
{
    // special case, the old and new cannot copy over each other.
    if (item->getInvIndex() == newIndex)
        return;

    Net::GameServer::Player::moveItem(
        item->getInvIndex(), newIndex, item->getQuantity());
}

Item* LocalPlayer::searchForItem(int itemId)
{
    for (int i = 0; i < INVENTORY_SIZE; i++)
    {
        if (itemId == mInventory->getItem(i)->getId()) {
            return mInventory->getItem(i);
        }
    }
    return NULL;
}

void LocalPlayer::equipItem(Item *item)
{
    Net::GameServer::Player::equip(item->getInvIndex());
}

void LocalPlayer::unequipItem(int slot)
{
    Net::GameServer::Player::unequip(slot);

    // Tidy equipment directly to avoid weapon still shown bug, for instance
    mEquipment->setEquipment(slot, 0);
}

void LocalPlayer::useItem(int slot)
{
    Net::GameServer::Player::useItem(slot);
}

void LocalPlayer::dropItem(Item *item, int quantity)
{
    Net::GameServer::Player::drop(item->getInvIndex(), quantity);
}

void LocalPlayer::splitItem(Item *item, int quantity)
{
    int newIndex = mInventory->getFreeSlot();
    if (newIndex > Inventory::NO_SLOT_INDEX)
    {
        Net::GameServer::Player::moveItem(
            item->getInvIndex(), newIndex, quantity);
    }

}

void LocalPlayer::pickUp(FloorItem *item)
{
    int dx = item->getX() - mX / 32;
    int dy = item->getY() - mY / 32;

    if (dx * dx + dy * dy < 4) {
        int id = item->getId();
        Net::GameServer::Player::pickUp(id >> 16, id & 0xFFFF);
        mPickUpTarget = NULL;
    } else {
        setDestination(item->getX() * 32 + 16, item->getY() * 32 + 16,
                       BY_SYSTEM);
        mPickUpTarget = item;
    }
}

void LocalPlayer::walk(unsigned char dir)
{
    if (!mMap || !dir)
        return;

    if (mAction == WALK && !mPath.empty())
    {
        // Just finish the current action, otherwise we get out of sync
        Being::setDestination(mX, mY);
        return;
    }

    int dx = 0, dy = 0;
    if (dir & UP)
        dy -= 32;
    if (dir & DOWN)
        dy += 32;
    if (dir & LEFT)
        dx -= 32;
    if (dir & RIGHT)
        dx += 32;

    // Prevent skipping corners over colliding tiles
    if (dx && !mMap->getWalk((mX + dx) / 32, mY / 32, getWalkMask()))
        dx = 16 - mX % 32;
    if (dy && !mMap->getWalk(mX / 32, (mY + dy) / 32, getWalkMask()))
        dy = 16 - mY % 32;

    // Choose a straight direction when diagonal target is blocked
    if (dx && dy && !mMap->getWalk((mX + dx) / 32, (mY + dy) / 32, getWalkMask()))
        dx = 16 - mX % 32;

    // Walk to where the player can actually go
    if ((dx || dy) && mMap->getWalk((mX + dx) / 32, (mY + dy) / 32, getWalkMask()))
    {
        setDestination(mX + dx, mY + dy, BY_SYSTEM);
    }
    else if (dir)
    {
        // If the being can't move, just change direction
        // TODO: Communicate this to the server (waiting on tmwserv)
        setDirection(dir);
    }
}

void LocalPlayer::setDestination(Uint16 x, Uint16 y, InputType inputType)
{
    // Fix coordinates so that the player does not seem to dig into walls.
    int tx = x / 32, ty = y / 32, fx = x % 32, fy = y % 32;
    if (fx != 16 && !mMap->getWalk(tx + fx / 16 * 2 - 1, ty, getWalkMask())) fx = 16;
    if (fy != 16 && !mMap->getWalk(tx, ty + fy / 16 * 2 - 1, getWalkMask())) fy = 16;
    if (fx != 16 && fy != 16 && !mMap->getWalk(tx + fx / 16 * 2 - 1, ty + fy / 16 * 2 - 1, getWalkMask())) fx = 16;
    x = tx * 32 + fx;
    y = ty * 32 + fy;

    // Only send a new message to the server when destination changes
    if (x != mDestX || y != mDestY)
    {
        // Using mouse, Walkings are allowed twice per second
        if (inputType == BY_MOUSE && get_elapsed_time(mLocalWalkTime) < 500) return;

        // Using keyboard, Walkings are allowed ten times per second
        if (inputType == BY_KEYBOARD && get_elapsed_time(mLocalWalkTime) < 100) return;

        mLocalWalkTime = tick_time;

        mDestX = x;
        mDestY = y;

        Net::GameServer::Player::walk(x, y);
        particleEngine->addEffect("graphics/particles/hit.particle.xml", x, y);
    }

    Being::setDestination(x, y);
    mPickUpTarget = NULL;
}

void LocalPlayer::setWalkingDir(int dir)
{
    if (mWalkingDir != dir)
    {
        mWalkingDir = dir;
    }

    // If we're not already walking, start walking.
    if (mAction != WALK && dir)
    {
        walk(dir);
    }
}

void LocalPlayer::toggleSit()
{
    if (mLastAction != -1)
        return;
    mLastAction = tick_time;

    Being::Action newAction;
    switch (mAction)
    {
        case Being::STAND: newAction = Being::SIT; break;
        case Being::SIT: newAction = Being::STAND; break;
        default: return;
    }

    setAction(newAction);
    Net::GameServer::Player::changeAction(newAction);
}

void LocalPlayer::emote(Uint8 emotion)
{
    if (mLastAction != -1)
        return;
    mLastAction = tick_time;

    // XXX Convert for new server
    /*
    MessageOut outMsg(0x00bf);
    outMsg.writeInt8(emotion);
    */
}

void LocalPlayer::trade(Being *being) const
{
    extern std::string tradePartnerName;
    extern int tradePartnerID;
    tradePartnerName = being->getName();
    tradePartnerID = being->getId();
    Net::GameServer::Player::requestTrade(tradePartnerID);
}

bool LocalPlayer::tradeRequestOk() const
{
    return !mTrading;
}

void LocalPlayer::attack()
{
    if (mLastAction != -1)
        return;

    // Can only attack when standing still
    if (mAction != STAND && mAction != ATTACK)
        return;

    mLastAction = tick_time;
    mWalkTime = tick_time;

    setAction(ATTACK);

    if (mEquippedWeapon)
    {
        std::string soundFile = mEquippedWeapon->getSound(EQUIP_EVENT_STRIKE);
        if (soundFile != "") sound.playSfx(soundFile);
    }
    else {
        sound.playSfx("sfx/fist-swish.ogg");
    }
    Net::GameServer::Player::attack(getSpriteDirection());
}

Being* LocalPlayer::getTarget() const
{
    return mTarget;
}

void LocalPlayer::revive()
{
    // XXX Convert for new server
    /*
    MessageOut outMsg(0x00b2);
    outMsg.writeInt8(0);
    */
}

void LocalPlayer::raiseAttribute(size_t attr)
{
    // we assume that the server allows the change. When not we will undo it later.
    mCharacterPoints--;
    mAttributeBase.at(attr)++;
    Net::GameServer::Player::raiseAttribute(attr + CHAR_ATTR_BEGIN);
}

void LocalPlayer::lowerAttribute(size_t attr)
{
    // we assume that the server allows the change. When not we will undo it later.
    mCorrectionPoints--;
    mCharacterPoints++;
    mAttributeBase.at(attr)--;
    Net::GameServer::Player::lowerAttribute(attr + CHAR_ATTR_BEGIN);
}

const std::string& LocalPlayer::getSkillName(int skill)
{
    static const std::string skills[CHAR_SKILL_NB + 1] =
    {
        _("Unarmed"),  // CHAR_SKILL_WEAPON_NONE
        _("Knife"),    // CHAR_SKILL_WEAPON_KNIFE
        _("Sword"),    // CHAR_SKILL_WEAPON_SWORD
        _("Polearm"),  // CHAR_SKILL_WEAPON_POLEARM
        _("Staff"),    // CHAR_SKILL_WEAPON_STAFF
        _("Whip"),     // CHAR_SKILL_WEAPON_WHIP
        _("Bow"),      // CHAR_SKILL_WEAPON_BOW
        _("Shooting"), // CHAR_SKILL_WEAPON_SHOOTING
        _("Mace"),     // CHAR_SKILL_WEAPON_MACE
        _("Axe"),      // CHAR_SKILL_WEAPON_AXE
        _("Thrown"),   // CHAR_SKILL_WEAPON_THROWN
        _("Magic"),    // CHAR_SKILL_MAGIC_IAMJUSTAPLACEHOLDER
        _("Craft"),    // CHAR_SKILL_CRAFT_IAMJUSTAPLACEHOLDER
        _("Unknown Skill")
    };

    if ((skill < 0) || (skill > CHAR_SKILL_NB))
    {
        return skills[CHAR_SKILL_NB];
    }
    else
    {
        return skills[skill];
    }

}

void LocalPlayer::setExperience(int skill, int current, int next)
{
    int diff = current - mExpCurrent.at(skill);
    if (mMap && mExpCurrent.at(skill) != -1 && diff > 0)
    {
        const std::string text = toString(diff) + " " + getSkillName(skill) + " xp";
        mExpMessages.push_back(text);
    }

    mExpCurrent.at(skill) = current;
    mExpNext.at(skill) = next;
}

std::pair<int, int> LocalPlayer::getExperience(int skill)
{
    return std::pair<int, int> (mExpCurrent.at(skill), mExpNext.at(skill));
}
