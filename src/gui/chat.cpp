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

#include "chat.h"
#include "../graphic/graphic.h"
#include <iostream>

ChatBox::ChatBox(const char *logfile, int item_num)
{
    chatlog_file.open(logfile, std::ios::out | std::ios::app);
    items = 0;
    items_keep = item_num;
}

ChatBox::~ChatBox()
{
    chatlog_file.flush();
    chatlog_file.close();
}

void ChatBox::chat_log(std::string line, int own)
{
    int pos;
    CHATLOG tmp;

    if (items <= items_keep) {
        items++;     // delete overhead from the end of the list
    }
    else {
        chatlog.pop_back();
    }

    pos = 0;
    pos = (int)line.find(" : ", 0);
    if (pos > 0) {
        tmp.nick = line.substr(0,pos);
        switch (own) {
            case ACT_IS :
                tmp.nick += CAT_IS;
                break;
            case ACT_WHISPER :
                tmp.nick += CAT_WHISPER;
                break;
            default :
                tmp.nick += CAT_NORMAL;
        }
        line.erase(0, pos + 3);
    } else {
        tmp.nick = "";
    }
    tmp.own  = own;
    tmp.text = line;

    chatlog_file << tmp.nick << tmp.text << "\n";
    chatlog_file.flush();

    chatlog.push_front(tmp);
}

void ChatBox::chat_log(CHATSKILL action) {
    chat_log(const_msg(action), BY_SERVER);
}


void ChatBox::draw(gcn::Graphics *graphics)
{
    int x, y;
    int n = 8;
    int texty = getHeight() - 5, i = 0;
    CHATLOG line;
    n -= 1;

    graphics->setColor(gcn::Color(203, 203, 203));
    graphics->drawLine(95, 5, 95, getHeight() - 5);
    graphics->drawRectangle(gcn::Rectangle(0, 0, getWidth(), getHeight()));

    getAbsolutePosition(x, y);
    
    SDL_Rect ScreenRect;
    ScreenRect.w = getWidth();
    ScreenRect.h = getHeight();
    ScreenRect.x = x;
    ScreenRect.y = y;
    Uint32 Color = SDL_MapRGBA(screen->format, 255, 255, 255, 120);
    SDL_FillRect(screen, &ScreenRect, Color);

    for (iter = chatlog.begin(); iter != chatlog.end(); iter++) {
        line = *iter;
        texty -= getFont()->getHeight() - 2;

        switch (line.own) {
            case BY_GM:
                graphics->setColor(gcn::Color(97, 156, 236)); // GM Bue
                graphics->drawText("Global announcement: ", 5, texty);
                graphics->setColor(gcn::Color(39, 197, 39)); // Green
                graphics->drawText(line.text, 100, texty);
                break;
            case BY_PLAYER:
                graphics->setColor(gcn::Color(255, 246, 98)); // Yellow
                graphics->drawText(line.nick, 5, texty);
                graphics->setColor(gcn::Color(255, 255, 255)); // White
                graphics->drawText(line.text, 100, texty);
                break;
            case BY_OTHER:
                graphics->setColor(gcn::Color(97, 156, 236)); // GM Bue
                graphics->drawText(line.nick, 5, texty);
                graphics->setColor(gcn::Color(39, 197, 39)); // Green
                graphics->drawText(line.text, 100, texty);
                break;
            default:
                graphics->setColor(gcn::Color(83, 233, 246)); // Light blue
                graphics->drawText(line.text, 5, texty);
        }

        if (i >= n) {
            return;
        }
        i++;
    }
}

char *ChatBox::chat_send(std::string nick, std::string msg)
{
    short packid = 0x008c;

    // prepare command
    if (msg.substr(0, 1) == "/") {
        // global announcement
        if(msg.substr(0, IS_ANNOUNCE_LENGTH) == IS_ANNOUNCE) {
            msg.erase(0, IS_ANNOUNCE_LENGTH);
            packid = 0x0099;
        }
        // prepare ordinary message
    }
    else {
        // temporary hack to make messed-up-keyboard-ppl able to send GM commands
        if (msg.substr(0, 1) == "#")
            msg.replace(0, 1, "@");
        // end temp. hack XD
        nick += " : ";
        nick += msg;
        msg = nick;
        packid = 0x008c;
    }

    msg += "\0";

    // send processed message
    WFIFOW(0) = net_w_value(packid);
    WFIFOW(2) = net_w_value((unsigned short)(msg.length()+4));
    memcpy(WFIFOP(4), msg.c_str(), msg.length());
    WFIFOSET((int)msg.length()+4);
    nick = msg = "";
    return "";
}

std::string ChatBox::const_msg(CHATSKILL action) {
    std::string msg;
    if (action.success == SKILL_FAILED && action.skill == SKILL_BASIC) {
        switch (action.bskill) {
            case BSKILL_TRADE :
                msg = "Trade failed!";
                break;
            case BSKILL_EMOTE :
                msg = "Emote failed!";
                break;
            case BSKILL_SIT :
                msg = "Sit failed!";
                break;
            case BSKILL_CREATECHAT :
                msg = "Chat creating failed!";
                break;
            case BSKILL_JOINPARTY :
                msg = "Could not join party!";
                break;
            case BSKILL_SHOUT :
                msg = "Cannot shout!";
                break;
        }

        switch (action.reason) {
            case RFAIL_SKILLDEP :
                msg += " You have not yet reached a high enough lvl!";
                break;
            case RFAIL_INSUFHP :
                msg += " Insufficient HP!";
                break;
            case RFAIL_INSUFSP :
                msg += " Insufficient SP!";
                break;
            case RFAIL_NOMEMO :
                msg += " You have no memos!";
                break;
            case RFAIL_SKILLDELAY :
                msg += " You cannot do that right now!";
                break;
            case RFAIL_ZENY :
                msg += " Seems you need more Zeny... ;-)";
                break;
            case RFAIL_WEAPON :
                msg += " You cannot use this skill with that kind of weapon!";
                break;
            case RFAIL_REDGEM :
                msg += " You need another red gem!";
                break;
            case RFAIL_BLUEGEM :
                msg += " You need another blue gem!";
                break;
            case RFAIL_OVERWEIGHT :
                msg += " You're carrying to much to do this!";
                break;
            default :
                msg += " Huh? What's that?";
                break;
        }
    } else {
        switch(action.skill) {
            case SKILL_WARP :
                msg = "Warp failed...";
                break;
            case SKILL_STEAL :
                msg = "Could not steal anything...";
                break;
            case SKILL_ENVENOM :
                msg = "Poison had no effect...";
                break;
        }
    }

    return msg;
}
