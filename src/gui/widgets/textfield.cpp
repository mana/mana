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

#include "gui/widgets/textfield.h"

#include "graphics.h"

#include "gui/gui.h"
#include "gui/sdlinput.h"

#include "resources/theme.h"

#include "utils/copynpaste.h"
#include "utils/stringutils.h"

#include <guichan/font.hpp>

#include <SDL.h>

TextField::TextField(const std::string &text, bool loseFocusOnTab)
    : gcn::TextField(text)
    , mLoseFocusOnTab(loseFocusOnTab)
{
    auto &skin = gui->getTheme()->getSkin(SkinType::TextField);
    setFrameSize(skin.frameSize);
    mPadding = skin.padding;

    setWidth(getFont()->getWidth(mText) + 2 * mPadding);
    setHeight(getFont()->getHeight() + 2 * mPadding);
    fixScroll();
}

void TextField::draw(gcn::Graphics *graphics)
{
    if (getFrameSize() == 0)
        drawFrame(graphics);

    if (isFocused())
    {
        drawCaret(graphics,
                  getFont()->getWidth(mText.substr(0, mCaretPosition)) - mXScroll);
    }

    graphics->setColor(Theme::getThemeColor(Theme::TEXT));
    graphics->setFont(getFont());
    graphics->drawText(mText, mPadding - mXScroll, mPadding);
}

void TextField::drawFrame(gcn::Graphics *graphics)
{
    const int bs = getFrameSize();

    WidgetState state(this);
    state.width += bs * 2;
    state.height += bs * 2;

    gui->getTheme()->drawSkin(static_cast<Graphics *>(graphics), SkinType::TextField, state);
}

void TextField::setNumeric(bool numeric)
{
    mNumeric = numeric;
    if (!numeric)
        return;

    const char *text = mText.c_str();
    for (const char *textPtr = text; *textPtr; ++textPtr)
    {
        if (*textPtr < '0' || *textPtr > '9')
        {
            setText(mText.substr(0, textPtr - text));
            return;
        }
    }
}

int TextField::getValue() const
{
    if (!mNumeric)
        return 0;

    int value = atoi(mText.c_str());
    if (value < mMinimum)
        return mMinimum;

    if (value > mMaximum)
        return mMaximum;

    return value;
}

void TextField::drawCaret(gcn::Graphics *graphics, int x)
{
    graphics->setColor(getForegroundColor());
    graphics->drawLine(mPadding + x, mPadding, mPadding + x, getHeight() - mPadding);
}

void TextField::keyPressed(gcn::KeyEvent &keyEvent)
{
    switch (keyEvent.getKey().getValue())
    {
        case Key::LEFT:
        {
            while (mCaretPosition > 0)
            {
                --mCaretPosition;
                if ((mText[mCaretPosition] & 192) != 128)
                    break;
            }
        } break;

        case Key::RIGHT:
        {
            unsigned sz = mText.size();
            while (mCaretPosition < sz)
            {
                ++mCaretPosition;
                if (mCaretPosition == sz ||
                    (mText[mCaretPosition] & 192) != 128)
                    break;
            }
        } break;

        case Key::UP:
        {
            if (mHistory && !mHistory->atBegining() && !mHistory->empty())
            {
                // Move backward through the history
                mHistory->current--;
                setText(*mHistory->current);
                setCaretPosition(getText().length());
            }
        } break;

        case Key::DOWN:
        {
            if (mHistory && !mHistory->atEnd())
            {
                // Move forward through the history
                auto prevHist = mHistory->current++;

                if (!mHistory->atEnd())
                {
                    setText(*mHistory->current);
                    setCaretPosition(getText().length());
                }
                else
                {
                    setText(std::string());
                    mHistory->current = prevHist;
                }
            }
            else if (!getText().empty())
            {
                // Always clear (easy access to useful function)
                setText(std::string());
            }
        } break;

        case Key::DELETE_KEY:
        {
            unsigned sz = mText.size();
            while (mCaretPosition < sz)
            {
                --sz;
                mText.erase(mCaretPosition, 1);
                if (mCaretPosition == sz ||
                    (mText[mCaretPosition] & 192) != 128)
                    break;
            }
        } break;

        case Key::BACKSPACE:
        {
            while (mCaretPosition > 0)
            {
                --mCaretPosition;
                int v = mText[mCaretPosition];
                mText.erase(mCaretPosition, 1);
                if ((v & 192) != 128) break;
            }
        } break;

        case Key::ENTER:
            if (mHistory)
            {
                // If the input is different from previous, put it in the history
                if (!getText().empty() && (mHistory->empty() ||
                    !mHistory->matchesLastEntry(getText())))
                {
                    mHistory->addEntry(getText());
                }

                mHistory->toEnd();
            }

            distributeActionEvent();
            break;

        case Key::HOME:
            mCaretPosition = 0;
            break;

        case Key::END:
            mCaretPosition = mText.size();
            break;

        case Key::TAB:
            autoComplete();
            if (mLoseFocusOnTab)
                return;
            break;

        case SDLK_v:
            if (keyEvent.isControlPressed())
                handlePaste();
            break;
    }

    keyEvent.consume();
    fixScroll();
}

void TextField::textInput(const TextInput &textInput)
{
    mText.insert(mCaretPosition, textInput.getText());
    mCaretPosition += textInput.getText().length();
}

void TextField::autoComplete()
{
    if (mAutoComplete && !mText.empty())
    {
        const int caretPos = getCaretPosition();
        int startName = 0;
        const std::string inputText = getText();
        std::string name = inputText.substr(0, caretPos);
        std::string newName;

        for (int f = caretPos - 1; f > -1; f--)
        {
            if (isWordSeparator(inputText[f]))
            {
                startName = f + 1;
                name = inputText.substr(f + 1, caretPos - startName);
                break;
            }
        }

        if (caretPos == startName)
            return;


        std::vector<std::string> nameList;
        mAutoComplete->getAutoCompleteList(nameList);
        newName = autocomplete(nameList, name);

        if (newName.empty() && mHistory)
        {

            auto i = mHistory->history.begin();
            std::vector<std::string> nameList;

            while (i != mHistory->history.end())
            {
                std::string line = *i;
                unsigned int f = 0;
                while (f < line.length() && !isWordSeparator(line.at(f)))
                {
                    f++;
                }
                line = line.substr(0, f);
                if (!line.empty())
                {
                    nameList.push_back(line);
                }
                ++i;
            }

            newName = autocomplete(nameList, name);
        }

        if (!newName.empty())
        {
            if(inputText[0] == '@' || inputText[0] == '/')
                newName = "\"" + newName + "\"";

            setText(inputText.substr(0, startName) + newName
                    + inputText.substr(caretPos, inputText.length()
                                       - caretPos));

            setCaretPosition(caretPos - name.length() + newName.length());
        }
    }
}

void TextField::handlePaste()
{
    std::string text = getText();
    std::string::size_type caretPos = getCaretPosition();

    if (insertFromClipboard(text, caretPos)) {
        setText(text);
        setCaretPosition(caretPos);
    }
}
