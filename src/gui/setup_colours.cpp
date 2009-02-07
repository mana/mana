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

#include <string>

#include <guichan/listmodel.hpp>
#include <guichan/widgets/label.hpp>
#include <guichan/widgets/slider.hpp>

#include "browserbox.h"
#include "colour.h"
#include "itemlinkhandler.h"
#include "listbox.h"
#include "scrollarea.h"
#include "setup_colours.h"
#include "slider.h"
#include "textfield.h"

#include "widgets/layouthelper.h"

#include "../configuration.h"

#include "../utils/gettext.h"
#include "../utils/tostring.h"

Setup_Colours::Setup_Colours() :
    mSelected(-1)
{
    setOpaque(false);

    mColourBox = new ListBox(textColour);
    mColourBox->setActionEventId("colour_box");
    mColourBox->addActionListener(this);

    mScroll = new ScrollArea(mColourBox);
    mScroll->setHorizontalScrollPolicy(gcn::ScrollArea::SHOW_NEVER);

    mPreview = new BrowserBox(BrowserBox::AUTO_WRAP);
    mPreview->setOpaque(false);

    // Replace this later with a more appropriate link handler. For now, this'll
    // do, as it'll do nothing when clicked on.
    mPreview->setLinkHandler(new ItemLinkHandler());

    mPreviewBox = new ScrollArea(mPreview);
    mPreviewBox->setHeight(20);
    mPreviewBox->setScrollPolicy(gcn::ScrollArea::SHOW_NEVER,
                                 gcn::ScrollArea::SHOW_NEVER);

    mRedLabel = new gcn::Label(_("Red: "));

    mRedText = new TextField();
    mRedText->setWidth(40);
    mRedText->setRange(0, 255);
    mRedText->setNumeric(true);
    mRedText->addListener(this);

    mRedSlider = new Slider(0, 255);
    mRedSlider->setWidth(160);
    mRedSlider->setValue(mRedText->getValue());
    mRedSlider->setActionEventId("slider_red");
    mRedSlider->addActionListener(this);

    mGreenLabel = new gcn::Label(_("Green: "));

    mGreenText = new TextField();
    mGreenText->setWidth(40);
    mGreenText->setRange(0, 255);
    mGreenText->setNumeric(true);
    mGreenText->addListener(this);

    mGreenSlider = new Slider(0, 255);
    mGreenSlider->setWidth(160);
    mGreenSlider->setValue(mGreenText->getValue());
    mGreenSlider->setActionEventId("slider_green");
    mGreenSlider->addActionListener(this);

    mBlueLabel = new gcn::Label(_("Blue: "));

    mBlueText = new TextField();
    mBlueText->setWidth(40);
    mBlueText->setRange(0, 255);
    mBlueText->setNumeric(true);
    mBlueText->addListener(this);

    mBlueSlider = new Slider(0, 255);
    mBlueSlider->setWidth(160);
    mBlueSlider->setValue(mBlueText->getValue());
    mBlueSlider->setActionEventId("slider_blue");
    mBlueSlider->addActionListener(this);

    setOpaque(false);

    // Do the layout
    LayoutHelper h(this);
    ContainerPlacer place = h.getPlacer(0, 0);

    place(0, 0, mScroll, 4, 7).setPadding(2);
    place(0, 7, mPreviewBox, 4).setPadding(2);
    place(0, 8, mRedLabel, 2);
    place(2, 8, mRedSlider);
    place(3, 8, mRedText).setPadding(1);
    place(0, 9, mGreenLabel, 2);
    place(2, 9, mGreenSlider);
    place(3, 9, mGreenText).setPadding(1);
    place(0, 10, mBlueLabel, 2);
    place(2, 10, mBlueSlider);
    place(3, 10, mBlueText).setPadding(1);

    setDimension(gcn::Rectangle(0, 0, 290, 250));
}

Setup_Colours::~Setup_Colours()
{
    delete mRedLabel;
    delete mRedSlider;
    delete mRedText;

    delete mGreenLabel;
    delete mGreenSlider;
    delete mGreenText;

    delete mBlueLabel;
    delete mBlueSlider;
    delete mBlueText;

    delete mScroll;
}

void Setup_Colours::action(const gcn::ActionEvent &event)
{
    if (event.getId() == "colour_box")
    {
        mSelected = mColourBox->getSelected();
        int col = textColour->getColourAt(mSelected);
        char ch = textColour->getColourCharAt(mSelected);
        std::string msg;

        if (ch == '<')
            msg = toString("@@|") + 
                  _("This is what the color looks like") + "@@";
        else
            msg = "##" + toString(ch) + 
                  _("This is what the color looks like");

        mPreview->clearRows();
        mPreview->addRow(msg);
        setEntry(mRedSlider, mRedText, col >> 16);
        setEntry(mGreenSlider, mGreenText, (col >> 8) & 0xff);
        setEntry(mBlueSlider, mBlueText, col & 0xff);
        return;
    }

    if (event.getId() == "slider_red")
    {
        char buffer[30];
        std::sprintf(buffer, "%d", static_cast<int>(mRedSlider->getValue()));
        mRedText->setText(buffer);
        updateColour();
        return;
    }

    if (event.getId() == "slider_green")
    {
        char buffer[30];
        std::sprintf(buffer, "%d", static_cast<int>(mGreenSlider->getValue()));
        mGreenText->setText(buffer);
        updateColour();
        return;
    }

    if (event.getId() == "slider_blue")
    {
        char buffer[30];
        std::sprintf(buffer, "%d", static_cast<int>(mBlueSlider->getValue()));
        mBlueText->setText(buffer);
        updateColour();
        return;
    }
}

void Setup_Colours::setEntry(gcn::Slider *s, TextField *t, int value)
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
    setEntry(mRedSlider, mRedText, col >> 16);
    setEntry(mGreenSlider, mGreenText, (col >> 8) & 0xff);
    setEntry(mBlueSlider, mBlueText, col & 0xff);
}

void Setup_Colours::listen(const TextField *tf)
{
    if (tf == mRedText)
    {
        mRedSlider->setValue(tf->getValue());
        updateColour();
        return;
    }
    if (tf == mGreenText)
    {
        mGreenSlider->setValue(tf->getValue());
        updateColour();
        return;
    }
    if (tf == mBlueText)
    {
        mBlueSlider->setValue(tf->getValue());
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
    int rgb = static_cast<int>(mRedSlider->getValue()) << 16 |
              static_cast<int>(mGreenSlider->getValue()) << 8 |
              static_cast<int>(mBlueSlider->getValue());
    textColour->setColourAt(mSelected, rgb);
}
