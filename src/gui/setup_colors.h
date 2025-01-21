/*
 *  Configurable text colors
 *  Copyright (C) 2008  Douglas Boffey <dougaboffey@netscape.net>
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

#include "guichanfwd.h"

#include "gui/widgets/setuptab.h"

#include <guichan/actionlistener.hpp>
#include <guichan/selectionlistener.hpp>

#include <string>

class BrowserBox;
class TextField;
class TextPreview;

class Setup_Colors : public SetupTab,
                     public gcn::ActionListener,
                     public gcn::SelectionListener
{
    public:
        Setup_Colors();
        ~Setup_Colors() override;

        void apply() override;
        void cancel() override;

        void action(const gcn::ActionEvent &event) override;

        void valueChanged(const gcn::SelectionEvent &event) override;

    private:
        static const std::string rawmsg;

        gcn::ListBox *mColorBox;
        gcn::ScrollArea *mScroll;
        BrowserBox *mPreview;
        TextPreview *mTextPreview;
        gcn::ScrollArea *mPreviewBox;
        int mSelected;

        gcn::Label *mGradTypeLabel;
        gcn::Slider *mGradTypeSlider;
        gcn::Label *mGradTypeText;

        gcn::Label *mGradDelayLabel;
        gcn::Slider *mGradDelaySlider;
        TextField *mGradDelayText;

        gcn::Label *mRedLabel;
        gcn::Slider *mRedSlider;
        TextField *mRedText;
        int mRedValue;

        gcn::Label *mGreenLabel;
        gcn::Slider *mGreenSlider;
        TextField *mGreenText;
        int mGreenValue;

        gcn::Label *mBlueLabel;
        gcn::Slider *mBlueSlider;
        TextField *mBlueText;
        int mBlueValue;

        void setEntry(gcn::Slider *s, TextField *t, int value);
        void updateColor();
        void updateGradType();
};
