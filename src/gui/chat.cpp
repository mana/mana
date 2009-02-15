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

#include <guichan/focushandler.hpp>

#include "browserbox.h"
#include "chat.h"
#include "chatinput.h"
#include "itemlinkhandler.h"
#include "recorder.h"
#include "scrollarea.h"
#include "sdlinput.h"
#include "windowcontainer.h"

#include "widgets/layout.h"

#include "../beingmanager.h"
#include "../configuration.h"
#include "../game.h"
#include "../localplayer.h"
#include "../party.h"

#include "../net/messageout.h"
#include "../net/protocol.h"

#include "../resources/iteminfo.h"
#include "../resources/itemdb.h"

#include "../utils/gettext.h"
#include "../utils/strprintf.h"
#include "../utils/tostring.h"
#include "../utils/trim.h"

ChatWindow::ChatWindow(Network * network):
Window(""), mNetwork(network), mTmpVisible(false)
{
    setWindowName("Chat");

    setResizable(true);
    setDefaultSize(0, windowContainer->getHeight() - 123, 600, 123);
    setMinWidth(150);
    setMinHeight(90);

    mItemLinkHandler = new ItemLinkHandler;

    mChatInput = new ChatInput;
    mChatInput->setActionEventId("chatinput");
    mChatInput->addActionListener(this);

    mTextOutput = new BrowserBox(BrowserBox::AUTO_WRAP);
    mTextOutput->setOpaque(false);
    mTextOutput->setMaxRow((int) config.getValue("ChatLogLength", 0));
    mTextOutput->setLinkHandler(mItemLinkHandler);

    mScrollArea = new ScrollArea(mTextOutput);
    mScrollArea->setScrollPolicy(gcn::ScrollArea::SHOW_NEVER,
                                 gcn::ScrollArea::SHOW_ALWAYS);
    mScrollArea->setScrollAmount(0, 1);
    mScrollArea->setOpaque(false);

    place(0, 0, mScrollArea, 5, 5).setPadding(0);
    place(0, 5, mChatInput, 5).setPadding(1);

    Layout &layout = getLayout();
    layout.setRowHeight(0, Layout::AUTO_SET);
    layout.setMargin(2);

    loadWindowState();

    // Add key listener to chat input to be able to respond to up/down
    mChatInput->addKeyListener(this);
    mCurHist = mHistory.end();

    // Read the party prefix
    std::string partyPrefix = config.getValue("PartyPrefix", "$");
    mPartyPrefix = (partyPrefix.empty() ? '$' : partyPrefix.at(0));
    mReturnToggles = config.getValue("ReturnToggles", "0") == "1";
    mRecorder = new Recorder(this);
    mParty = new Party(this, mNetwork);

    // If the player had @assert on in the last session, ask the server to
    // run the @assert command for the player again. Convenience for GMs.
    if (config.getValue(player_node->getName() + "GMassert", 0))
        chatSend(player_node->getName(), "@assert");
}

ChatWindow::~ChatWindow()
{
    char partyPrefix[2] = ".";
    *partyPrefix = mPartyPrefix;
    config.setValue("PartyPrefix", partyPrefix);
    config.setValue("ReturnToggles", mReturnToggles ? "1" : "0");
    delete mRecorder;
}

void ChatWindow::chatLog(std::string line, int own, bool ignoreRecord)
{
    // Trim whitespace
    trim(line);

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
        case BY_PARTY:
            tmp.nick += CAT_NORMAL;
            lineColor = "##P";
            break;
        case ACT_WHISPER:
            tmp.nick = strprintf(_("%s whispers:"), tmp.nick.c_str());
            tmp.nick += " ";
            lineColor = "##W";
            break;
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

    if (tmp.nick.empty() && tmp.text.substr(0, 17) == "Visible GM status")
    {
        player_node->setGM();
    }

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
    if (mScrollArea->getVerticalScrollAmount() ==
        mScrollArea->getVerticalMaxScroll())
    {
        mTextOutput->addRow(line);
        mScrollArea->setVerticalScrollAmount(mScrollArea->
                                             getVerticalMaxScroll());
    }
    else
    {
        mTextOutput->addRow(line);
    }

    mRecorder->record(line.substr(3));
}

