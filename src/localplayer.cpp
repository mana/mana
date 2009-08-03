/*
 *  The Mana World
 *  Copyright (C) 2004  The Mana World Development Team
 *
 *  This file is part of The Mana World.
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
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#include "localplayer.h"

#include "configuration.h"
#include "equipment.h"
#include "flooritem.h"
#include "game.h"
#include "graphics.h"
#include "inventory.h"
#include "item.h"
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

#include "net/inventoryhandler.h"
#include "net/net.h"
#include "net/partyhandler.h"
#include "net/playerhandler.h"
#include "net/specialhandler.h"
#include "net/tradehandler.h"

#include "effectmanager.h"
#ifdef TMWSERV_SUPPORT
#include "guild.h"

//#include "net/tmwserv/gameserver/player.h"
#include "net/tmwserv/chatserver/guild.h"
#endif

#include "resources/animation.h"
#include "resources/imageset.h"
#include "resources/iteminfo.h"
#include "resources/resourcemanager.h"

#include "utils/gettext.h"
#include "utils/stringutils.h"

#include <cassert>

#ifdef TMWSERV_SUPPORT
// This is shorter then it really needs to be for normal use
// But if we ever wanted to increase player speed, having this lower
// Won't hurt
const short walkingKeyboardDelay = 40;
#endif

LocalPlayer *player_node = NULL;

LocalPlayer::LocalPlayer(int id, int job, Map *map):
    Player(id, job, map),
#ifdef EATHENA_SUPPORT
    mAttackRange(0),
#endif
    mEquipment(new Equipment),
    mInStorage(false),
#ifdef EATHENA_SUPPORT
    mTargetTime(-1),
#endif
    mLastTarget(-1),
    mCharacterPoints(0),
    mCorrectionPoints(0),
    mLevel(1),
    mExp(0), mExpNeeded(0),
    mMp(0), mMaxMp(0),
    mMoney(0),
    mTotalWeight(1), mMaxWeight(1),
    mHp(1), mMaxHp(1),
    mSkillPoints(0),
    mTarget(NULL), mPickUpTarget(NULL),
    mTrading(false), mGoingToTarget(false), mKeepAttacking(false),
    mLastAction(-1),
    mWalkingDir(0),
    mDestX(0), mDestY(0),
    mInventory(new Inventory(INVENTORY_SIZE)),
#ifdef TMWSERV_SUPPORT
    mLocalWalkTime(-1),
#endif
    mStorage(new Inventory(STORAGE_SIZE)),
    mMessageTime(0)
{
    // Variable to keep the local player from doing certain actions before a map
    // is initialized. e.g. drawing a player's name using the TextManager, since
    // it appears to be dependant upon map coordinates for updating drawing.
    mMapInitialized = false;

    mUpdateName = true;

    initTargetCursor();
}

LocalPlayer::~LocalPlayer()
{
    delete mInventory;
#ifdef EATHENA_SUPPORT
    delete mStorage;
#endif

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

#ifdef EATHENA_SUPPORT
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
#endif

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
#ifdef TMWSERV_SUPPORT
            // Find whether target is in range
            const int rangeX = abs(mTarget->getPosition().x - getPosition().x);
            const int rangeY = abs(mTarget->getPosition().y - getPosition().y);
#else
            // Find whether target is in range
            const int rangeX = abs(mTarget->getTileX() - getTileX());
            const int rangeY = abs(mTarget->getTileY() - getTileY());
#endif
            const int attackRange = getAttackRange();
            const int inRange = rangeX > attackRange || rangeY > attackRange
                                                                    ? 1 : 0;
            mTarget->setTargetAnimation(
                mTargetCursor[inRange][mTarget->getTargetCursorSize()]);

            if (mTarget->mAction == DEAD)
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

void LocalPlayer::setName(const std::string &name)
{
    if (mName)
    {
        delete mName;
        mName = 0;
    }

    if (config.getValue("showownname", false) && mMapInitialized)
        Player::setName(name);
    else
        Being::setName(name);
}

void LocalPlayer::nextStep()
{
    // TODO: Fix picking up when reaching target (this method is obsolete)
    // TODO: Fix holding walking button to keep walking smoothly
    if (mPath.empty())
    {
        if (mPickUpTarget)
            pickUp(mPickUpTarget);

        if (mWalkingDir)
            walk(mWalkingDir);
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

#ifdef EATHENA_SUPPORT
    Player::nextStep();
#endif
}

#ifdef TMWSERV_SUPPORT
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

void LocalPlayer::clearInventory()
{
    mEquipment->clear();
    mInventory->clear();
}

void LocalPlayer::setInvItem(int index, int id, int amount)
{
    mInventory->setItem(index, id, amount);
}

#endif

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
#ifdef TMWSERV_SUPPORT
        setDestination(item->getX() * 32 + 16, item->getY() * 32 + 16);
#else
        setDestination(item->getX(), item->getY());
#endif
        mPickUpTarget = item;
#ifdef EATHENA_SUPPORT
        stopAttack();
#endif
    }
}

void LocalPlayer::walk(unsigned char dir)
{
    // TODO: Evaluate the implementation of this method for tmwserv
    if (!mMap || !dir)
        return;

#ifdef TMWSERV_SUPPORT
    const Vector &pos = getPosition();
#endif

    if (mAction == WALK && !mPath.empty())
    {
        // Just finish the current action, otherwise we get out of sync
#ifdef TMWSERV_SUPPORT
        Being::setDestination(pos.x, pos.y);
#else
        Being::setDestination(getTileX(), getTileY());
#endif
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

    // Prevent skipping corners over colliding tiles
#ifdef TMWSERV_SUPPORT
    if (dx && !mMap->getWalk(((int) pos.x + dx) / 32,
                             (int) pos.y / 32, getWalkMask()))
        dx = 16 - (int) pos.x % 32;
    if (dy && !mMap->getWalk((int) pos.x / 32,
                             ((int) pos.y + dy) / 32, getWalkMask()))
        dy = 16 - (int) pos.y % 32;
#else
    if (dx && !mMap->getWalk(getTileX() + dx, getTileY(), getWalkMask()))
        dx = 0;
    if (dy && !mMap->getWalk(getTileX(), getTileY() + dy, getWalkMask()))
        dy = 0;
#endif

    // Choose a straight direction when diagonal target is blocked
#ifdef TMWSERV_SUPPORT
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

    if (dScaler >= 0)
    {
        effectManager->trigger(15, (int) pos.x + (dx * dScaler), (int) pos.y + (dy * dScaler));
        setDestination((int) pos.x + (dx * dScaler), (int) pos.y + (dy * dScaler));
    }
#else
    if (dx && dy && !mMap->getWalk(getTileX() + dx, getTileY() + dy, getWalkMask()))
        dx = 0;

    // Walk to where the player can actually go
    if ((dx || dy) && mMap->getWalk(getTileX() + dx, getTileY() + dy, getWalkMask()))
    {
        setDestination(getTileX() + dx, getTileY() + dy);
    }
#endif
    else if (dir)
    {
        // If the being can't move, just change direction
        Net::getPlayerHandler()->setDirection(dir);
        setDirection(dir);
    }
}

Being *LocalPlayer::getTarget() const
{
    return mTarget;
}

void LocalPlayer::setTarget(Being *target)
{
#ifdef EATHENA_SUPPORT
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
#endif

    if (mTarget)
        mTarget->untarget();

    if (mTarget && mTarget->getType() == Being::MONSTER)
        static_cast<Monster *>(mTarget)->setShowName(false);

    mTarget = target;

    if (target && target->getType() == Being::MONSTER)
        static_cast<Monster *>(target)->setShowName(true);
}

#ifdef TMWSERV_SUPPORT
void LocalPlayer::setDestination(int x, int y)
#else
void LocalPlayer::setDestination(Uint16 x, Uint16 y)
#endif
{
#ifdef TMWSERV_SUPPORT
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
#endif

    // Only send a new message to the server when destination changes
    if (x != mDestX || y != mDestY)
    {
        mDestX = x;
        mDestY = y;

        Net::getPlayerHandler()->setDestination(x, y, mDirection);
    }

    mPickUpTarget = NULL;
    mKeepAttacking = false;

    Being::setDestination(x, y);
}

void LocalPlayer::setWalkingDir(int dir)
{
    mWalkingDir = dir;

    // If we're not already walking, start walking.
    if (mAction != WALK && dir
#ifdef TMWSERV_SUPPORT
        && get_elapsed_time(mLocalWalkTime) >= walkingKeyboardDelay
#endif
       )
    {
        walk(dir);
    }
}

void LocalPlayer::stopWalking(bool sendToServer)
{
    if (mAction == WALK && mWalkingDir) {
        mWalkingDir = 0;
#ifdef TMWSERV_SUPPORT
        mLocalWalkTime = 0;
#endif
        Being::setDestination(getPosition().x,getPosition().y);
        if (sendToServer)
             Net::getPlayerHandler()->setDestination(getPosition().x,
                                                     getPosition().y);
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

#ifdef TMWSERV_SUPPORT
/*
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
*/
#endif

