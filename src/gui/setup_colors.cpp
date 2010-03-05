/*
 *  Configurable text colors
 *  Copyright (C) 2008  Douglas Boffey <dougaboffey@netscape.net>
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

#include "gui/setup_colors.h"

#include "configuration.h"

#include "gui/gui.h"
#include "gui/theme.h"
#include "gui/userpalette.h"

#include "gui/widgets/browserbox.h"
#include "gui/widgets/itemlinkhandler.h"
#include "gui/widgets/label.h"
#include "gui/widgets/layouthelper.h"
#include "gui/widgets/listbox.h"
#include "gui/widgets/scrollarea.h"
#include "gui/widgets/slider.h"
#include "gui/widgets/textfield.h"
#include "gui/widgets/textpreview.h"

#include "utils/gettext.h"
#include "utils/stringutils.h"

#include <string>
#include <cmath>

const std::string Setup_Colors::rawmsg = _("This is what the color looks like");

Setup_Colors::Setup_Colors() :
    mSelected(-1)
{
    setName(_("Colors"));

    mColorBox = new ListBox(userPalette);
    mColorBox->addSelectionListener(this);

    mScroll = new ScrollArea(mColorBox);
    mScroll->setHorizontalScrollPolicy(gcn::ScrollArea::SHOW_NEVER);

    mTextPreview = new TextPreview(rawmsg);

    mPreview = new BrowserBox(BrowserBox::AUTO_WRAP);
    mPreview->setOpaque(false);

    // don't do anything with links
    mPreview->setLinkHandler(NULL);

    mPreviewBox = new ScrollArea(mPreview);
    mPreviewBox->setHeight(20);
    mPreviewBox->setScrollPolicy(gcn::ScrollArea::SHOW_NEVER,
                                 gcn::ScrollArea::SHOW_NEVER);

    mGradTypeLabel = new Label(_("Type:"));

    mGradTypeSlider = new Slider(0, 3);
    mGradTypeSlider->setWidth(180);
    mGradTypeSlider->setActionEventId("slider_grad");
    mGradTypeSlider->setValue(0);
    mGradTypeSlider->addActionListener(this);
    mGradTypeSlider->setEnabled(false);

    mGradTypeText = new Label;

    std::string longText = _("Static");

    if (getFont()->getWidth(_("Pulse")) > getFont()->getWidth(longText))
        longText = _("Pulse");
    if (getFont()->getWidth(_("Rainbow")) > getFont()->getWidth(longText))
        longText = _("Rainbow");
    if (getFont()->getWidth(_("Spectrum")) > getFont()->getWidth(longText))
        longText = _("Spectrum");

    mGradTypeText->setCaption(longText);

    mGradDelayLabel = new Label(_("Delay:"));

    mGradDelayText = new TextField();
    mGradDelayText->setWidth(40);
    mGradDelayText->setRange(20, 100);
    mGradDelayText->setNumeric(true);
    mGradDelayText->setEnabled(false);

    mGradDelaySlider = new Slider(20, 100);
    mGradDelaySlider->setWidth(180);
    mGradDelaySlider->setValue(mGradDelayText->getValue());
    mGradDelaySlider->setActionEventId("slider_graddelay");
    mGradDelaySlider->addActionListener(this);
    mGradDelaySlider->setEnabled(false);

    mRedLabel = new Label(_("Red:"));

    mRedText = new TextField;
    mRedText->setWidth(40);
    mRedText->setRange(0, 255);
    mRedText->setNumeric(true);
    mRedText->setEnabled(false);

    mRedSlider = new Slider(0, 255);
    mRedSlider->setWidth(180);
    mRedSlider->setValue(mRedText->getValue());
    mRedSlider->setActionEventId("slider_red");
    mRedSlider->addActionListener(this);
    mRedSlider->setEnabled(false);

    mGreenLabel = new Label(_("Green:"));

    mGreenText = new TextField;
    mGreenText->setWidth(40);
    mGreenText->setRange(0, 255);
    mGreenText->setNumeric(true);
    mGreenText->setEnabled(false);

    mGreenSlider = new Slider(0, 255);
    mGreenSlider->setWidth(180);
    mGreenSlider->setValue(mGreenText->getValue());
    mGreenSlider->setActionEventId("slider_green");
    mGreenSlider->addActionListener(this);
    mGreenSlider->setEnabled(false);

    mBlueLabel = new Label(_("Blue:"));

    mBlueText = new TextField;
    mBlueText->setWidth(40);
    mBlueText->setRange(0, 255);
    mBlueText->setNumeric(true);
    mBlueText->setEnabled(false);

    mBlueSlider = new Slider(0, 255);
    mBlueSlider->setWidth(180);
    mBlueSlider->setValue(mBlueText->getValue());
    mBlueSlider->setActionEventId("slider_blue");
    mBlueSlider->addActionListener(this);
    mBlueSlider->setEnabled(false);

    setOpaque(false);

    // Do the layout
    LayoutHelper h(this);
    ContainerPlacer place = h.getPlacer(0, 0);

    place(0, 0, mScroll, 6, 6).setPadding(2);
    place(0, 6, mPreviewBox, 6).setPadding(2);
    place(0, 7, mGradTypeLabel, 3);
    place(3, 7, mGradTypeSlider);
    place(4, 7, mGradTypeText, 2).setPadding(1);
    place(0, 8, mRedLabel, 3);
    place(3, 8, mRedSlider);
    place(5, 8, mRedText).setPadding(1);
    place(0, 9, mGreenLabel, 3);
    place(3, 9, mGreenSlider);
    place(5, 9, mGreenText).setPadding(1);
    place(0, 10, mBlueLabel, 3);
    place(3, 10, mBlueSlider);
    place(5, 10, mBlueText).setPadding(1);
    place(0, 11, mGradDelayLabel, 3);
    place(3, 11, mGradDelaySlider);
    place(5, 11, mGradDelayText).setPadding(1);

    mGradTypeText->setCaption("");

    setDimension(gcn::Rectangle(0, 0, 365, 280));
}

Setup_Colors::~Setup_Colors()
{
    if (mPreviewBox->getContent() == mPreview)
        delete mTextPreview;
    else
        delete mPreview;
}

void Setup_Colors::action(const gcn::ActionEvent &event)
{
    if (event.getId() == "slider_grad")
    {
        updateGradType();
        updateColor();
        return;
    }

    if (event.getId() == "slider_graddelay")
    {
        mGradDelayText->setText(toString(std::floor(mGradDelaySlider->getValue())));
        updateColor();
        return;
    }

    if (event.getId() == "slider_red")
    {
        mRedText->setText(toString(std::floor(mRedSlider->getValue())));
        updateColor();
        return;
    }

    if (event.getId() == "slider_green")
    {
        mGreenText->setText(toString(std::floor(mGreenSlider->getValue())));
        updateColor();
        return;
    }

    if (event.getId() == "slider_blue")
    {
        mBlueText->setText(toString(std::floor(mBlueSlider->getValue())));
        updateColor();
        return;
    }
}

void Setup_Colors::valueChanged(const gcn::SelectionEvent &event)
{
    mSelected = mColorBox->getSelected();
    int type = userPalette->getColorTypeAt(mSelected);
    const gcn::Color *col = &userPalette->getColor(type);
    Palette::GradientType grad = userPalette->getGradientType(type);
    const int delay = userPalette->getGradientDelay(type);

    mPreview->clearRows();
    mPreviewBox->setContent(mTextPreview);
    mTextPreview->setFont(boldFont);
    mTextPreview->setTextColor(col);
    mTextPreview->setTextBGColor(NULL);
    mTextPreview->setOpaque(false);
    mTextPreview->setShadow(true);
    mTextPreview->setOutline(true);
    mTextPreview->useTextAlpha(false);

    switch (type)
    {
        case UserPalette::PARTICLE:
        case UserPalette::EXP_INFO:
        case UserPalette::PICKUP_INFO:
        case UserPalette::HIT_PLAYER_MONSTER:
        case UserPalette::HIT_MONSTER_PLAYER:
        case UserPalette::HIT_CRITICAL:
        case UserPalette::MISS:
            mTextPreview->setShadow(false);
    }

    if (grad != Palette::STATIC && grad != Palette::PULSE)
    { // If nonstatic color, don't display the current, but the committed
      // color at the sliders
        col = &userPalette->getCommittedColor(type);
    }
    else if (grad == Palette::PULSE)
    {
        col = &userPalette->getTestColor(type);
    }

    setEntry(mGradDelaySlider, mGradDelayText, delay);
    setEntry(mRedSlider, mRedText, col->r);
    setEntry(mGreenSlider, mGreenText, col->g);
    setEntry(mBlueSlider, mBlueText, col->b);

    mGradTypeSlider->setValue(grad);
    updateGradType();
    mGradTypeSlider->setEnabled(true);
}

void Setup_Colors::setEntry(gcn::Slider *s, TextField *t, int value)
{
    s->setValue(value);
    char buffer[100];
    sprintf(buffer, "%d", value);
    t->setText(buffer);
}

void Setup_Colors::apply()
{
    userPalette->commit();
}

void Setup_Colors::cancel()
{
    userPalette->rollback();
    int type = userPalette->getColorTypeAt(mSelected);
    const gcn::Color *col = &userPalette->getColor(type);
    mGradTypeSlider->setValue(userPalette->getGradientType(type));
    const int delay = userPalette->getGradientDelay(type);
    setEntry(mGradDelaySlider, mGradDelayText, delay);
    setEntry(mRedSlider, mRedText, col->r);
    setEntry(mGreenSlider, mGreenText, col->g);
    setEntry(mBlueSlider, mBlueText, col->b);
}

#if 0
void Setup_Colors::listen(const TextField *tf)
{
    if (tf == mGradDelayText)
        mGradDelaySlider->setValue(tf->getValue());
    else if (tf == mRedText)
        mRedSlider->setValue(tf->getValue());
    else if (tf == mGreenText)
        mGreenSlider->setValue(tf->getValue());
    else if (tf == mBlueText)
        mBlueSlider->setValue(tf->getValue());

    updateColor();
}
#endif

void Setup_Colors::updateGradType()
{
    if (mSelected == -1)
        return;

    mSelected = mColorBox->getSelected();
    int type = userPalette->getColorTypeAt(mSelected);
    Palette::GradientType grad = userPalette->getGradientType(type);

    mGradTypeText->setCaption(
            (grad == Palette::STATIC) ? _("Static") :
            (grad == Palette::PULSE) ? _("Pulse") :
            (grad == Palette::RAINBOW) ? _("Rainbow") : _("Spectrum"));

    const bool enable = (grad == Palette::STATIC || grad == Palette::PULSE);
    const bool delayEnable = (grad != Palette::STATIC);

    mGradDelayText->setEnabled(delayEnable);
    mGradDelaySlider->setEnabled(delayEnable);

    mRedText->setEnabled(enable);
    mRedSlider->setEnabled(enable);
    mGreenText->setEnabled(enable);
    mGreenSlider->setEnabled(enable);
    mBlueText->setEnabled(enable);
    mBlueSlider->setEnabled(enable);
}

void Setup_Colors::updateColor()
{
    if (mSelected == -1)
        return;

    int type = userPalette->getColorTypeAt(mSelected);
    Palette::GradientType grad = static_cast<Palette::GradientType>((int)
                                                mGradTypeSlider->getValue());
    int delay = (int) mGradDelaySlider->getValue();
    userPalette->setGradient(type, grad);
    userPalette->setGradientDelay(type, delay);

    if (grad == Palette::STATIC)
    {
        userPalette->setColor(type,
                static_cast<int>(mRedSlider->getValue()),
                static_cast<int>(mGreenSlider->getValue()),
                static_cast<int>(mBlueSlider->getValue()));
    }
    else if (grad == Palette::PULSE)
    {
        userPalette->setTestColor(type, gcn::Color(
                static_cast<int>(mRedSlider->getValue()),
                static_cast<int>(mGreenSlider->getValue()),
                static_cast<int>(mBlueSlider->getValue())));
    }
}
