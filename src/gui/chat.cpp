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

#include <algorithm>
#include <fstream>

#include <guichan/focushandler.hpp>
#include <guichan/key.hpp>

#include "browserbox.h"
#include "chat.h"
#include "chatinput.h"
#include "scrollarea.h"
#include "windowcontainer.h"

#include "../beingmanager.h"
#include "../configuration.h"
#include "../extensions.h"
#include "../game.h"
#include "../localplayer.h"
#include "../party.h"
#include "../recorder.h"

#include "../net/messageout.h"
#include "../net/protocol.h"

#include "../utils/trim.h"

ChatWindow::ChatWindow(Network * network):
Window(""), mNetwork(network), mTmpVisible(false)
{
    setWindowName("Chat");

    setResizable(true);
    setDefaultSize(0, windowContainer->getHeight() - 123, 600, 123);

    mChatInput = new ChatInput;
    mChatInput->setActionEventId("chatinput");
    mChatInput->addActionListener(this);

    mTextOutput = new BrowserBox(BrowserBox::AUTO_WRAP);
    mTextOutput->setOpaque(false);
    mTextOutput->disableLinksAndUserColors();
    mTextOutput->setMaxRow((int) config.getValue("ChatLogLength", 0));
    mScrollArea = new ScrollArea(mTextOutput);
    mScrollArea->setPosition(mScrollArea->getFrameSize(),
                             mScrollArea->getFrameSize());
    mScrollArea->setScrollPolicy(gcn::ScrollArea::SHOW_NEVER,
                                 gcn::ScrollArea::SHOW_ALWAYS);
    mScrollArea->setOpaque(false);

    add(mScrollArea);
    add(mChatInput);

    loadWindowState();

    // Add key listener to chat input to be able to respond to up/down
    mChatInput->addKeyListener(this);
    mCurHist = mHistory.end();

    // Read the party prefix
    std::string partyPrefix = config.getValue("PartyPrefix", "$");
    mPartyPrefix = (partyPrefix == "" ? '$' : partyPrefix.at(0));
    mReturnToggles = config.getValue("ReturnToggles", "0") == "1";
    mRecorder = new Recorder(this);
    mParty = new Party(this, mNetwork);
}

ChatWindow::~ChatWindow()
{
    char partyPrefix[2] = ".";
    *partyPrefix = mPartyPrefix;
    config.setValue("PartyPrefix", partyPrefix);
    config.setValue("ReturnToggles", mReturnToggles ? "1" : "0");
    delete mRecorder;
}

void
 ChatWindow::logic()
{
    // todo: only do this when the size changes (updateWidgets?)

    const gcn::Rectangle area = getChildrenArea();

    mChatInput->setPosition(mChatInput->getFrameSize(),
                            area.height - mChatInput->getHeight() -
                            mChatInput->getFrameSize());
    mChatInput->setWidth(area.width - 2 * mChatInput->getFrameSize());
    mScrollArea->setWidth(area.width - 2 * mScrollArea->getFrameSize());
    mScrollArea->setHeight(area.height - 2 * mScrollArea->getFrameSize() -
                           mChatInput->getHeight() - 5);
    mScrollArea->logic();
}

void ChatWindow::chatLog(std::string line, int own, bool ignoreRecord)
{
    // Trim whitespace
    trim(line);

    CHATLOG tmp;
    tmp.own = own;
    tmp.nick = "";
    tmp.text = line;

    // Fix the owner of welcome message.
    if (line.substr(0, 7) == "Welcome") {
        own = BY_SERVER;
    }

    std::string::size_type pos = line.find(" : ");
    if (pos != std::string::npos) {
        tmp.nick = line.substr(0, pos);
        tmp.text = line.substr(pos + 3);
    }

    std::string lineColor = "##C";
    switch (own) {
        case BY_GM:
            tmp.nick += std::string("Global announcement: ");
            lineColor = "##G";
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
            tmp.nick = "Server: ";
            tmp.text = line;
            lineColor = "##S";
            break;
        case BY_PARTY:
            tmp.nick += CAT_NORMAL;
            lineColor = "##P";
            break;
        case ACT_WHISPER:
            tmp.nick += CAT_WHISPER;
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
        mScrollArea->getVerticalMaxScroll()) {
        mTextOutput->addRow(line);
        mScrollArea->setVerticalScrollAmount(mScrollArea->
                                             getVerticalMaxScroll());
    } else {
        mTextOutput->addRow(line);
    }

    mRecorder->record(line.substr(3));
}

