/*
 *  The Mana World
 *  Copyright (C) 2004  The Mana World Development Team
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

#include "gui/magic.h"

#include "gui/widgets/button.h"

#include "localplayer.h"

#include "utils/dtor.h"
#include "utils/gettext.h"

MagicDialog::MagicDialog():
    Window(_("Magic"))
{
    setWindowName("Magic");
    setCloseButton(true);
    setSaveVisible(true);
    setDefaultSize(255, 30, 175, 225);

    mSpellButtons.resize(4);

    mSpellButtons[1] = new Button(_("Spell 1"), "spell_1", this);
    mSpellButtons[2] = new Button(_("Spell 2"), "spell_2", this);
    mSpellButtons[3] = new Button(_("Spell 3"), "spell_3", this);

    mSpellButtons[1]->setPosition(10, 60);
    mSpellButtons[2]->setPosition(10, 90);
    mSpellButtons[3]->setPosition(10, 120);

    add(mSpellButtons[1]);
    add(mSpellButtons[2]);
    add(mSpellButtons[3]);

    update();

    setLocationRelativeTo(getParent());
    loadWindowState();
}

MagicDialog::~MagicDialog()
{
}

void MagicDialog::action(const gcn::ActionEvent &event)
{
    if (event.getId() == "spell_1")
    {
        player_node->useSpecial(1);
    }
    if (event.getId() == "spell_2")
    {
        player_node->useSpecial(2);
    }
    if (event.getId() == "spell_3")
    {
        player_node->useSpecial(3);
    }
    else if (event.getId() == "close")
    {
        setVisible(false);
    }
}

void MagicDialog::draw(gcn::Graphics *g)
{
    update();

    Window::draw(g);
}

void MagicDialog::update()
{
    std::map<int, Special> specials = player_node->getSpecialStatus();

    for (size_t i = 1; i < mSpellButtons.size(); i++)
    {
        if (specials.find(i) != specials.end())
        {
            std::stringstream s;
            s <<
                "Spell" <<
                i <<
                " (" <<
                specials[i].currentMana <<
                "/" <<
                specials[i].neededMana <<
                ")";
            mSpellButtons[i]->setCaption(s.str());
            mSpellButtons[i]->adjustSize();
        }
    }
}
