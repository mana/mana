/*
 *  The Mana World
 *  Copyright (C) 2009  The Mana World Development Team
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

#ifndef CHATTAB_H
#define CHATTAB_H

#include "gui/widgets/tab.h"
#include "gui/chat.h"

class BrowserBox;
class Recorder;
class ScrollArea;

enum
{
    BY_GM,
    BY_PLAYER,
    BY_OTHER,
    BY_SERVER,
    BY_CHANNEL,
    ACT_WHISPER,      // getting whispered at
    ACT_IS,           // equivalent to "/me" on IRC
    BY_LOGGER
};

/**
 * A tab for the chat window. This is special to ease chat handling.
 */
class ChatTab : public Tab
{
    public:
        /**
         * Constructor.
         */
        ChatTab(const std::string &name);
        ~ChatTab();

        /**
         * Adds a line of text to our message list. Parameters:
         *
         * @param line Text message.
         * @param own  Type of message (usually the owner-type).
         * @param channelName which channel to send the message to.
         * @param ignoreRecord should this not be recorded?
         */
        void chatLog(std::string line, int own = BY_SERVER, bool ignoreRecord = false);

        /**
         * Adds the text to the message list
         *
         * @param msg  The message text which is to be sent.
         */
        void chatLog(const std::string &nick, const std::string &msg);

        /**
         * Determines whether the message is a command or message, then
         * sends the given message to the game server to be said, or to the
         * command handler
         *
         * @param msg  The message text which is to be sent.
         */
        void chatInput(const std::string &msg);

        /**
         * Scrolls the chat window
         *
         * @param amount direction and amount to scroll.  Negative numbers scroll
         * up, positive numbers scroll down.  The absolute amount indicates the
         * amount of 1/8ths of chat window real estate that should be scrolled.
         */
        void scroll(int amount);

        /**
         * Clears the text from the tab
         */
        void clearText();

        /**
         * Add any extra help text to the output. Allows tabs to define help
         * for commands defined by the tab itself.
         */
        virtual void showHelp() {}

        /**
         * Handle special commands. Allows a tab to handle commands it
         * defines itself.
         *
         * @returns true  if the command was handled
         *          false if the command was not handled
         */
        virtual bool handleCommand(const std::string &type,
                                   const std::string &args)
        { return false; }

    protected:
        friend class ChatWindow;
        friend class WhisperWindow;

        virtual void setCurrent() { setHighlighted(false); }

        virtual void handleInput(const std::string &msg);

        virtual void handleCommand(const std::string &msg);

        ScrollArea *mScrollArea;
        BrowserBox *mTextOutput;
        //Recorder *mRecorder;
};

extern ChatTab *localChatTab;

#endif // CHATTAB_H
