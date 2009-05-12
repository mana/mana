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

#include "net/ea/skillhandler.h"

#include "net/ea/protocol.h"

#include "net/messagein.h"
#include "net/messageout.h"

#include "localplayer.h"
#include "log.h"

#include "gui/skill.h"

#include "gui/widgets/chattab.h"

#include "utils/gettext.h"

/** job dependend identifiers (?)  */
#define SKILL_BASIC       0x0001
#define SKILL_WARP        0x001b
#define SKILL_STEAL       0x0032
#define SKILL_ENVENOM     0x0034

/** basic skills identifiers       */
#define BSKILL_TRADE      0x0000
#define BSKILL_EMOTE      0x0001
#define BSKILL_SIT        0x0002
#define BSKILL_CREATECHAT 0x0003
#define BSKILL_JOINPARTY  0x0004
#define BSKILL_SHOUT      0x0005
#define BSKILL_PK         0x0006 // ??
#define BSKILL_SETALLIGN  0x0007 // ??

/** reasons why action failed      */
#define RFAIL_SKILLDEP    0x00
#define RFAIL_INSUFHP     0x01
#define RFAIL_INSUFSP     0x02
#define RFAIL_NOMEMO      0x03
#define RFAIL_SKILLDELAY  0x04
#define RFAIL_ZENY        0x05
#define RFAIL_WEAPON      0x06
#define RFAIL_REDGEM      0x07
#define RFAIL_BLUEGEM     0x08
#define RFAIL_OVERWEIGHT  0x09
#define RFAIL_GENERIC     0x0a

/** should always be zero if failed */
#define SKILL_FAILED      0x00

Net::SkillHandler *skillHandler;

namespace EAthena {

SkillHandler::SkillHandler()
{
    static const Uint16 _messages[] = {
        SMSG_PLAYER_SKILLS,
        SMSG_SKILL_FAILED,
        0
    };
    handledMessages = _messages;
    skillHandler = this;
}

void SkillHandler::handleMessage(MessageIn &msg)
{
    int skillCount;

    switch (msg.getId())
    {
        case SMSG_PLAYER_SKILLS:
            msg.readInt16();  // length
            skillCount = (msg.getLength() - 4) / 37;
            skillDialog->cleanList();

            for (int k = 0; k < skillCount; k++)
            {
                int skillId = msg.readInt16();
                msg.readInt16();  // target type
                msg.readInt16();  // unknown
                int level = msg.readInt16();
                int sp = msg.readInt16();
                msg.readInt16();  // range
                std::string skillName = msg.readString(24);
                int up = msg.readInt8();

                if (level != 0 || up != 0)
                {
                    if (skillDialog->hasSkill(skillId)) {
                        skillDialog->setSkill(skillId, level, sp);
                    }
                    else {
                        skillDialog->addSkill(skillId, level, sp);
                    }
                }
            }
            skillDialog->update();
            break;

        case SMSG_SKILL_FAILED:
            // Action failed (ex. sit because you have not reached the
            // right level)
            short skill   = msg.readInt16();
            short bskill  = msg.readInt16();
            msg.readInt16(); // unknown
            char success = msg.readInt8();
            char reason  = msg.readInt8();
            if (success != SKILL_FAILED && bskill == BSKILL_EMOTE)
            {
                logger->log("Action: %d/%d", bskill, success);
            }

            std::string msg;
            if (success == SKILL_FAILED && skill == SKILL_BASIC)
            {
                switch (bskill)
                {
                    case BSKILL_TRADE:
                        msg = _("Trade failed!");
                        break;
                    case BSKILL_EMOTE:
                        msg = _("Emote failed!");
                        break;
                    case BSKILL_SIT:
                        msg = _("Sit failed!");
                        break;
                    case BSKILL_CREATECHAT:
                        msg = _("Chat creating failed!");
                        break;
                    case BSKILL_JOINPARTY:
                        msg = _("Could not join party!");
                        break;
                    case BSKILL_SHOUT:
                        msg = _("Cannot shout!");
                        break;
                }

                msg += " ";

                switch (reason)
                {
                    case RFAIL_SKILLDEP:
                        msg += _("You have not yet reached a high enough lvl!");
                        break;
                    case RFAIL_INSUFHP:
                        msg += _("Insufficient HP!");
                        break;
                    case RFAIL_INSUFSP:
                        msg += _("Insufficient SP!");
                        break;
                    case RFAIL_NOMEMO:
                        msg += _("You have no memos!");
                        break;
                    case RFAIL_SKILLDELAY:
                        msg += _("You cannot do that right now!");
                        break;
                    case RFAIL_ZENY:
                        msg += _("Seems you need more money... ;-)");
                        break;
                    case RFAIL_WEAPON:
                        msg += _("You cannot use this skill with that kind of weapon!");
                        break;
                    case RFAIL_REDGEM:
                        msg += _("You need another red gem!");
                        break;
                    case RFAIL_BLUEGEM:
                        msg += _("You need another blue gem!");
                        break;
                    case RFAIL_OVERWEIGHT:
                        msg += _("You're carrying to much to do this!");
                        break;
                    default:
                        msg += _("Huh? What's that?");
                        break;
                }
            }
            else
            {
                switch (skill)
                {
                    case SKILL_WARP :
                        msg = _("Warp failed...");
                        break;
                    case SKILL_STEAL :
                        msg = _("Could not steal anything...");
                        break;
                    case SKILL_ENVENOM :
                        msg = _("Poison had no effect...");
                        break;
                }
            }

            localChatTab->chatLog(msg);
            break;
    }
}

void SkillHandler::up(int skillId)
{
    if (player_node->mSkillPoint <= 0)
        return;

    MessageOut outMsg(CMSG_SKILL_LEVELUP_REQUEST);
    outMsg.writeInt16(skillId);
}

void SkillHandler::use(int skillId, int level, int beingId)
{
    // TODO
}

void SkillHandler::use(int skillId, int level, int x, int y)
{
    // TODO
}

void SkillHandler::use(int skillId, const std::string &map)
{
    // TODO
}

} // namespace EAthena
