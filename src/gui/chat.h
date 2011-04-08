/*
 *  The Mana Client
 *  Copyright (C) 2004-2009  The Mana World Development Team
 *  Copyright (C) 2009-2010  The Mana Developers
 *
 *  This file is part of The Mana Client.
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
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef CHAT_H
#define CHAT_H

#include "eventlistener.h"

#include "gui/widgets/window.h"
#include "gui/widgets/textfield.h"

#include <guichan/actionlistener.hpp>
#include <guichan/keylistener.hpp>
#include <guichan/widget.hpp>
#include <guichan/widgetlistener.hpp>

#include <list>
#include <string>
#include <map>
#include <vector>

class BrowserBox;
class ChatTab;
class Channel;
class ChatInput;
class Recorder;
class ScrollArea;
class TabbedArea;
class ItemLinkHandler;
class Tab;
class WhisperTab;

#define DEFAULT_CHAT_WINDOW_SCROLL 7 // 1 means `1/8th of the window size'.

enum Own
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

/** One item in the chat log */
struct CHATLOG
{
    std::string nick;
    std::string text;
    Own own;
};

/**
 * The chat window.
 *
 * \ingroup Interface
 */
class ChatWindow : public Window,
                   public gcn::ActionListener,
                   public EventListener
{
    public:
        /**
         * Constructor.
         */
        ChatWindow();

        /**
         * Destructor: used to write back values to the config file
         */
        ~ChatWindow();

        /**
         * Reset the chat window and recorder window attached to it to their
         * default positions.
         */
        void resetToDefaultSize();

        /**
         * Gets the focused tab.
         */
        ChatTab *getFocused() const;

        /**
         * Clear the given tab.
         */
        void clearTab(ChatTab *tab);

        /**
         * Clear the current tab.
         */
        void clearTab();

        /**
         * Switch to the previous tab in order
         */
        void prevTab();

        /**
         * Switch to the next tab in order
         */
        void nextTab();

        /**
         * Performs action.
         */
        void action(const gcn::ActionEvent &event);

        /**
         * Request focus for typing chat message.
         *
         * \returns true if the input was shown
         *          false otherwise
         */
        bool requestChatFocus();

        /**
         * Checks whether ChatWindow is Focused or not.
         */
        bool isInputFocused() const;

        /**
         * Passes the text to the current tab as input
         *
         * @param msg  The message text which is to be sent.
         */
        void chatInput(const std::string &msg);

        /** Add the given text to the chat input. */
        void addInputText(const std::string &text);

        /** Called to add item to chat */
        void addItemText(const std::string &item);

        /** Override to reset mTmpVisible */
        void setVisible(bool visible);

	void mousePressed(gcn::MouseEvent &event);
	void mouseDragged(gcn::MouseEvent &event);

	void event(Event::Channel channel, const Event &event);

        /**
         * Scrolls the chat window
         *
         * @param amount direction and amount to scroll.  Negative numbers scroll
         * up, positive numbers scroll down.  The absolute amount indicates the
         * amount of 1/8ths of chat window real estate that should be scrolled.
         */
        void scroll(int amount);

        /**
         * Sets the file being recorded to
         *
         * @param msg The file to write out to. If null, then stop recording.
         */
        void setRecordingFile(const std::string &msg);

        bool getReturnTogglesChat() const { return mReturnToggles; }
        void setReturnTogglesChat(bool toggles) { mReturnToggles = toggles; }

        void doPresent();

        void whisper(const std::string &nick, const std::string &mes,
                     Own own = BY_OTHER);

        ChatTab *addWhisperTab(const std::string &nick, bool switchTo = false);

    protected:
        friend class ChatTab;
        friend class WhisperTab;
        friend class ChatAutoComplete;

        /** Remove the given tab from the window */
        void removeTab(ChatTab *tab);

        /** Add the tab to the window */
        void addTab(ChatTab *tab);

        void removeWhisper(const std::string &nick);

        void removeAllWhispers();

        /** Used for showing item popup on clicking links **/
        ItemLinkHandler *mItemLinkHandler;
        Recorder *mRecorder;

        /** Input box for typing chat messages. */
        ChatInput *mChatInput;

        TextHistory *mHistory;
        AutoCompleteLister *mAutoComplete;

    private:
        bool mTmpVisible;

        /** Tabbed area for holding each channel. */
        TabbedArea *mChatTabs;

        typedef std::map<const std::string, ChatTab*> TabMap;
        /** Manage whisper tabs */
        TabMap mWhispers;

        bool mReturnToggles; /**< Marks whether <Return> toggles the chat log
                                or not */
};

extern ChatWindow *chatWindow;

#endif
