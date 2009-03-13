/*
 *  Configurable text colors
 *  Copyright (C) 2008  Douglas Boffey <dougaboffey@netscape.net>
 *
 *  This file is part of Aethyra.
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

#include <string>
#include <cmath>

#include <guichan/listmodel.hpp>
#include <guichan/widgets/label.hpp>
#include <guichan/widgets/slider.hpp>

#include "browserbox.h"
#include "gui.h"
#include "itemlinkhandler.h"
#include "listbox.h"
#include "palette.h"
#include "scrollarea.h"
#include "setup_colors.h"
#include "slider.h"
#include "textfield.h"

#include "widgets/layouthelper.h"

#include "../configuration.h"

#include "../utils/gettext.h"
#include "../utils/stringutils.h"

const std::string Setup_Colors::rawmsg = _("This is what the color looks like");

Setup_Colors::Setup_Colors() :
    mSelected(-1)
{
    setOpaque(false);

    mColorBox = new ListBox(guiPalette);
    mColorBox->setActionEventId("color_box");
    mColorBox->addActionListener(this);

    mScroll = new ScrollArea(mColorBox);
    mScroll->setHorizontalScrollPolicy(gcn::ScrollArea::SHOW_NEVER);

    mTextPreview = new TextPreview(&rawmsg);

    mPreview = new BrowserBox(BrowserBox::AUTO_WRAP);
    mPreview->setOpaque(false);

    // Replace this later with a more appropriate link handler. For now, this'll
    // do, as it'll do nothing when clicked on.
    mPreview->setLinkHandler(new ItemLinkHandler());

    mPreviewBox = new ScrollArea(mPreview);
    mPreviewBox->setHeight(20);
    mPreviewBox->setScrollPolicy(gcn::ScrollArea::SHOW_NEVER,
                                 gcn::ScrollArea::SHOW_NEVER);

    mGradTypeLabel = new gcn::Label(_("Type: "));

    mGradTypeSlider = new Slider(0, 2);
    mGradTypeSlider->setWidth(160);
    mGradTypeSlider->setActionEventId("slider_grad");
    mGradTypeSlider->setValue(0);
    mGradTypeSlider->addActionListener(this);
    mGradTypeSlider->setEnabled(false);

    mGradTypeText = new gcn::Label();

    mRedLabel = new gcn::Label(_("Red: "));

    mRedText = new TextField();
    mRedText->setWidth(40);
    mRedText->setRange(0, 255);
    mRedText->setNumeric(true);
    mRedText->addListener(this);
    mRedText->setEnabled(false);

    mRedSlider = new Slider(0, 255);
    mRedSlider->setWidth(160);
    mRedSlider->setValue(mRedText->getValue());
    mRedSlider->setActionEventId("slider_red");
    mRedSlider->addActionListener(this);
    mRedSlider->setEnabled(false);

    mGreenLabel = new gcn::Label(_("Green: "));

    mGreenText = new TextField();
    mGreenText->setWidth(40);
    mGreenText->setRange(0, 255);
    mGreenText->setNumeric(true);
    mGreenText->addListener(this);
    mGreenText->setEnabled(false);

    mGreenSlider = new Slider(0, 255);
    mGreenSlider->setWidth(160);
    mGreenSlider->setValue(mGreenText->getValue());
    mGreenSlider->setActionEventId("slider_green");
    mGreenSlider->addActionListener(this);
    mGreenSlider->setEnabled(false);

    mBlueLabel = new gcn::Label(_("Blue: "));

    mBlueText = new TextField();
    mBlueText->setWidth(40);
    mBlueText->setRange(0, 255);
    mBlueText->setNumeric(true);
    mBlueText->addListener(this);
    mBlueText->setEnabled(false);

    mBlueSlider = new Slider(0, 255);
    mBlueSlider->setWidth(160);
    mBlueSlider->setValue(mBlueText->getValue());
    mBlueSlider->setActionEventId("slider_blue");
    mBlueSlider->addActionListener(this);
    mBlueSlider->setEnabled(false);

    setOpaque(false);

    // Do the layout
    LayoutHelper h(this);
    ContainerPlacer place = h.getPlacer(0, 0);

    place(0, 0, mScroll, 4, 6).setPadding(2);
    place(0, 6, mPreviewBox, 4).setPadding(2);
    place(0, 7, mGradTypeLabel, 2);
    place(2, 7, mGradTypeSlider);
    place(3, 7, mGradTypeText);
    place(0, 8, mRedLabel, 2);
    place(2, 8, mRedSlider);
    place(3, 8, mRedText).setPadding(1);
    place(0, 9, mGreenLabel, 2);
    place(2, 9, mGreenSlider);
    place(3, 9, mGreenText).setPadding(1);
    place(0, 10, mBlueLabel, 2);
    place(2, 10, mBlueSlider);
    place(3, 10, mBlueText).setPadding(1);

    setDimension(gcn::Rectangle(0, 0, 325, 280));
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
    if (event.getId() == "color_box")
    {
        mSelected = mColorBox->getSelected();
        Palette::ColorType type = guiPalette->getColorTypeAt(mSelected);
        const gcn::Color *col = &guiPalette->getColor(type);
        Palette::GradientType grad = guiPalette->getGradientType(type);

        std::string msg;
        char ch = guiPalette->getColorChar(type);

        mPreview->clearRows();
        mPreviewBox->setContent(mTextPreview);
        mTextPreview->setFont(gui->getFont());
        mTextPreview->setTextColor(
                &guiPalette->getColor(Palette::TEXT));
        mTextPreview->setTextBGColor(NULL);
         mTextPreview->setOpaque(false);
        mTextPreview->setShadow(true);
        mTextPreview->setOutline(true);

        switch (type)
        {
            case Palette::TEXT:
            case Palette::SHADOW:
            case Palette::OUTLINE:
                mTextPreview->setFont(gui->getFont());
                mTextPreview->setOutline(true);
                mTextPreview->setShadow(type == Palette::SHADOW);
                mTextPreview->setOutline(type == Palette::OUTLINE);
                break;
            case Palette::BACKGROUND:
            case Palette::HIGHLIGHT:
            case Palette::SHOP_WARNING:
                mTextPreview->setTextBGColor(col);
                //mTextPreview->setOpaque(true);
                mTextPreview->setOutline(false);
                mTextPreview->setShadow(false);
                mPreview->addRow(rawmsg);
                break;
            case Palette::CHAT:
            case Palette::GM:
            case Palette::PLAYER:
            case Palette::WHISPER:
            case Palette::IS:
            case Palette::PARTY:
            case Palette::SERVER:
            case Palette::LOGGER:
            case Palette::HYPERLINK:
                mPreviewBox->setContent(mPreview);
                mPreview->clearRows();

                if (ch == '<')
                {
                    msg = toString("@@|") + rawmsg + "@@";
                }
                else
                {
                    msg = "##" + toString(ch) + rawmsg;
                }
                mPreview->addRow(msg);
                break;
            case Palette::PARTICLE:
            case Palette::EXP_INFO:
            case Palette::PICKUP_INFO:
            case Palette::HIT_PLAYER_MONSTER:
            case Palette::HIT_MONSTER_PLAYER:
            case Palette::HIT_CRITICAL:
            case Palette::MISS:
                mTextPreview->setShadow(false);
            case Palette::BEING:
            case Palette::PC:
            case Palette::SELF:
            case Palette::GM_NAME:
            case Palette::NPC:
            case Palette::MONSTER:
                mTextPreview->setFont(boldFont);
                mTextPreview->setTextColor(col);
                break;
        }

        if (grad != Palette::STATIC)
        { // If nonstatic color, don't display the current, but the committed
          // color at the sliders
            col = &guiPalette->getCommittedColor(type);
        }

        setEntry(mRedSlider, mRedText, col->r);
        setEntry(mGreenSlider, mGreenText, col->g);
        setEntry(mBlueSlider, mBlueText, col->b);

        mGradTypeSlider->setValue(grad);
        updateGradType();
        mGradTypeSlider->setEnabled(true);

        return;
    }

    if (event.getId() == "slider_grad")
    {
        updateGradType();
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

void Setup_Colors::setEntry(gcn::Slider *s, TextField *t, int value)
{
    s->setValue(value);
    char buffer[100];
    sprintf(buffer, "%d", value);
    t->setText(buffer);
}

void Setup_Colors::apply()
{
    guiPalette->commit();
}

void Setup_Colors::cancel()
{
    guiPalette->rollback();
    Palette::ColorType type = guiPalette->getColorTypeAt(mSelected);
    const gcn::Color *col = &guiPalette->getColor(type);
    mGradTypeSlider->setValue(guiPalette->getGradientType(type));
    setEntry(mRedSlider, mRedText, col->r);
    setEntry(mGreenSlider, mGreenText, col->g);
    setEntry(mBlueSlider, mBlueText, col->b);
}

void Setup_Colors::listen(const TextField *tf)
{
    if (tf == mRedText)
    {
        mRedSlider->setValue(tf->getValue());
        updateColor();
        return;
    }
    if (tf == mGreenText)
    {
        mGreenSlider->setValue(tf->getValue());
        updateColor();
        return;
    }
    if (tf == mBlueText)
    {
        mBlueSlider->setValue(tf->getValue());
        updateColor();
        return;
    }
}

void Setup_Colors::updateGradType()
{
    if (mSelected == -1)
        return;

    mSelected = mColorBox->getSelected();
    Palette::ColorType type = guiPalette->getColorTypeAt(mSelected);
    Palette::GradientType grad = guiPalette->getGradientType(type);

    mGradTypeText->setCaption(
            (grad == Palette::STATIC) ? _("Static") :
            (grad == Palette::RAINBOW) ? _("Rainbow") : _("Spectrum"));

    bool enable = (grad == Palette::STATIC);
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

    Palette::ColorType type = guiPalette->getColorTypeAt(mSelected);
    Palette::GradientType grad =
            static_cast<Palette::GradientType>(mGradTypeSlider->getValue());
    guiPalette->setGradient(type, grad);

    if (grad == Palette::STATIC)
    {
        guiPalette->setColor(type,
                static_cast<int>(mRedSlider->getValue()),
                static_cast<int>(mGreenSlider->getValue()),
                static_cast<int>(mBlueSlider->getValue()));
    }
}
