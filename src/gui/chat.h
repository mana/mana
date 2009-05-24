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

#include "gui/widgets/window.h"

#include <guichan/actionlistener.hpp>
#include <guichan/keylistener.hpp>
#include <guichan/widget.hpp>
#include <guichan/widgetlistener.hpp>

#include <list>
#include <string>
#include <map>

class BrowserBox;
class Channel;
class ChatTab;
class ChatInput;
class Recorder;
class ScrollArea;
class TabbedArea;
class ItemLinkHandler;
class Tab;
class WhisperTab;

#define DEFAULT_CHAT_WINDOW_SCROLL 7 // 1 means `1/8th of the window size'.

/** One item in the chat log */
struct CHATLOG
{
    std::string nick;
    std::string text;
    int own;
};

/**
 * The chat window.
 *
 * \ingroup Interface
 */
class ChatWindow : public Window,
                   public gcn::ActionListener,
                   public gcn::KeyListener
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
         * Called when the widget changes size. Used for adapting the size of
         * the tabbed area.
         */
        void widgetResized(const gcn::Event &event);

        void logic();

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

        /** Called when key is pressed */
        void keyPressed(gcn::KeyEvent &event);

        /** Add the given text to the chat input. */
        void addInputText(const std::string &text);

        /** Called to add item to chat */
        void addItemText(const std::string &item);

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
         * Sets the file being recorded to
         *
         * @param msg The file to write out to. If null, then stop recording.
         */
        void setRecordingFile(const std::string &msg);

        bool getReturnTogglesChat() const { return mReturnToggles; }
        void setReturnTogglesChat(bool toggles) { mReturnToggles = toggles; }

        void doPresent();

        void whisper(const std::string &nick, const std::string &mes,
                     bool own = false);

        ChatTab *addWhisperTab(const std::string &nick, bool switchTo = false);

    protected:
        friend class ChatTab;
        friend class WhisperTab;

        /** Remove the given tab from the window */
        void removeTab(ChatTab *tab);

        /** Add the tab to the window */
        void addTab(ChatTab *tab);

        void removeWhisper(const std::string &nick);

        void adjustTabSize();

        /** Used for showing item popup on clicking links **/
        ItemLinkHandler *mItemLinkHandler;
        Recorder *mRecorder;

        /** Input box for typing chat messages. */
        ChatInput *mChatInput;

    private:
        bool mTmpVisible;

        /** Tabbed area for holding each channel. */
        TabbedArea *mChatTabs;
        Tab *mCurrentTab;

        typedef std::map<const std::string, ChatTab*> TabMap;
        /** Manage whisper tabs */
        TabMap mWhispers;

        typedef std::list<std::string> History;
        typedef History::iterator HistoryIterator;
        History mHistory;           /**< Command history. */
        HistoryIterator mCurHist;   /**< History iterator. */
        bool mReturnToggles; /**< Marks whether <Return> toggles the chat log
                                or not */
};

extern ChatWindow *chatWindow;

#endif
