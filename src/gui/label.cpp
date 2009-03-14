/*
 *  Aethyra
 *  Copyright (c) 2004 - 2008 Olof Naessén and Per Larsson
 *  Copyright (c) 2009  Aethyra Development Team 
 *
 *  This file is part of Aethyra based on original code
 *  from GUIChan.
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

#include <guichan/exception.hpp>
#include <guichan/font.hpp>

#include "gui.h"
#include "label.h"
#include "palette.h"

#include "../graphics.h"

Label::Label() :
    gcn::Label()
{
}

Label::Label(const std::string& caption) :
    gcn::Label(caption)
{
}

void Label::draw(gcn::Graphics* graphics)
{
    int textX;
    int textY = getHeight() / 2 - getFont()->getHeight() / 2;

    switch (getAlignment())
    {
        case gcn::Graphics::LEFT:
            textX = 0;
            break;
        case gcn::Graphics::CENTER:
            textX = getWidth() / 2;
            break;
        case gcn::Graphics::RIGHT:
            textX = getWidth();
            break;
        default:
            throw GCN_EXCEPTION("Unknown alignment.");
    }

    setForegroundColor(guiPalette->getColor(Palette::TEXT));

    graphics->setFont(getFont());
    graphics->setColor(getForegroundColor());
    graphics->drawText(getCaption(), textX, textY, getAlignment());
}
