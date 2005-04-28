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
#include "textfield.h"
#include "textbox.h"
#include "../graphics.h"
#include "../main.h"
#include <iostream>

/** History */
#define HIST_LEN 10
static char *buf[HIST_LEN];
static int f,s,t;

ChatWindow::ChatWindow(const char *logfile, int item_num):
    Window("")
{
    chatlog_file.open(logfile, std::ios::out | std::ios::app);
    items = 0;
    items_keep = item_num;

    setContentSize(600, 100);
    textOutput = new TextBox();
    chatInput = new TextField();
    textOutput->setEditable(false);
    scrollArea = new ScrollArea(textOutput);
    scrollArea->setDimension(gcn::Rectangle(
                2, 0, 596, 98 - chatInput->getHeight() - 5));
    scrollArea->setOpaque(false);
    chatInput->setPosition(
            chatInput->getBorderSize(),
            100 - chatInput->getHeight() - chatInput->getBorderSize());
    chatInput->setWidth(600 - 2 * chatInput->getBorderSize());
    chatInput->setEventId("chatinput");
    chatInput->addActionListener(this);
    add(scrollArea);
    add(chatInput);
}

ChatWindow::~ChatWindow()
{
    delete chatInput;

    chatlog_file.flush();
    chatlog_file.close();
}

void ChatWindow::chat_log(std::string line, int own)
{
    int pos;
    CHATLOG tmp;

    // Delete overhead from the end of the list
    while (chatlog.size() > items_keep) {
        chatlog.pop_back();
    }

    pos = 0;
    pos = (int)line.find(" : ", 0);
    if (pos > 0) {
        tmp.nick = line.substr(0, pos);
        switch (own) {
            case ACT_IS:
                tmp.nick += CAT_IS;
                break;
            case ACT_WHISPER:
                tmp.nick += CAT_WHISPER;
                break;
            case BY_GM:
                tmp.nick += std::string("Global announcement: ");
            default:
                tmp.nick += CAT_NORMAL;
        }
        line.erase(0, pos + 3);
    } else {
        tmp.nick = "";
    }
    tmp.own  = own;

    line = tmp.nick + line;

    // A try to get text sentences no too long...
    bool finished = false;
    unsigned int maxLength = 98;

    while (!finished)
    {
        std::string tempText = line;

        if (line.length() > maxLength)
        {
            if (line.length() > maxLength) {
                line = cut_string(tempText, maxLength);
            }

            //tmp.text = tempText;

            //chatlog_file << tmp.nick << tmp.text << "\n";
            //chatlog_file.flush();

            //chatlog.push_front(tmp);
        }
        else // Normal message
        {
            //tmp.text = line;
            //chatlog_file << tmp.nick << tmp.text << "\n";
            //chatlog_file.flush();

            //chatlog.push_front(tmp);
            finished = true;
        }

        textOutput->setText(
                textOutput->getText() + std::string("\n") + tempText);
        scrollArea->setVerticalScrollAmount(
                scrollArea->getVerticalMaxScroll());
    }
}

void ChatWindow::chat_log(CHATSKILL action)
{
    chat_log(const_msg(action), BY_SERVER);
}


void ChatWindow::draw(gcn::Graphics *graphics)
{
    // Draw the window border/background and children
    Window::draw(graphics);

    // Draw the chat log
    /*
    int x, y;
    int n = 8;
    int texty = getHeight() - 5 - chatInput->getHeight() -
        2 * chatInput->getBorderSize();
    int i = 0;
    CHATLOG line;
    n -= 1;

    graphics->setColor(gcn::Color(203, 203, 203));
    graphics->drawLine(95, 5, 95, texty);

    getAbsolutePosition(x, y);

    for (iter = chatlog.begin(); iter != chatlog.end(); iter++)
    {
        line = *iter;

        texty -= getFont()->getHeight() - 2;

        switch (line.own) {
            case BY_GM:
                graphics->setColor(gcn::Color(97, 156, 236)); // GM Bue
                //graphics->drawText("Global announcement: ", 5, texty);
                addOutput(std::string("Global announcement: "));
                break;
            case BY_PLAYER:
                graphics->setColor(gcn::Color(255, 246, 98)); // Yellow
                break;
            case BY_OTHER:
                graphics->setColor(gcn::Color(97, 156, 236)); // GM Bue
                break;
        }

        switch (line.own) {
            case BY_GM:
                graphics->setColor(gcn::Color(39, 197, 39)); // Green
                break;
            case BY_PLAYER:
                graphics->setColor(gcn::Color(255, 255, 255)); // White
                break;
            case BY_OTHER:
                graphics->setColor(gcn::Color(39, 197, 39)); // Green
                break;
            default:
                graphics->setColor(gcn::Color(83, 233, 246)); // Light blue
        }

        if (i >= n) {
            return;
        }
        i++;
    }
    */
}

