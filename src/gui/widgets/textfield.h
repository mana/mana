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

#include <guichan/widgets/textfield.hpp>

#include <vector>

class TextInput;

struct TextHistory
{
    std::list<std::string> history;             /**< Command history. */
    std::list<std::string>::iterator current;   /**< History iterator. */

    TextHistory()
    { current = history.end(); }

    bool empty() const
    { return history.empty(); }

    bool atBegining() const
    { return current == history.begin(); }

    bool atEnd() const
    { return current == history.end(); }

    void toBegining()
    { current = history.begin(); }

    void toEnd()
    { current = history.end(); }

    void addEntry(const std::string &text)
    { history.push_back(text); }

    bool matchesLastEntry(const std::string &text)
    { return history.back() == text; }
};

class AutoCompleteLister {
public:
    virtual ~AutoCompleteLister() = default;
    virtual void getAutoCompleteList(std::vector<std::string>&) const {}
};

/**
 * A text field.
 *
 * \ingroup GUI
 */
class TextField : public gcn::TextField
{
    public:
        /**
         * Constructor, initializes the text field with the given string.
         */
        TextField(const std::string &text = std::string(),
                  bool loseFocusOnTab = true);

        /**
         * Draws the text field.
         */
        void draw(gcn::Graphics *graphics) override;

        /**
         * Draws the background and border.
         */
        void drawFrame(gcn::Graphics *graphics) override;

        /**
         * Determine whether the field should be numeric or not
         */
        void setNumeric(bool numeric);

        /**
         * Set the range on the field if it is numeric
         */
        void setRange(int min, int max)
        {
            mMinimum = min;
            mMaximum = max;
        }

        /**
         * Processes one keypress.
         */
        void keyPressed(gcn::KeyEvent &keyEvent) override;

        /**
         * Handle text input (should possibly be new event in Guichan).
         */
        void textInput(const TextInput &textInput);

        /**
         * Set the minimum value for a range
         */
        void setMinimum(int min) { mMinimum = min; }

        /**
         * Set the maximum value for a range
         */
        void setMaximum(int max) { mMaximum = max; }

        /**
         * Return the value for a numeric field
         */
        int getValue() const;

        /**
         * Sets the TextField's source of autocomplete. Passing null will
         * disable autocomplete.
         */
        void setAutoComplete(AutoCompleteLister *lister)
        { mAutoComplete = lister; }

        /**
         * Returns the TextField's source of autocomplete.
         */
        AutoCompleteLister *getAutoComplete() const
        { return mAutoComplete; }

        /**
         * Sets the TextField's source of input history.
         */
        void setHistory(TextHistory *history)
        { mHistory = history; }

        /**
         * Returns the TextField's source of input history.
         */
        TextHistory *getHistory() const
        { return mHistory; }

    protected:
        void drawCaret(gcn::Graphics *graphics, int x) override;

    private:
        void autoComplete();
        void handlePaste();

        bool mNumeric = false;
        int mMinimum = 0;
        int mMaximum = 0;
        bool mLoseFocusOnTab;
        int mPadding = 1;

        AutoCompleteLister *mAutoComplete = nullptr;

        TextHistory *mHistory = nullptr; /**< Text history. */
};
