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

#include <vector>

#include <guichan/listmodel.hpp>
#include <guichan/widgets/label.hpp>
#include <guichan/widgets/slider.hpp>

#include "colour.h"
#include "scrollarea.h"
#include "setup_colours.h"
#include "slider.h"

#include "../configuration.h"

#include "../utils/gettext.h"

Setup_Colours::Setup_Colours() :
    mColourLabel(_("Color:")),
    mSelected(-1)
{
    mColourBox = new gcn::ListBox(textColour);
    mScroll = new ScrollArea(mColourBox);

    mColourLabel.setX(5);
    mColourLabel.setY(5);

    mColourBox->setDimension(gcn::Rectangle(0, 10 + mColourLabel.getHeight(),
                                            80,
                                            115 - mColourLabel.getHeight()));
    mScroll->setDimension(gcn::Rectangle(5, 10 + mColourLabel.getHeight(),
                                         100, 115 - mColourLabel.getHeight()));
    mColourBox->setSelected(-1);
    mColourBox->setActionEventId("colour_box");
    mColourBox->addActionListener(this);

    setOpaque(false);

    add(&mColourLabel);
    add(mScroll);

    setupPlacer(140, mLabel1, mSlider1, mText1, "R", "1");
    setupPlacer(165, mLabel2, mSlider2, mText2, "G", "2");
    setupPlacer(190, mLabel3, mSlider3, mText3, "B", "3");

}

Setup_Colours::~Setup_Colours()
{
    delete mLabel1;
    delete mSlider1;
    delete mText1;

    delete mLabel2;
    delete mSlider2;
    delete mText2;

    delete mLabel3;
    delete mSlider3;
    delete mText3;

    delete mScroll;
}

void Setup_Colours::action(const gcn::ActionEvent &event)
{
    if (event.getId() == "colour_box")
    {
        mSelected = mColourBox->getSelected();
        int col = textColour->getColourAt(mSelected);
        setEntry(mSlider1, mText1, col >> 16);
        setEntry(mSlider2, mText2, (col >> 8) & 0xff);
        setEntry(mSlider3, mText3, col & 0xff);
        return;
    }

    if (event.getId() == "slider1")
    {
        char buffer[30];
        std::sprintf(buffer, "%d", static_cast<int>(mSlider1->getValue()));
        mText1->setText(buffer);
        updateColour();
        return;
    }

    if (event.getId() == "slider2")
    {
        char buffer[30];
        std::sprintf(buffer, "%d", static_cast<int>(mSlider2->getValue()));
        mText2->setText(buffer);
        updateColour();
        return;
    }

    if (event.getId() == "slider3")
    {
        char buffer[30];
        std::sprintf(buffer, "%d", static_cast<int>(mSlider3->getValue()));
        mText3->setText(buffer);
        updateColour();
        return;
    }
}

void Setup_Colours::setEntry(Slider *s, TextField *t, int value)
{
    s->setValue(value);
    char buffer[100];
    sprintf(buffer, "%d", value);
    t->setText(buffer);
}

void Setup_Colours::apply()
{
    textColour->commit();
}

void Setup_Colours::cancel()
{
    textColour->rollback();
    int col = textColour->getColourAt(mSelected);
    setEntry(mSlider1, mText1, col >> 16);
    setEntry(mSlider2, mText2, (col >> 8) & 0xff);
    setEntry(mSlider3, mText3, col & 0xff);
}

void Setup_Colours::setupPlacer(int v, gcn::Label *&l, Slider *&s,
                                TextField *&t, std::string lbl,
                                std::string sfx)
{
    l = new gcn::Label(lbl + ":");
    l->setX(5);
    l->setY(v - l->getHeight() / 2);

    s = new Slider(0, 255);
    s->setHeight(10);
    s->setX(25);
    s->setY(v - s->getHeight() / 2);
    s->setWidth(128);
    s->setScale(0, 255);

    t = new TextField();
    t->setX(165);
    t->setY(v - t->getHeight() / 2);
    t->setWidth(40);
    t->setNumeric(true);
    t->setRange(0, 255);
    t->addListener(this);

    s->setActionEventId("slider" + sfx);
    s->addActionListener(this);

    add(l);
    add(s);
    add(t);
}

void Setup_Colours::listen(const TextField *tf)
{
    if (tf == mText1)
    {
        mSlider1->setValue(tf->getValue());
        updateColour();
        return;
    }
    if (tf == mText2)
    {
        mSlider2->setValue(tf->getValue());
        updateColour();
        return;
    }
    if (tf == mText3)
    {
        mSlider3->setValue(tf->getValue());
        updateColour();
        return;
    }
}

void Setup_Colours::updateColour()
{
    if (mSelected == -1)
    {
        return;
    }
    int rgb = static_cast<int>(mSlider1->getValue()) << 16 |
              static_cast<int>(mSlider2->getValue()) << 8 |
              static_cast<int>(mSlider3->getValue());
    textColour->setColourAt(mSelected, rgb);
}
