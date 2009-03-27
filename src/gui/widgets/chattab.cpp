/*
 *  The Mana World
 *  Copyright (C) 2008  The Mana World Development Team
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

#include "chattab.h"

#include "commandhandler.h"
#include "configuration.h"
#include "localplayer.h"

#include "gui/browserbox.h"
#include "gui/itemlinkhandler.h"
#include "gui/recorder.h"
#include "gui/scrollarea.h"

#ifdef TMWSERV_SUPPORT
#include "net/tmwserv/chatserver/chatserver.h"
#include "net/tmwserv/gameserver/player.h"
#else
#include "net/messageout.h"
#include "net/ea/protocol.h"
#endif

#include "resources/iteminfo.h"
#include "resources/itemdb.h"

#include "utils/gettext.h"
#include "utils/strprintf.h"
#include "utils/stringutils.h"

ChatTab::ChatTab(const std::string &name) : Tab()
{
    setCaption(name);

    mTextOutput = new BrowserBox;
    mTextOutput->setOpaque(false);
    mTextOutput->setMaxRow((int) config.getValue("ChatLogLength", 0));
    mTextOutput->setLinkHandler(chatWindow->mItemLinkHandler);

    mScrollArea = new ScrollArea(mTextOutput);
    mScrollArea->setScrollPolicy(gcn::ScrollArea::SHOW_NEVER,
                                 gcn::ScrollArea::SHOW_ALWAYS);
    mScrollArea->setScrollAmount(0, 1);
    mScrollArea->setOpaque(false);

    chatWindow->addTab(this);
}

ChatTab::~ChatTab()
{
    chatWindow->removeTab(this);
    delete mTextOutput;
    delete mScrollArea;
}

void ChatTab::chatLog(const char* line, int own, bool ignoreRecord)
{
    chatLog(std::string(line), own, ignoreRecord);
}

void ChatTab::chatLog(std::string line, int own, bool ignoreRecord)
{
    // Trim whitespace
    trim(line);

    if (line.empty())
        return;

    CHATLOG tmp;
    tmp.own = own;
    tmp.nick = "";
    tmp.text = line;

    std::string::size_type pos = line.find(" : ");
    if (pos != std::string::npos)
    {
        tmp.nick = line.substr(0, pos);
        tmp.text = line.substr(pos + 3);
    }
    else
    {
        // Fix the owner of welcome message.
        if (line.substr(0, 7) == "Welcome")
        {
            own = BY_SERVER;
        }
    }

    // *implements actions in a backwards compatible way*
    if (own == BY_PLAYER &&
        tmp.text.at(0) == '*' &&
        tmp.text.at(tmp.text.length()-1) == '*')
    {
        tmp.text[0] = ' ';
        tmp.text.erase(tmp.text.length() - 1);
        own = ACT_IS;
    }

    std::string lineColor = "##C";
    switch (own)
    {
        case BY_GM:
            if (tmp.nick.empty())
            {
                tmp.nick = std::string(_("Global announcement:"));
                tmp.nick += " ";
                lineColor = "##G";
            }
            else
            {
                tmp.nick = strprintf(_("Global announcement from %s:"),
                                     tmp.nick.c_str());
                tmp.nick += " ";
                lineColor = "##1"; // Equiv. to BrowserBox::RED
            }
            break;
        case BY_PLAYER:
            tmp.nick += CAT_NORMAL;
            lineColor = "##Y";
            break;
        case BY_OTHER:
            tmp.nick += CAT_NORMAL;
            lineColor = "##C";
            break;
        case BY_SERVER:
            tmp.nick = _("Server:");
            tmp.nick += " ";
            tmp.text = line;
            lineColor = "##S";
            break;
        case BY_CHANNEL:
            tmp.nick = "";
            // TODO: Use a predefined color
            lineColor = "##2"; // Equiv. to BrowserBox::GREEN
            break;
#ifdef EATHENA_SUPPORT
        case BY_PARTY:
            tmp.nick += CAT_NORMAL;
            lineColor = "##P";
            break;
#endif
        case ACT_WHISPER:
            // Resend whisper through normal mechanism
            chatWindow->whisper(tmp.nick, tmp.text);
            return;
        case ACT_IS:
            tmp.nick += CAT_IS;
            lineColor = "##I";
            break;
        case BY_LOGGER:
            tmp.nick = "";
            tmp.text = line;
            lineColor = "##L";
            break;
    }

    if (tmp.nick == ": ")
    {
        tmp.nick = "";
        lineColor = "##S";
    }

#ifdef EATHENA_SUPPORT
    if (tmp.nick.empty() && tmp.text.substr(0, 17) == "Visible GM status")
    {
        player_node->setGM();
    }
#endif

    // Get the current system time
    time_t t;
    time(&t);

    // Format the time string properly
    std::stringstream timeStr;
    timeStr << "[" << ((((t / 60) / 60) % 24 < 10) ? "0" : "")
        << (int) (((t / 60) / 60) % 24)
        << ":" << (((t / 60) % 60 < 10) ? "0" : "")
        << (int) ((t / 60) % 60)
        << "] ";

    line = lineColor + timeStr.str() + tmp.nick + tmp.text;

    // We look if the Vertical Scroll Bar is set at the max before
    // adding a row, otherwise the max will always be a row higher
    // at comparison.
    if (mScrollArea->getVerticalScrollAmount() >= mScrollArea->getVerticalMaxScroll())
    {
        mTextOutput->addRow(line);
        mScrollArea->setVerticalScrollAmount(mScrollArea->getVerticalMaxScroll());
    }
    else
    {
        mTextOutput->addRow(line);
    }

    mScrollArea->logic();
    chatWindow->mRecorder->record(line.substr(3));
}

void ChatTab::chatLog(std::string &nick, std::string &msg)
{
    chatLog(nick + CAT_NORMAL + msg, nick == player_node->getName() ?
                BY_PLAYER : BY_OTHER, false);
}

void ChatTab::chatInput(std::string &msg)
{
    trim(msg);

    if (msg.empty()) return;

#ifdef EATHENA_SUPPORT
    // Send party message
    if (msg.at(0) == chatWindow->mPartyPrefix)
    {
        msg.erase(0, 1);
        std::size_t length = msg.length() + 1;

        if (length == 0)
        {
            chatLog(_("Trying to send a blank party message."), BY_SERVER, true);
            return;
        }
        MessageOut outMsg(CMSG_PARTY_MESSAGE);
        outMsg.writeInt16(length + 4);
        outMsg.writeString(msg, length);
        return;
    }
#endif

    // Check for item link
    std::string::size_type start = msg.find('[');
    while (start != std::string::npos && msg[start+1] != '@')
    {
        std::string::size_type end = msg.find(']', start);
        if (start+1 != end && end != std::string::npos)
        {
            // Catch multiple embeds and ignore them
            // so it doesn't crash the client.
            while ((msg.find('[', start + 1) != std::string::npos) &&
                   (msg.find('[', start + 1) < end))
            {
                start = msg.find('[', start + 1);
            }

            std::string temp = msg.substr(start + 1, end - start - 1);

            toLower(trim(temp));

            const ItemInfo itemInfo = ItemDB::get(temp);
            if (itemInfo.getName() != _("Unknown item"))
            {
                msg.insert(end, "@@");
                msg.insert(start+1, "|");
                msg.insert(start+1, toString(itemInfo.getId()));
                msg.insert(start+1, "@@");
            }
        }
        start =  msg.find('[', start + 1);
    }

    // Prepare ordinary message
    if (msg[0] != '/')
        handleInput(msg);
    else
        handleCommand(std::string(msg, 1));
}

void ChatTab::scroll(int amount)
{
    int range = mScrollArea->getHeight() / 8 * amount;
    gcn::Rectangle scr;
    scr.y = mScrollArea->getVerticalScrollAmount() + range;
    scr.height = abs(range);
    mTextOutput->showPart(scr);
}

void ChatTab::clearText()
{
    mTextOutput->clearRows();
}

void ChatTab::handleInput(const std::string &msg) {
#ifdef TMWSERV_SUPPORT
    Net::GameServer::Player::say(msg);
#else
    std::string mes = player_node->getName() + " : " + msg;

    MessageOut outMsg(CMSG_CHAT_MESSAGE);
    // Added + 1 in order to let eAthena parse admin commands correctly
    outMsg.writeInt16(mes.length() + 4 + 1);
    outMsg.writeString(mes, mes.length() + 1);
    return;
#endif
}

void ChatTab::handleCommand(std::string msg)
{
    commandHandler->handleCommand(msg);
}