void LocalPlayer::useSpecial(int special)
{
    Net::getSpecialHandler()->use(special);
}

void LocalPlayer::attack(Being *target, bool keep)
{
#ifdef TMWSERV_SUPPORT
    if (mLastAction != -1)
        return;

    // Can only attack when standing still
    if (mAction != STAND && mAction != ATTACK)
        return;
#endif

    mKeepAttacking = keep;

    if (!target || target->getType() == Being::NPC)
        return;

    if (mTarget != target || !mTarget)
    {
        mLastTarget = -1;
        setTarget(target);
    }
#ifdef TMWSERV_SUPPORT
    Vector plaPos = this->getPosition();
    Vector tarPos = mTarget->getPosition();
    int dist_x = plaPos.x - tarPos.x;
    int dist_y = plaPos.y - tarPos.y;
#else
    int dist_x = target->getTileX() - getTileX();
    int dist_y = target->getTileY() - getTileY();

    // Must be standing to attack
    if (mAction != STAND)
        return;
#endif

#ifdef TMWSERV_SUPPORT
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
#else
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
#endif

#ifdef TMWSERV_SUPPORT
    mLastAction = tick_time;
#else
    mWalkTime = tick_time;
    mTargetTime = tick_time;
#endif

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
#ifdef EATHENA_SUPPORT
    if (!keep)
        stopAttack();
#endif
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

void LocalPlayer::raiseAttribute(size_t attr)
{
    // we assume that the server allows the change. When not we will undo it later.
    mCharacterPoints--;
    IntMap::iterator it = mAttributeBase.find(attr);
    if (it != mAttributeBase.end())
        (*it).second++;
    Net::getPlayerHandler()->increaseAttribute(attr);
}

void LocalPlayer::lowerAttribute(size_t attr)
{
    // we assume that the server allows the change. When not we will undo it later.
    mCorrectionPoints--;
    mCharacterPoints++;
    IntMap::iterator it = mAttributeBase.find(attr);
    if (it != mAttributeBase.end())
        (*it).second--;
    Net::getPlayerHandler()->decreaseAttribute(attr);
}

void LocalPlayer::setAttributeBase(int num, int value)
{
    int old = mAttributeBase[num];

    mAttributeBase[num] = value;
    if (skillDialog)
    {
        if (skillDialog->update(num).empty() || !(value > old))
            return;

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

void LocalPlayer::setExperience(int skill, int current, int next)
{
    std::pair<int, int> cur = getExperience(skill);
    int diff = current - cur.first;

    cur = std::pair<int, int>(current, next);

    mSkillExp[skill] = cur;
    std::string name;
    if (skillDialog)
        name = skillDialog->update(skill);

    if (mMap && cur.first != -1 && diff > 0 && !name.empty())
    {
        addMessageToQueue(strprintf("%d %s xp", diff, name.c_str()));
    }
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

void LocalPlayer::setExp(int value)
{
    if (mMap && value > mExp)
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
        const std::string amountStr = (amount > 1) ? toString(amount) : _("a");

        if (config.getValue("showpickupchat", 1))
        {
            if (amount == 1)
            {
                // TRANSLATORS: Used as in "You picked up a [Candy]", when
                // picking up only one item.
                localChatTab->chatLog(strprintf(_("You picked up a [@@%d|%s@@]."),
                    itemInfo.getId(), itemInfo.getName().c_str()), BY_SERVER);
            }
            else
            {
                // TRANSLATORS: Used as in "You picked up 4 [Candy]", when
                // picking up more than one item.
                localChatTab->chatLog(strprintf(_("You picked up %d [@@%d|%s@@]."),
                    amount, itemInfo.getId(), itemInfo.getName().c_str()), BY_SERVER);
            }
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
#ifdef TMWSERV_SUPPORT
    Item *weapon = mEquipment->getEquipment(EQUIP_FIGHT1_SLOT);
    if (weapon)
    {
        const ItemInfo info = weapon->getInfo();
        return info.getAttackRange();
    }
    return 48; // unarmed range
#else
    return mAttackRange;
#endif
}

bool LocalPlayer::withinAttackRange(Being *target)
{
#ifdef TMWSERV_SUPPORT
    const Vector &targetPos = target->getPosition();
    const Vector &pos = getPosition();
    const int dx = abs(targetPos.x - pos.x);
    const int dy = abs(targetPos.y - pos.y);
    const int range = getAttackRange();

    return !(dx > range || dy > range);
#else
    int dist_x = abs(target->getTileX() - getTileY());
    int dist_y = abs(target->getTileY() - getTileX());

    if (dist_x > getAttackRange() || dist_y > getAttackRange())
    {
        return false;
    }

    return true;
#endif
}

void LocalPlayer::setGotoTarget(Being *target)
{
    mLastTarget = -1;
#ifdef TMWSERV_SUPPORT
    mTarget = target;
    mGoingToTarget = true;
    const Vector &targetPos = target->getPosition();
    setDestination(targetPos.x, targetPos.y);
#else
    setTarget(target);
    mGoingToTarget = true;
    setDestination(target->getTileX(), target->getTileY());
#endif
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

        if (!sprite) {
            // delete sprite, if necessary
            for (unsigned int i = 0; i < mStatusEffectIcons.size();)
                if (mStatusEffectIcons[i] == effectId) {
                    mStatusEffectIcons.erase(mStatusEffectIcons.begin() + i);
                    miniStatusWindow->eraseIcon(i);
                } else i++;
        } else {
            // replace sprite or append
            bool found = false;

            for (unsigned int i = 0; i < mStatusEffectIcons.size(); i++)
                if (mStatusEffectIcons[i] == effectId) {
                    miniStatusWindow->setIcon(i, sprite);
                    found = true;
                    break;
                }

            if (!found) { // add new
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

#ifdef EATHENA_SUPPORT
void LocalPlayer::setInStorage(bool inStorage)
{
    mInStorage = inStorage;

    storageWindow->setVisible(inStorage);
}
#endif

void LocalPlayer::addMessageToQueue(const std::string &message,
                                    Palette::ColorType color)
{
    mMessages.push_back(MessagePair(message, color));
}