void ChatWindow::chatLog(CHATSKILL act)
{
    chatLog(const_msg(act), BY_SERVER);
}

void ChatWindow::action(const gcn::ActionEvent &event)
{
    if (event.getId() == "chatinput")
    {
        std::string message = mChatInput->getText();

        if (!message.empty())
        {
            // If message different from previous, put it in the history
            if (mHistory.empty() || message != mHistory.back())
            {
                mHistory.push_back(message);
            }
            // Reset history iterator
            mCurHist = mHistory.end();

            // Send the message to the server
            chatSend(player_node->getName(), message);

            // Clear the text from the chat input
            mChatInput->setText("");
        }

        if (message.empty() || !mReturnToggles)
        {
            // Remove focus and hide input
            mFocusHandler->focusNone();

            // If the chatWindow is shown up because you want to send a message
            // It should hide now
            if (mTmpVisible)
                setVisible(false);
        }
    }
}

void ChatWindow::requestChatFocus()
{
    // Make sure chatWindow is visible
    if (!isVisible())
    {
        setVisible(true);

        /*
         * This is used to hide chatWindow after sending the message. There is
         * a trick here, because setVisible will set mTmpVisible to false, you
         * have to put this sentence *after* setVisible, not before it
         */
        mTmpVisible = true;
    }

    // Give focus to the chat input
    mChatInput->setVisible(true);
    mChatInput->requestFocus();
}

bool ChatWindow::isInputFocused()
{
    return mChatInput->isFocused();
}

void ChatWindow::whisper(const std::string &nick, std::string msg)
{
    std::string recvnick = "";

    if (msg.substr(0, 1) == "\"")
    {
        const std::string::size_type pos = msg.find('"', 1);
        if (pos != std::string::npos)
        {
            recvnick = msg.substr(1, pos - 1);
            msg.erase(0, pos + 2);
        }
    }
    else
    {
        const std::string::size_type pos = msg.find(" ");
        if (pos != std::string::npos)
        {
            recvnick = msg.substr(0, pos);
            msg.erase(0, pos + 1);
        }
    }

    MessageOut outMsg(mNetwork);
    outMsg.writeInt16(CMSG_CHAT_WHISPER);
    outMsg.writeInt16(msg.length() + 28);
    outMsg.writeString(recvnick, 24);
    outMsg.writeString(msg, msg.length());

    chatLog(strprintf(_("Whispering to %s: %s"),
                        recvnick.c_str(), msg.c_str()),
                        BY_PLAYER);
}

