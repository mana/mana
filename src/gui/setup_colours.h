/***************************************************************************
 *   Copyright (C) 2008 by Douglas Boffey                                  *
 *                                                                         *
 *       DougABoffey@netscape.net                                          *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed with The Mana Experiment                  *
 *   in the hope that it will be useful,                                   *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/

#ifndef _SETUP_COLOURS_H
#define _SETUP_COLOURS_H

#include <string>
#include <vector>

#include <guichan/actionlistener.hpp>
#include <guichan/widgets/label.hpp>
#include <guichan/widgets/listbox.hpp>

#include "scrollarea.h"
#include "setuptab.h"
#include "slider.h"
#include "textfield.h"

#include "../guichanfwd.h"

class Setup_Colours : public SetupTab, public gcn::ActionListener,
                      public TextFieldListener
{
    public:
        Setup_Colours();
        ~Setup_Colours();
        void apply();
        void cancel();
        void action(const gcn::ActionEvent &event);

        void listen(const TextField *tf);
    private:
        gcn::ListBox *mColourBox;
        ScrollArea *mScroll;
        int mSelected;

        gcn::Label *mRedLabel;
        Slider *mRedSlider;
        TextField *mRedText;
        int mRedValue;

        gcn::Label *mGreenLabel;
        Slider *mGreenSlider;
        TextField *mGreenText;
        int mGreenValue;

        gcn::Label *mBlueLabel;
        Slider *mBlueSlider;
        TextField *mBlueText;
        int mBlueValue;

        void setEntry(Slider *s, TextField *t, int value);
        void updateColour();
};
#endif
