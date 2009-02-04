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

#ifndef CHAT_H
#define CHAT_H

#include <list>
#include <string>

#include <guichan/actionlistener.hpp>
#include <guichan/keylistener.hpp>

#include "window.h"

class BrowserBox;
class Network;
class Recorder;
class Party;
class ScrollArea;
class ItemLinkHandler;

#define BY_GM         0   // those should be self-explanatory =)
#define BY_PLAYER     1
#define BY_OTHER      2
#define BY_SERVER     3
#define BY_PARTY      4

#define ACT_WHISPER   5   // getting whispered at
#define ACT_IS        6   // equivalent to "/me" on IRC

#define BY_LOGGER     7

/**
 * gets in between usernick and message text depending on
 * message type
 */
#define CAT_NORMAL        ": "
#define CAT_IS            ""
#define CAT_WHISPER       " whispers: "

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

#define DEFAULT_CHAT_WINDOW_SCROLL 7 // 1 means `1/8th of the window size'.

struct CHATSKILL
{
    short skill;
    short bskill;
    short unused;
    char success;
    char reason;
};

/**
 * The chat window.
 *
 * \ingroup Interface
 */
class ChatWindow : public Window, public gcn::ActionListener,
                   public gcn::KeyListener
{
    public:
        /**
         * Constructor.
         */
        ChatWindow(Network *network);

        /**
         * Destructor: used to write back values to the config file
         */
        ~ChatWindow();

        /**
         * Adds a line of text to our message list. Parameters:
         *
         * @param line Text message.
         * @parem own  Type of message (usually the owner-type).
         */
        void chatLog(std::string line, int own, bool ignoreRecord = false);

        /**
         * Calls original chat_log() after processing the packet.
         */
        void chatLog(CHATSKILL);

        /**
         * Performs action.
         */
        void action(const gcn::ActionEvent &event);

        /**
         * Request focus for typing chat message.
         */
        void requestChatFocus();

        /**
         * Checks whether ChatWindow is Focused or not.
         */
        bool isInputFocused();

        /**
         * Determines whether to send a command or an ordinary message, then
         * contructs packets & sends them.
         *
         * @param nick The character's name to display in front.
         * @param msg  The message text which is to be send.
         *
         * NOTE:
         * The nickname is required by the server, if not specified
         * the message may not be sent unless a command was intended
         * which requires another packet to be constructed! you can
         * achieve this by putting a slash ("/") infront of the
         * message followed by the command name and the message.
         * of course all slash-commands need implemented handler-
         * routines. ;-)
         * remember, a line starting with "@" is not a command that needs
         * to be parsed rather is sent using the normal chat-packet.
         *
         * EXAMPLE:
         * // for an global announcement   /- command
         * chatlog.chat_send("", "/announce Hello to all logged in users!");
         * // for simple message by a user /- message
         * chatlog.chat_send("Zaeiru", "Hello to all users on the screen!");
         */
        void chatSend(const std::string &nick, std::string msg);

        /** Called when key is pressed */
        void keyPressed(gcn::KeyEvent &event);

        /** Called to set current text */
        void setInputText(std::string input_str);

        /** Called to add item to chat */
        void addItemText(int itemid, const std::string &item);

        /** Override to reset mTmpVisible */
        void setVisible(bool visible);

       /**
        * Scrolls the chat window
        *
        * @param amount direction and amount to scroll.  Negative numbers scroll
        * up, positive numbers scroll down.  The absolute amount indicates the
        * amount of 1/8ths of chat window real estate that should be scrolled.
        */
        void scroll(int amount);

        /**
         * party implements the partying chat commands
         *
         * @param command is the party command to perform
         * @param msg is the remainder of the message
         */
        void party(const std::string &command, const std::string &msg);

        /**
         * help implements the /help command
         *
         * @param msg1 is the command that the player needs help on
         * @param msg2 is the sub-command relating to the command
         */
        void help(const std::string &msg1, const std::string &msg2);

    private:

        Network *mNetwork;
        bool mTmpVisible;

        void whisper(const std::string &nick, std::string msg);

        /** One item in the chat log */
        struct CHATLOG
        {
            std::string nick;
            std::string text;
            int own;
        };

        /** Constructs failed messages for actions */
        std::string const_msg(CHATSKILL);

        gcn::TextField *mChatInput; /**< Input box for typing chat messages */
        BrowserBox *mTextOutput;    /**< Text box for displaying chat history */
        ScrollArea *mScrollArea;    /**< Scroll area around text output */
        ItemLinkHandler *mItemLinkHandler; /** Used for showing item popup on
                                               clicking links **/
        typedef std::list<std::string> History;
        typedef History::iterator HistoryIterator;
        History mHistory;           /**< Command history */
        HistoryIterator mCurHist; /**< History iterator */
        Recorder *mRecorder; /**< Recording class */
        char mPartyPrefix; /**< Messages beginning with the prefix are sent to
                              the party */
        bool mReturnToggles; /**< Marks whether <Return> toggles the chat log
                                or not */
        Party *mParty;
};
extern ChatWindow *chatWindow;

#endif