void ChatWindow::chatSend(const std::string &nick, std::string msg)
{
    /* Some messages are managed client side, while others
     * require server handling by proper packet. Probably
     * those if elses should be replaced by protocol calls */

    trim(msg);

    if (msg.compare("") == 0)
        return;

    // Send party message
    if (msg.at(0) == mPartyPrefix)
    {
        msg.erase(0, 1);
        std::size_t length = msg.length() + 1;

        if (length == 0)
        {
            chatLog(_("Trying to send a blank party message."), BY_SERVER);
            return;
        }
        MessageOut outMsg(mNetwork);

        outMsg.writeInt16(CMSG_PARTY_MESSAGE);
        outMsg.writeInt16(length + 4);
        outMsg.writeString(msg, length);
        return;
    }

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

            std::string temp = msg.substr(start+1, end - start - 1);

            trim(temp);

            for (unsigned int i = 0; i < temp.size(); i++)
            {
                temp[i] = (char) tolower(temp[i]);
            }

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
    if (msg.substr(0, 1) != "/")
    {
        msg = nick + " : " + msg;

        MessageOut outMsg(mNetwork);
        outMsg.writeInt16(CMSG_CHAT_MESSAGE);
        // Added + 1 in order to let eAthena parse admin commands correctly
        outMsg.writeInt16(msg.length() + 4 + 1);
        outMsg.writeString(msg, msg.length() + 1);
        return;
    }

    msg.erase(0, 1);
    trim(msg);

    std::size_t space = msg.find(" ");
    std::string command = msg.substr(0, space);

    if (space == std::string::npos)
    {
        msg = "";
    }
    else
    {
        msg = msg.substr(space);
        trim(msg);
    }

    if (command == "announce")
    {
        MessageOut outMsg(mNetwork);
        outMsg.writeInt16(0x0099);
        outMsg.writeInt16(msg.length() + 4);
        outMsg.writeString(msg, msg.length());
    }
    else if (command == "help")
    {
        trim(msg);
        std::size_t space = msg.find(" ");
        std::string msg1;

        if (space == std::string::npos)
        {
            msg1 = "";
        }
        else
        {
            msg1 = msg.substr(space + 1, msg.length());
            msg = msg.substr(0, space);
        }

        if (!msg.empty() && msg.at(0) == '/')
        {
            msg.erase(0, 1);
        }

        trim(msg1);
        help(msg, msg1);
    }
    else if (command == "where")
    {
        // Display the current map, X, and Y
        std::ostringstream where;
        where << map_path << " " << player_node->mX << "," << player_node->mY;
        chatLog(where.str(), BY_SERVER);
    }
    else if (command == "who")
    {
        MessageOut outMsg(mNetwork);
        outMsg.writeInt16(0x00c1);
    }
    else if (command == "clear")
        mTextOutput->clearRows();
    else if (command == "whisper" || command == "msg" || command == "w")
        whisper(nick, msg);
    else if (command == "record")
        mRecorder->changeRecordingStatus(msg);
    else if (command == "toggle")
    {
        if (msg.empty())
        {
            chatLog(mReturnToggles ? _("Return toggles chat.")
                    : _("Message closes chat."), BY_SERVER);
            return;
        }

        msg = msg.substr(0, 1);

        if (msg == "1" ||
            msg == "y" || msg == "Y" ||
            msg == "t" || msg == "T")
        {
            chatLog(_("Return now toggles chat."), BY_SERVER);
            mReturnToggles = true;
            return;
        }
        else if (msg == "0" ||
                 msg == "n" || msg == "N" ||
                 msg == "f" || msg == "F")
        {
            chatLog(_("Message now closes chat."), BY_SERVER);
            mReturnToggles = false;
            return;
        }
        else
            chatLog(_("Options to /toggle are \"yes\", \"no\", \"true\", "
                      "\"false\", \"1\", \"0\"."), BY_SERVER);
    }
    else if (command == "party")
    {
        if (msg.empty())
        {
            chatLog(_("Unknown party command... Type \"/help\" party for more "
                    "information."), BY_SERVER);
            return;
        }

        const std::string::size_type space = msg.find(" ");
        std::string rest = (space == std::string::npos ? ""
                            : msg.substr(space + 1, msg.length()));

        if (!rest.empty())
        {
            msg = msg.substr(0, space);
            trim(msg);
        }

        party(msg, rest);
        return;
    }
    else if (command == "cast")
    {
        /*
         * This will eventually be replaced by a GUI, so
         * we don't need to get too sophisticated
         */
        MessageOut outMsg(mNetwork);
        if (msg == "heal")
        {
            outMsg.writeInt16(0x03f3);
            outMsg.writeInt16(0x01);
            outMsg.writeInt32(0);
            outMsg.writeInt8(0);
            outMsg.writeInt8(0);
            outMsg.writeString("", 24);
        }
        else if (msg == "gather")
        {
            outMsg.writeInt16(0x03f3);
            outMsg.writeInt16(0x02);
            outMsg.writeInt32(0);
            outMsg.writeInt8(0);
            outMsg.writeInt8(0);
            outMsg.writeString("", 24);
        }
        else
            chatLog(_("No such spell!"), BY_SERVER);
    }
    else if (command == "present")
    {
        Beings & beings = beingManager->getAll();
        std::string response = "";

        for (BeingIterator bi = beings.begin(), be = beings.end();
             bi != be; ++bi)
        {
            if ((*bi)->getType() == Being::PLAYER)
            {
                if (!response.empty())
                {
                    response += ", ";
                }
                response += (*bi)->getName();
            }
        }

        if (mRecorder->isRecording())
        {
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


            mRecorder->record(timeStr.str() + _("Present: ") + response + ".");
            chatLog(_("Attendance written to record log."), BY_SERVER, true);
        }
        else
        {
            chatLog(_("Present: ") + response, BY_SERVER);
        }
    }
    else if (command == "me")
    {
	std::stringstream actionStr;
	actionStr << "*" << msg << "*";
	chatSend(player_node->getName(), actionStr.str());
    }
    else
    {
        chatLog(_("Unknown command"), BY_SERVER);
    }
}

