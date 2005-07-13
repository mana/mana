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
#include "chatinput.h"
#include "gui.h"
#include "../playerinfo.h"
#include "../net/network.h"
#include <iostream>
#include <guichan.hpp>

ChatWindow::ChatWindow(const std::string &logfile):
    Window("")
{
    chatlog_file.open(logfile.c_str(), std::ios::out | std::ios::app);
    items = 0;
    items_keep = 20;

    setContentSize(600, 100);
    setResizable(true);
    
    chatInput = new ChatInput();
    chatInput->setEventId("chatinput");
    chatInput->addActionListener(this);
    
    textOutput = new BrowserBox(BrowserBox::AUTO_WRAP);
    textOutput->setOpaque(false);
    textOutput->disableLinksAndUserColors();
    scrollArea = new ScrollArea(textOutput);
    scrollArea->setPosition(
            scrollArea->getBorderSize(), scrollArea->getBorderSize());
    scrollArea->setScrollPolicy(
            gcn::ScrollArea::SHOW_NEVER, gcn::ScrollArea::SHOW_ALWAYS);
    scrollArea->setOpaque(false);

    add(scrollArea);
    add(chatInput);

    // Add key listener to chat input to be able to respond to up/down
    chatInput->addKeyListener(this);
    curHist = history.end();
}

ChatWindow::~ChatWindow()
{
    delete chatInput;

    chatlog_file.flush();
    chatlog_file.close();
}

void ChatWindow::logic()
{
    chatInput->setPosition(
            chatInput->getBorderSize(),
            getContent()->getHeight() - chatInput->getHeight() -
                chatInput->getBorderSize());
    chatInput->setWidth(
            getContent()->getWidth() - 2 * chatInput->getBorderSize());

    scrollArea->setWidth(
            getContent()->getWidth() - 2 * scrollArea->getBorderSize());
    scrollArea->setHeight(
            getContent()->getHeight() - 2 * scrollArea->getBorderSize() -
                chatInput->getHeight() - 5);
    scrollArea->logic();
}

void ChatWindow::chat_log(std::string line, int own)
{
    // Delete overhead from the end of the list
    while ((int)chatlog.size() > items_keep) {
        chatlog.pop_back();
    }

    CHATLOG tmp;
    tmp.own  = own;
    tmp.nick = "";

    // Fix the owner of welcome message.
    if (line.substr(0, 7) == "Welcome")
    {
        own = BY_SERVER;
    }
    
    int pos = line.find(" : ");
    if (pos > 0) {
        tmp.nick = line.substr(0, pos);
        line.erase(0, pos + 3);
    }
        
    std::string lineColor = "##0"; // Equiv. to BrowserBox::BLACK
    switch (own) {
        case BY_GM:
            tmp.nick += std::string("Global announcement: ");
            lineColor = "##1"; // Equiv. to BrowserBox::RED
            break;
        case BY_PLAYER:
            tmp.nick += CAT_NORMAL;
            lineColor = "##2"; // Equiv. to BrowserBox::GREEN
            break;
        case BY_OTHER:
            tmp.nick += CAT_NORMAL;
            lineColor = "##4"; // Equiv. to BrowserBox::ORANGE
            break;
        case BY_SERVER:
            tmp.nick += std::string("Server: ");
            lineColor = "##7"; // Equiv. to BrowserBox::PINK
            break;
        case ACT_WHISPER:
            tmp.nick += CAT_WHISPER;
            lineColor = "##3"; // Equiv. to BrowserBox::BLUE
            break;
        case ACT_IS:
            tmp.nick += CAT_IS;
            lineColor = "##5"; // Equiv. to BrowserBox::YELLOW
            break;
    }
    
    line = lineColor + tmp.nick + line;

    textOutput->addRow(line);
    textOutput->draw(gui->getGraphics());
    scrollArea->setVerticalScrollAmount(scrollArea->getVerticalMaxScroll());
}

void ChatWindow::chat_log(CHATSKILL action)
{
    chat_log(const_msg(action), BY_SERVER);
}

void ChatWindow::action(const std::string& eventId)
{
    if (eventId == "chatinput")
    {
        std::string message = chatInput->getText();

        if (message.length() > 0) {
            // If message different from previous, put it in the history
            if (history.size() == 0 || message != history.back()) {
                history.push_back(message);
            }

            // Reset history iterator
            curHist = history.end();

            // Send the message to the server
            chat_send(char_info[0].name, message.c_str());

            // Clear the text from the chat input
            chatInput->setText("");
        }

        // Remove focus and hide input
        gui->focusNone();
    }
}

void ChatWindow::requestChatFocus()
{
    // Give focus to the chat input
    chatInput->setVisible(true);
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

void ChatWindow::keyPress(const gcn::Key &key)
{
    if (key.getValue() == key.DOWN && curHist != history.end())
    {
        // Move forward through the history
        std::list<std::string>::iterator prevHist = curHist++;
        if (curHist != history.end()) {
            chatInput->setText(*curHist);
            chatInput->setCaretPosition(chatInput->getText().length());
        }
        else {
            curHist = prevHist;
        }
    }
    else if (key.getValue() == key.UP && curHist != history.begin() &&
            history.size() > 0)
    {
        // Move backward through the history
        curHist--;
        chatInput->setText(*curHist);
        chatInput->setCaretPosition(chatInput->getText().length());
    }
}

void ChatWindow::setInputText(std::string input_str)
{
     chatInput->setText(input_str + " ");
     requestChatFocus();
}
