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
#include "effectmanager.h"

#include "net/gameserver/player.h"
#include "net/chatserver/guild.h"
#include "net/chatserver/party.h"

#include "gui/gui.h"

#include "net/messageout.h"
#include "net/protocol.h"

#include "utils/tostring.h"
#include "utils/gettext.h"

const short walkingKeyboardDelay = 100;

LocalPlayer *player_node = NULL;

LocalPlayer::LocalPlayer():
    Player(65535, 0, NULL),
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
    mTrading(false), mGoingToTarget(false),
    mLastAction(-1),
    mWalkingDir(0),
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
    if (!mExpMessages.empty())
    {
        if (mExpMessageTime == 0)
        {
            const Vector &pos = getPosition();
            particleEngine->addTextRiseFadeOutEffect(mExpMessages.front(),
                                                     0, 128, 255,
                                                     speechFont,
                                                     (int) pos.x + 16,
                                                     (int) pos.y - 16);
            mExpMessages.pop_front();
            mExpMessageTime = 30;
        }
        mExpMessageTime--;
    }

    Being::logic();
}

void LocalPlayer::nextStep()
{
    // TODO: Fix picking up when reaching target (this method is obsolete)
    // TODO: Fix holding walking button to keep walking smoothly
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

    // TODO: Fix automatically walking within range of target, when wanted
    if (mGoingToTarget && mTarget && withinAttackRange(mTarget))
    {
        mAction = Being::STAND;
        //attack(mTarget, true);
        mGoingToTarget = false;
        mPath.clear();
        return;
    }
    else if (mGoingToTarget && !mTarget)
    {
        mGoingToTarget = false;
        mPath.clear();
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

void LocalPlayer::inviteToParty(const std::string &name)
{
    Net::ChatServer::Party::invitePlayer(name);
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

void LocalPlayer::moveInvItem(Item *item, int newIndex)
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
        if (Item *item = mInventory->getItem(i)) {
            if (item->getId() == itemId) {
                return item;
            }
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
    int dx = item->getX() - (int) getPosition().x / 32;
    int dy = item->getY() - (int) getPosition().y / 32;

    if (dx * dx + dy * dy < 4) {
        int id = item->getId();
        Net::GameServer::Player::pickUp(id >> 16, id & 0xFFFF);
        mPickUpTarget = NULL;
    } else {
        setDestination(item->getX() * 32 + 16, item->getY() * 32 + 16);
        mPickUpTarget = item;
    }
}

void LocalPlayer::walk(unsigned char dir)
{
    // TODO: Evaluate the implementation of this method
    if (!mMap || !dir)
        return;

    const Vector &pos = getPosition();
    int dScaler; // Distance to walk

    if (mAction == WALK && !mPath.empty())
    {
        // Just finish the current action, otherwise we get out of sync
        Being::setDestination(pos.x, pos.y);
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

    // Checks our path up to 5 tiles, if a blocking tile is found
    // We go to the last good tile, and break out of the loop
    for (dScaler = 1; dScaler <= 10; dScaler++)
    {
        if ( (dx || dy) &&
             !mMap->getWalk( ((int) pos.x + (dx * dScaler)) / 32,
                             ((int) pos.y + (dy * dScaler)) / 32, getWalkMask()) )
        {
            dScaler--;
            break;
        }
    }

    if (dScaler >= 0)
    {
         setDestination((int) pos.x + (dx * dScaler), (int) pos.y + (dy * dScaler));
    }
    else if (dir)
    {
        // If the being can't move, just change direction
        Net::GameServer::Player::changeDir(dir);
        setDirection(dir);
    }
}

void LocalPlayer::setDestination(Uint16 x, Uint16 y)
{
    // Fix coordinates so that the player does not seem to dig into walls.
    const int tx = x / 32;
    const int ty = y / 32;
    int fx = x % 32;
    int fy = y % 32;

    if (fx != 16 && !mMap->getWalk(tx + fx / 16 * 2 - 1, ty, getWalkMask()))
        fx = 16;
    if (fy != 16 && !mMap->getWalk(tx, ty + fy / 16 * 2 - 1, getWalkMask()))
        fy = 16;
    if (fx != 16 && fy != 16 && !mMap->getWalk(tx + fx / 16 * 2 - 1,
                                               ty + fy / 16 * 2 - 1,
                                               getWalkMask()))
        fx = 16;

    x = tx * 32 + fx;
    y = ty * 32 + fy;
    // Only send a new message to the server when destination changes
    if (x != mDestX || y != mDestY)
    {
        mDestX = x;
        mDestY = y;

        Net::GameServer::Player::walk(x, y);
        //Debugging fire burst
        effectManager->trigger(15,x,y);
    }

    mPickUpTarget = NULL;
    Being::setDestination(x, y);
}

void LocalPlayer::setWalkingDir(int dir)
{
    if (mWalkingDir != dir)
    {
        mWalkingDir = dir;
    }

    // If we're not already walking, start walking.
    if (mAction != WALK && dir
        && get_elapsed_time(mLocalWalkTime) >= walkingKeyboardDelay)
    {
        walk(dir);
    }
}

void LocalPlayer::stopWalking(bool sendToServer)
{
    if(mAction == WALK && mWalkingDir){
        mWalkingDir = 0;
        mLocalWalkTime = 0;
        Being::setDestination(getPosition().x,getPosition().y);
        if (sendToServer)
             Net::GameServer::Player::walk(getPosition().x, getPosition().y);
        setAction(STAND);
    }

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

    //Face direction of the target
    if(mTarget){
        unsigned char dir = 0;
        int x = 0, y = 0;
        Vector plaPos = this->getPosition();
        Vector tarPos = mTarget->getPosition();
        x = plaPos.x - tarPos.x;
        y = plaPos.y - tarPos.y;
        if(abs(x) < abs(y)){
            //Check to see if target is above me or below me
            if(y > 0){
               dir = UP;
            } else {
               dir = DOWN;
            }
        } else {
            //check to see if the target is to the left or right of me
            if(x > 0){
               dir = LEFT;
            } else {
               dir = RIGHT;
            }
        }
        setDirection(dir);
    }

    mLastAction = tick_time;

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

void LocalPlayer::useSpecial(int special)
{
    Net::GameServer::Player::useSpecial(special);
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

const struct LocalPlayer::SkillInfo& LocalPlayer::getSkillInfo(int skill)
{
    static const SkillInfo skills[CHAR_SKILL_NB + 1] =
    {
        { _("Unarmed"), "graphics/images/unarmed.png" },   // CHAR_SKILL_WEAPON_NONE
        { _("Knife"), "graphics/images/knife.png" },       // CHAR_SKILL_WEAPON_KNIFE
        { _("Sword"), "graphics/images/sword.png" },       // CHAR_SKILL_WEAPON_SWORD
        { _("Polearm"), "graphics/images/polearm.png" },   // CHAR_SKILL_WEAPON_POLEARM
        { _("Staff"), "graphics/images/staff.png" },       // CHAR_SKILL_WEAPON_STAFF
        { _("Whip"), "graphics/images/whip.png" },         // CHAR_SKILL_WEAPON_WHIP
        { _("Bow"), "graphics/images/bow.png" },           // CHAR_SKILL_WEAPON_BOW
        { _("Shooting"), "graphics/images/shooting.png" }, // CHAR_SKILL_WEAPON_SHOOTING
        { _("Mace"), "graphics/images/mace.png" },         // CHAR_SKILL_WEAPON_MACE
        { _("Axe"), "graphics/images/axe.png" },           // CHAR_SKILL_WEAPON_AXE
        { _("Thrown"), "graphics/images/thrown.png" },     // CHAR_SKILL_WEAPON_THROWN
        { _("Magic"), "graphics/images/magic.png" },       // CHAR_SKILL_MAGIC_IAMJUSTAPLACEHOLDER
        { _("Craft"), "graphics/images/craft.png" },       // CHAR_SKILL_CRAFT_IAMJUSTAPLACEHOLDER
        { _("Unknown Skill"), "graphics/images/unknown.png" }
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
        const std::string text = toString(diff) + " " + getSkillInfo(skill).name + " xp";
        mExpMessages.push_back(text);
    }

    mExpCurrent.at(skill) = current;
    mExpNext.at(skill) = next;
}

std::pair<int, int> LocalPlayer::getExperience(int skill)
{
    return std::pair<int, int> (mExpCurrent.at(skill), mExpNext.at(skill));
}

int LocalPlayer::getAttackRange()
{
    Item *weapon = mEquipment->getEquipment(EQUIP_FIGHT1_SLOT);
    if (weapon)
    {
        const ItemInfo info = weapon->getInfo();
        return info.getAttackRange();
    }
    return 32; // unarmed range
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
    mTarget = target;
    mGoingToTarget = true;
    const Vector &targetPos = target->getPosition();
    setDestination(targetPos.x, targetPos.y);
}