std::string ChatWindow::const_msg(CHATSKILL act)
{
    std::string msg;
    if (act.success == SKILL_FAILED && act.skill == SKILL_BASIC)
    {
        switch (act.bskill)
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

        switch (act.reason)
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
        switch (act.skill)
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

    return msg;
}

void ChatWindow::scroll(int amount)
{
    if (!isVisible())
        return;

    int range = mScrollArea->getHeight() / 8 * amount;
    gcn::Rectangle scr;
    scr.y = mScrollArea->getVerticalScrollAmount() + range;
    scr.height = abs(range);
    mTextOutput->showPart(scr);
}

void ChatWindow::keyPressed(gcn::KeyEvent &event)
{
    if (event.getKey().getValue() == Key::DOWN &&
            mCurHist != mHistory.end())
    {
        // Move forward through the history
        HistoryIterator prevHist = mCurHist++;

        if (mCurHist != mHistory.end())
        {
            mChatInput->setText(*mCurHist);
            mChatInput->setCaretPosition(mChatInput->getText().length());
        }
        else
        {
            mCurHist = prevHist;
        }
    }
    else if (event.getKey().getValue() == Key::UP &&
            mCurHist != mHistory.begin() && mHistory.size() > 0)
    {
        // Move backward through the history
        mCurHist--;
        mChatInput->setText(*mCurHist);
        mChatInput->setCaretPosition(mChatInput->getText().length());
    }
}

void ChatWindow::setInputText(std::string input_str)
{
     mChatInput->setText(mChatInput->getText() + input_str + " ");
     requestChatFocus();
}

void ChatWindow::addItemText(const std::string &item)
{
    std::ostringstream text;
    text << "[" << item << "] ";
    mChatInput->setText(mChatInput->getText() + text.str());
    requestChatFocus();
}

void ChatWindow::setVisible(bool isVisible)
{
    Window::setVisible(isVisible);

    /*
     * For whatever reason, if setVisible is called, the mTmpVisible effect
     * should be disabled.
     */

    mTmpVisible = false;
}

void ChatWindow::party(const std::string & command, const std::string & rest)
{
    if (command == "prefix")
    {
        if (rest.empty())
        {
            char temp[2] = ".";
            *temp = mPartyPrefix;
            chatLog(_("The current party prefix is ") + std::string(temp),
                    BY_SERVER);
        }
        else if (rest.length() != 1)
        {
            chatLog(_("Party prefix must be one character long."), BY_SERVER);
        }
        else
        {
            if (rest == "/")
            {
                chatLog(_("Cannot use a '/' as the prefix."), BY_SERVER);
            }
            else
            {
                mPartyPrefix = rest.at(0);
                chatLog(_("Changing prefix to ") + rest, BY_SERVER);
            }
        }
    }
    else
        mParty->respond(command, rest);
}

void ChatWindow::help(const std::string & msg1, const std::string & msg2)
{
    chatLog(_("-- Help --"), BY_SERVER);
    if (msg1.empty())
    {
        chatLog(_("/announce: Global announcement (GM only)"), BY_SERVER);
        chatLog(_("/clear: Clears this window"), BY_SERVER);
        chatLog(_("/help: Display this help"), BY_SERVER);
        chatLog(_("/me <message>: Tell something about yourself"), BY_SERVER);
        chatLog(_("/msg <nick> <message>: Alternate form for /whisper"),
                BY_SERVER);
        chatLog(_("/party <command> <params>: Party commands"), BY_SERVER);
        chatLog(_("/present: Get list of players present"), BY_SERVER);
        chatLog(_("/record <filename>: Start recording the chat to an "
                  "external file"), BY_SERVER);
        chatLog(_("/toggle: Determine whether <return> toggles the chat log"),
                BY_SERVER);
        chatLog(_("/w <nick> <message>: Short form for /whisper"), BY_SERVER);
        chatLog(_("/where: Display map name"), BY_SERVER);
        chatLog(_("/whisper <nick> <message>: Sends a private <message> "
                    "to <nick>"), BY_SERVER);
        chatLog(_("/who: Display number of online users"), BY_SERVER);
        chatLog(_("For more information, type /help <command>"), BY_SERVER);
    }
    else if (msg1 == "announce")
    {
        chatLog(_("Command: /announce <msg>"), BY_SERVER);
        chatLog(_("*** only available to a GM ***"), BY_SERVER);
        chatLog(_("This command sends the message <msg> to "
                    "all players currently online."), BY_SERVER);
    }
    else if (msg1 == "clear")
    {
        chatLog(_("Command: /clear"), BY_SERVER);
        chatLog(_("This command clears the chat log of previous chat."),
                BY_SERVER);
    }
    else if (msg1 == "help")
    {
        chatLog(_("Command: /help"), BY_SERVER);
        chatLog(_("This command displays a list of all commands available."),
                BY_SERVER);
        chatLog(_("Command: /help <command>"), BY_SERVER);
        chatLog(_("This command displays help on <command>."), BY_SERVER);
    }
    else if (msg1 == "me")
    {
        chatLog(_("Command: /me <msg>"), BY_SERVER);
        chatLog(_("This command tell others you are (doing) <msg>."),
                BY_SERVER);
    }
    else if (msg1 == "party")
    {
        mParty->help(msg2);
    }
    else if (msg1 == "present")
    {
        chatLog(_("Command: /present"), BY_SERVER);
        chatLog(_("This command gets a list of players within hearing and "
                  "sends it to either the record log if recording, or the chat "
                  "log otherwise."), BY_SERVER);
    }
    else if (msg1 == "record")
    {
        chatLog(_("Command: /record <filename>"), BY_SERVER);
        chatLog(_("This command starts recording the chat log to the file "
                  "<filename>."), BY_SERVER);
        chatLog(_("Command: /record"), BY_SERVER);
        chatLog(_("This command finishes a recording session."), BY_SERVER);
    }
    else if (msg1 == "toggle")
    {
        chatLog(_("Command: /toggle <state>"), BY_SERVER);
        chatLog(_("This command sets whether the return key should toggle the "
                  "chat log, or whether the chat log turns off automatically."),
                  BY_SERVER);
        chatLog(_("<state> can be one of \"1\", \"yes\", \"true\" to "
                  "turn the toggle on, or \"0\", \"no\", \"false\" to turn the "
                  "toggle off."), BY_SERVER);
        chatLog(_("Command: /toggle"), BY_SERVER);
        chatLog(_("This command displays the return toggle status."),
                BY_SERVER);
    }
    else if (msg1 == "where")
    {
        chatLog(_("Command: /where"), BY_SERVER);
        chatLog(_("This command displays the name of the current map."),
                BY_SERVER);
    }
    else if (msg1 == "whisper" || msg1 == "msg" || msg1 == "w")
    {
        chatLog(_("Command: /msg <nick> <msg>"), BY_SERVER);
        chatLog(_("Command: /whisper <nick> <msg>"), BY_SERVER);
        chatLog(_("Command: /w <nick> <msg>"), BY_SERVER);
        chatLog(_("This command sends the message <msg> to <nick>."),
                BY_SERVER);
        chatLog(_("If the <nick> has spaces in it, enclose it in "
                  "double quotes (\")."), BY_SERVER);
    }
    else if (msg1 == "who")
    {
        chatLog(_("Command: /who"), BY_SERVER);
        chatLog(_("This command displays the number of players currently "
                  "online."), BY_SERVER);
    }
    else
    {
        chatLog(_("Unknown command."), BY_SERVER);
        chatLog(_("Type /help for a list of commands."), BY_SERVER);
    }
}
