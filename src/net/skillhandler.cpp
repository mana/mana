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

#include "messagein.h"
#include "protocol.h"
#include "skillhandler.h"

#include "../log.h"

#include "../gui/chat.h"
#include "../gui/skill.h"

SkillHandler::SkillHandler()
{
    static const Uint16 _messages[] = {
        SMSG_PLAYER_SKILLS,
        SMSG_SKILL_FAILED,
        0
    };
    handledMessages = _messages;
}

void SkillHandler::handleMessage(MessageIn *msg)
{
    int skillCount;

    switch (msg->getId())
    {
        case SMSG_PLAYER_SKILLS:
            msg->readInt16();  // length
            skillCount = (msg->getLength() - 4) / 37;
            skillDialog->cleanList();

            for (int k = 0; k < skillCount; k++)
            {
                int skillId = msg->readInt16();
                msg->readInt16();  // target type
                msg->readInt16();  // unknown
                int level = msg->readInt16();
                int sp = msg->readInt16();
                msg->readInt16();  // range
                std::string skillName = msg->readString(24);
                int up = msg->readInt8();

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
            CHATSKILL action;
            action.skill   = msg->readInt16();
            action.bskill  = msg->readInt16();
            action.unused  = msg->readInt16(); // unknown
            action.success = msg->readInt8();
            action.reason  = msg->readInt8();
            if (action.success != SKILL_FAILED &&
                action.bskill == BSKILL_EMOTE)
            {
                logger->log("Action: %d/%d", action.bskill, action.success);
            }
            chatWindow->chatLog(action);
            break;
    }
}