void ChatWindow::action(const std::string& eventId)
{
    if (eventId == "chatinput") {
        std::string message = chatInput->getText();

        if (message.length() > 0) {
            chat_send(char_info[0].name, message.c_str());
            chatInput->setText("");
        }
        gui->focusNone();

    }
}

void ChatWindow::requestFocus()
{
    // Give focus to the chat input
    chatInput->requestFocus();
}

bool ChatWindow::isFocused()
{
    return chatInput->hasFocus();
}

char *ChatWindow::chat_send(std::string nick, std::string msg)
{
    short packid = 0x008c;

    // prepare command
    if (msg.substr(0, 1) == "/") {
        // global announcement
        /*if(msg.substr(0, IS_ANNOUNCE_LENGTH) == IS_ANNOUNCE) {
            msg.erase(0, IS_ANNOUNCE_LENGTH);
            packid = 0x0099;
        }*/
        // prepare ordinary message
        chat_log("Sorry but /commands are not available yet", BY_PLAYER);
        return "";
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

std::string ChatWindow::const_msg(CHATSKILL action)
{
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

std::string ChatWindow::cut_string(std::string& value, unsigned int maximumLength)
{
    // If the string exceeds the maximum length
    if(value.length() > maximumLength)
    {
        unsigned int index = 0;
        unsigned int lastSpace = 0;
        std::string  cutOff = "";

        for(index = 0; index < maximumLength; index++) {
            if(value.at(index) == ' ') {
                lastSpace = index;
            }
        }

        // If the last space is at the beginning of the string
        if(lastSpace == 0) {
            // Just cut it right off from the end
            cutOff = value.substr(maximumLength);
            value  = value.substr(0, maximumLength);
        } else {
            // Cut it off from the last space forward
            cutOff = value.substr(lastSpace + 1);
            value  = value.substr(0, lastSpace);
        }

        return cutOff;
    }

    return std::string("");
}

void ChatWindow::update_history(const char *ptr)
{
	f = t;
	if(*ptr == 0) return;
	
	// prevent duplicates
	if(f != s && strcmp(ptr, buf[(f + HIST_LEN -1) % HIST_LEN]) == 0) return;

	buf[f] = strdup(ptr);
	f = ( f + 1) % HIST_LEN;

	if(f == s) {
		free(buf[f]);
		buf[s] = 0;
		s = (s + 1) % HIST_LEN;
	}

	t = f;
}

void ChatWindow::arrow_up(void)
{
	const char *ptr;

	ptr = chatInput->getText().c_str();
	
	if(*ptr) {
		if(t == f || strcmp(ptr, buf[t]) != 0) {
			update_history(ptr);
			t = (f + HIST_LEN -1) % HIST_LEN;
		}
	}

	if(t != s)
		t = (t + HIST_LEN -1) % HIST_LEN;
	if(buf[t])
		update_history(buf[t]);
	else
		update_history("");
}

void ChatWindow::arrow_down(void)
{
	const char *ptr;

	ptr = chatInput->getText().c_str();

	if(*ptr) {
		if(t == f || strcmp(ptr, buf[t]) != 0) {
			update_history(ptr);
			t = (f + HIST_LEN -1) % HIST_LEN;
		}
	}

	if(t != f)
		t = (t + 1) % HIST_LEN;
	
	if(buf[t])
		update_history(buf[t]);
	else
		update_history("");
}

void ChatWindow::keyPress(const gcn::Key& key) 
{
	if(key.getValue() == key.DOWN)
		arrow_down();
	else if(key.getValue() == key.UP) 
		arrow_up();

	chatInput->setText(std::string(buf[t]));
}
