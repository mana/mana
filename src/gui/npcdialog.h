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

#ifndef NPCDIALOG_H
#define NPCDIALOG_H

#include "eventlistener.h"

#include "gui/widgets/window.h"

#include <guichan/actionlistener.hpp>
#include <guichan/keylistener.hpp>
#include <guichan/listmodel.hpp>


#include <list>
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
class NpcDialog : public Window,
                  public gcn::ActionListener,
                  public gcn::ListModel,
                  public gcn::KeyListener
{
    public:
        NpcDialog(int npcId);

        ~NpcDialog();

        /**
         * Called when receiving actions from the widgets.
         */
        void action(const gcn::ActionEvent &event);

        void keyPressed(gcn::KeyEvent &keyEvent);

        /**
         * Moves the dialog forward
         */
        void proceed();

        /**
         * Update the text being written to the screen
         *
         * @overload Window::logic
         */
        void logic();

        /**
         * Has the dialog window animate playing the text
         *
         * @param string The text that will be played
         */
        void playText(const std::string &string);

        /**
        * Sets the text shows in the dialog.
        *
        * @param string The new text.
        */
        void setText(const std::string &string);

        /**
         * When called the window's next interaction
         * with the player will be to request the next
         * stage in the interaction.
         */
        void setStateNext();

        /**
         * When called the window's next interaction
         * with the player will be to close the window.
         */
        void setStateClose();

        /**
         * Notifies the server that the client has performed a close action.
         * @overrides Window::close()
         */
        void close();

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
         * Put focus on the first choice.
         */
        void initChoiceSelection();

        /**
         * Requests a text string from the user.
         */
        void textRequest(const std::string &defaultText = "");

        bool isInputFocused() const;

        bool isTextInputFocused() const;

        static bool isAnyInputFocused();

        /**
         * Requests an integer from the user.
         */
        void integerRequest(int defaultValue, int min, int max);

        void move(int amount);

        void setVisible(bool visible);

        void mousePressed(gcn::MouseEvent &mouseEvent);

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
        typedef std::list<NpcDialog*> DialogList;
        static DialogList instances;

        void buildLayout();

        int mNpcId;

        int mDefaultInt;
        std::string mDefaultString;

        // Used for the main input area
        TextBox *mTextBox;
        // Target string to be displayed into mTextBox
        std::string mText;
        // Timer for when to add a new character
        int mTextPlayTime;
        // When set, if playText() is called again
        // It will clear the existing values
        bool mClearTextOnNextPlay;

        // Used for choice input
        ListBox *mItemList;
        gcn::ScrollArea *mListScrollArea;
        std::vector<std::string> mItems;

        // Used for string and integer input
        TextField *mTextField;
        IntTextField *mIntField;
        Button *mSubmitButton;

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

#endif // NPCDIALOG_H
