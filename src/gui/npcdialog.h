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

#ifndef NPCDIALOG_H
#define NPCDIALOG_H

#include "gui/widgets/window.h"

#include "npc.h"

#include <guichan/actionlistener.hpp>
#include <guichan/listmodel.hpp>

#include <string>
#include <vector>

class TextBox;
class ListBox;
class TextField;
class IntTextField;
class Button;

/**
 * The npc dialog.
 *
 * \ingroup Interface
 */
class NpcDialog : public Window, public gcn::ActionListener,
                  public gcn::ListModel
{
    public:
        /**
         * Constructor.
         *
         * @see Window::Window
         */
        NpcDialog();

        ~NpcDialog();

        /**
         * Called when receiving actions from the widgets.
         */
        void action(const gcn::ActionEvent &event);

        void setNpc(int npc)
        { mNpcId = npc; }

        void clearText();

        /**
        * Sets the text shows in the dialog.
        *
        * @param string The new text.
        */
        void setText(const std::string &string);

        /**
         * Adds the text to the text shows in the dialog. Also adds a newline
         * to the end.
         *
         * @param string The text to add.
         */
        void addText(const std::string &string);

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
         */
        void closeDialog();

        /**
         * Returns the number of items in the choices list.
         */
        int getNumberOfElements();

        /**
         * Returns the name of item number i of the choices list.
         */
        std::string getElementAt(int i);

        /**
         * Makes this dialog request a choice selection from the user.
         */
        void choiceRequest();

        /**
         * Adds a choice to the list box.
         */
        void addChoice(const std::string &);

        /**
          * Fills the options list for an NPC dialog.
          *
          * @param itemString A string with the options separated with colons.
          */
        void parseListItems(const std::string &itemString);

        /**
         * Requests a text string from the user.
         */
        void textRequest(const std::string &defaultText = "");

        bool isInputFocused() const;

        /**
         * Requests a interger from the user.
         */
        void integerRequest(int defaultValue = 0, int min = 0, int max = 2000);

        void move(int amount);

        /**
         * Called when resizing the window.
         *
         * @param event The calling event
         */
        void widgetResized(const gcn::Event &event);

    private:
        void buildLayout();

        int mNpcId;

        int mDefaultInt;
        std::string mDefaultString;

        // Used for the main input area
        gcn::ScrollArea *mScrollArea;
        TextBox *mTextBox;
        std::string mText;

        // Used for choice input
        ListBox *mItemList;
        gcn::ScrollArea *mListScrollArea;
        std::vector<std::string> mItems;

        // Used for string and integer input
        TextField *mTextField;
        IntTextField *mIntField;
        Button *mPlusButton;
        Button *mMinusButton;

        // Used for the button
        Button *mButton;

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

        NpcInputState mInputState;
        NpcActionState mActionState;
};

// TODO: This should be made not to be global, later.

extern NpcDialog* npcDialog;

#endif // NPCDIALOG_H