void ChatWindow::chatLog(CHATSKILL act)
{
    chatLog(const_msg(act), BY_SERVER);
}

void ChatWindow::action(const gcn::ActionEvent & event)
{
    if (event.getId() == "chatinput") {
        std::string message = mChatInput->getText();

        if (!message.empty()) {
            // If message different from previous, put it in the history
            if (mHistory.empty() || message != mHistory.back()) {
                mHistory.push_back(message);
            }
            // Reset history iterator
            mCurHist = mHistory.end();

            // Send the message to the server
            chatSend(player_node->getName(), message);

            // Clear the text from the chat input
            mChatInput->setText("");
        }
        if (message.empty() || !mReturnToggles) {
            // Remove focus and hide input
            mFocusHandler->focusNone();

            // If the chatWindow is shown up because you want to send a message
            // It should hide now
            if (mTmpVisible) {
                setVisible(false);
            }
        }
    }
}

void ChatWindow::requestChatFocus()
{
    // Make sure chatWindow is visible
    if (!isVisible()) {
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

void ChatWindow::chatSend(const std::string &nick, std::string msg)
{
    /* Some messages are managed client side, while others
     * require server handling by proper packet. Probably
     * those if elses should be replaced by protocol calls */

    // Send party message
    if (msg.at(0) == mPartyPrefix) {
        msg.erase(0, 1);
        std::size_t length = msg.length() + 1;

        if (length == 0) {
            chatLog("Trying to send a blank party message.", BY_SERVER);
            return;
        }
        MessageOut outMsg(mNetwork);

        outMsg.writeInt16(CMSG_PARTY_MESSAGE);
        outMsg.writeInt16(length + 4);
        outMsg.writeString(msg, length);
        return;
    }
    // Prepare ordinary message
    if (msg.substr(0, 1) != "/") {
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
    if (space == std::string::npos) {
        msg = "";
    } else {
        msg = msg.substr(space);
        trim(msg);
    }

    if (command == "announce") 
    {
        MessageOut outMsg(mNetwork);
        outMsg.writeInt16(0x0099);
        outMsg.writeInt16(msg.length() + 4);
        outMsg.writeString(msg, msg.length());
        return;
    }
    else if (command == "help") 
    {
        msg.erase(0, 6);
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
        if (msg != "" && msg.at(0) == '/')
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
    if (command == "who") {
        MessageOut outMsg(mNetwork);
        outMsg.writeInt16(0x00c1);
        return;
    }
    if (command == "clear") {
        mTextOutput->clearRows();
        return;
    }
    if (command == "whisper" || command == "msg" || command == "w") {
        std::string recvnick = "";

        if (msg.substr(0, 1) == "\"") {
            const std::string::size_type pos = msg.find('"', 1);
            if (pos != std::string::npos) {
                recvnick = msg.substr(1, pos - 1);
                msg.erase(0, pos + 2);
            }
        } else {
            const std::string::size_type pos = msg.find(" ");
            if (pos != std::string::npos) {
                recvnick = msg.substr(0, pos);
                msg.erase(0, pos + 1);
            }
        }

        MessageOut outMsg(mNetwork);
        outMsg.writeInt16(CMSG_CHAT_WHISPER);
        outMsg.writeInt16(msg.length() + 28);
        outMsg.writeString(recvnick, 24);
        outMsg.writeString(msg, msg.length());

        chatLog("Whispering to " + recvnick + " : " + msg, BY_PLAYER);
        return;
    }
    if (command == "record") {
        mRecorder->respond(msg);
        return;
    }
    if (command == "toggle") {
        if (msg == "") {
            chatLog(mReturnToggles ? "Return toggles chat."
                    : "Message closes chat.", BY_SERVER);
            return;
        }
        msg = msg.substr(0, 1);
        if (msg == "1" || msg == "y" || msg == "t" || msg == "Y" || msg == "T") {
            chatLog("Return now toggles chat.", BY_SERVER);
            mReturnToggles = true;
            return;
        }
        if (msg == "0" || msg == "n" || msg == "f" || msg == "N" || msg == "F") {
            chatLog("Message now closes chat.", BY_SERVER);
            mReturnToggles = false;
            return;
        }
        chatLog("Options to /toggle are \"yes\", \"no\", \"true\", \"false\", "
                "\"1\", \"0\".", BY_SERVER);
        return;
    }
    if (command == "party") {
        if (msg == "") {
            chatLog("Unknown party command... Type \"/help\" party for more "
                    "information.", BY_SERVER);
            return;
        }
        const std::string::size_type space = msg.find(" ");
        std::string rest = (space == std::string::npos ? ""
                            : msg.substr(space + 1, msg.length()));
        if (rest != "") {
            msg = msg.substr(0, space);
            while (msg != "" && msg[0] == ' ') {
                msg = msg.substr(1, msg.length());
            }
        }
        party(msg, rest);
        return;
    }
    if (command == "cast") {
        /*
         * This will eventually be replaced by a GUI, so
         * we don't need to get too sophisticated
         */
        if (extensions.aethyra_spells) {
            MessageOut outMsg(mNetwork);
            if (msg == "heal") {
                outMsg.writeInt16(0x03f3);
                outMsg.writeInt16(0x01);
                outMsg.writeInt32(0);
                outMsg.writeInt8(0);
                outMsg.writeInt8(0);
                outMsg.writeString("", 24);
            } else if (msg == "gather") {
                outMsg.writeInt16(0x03f3);
                outMsg.writeInt16(0x02);
                outMsg.writeInt32(0);
                outMsg.writeInt8(0);
                outMsg.writeInt8(0);
                outMsg.writeString("", 24);
            } else {
                chatLog("No such spell!", BY_SERVER);
            }
        } else {
            chatLog("The current server doesn't support spells", BY_SERVER);
        }
        return;
    }
    if (command == "present") {
        Beings & beings = beingManager->getAll();
        std::string response = "";
        for (BeingIterator bi = beings.begin(), be = beings.end();
             bi != be; ++bi) {
            if ((*bi)->getType() == Being::PLAYER) {
                if (response != "") {
                    response += ", ";
                }
                response += (*bi)->getName();
            }
        }
        if (mRecorder->isRecording()) {
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


            mRecorder->record(timeStr.str() + "Present: " + response + ".");
            chatLog("Attendance written to record log.", BY_SERVER, true);
        } else {
            chatLog("Present: " + response, BY_SERVER);
        }
        return;
    }
    chatLog("Unknown command", BY_SERVER);
}

std::string ChatWindow::const_msg(CHATSKILL act)
{
    std::string msg;
    if (act.success == SKILL_FAILED && act.skill == SKILL_BASIC) {
        switch (act.bskill) {
            case BSKILL_TRADE:
                msg = "Trade failed!";
                break;
            case BSKILL_EMOTE:
                msg = "Emote failed!";
                break;
            case BSKILL_SIT:
                msg = "Sit failed!";
                break;
            case BSKILL_CREATECHAT:
                msg = "Chat creating failed!";
                break;
            case BSKILL_JOINPARTY:
                msg = "Could not join party!";
                break;
            case BSKILL_SHOUT:
                msg = "Cannot shout!";
                break;
        }

        switch (act.reason) {
            case RFAIL_SKILLDEP:
                msg += " You have not yet reached a high enough level!";
                break;
            case RFAIL_INSUFHP:
                msg += " Insufficient HP!";
                break;
            case RFAIL_INSUFSP:
                msg += " Insufficient SP!";
                break;
            case RFAIL_NOMEMO:
                msg += " You have no memos!";
                break;
            case RFAIL_SKILLDELAY:
                msg += " You cannot do that right now!";
                break;
            case RFAIL_ZENY:
                msg += " Seems you need more Zeny... ;-)";
                break;
            case RFAIL_WEAPON:
                msg += " You cannot use this skill with that kind of weapon!";
                break;
            case RFAIL_REDGEM:
                msg += " You need another red gem!";
                break;
            case RFAIL_BLUEGEM:
                msg += " You need another blue gem!";
                break;
            case RFAIL_OVERWEIGHT:
                msg += " You're carrying to much to do this!";
                break;
            default:
                msg += " Huh? What's that?";
                break;
        }
    } else {
        switch (act.skill) {
            case SKILL_WARP:
                msg = "Warp failed...";
                break;
            case SKILL_STEAL:
                msg = "Could not steal anything...";
                break;
            case SKILL_ENVENOM:
                msg = "Poison had no effect...";
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

void ChatWindow::keyPressed(gcn::KeyEvent & event)
{
    if (event.getKey().getValue() == gcn::Key::DOWN &&
        mCurHist != mHistory.end()) {
        // Move forward through the history
        HistoryIterator prevHist = mCurHist++;
        if (mCurHist != mHistory.end()) {
            mChatInput->setText(*mCurHist);
            mChatInput->setCaretPosition(mChatInput->getText().length());
        } else {
            mCurHist = prevHist;
        }
    } else if (event.getKey().getValue() == gcn::Key::UP &&
               mCurHist != mHistory.begin() && mHistory.size() > 0) {
        // Move backward through the history
        mCurHist--;
        mChatInput->setText(*mCurHist);
        mChatInput->setCaretPosition(mChatInput->getText().length());
    }
}

void ChatWindow::setInputText(std::string input_str)
{
    mChatInput->setText(input_str + " ");
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
    if (command == "prefix") {
        if (rest == "") {
            char temp[2] = ".";
            *temp = mPartyPrefix;
            chatLog("The current party prefix is " + std::string(temp),
                    BY_SERVER);
            return;
        }
        if (rest.length() != 1) {
            chatLog("Party prefix must be one character long.", BY_SERVER);
        } else {
            if (rest == "/") {
                chatLog("Cannot use a '/' as the prefix.", BY_SERVER);
            } else {
                mPartyPrefix = rest.at(0);
                chatLog("Changing prefix to " + rest, BY_SERVER);
            }
        }
        return;
    }
    mParty->respond(command, rest);
}

void ChatWindow::help(const std::string & msg1, const std::string & msg2)
{
    chatLog("-- Help --", BY_SERVER);
    if (msg1 == "") {
        chatLog("/announce: Global announcement (GM only)", BY_SERVER);
        chatLog("/clear: Clears this window", BY_SERVER);
        chatLog("/help: Display this help.", BY_SERVER);
        mParty->help();
        chatLog("/present: Get list of players present", BY_SERVER);
        mRecorder->help();
        chatLog("/toggle: Determine whether <return> toggles the chat log.",
                BY_SERVER);
        chatLog("/where: Display map name", BY_SERVER);
        chatLog("/whisper <nick> <message>: Sends a private <message>"
                " to <nick>", BY_SERVER);
        chatLog("/who: Display number of online users", BY_SERVER);
        chatLog("For more information, type /help <command>", BY_SERVER);
        return;
    }
    if (msg1 == "announce")
    {
        chatLog("Command: /announce <msg>", BY_SERVER);
        chatLog("*** only available to a GM ***", BY_SERVER);
        chatLog("This command sends the message <msg> to "
                "all players currently online.", BY_SERVER);
        return;
    }
    if (msg1 == "clear")
    {
        chatLog("Command: /clear", BY_SERVER);
        chatLog("This command clears the chat log of previous chat.",
                BY_SERVER);
        return;
    }
    if (msg1 == "help")
    {
        chatLog("Command: /help", BY_SERVER);
        chatLog("This command displays a list of all commands available.",
                BY_SERVER);
        chatLog("Command: /help <command>", BY_SERVER);
        chatLog("This command displays help on <command>.", BY_SERVER);
        return;
    }
    if (msg1 == "party") 
    {
        mParty->help(msg2);
        return;
    }
    if (msg1 == "present") 
    {
        chatLog("Command: /present", BY_SERVER);
        chatLog("This command gets a list of players within hearing "
                "and sends it to either the record log if recording, or the "
                "chat log otherwise.", BY_SERVER);
        return;
    }
    if (msg1 == "record") 
    {
        mRecorder->help(msg2);
        return;
    }
    if (msg1 == "toggle") 
    {
        chatLog("Command: /toggle <state>", BY_SERVER);
        chatLog("This command sets whether the return key should toggle the "
                "chat log, or whether the chat log turns off automatically.",
                BY_SERVER);
        chatLog("<state> can be one of \"1\", \"yes\", \"true\" to turn "
                "the toggle on, or \"0\", \"no\", \"false\" to turn the "
                "toggle off.", BY_SERVER);
        chatLog("Command: /toggle", BY_SERVER);
        chatLog("This command displays the return toggle status.", BY_SERVER);
        return;
    }
    if (msg1 == "where")
    {
        chatLog("Command: /where", BY_SERVER);
        chatLog("This command displays the name of the current map.",
                BY_SERVER);
        return;
    }
    if (msg1 == "whisper" || msg1 == "msg" || msg1 == "w") {
        chatLog("Command: /whisper <nick> <msg>", BY_SERVER);
        chatLog("This command sends the message <msg> to <nick.", BY_SERVER);
        chatLog("If the <nick> has spaces in it, enclose it in "
                "double quotes (\").", BY_SERVER);
        return;
    }
    if (msg1 == "who")
    {
        chatLog("Command: /who", BY_SERVER);
        chatLog("This command displays the number of players currently "
                "online.", BY_SERVER);
        return;
    }
    chatLog("Unknown command.", BY_SERVER);
    chatLog("Type /help for a list of commands.", BY_SERVER);
}
