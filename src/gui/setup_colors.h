/*
 *  Configurable text colors
 *  Copyright (C) 2008  Douglas Boffey <dougaboffey@netscape.net>
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

#ifndef SETUP_COLOURS_H
#define SETUP_COLOURS_H

#include <string>

#include <guichan/actionlistener.hpp>

#include <guichan/widgets/label.hpp>
#include <guichan/widgets/listbox.hpp>

#include "setuptab.h"
#include "textfield.h"

#include "../guichanfwd.h"

class BrowserBox;

class Setup_Colors : public SetupTab, public gcn::ActionListener,
                      public TextFieldListener
{
    public:
        Setup_Colors();
        ~Setup_Colors();
        void apply();
        void cancel();
        void action(const gcn::ActionEvent &event);

        void listen(const TextField *tf);
    private:
        gcn::ListBox *mcolorBox;
        gcn::ScrollArea *mScroll;
        BrowserBox *mPreview;
        gcn::ScrollArea *mPreviewBox;
        int mSelected;

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
};
#endif
