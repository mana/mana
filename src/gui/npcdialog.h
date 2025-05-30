/*
 *  The Mana Client
 *  Copyright (C) 2004-2009  The Mana World Development Team
 *  Copyright (C) 2009-2012  The Mana Developers
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

#pragma once

#include "gui/widgets/window.h"

#include <guichan/actionlistener.hpp>
#include <guichan/listmodel.hpp>

#include <list>
#include <memory>
#include <string>
#include <vector>

class BrowserBox;
class ListBox;
class TextField;
class IntTextField;
class ItemLinkHandler;
class Button;

/**
 * The npc dialog.
 *
 * \ingroup Interface
 */
class NpcDialog final : public Window,
                        public gcn::ActionListener,
                        public gcn::ListModel
{
    public:
        NpcDialog(int npcId);

        ~NpcDialog() override;

        /**
         * Called when receiving actions from the widgets.
         */
        void action(const gcn::ActionEvent &event) override;

        /**
        * Sets the text shows in the dialog.
        *
        * @param string The new text.
        */
        void setText(const std::vector<std::string> &string);

        /**
         * Adds the text to the text shows in the dialog. Also adds a newline
         * to the end.
         *
         * @param string The text to add.
         */
        void addText(const std::string &string, bool save = true);

        /**
         * When called, the widget will show a "Next" button.
         */
        void showNextButton();

        /**
         * When called, the widget will show a "Close" button and will close
         * the dialog when clicked.
         */
        void showCloseButton();

        /**
         * Notifies the server that client has performed a next action.
         */
        void nextDialog();

        /**
         * Notifies the server that the client has performed a close action.
         * @overrides Window::close()
         */
        void close() override;

        /**
         * Returns the number of items in the choices list.
         */
        int getNumberOfElements() override;

        /**
         * Returns the name of item number i of the choices list.
         */
        std::string getElementAt(int i) override;

        /**
         * Makes this dialog request a choice selection from the user.
         */
        void choiceRequest();

        /**
         * Adds a choice to the list box.
         */
        void addChoice(const std::string &);

        /**
         * Put focus on the first choice.
         */
        void initChoiceSelection();

        /**
         * Requests a text string from the user.
         */
        void textRequest(const std::string &defaultText = std::string());

        bool isInputFocused() const;

        bool isTextInputFocused() const;

        static bool isAnyInputFocused();

        /**
         * Requests an integer from the user.
         */
        void integerRequest(int defaultValue, int min, int max);

        void move(int amount);

        void setVisible(bool visible) override;

        void mouseClicked(gcn::MouseEvent &mouseEvent) override;

        /**
         * Returns the first active instance. Useful for pushing user
         * interaction.
         */
        static NpcDialog *getActive();

        bool isWaitingForTheServer() const
        { return mActionState == NPC_ACTION_WAIT; }

        /**
         * Closes all instances.
         */
        static void closeAll();

        static void setup();

    private:
        using DialogList = std::list<NpcDialog *>;
        static DialogList instances;

        void buildLayout();

        int mNpcId;

        int mDefaultInt = 0;
        std::string mDefaultString;

        // Used for the main input area
        gcn::ScrollArea *mScrollArea;
        BrowserBox *mTextBox;
        std::vector<std::string> mNewText;

        std::unique_ptr<ItemLinkHandler> mItemLinkHandler;

        // Used for choice input
        ListBox *mItemList;
        gcn::ScrollArea *mListScrollArea;
        std::vector<std::string> mItems;

        // Used for string and integer input
        TextField *mTextField;
        IntTextField *mIntField;
        Button *mPlusButton;
        Button *mMinusButton;

        Button *mClearButton;

        // Used for the button
        Button *mNextButton;

        // Will reset the text and integer input to the provided default
        Button *mResetButton;

        enum NpcInputState
        {
            NPC_INPUT_NONE,
            NPC_INPUT_LIST,
            NPC_INPUT_STRING,
            NPC_INPUT_INTEGER
        };

        enum NpcActionState
        {
            NPC_ACTION_WAIT,
            NPC_ACTION_NEXT,
            NPC_ACTION_INPUT,
            NPC_ACTION_CLOSE
        };

        NpcInputState mInputState = NPC_INPUT_NONE;
        NpcActionState mActionState = NPC_ACTION_WAIT;
};
