/*
 *  The Mana Client
 *  Copyright (c) 2009  Aethyra Development Team
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

#include <guichan/widgets/label.hpp>
#include <optional>

/**
 * Label widget. Same as the Guichan label but modified to use the palette
 * system and support outlines and shadows.
 *
 * \ingroup GUI
 */
class Label : public gcn::Label
{
    public:
        Label();

        /**
         * Constructor. This version of the constructor sets the label with an
         * inintialization string.
         */
        Label(const std::string &caption);

        /**
         * Sets the color of the outline.
         */
        void setOutlineColor(std::optional<gcn::Color> color);

        /**
         * Sets the color of the shadow.
         */
        void setShadowColor(std::optional<gcn::Color> color);

        /**
         * Draws the label.
         */
        void draw(gcn::Graphics *graphics) override;

    private:
        std::optional<gcn::Color> mOutlineColor;
        std::optional<gcn::Color> mShadowColor;
};

inline void Label::setOutlineColor(std::optional<gcn::Color> color)
{
    mOutlineColor = color;
}

inline void Label::setShadowColor(std::optional<gcn::Color> color)
{
    mShadowColor = color;
}
