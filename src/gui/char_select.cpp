/*
 *  The Mana World
 *  Copyright 2004 The Mana World Development Team
 *
 *  This file is part of The Mana World.
 *
 *  The Mana World is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  any later version.
 *
 *  The Mana World is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with The Mana World; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#include "char_select.h"
#include "../graphic/graphic.h"
#include "../graphic/2xsai.h"

CharSelectDialog::CharSelectDialog(gcn::Container *parent)
    : Window(parent, "Select Character")
{
    selectButton = new Button(" OK ");
    cancelButton = new Button("Cancel");
    newCharButton = new Button(" New ");
    delCharButton = new Button("Delete");
    nameLabel = new gcn::Label("Name");
    levelLabel = new gcn::Label("Level");
    jobLevelLabel = new gcn::Label("Job Level");
    moneyLabel = new gcn::Label("Money");

    nameLabel->setDimension(gcn::Rectangle(0, 0, 128, 16));
    levelLabel->setDimension(gcn::Rectangle(0, 0, 128, 16));
    jobLevelLabel->setDimension(gcn::Rectangle(0, 0, 128, 16));
    moneyLabel->setDimension(gcn::Rectangle(0, 0, 128, 16));

    selectButton->setEventId("ok");
    newCharButton->setEventId("new");
    cancelButton->setEventId("cancel");
    delCharButton->setEventId("delete");

    selectButton->setPosition(128, 180);
    cancelButton->setPosition(166, 180);
    newCharButton->setPosition(4, 180);
    delCharButton->setPosition(64, 180);
    nameLabel->setPosition(10, 100);
    levelLabel->setPosition(10, 116);
    jobLevelLabel->setPosition(10, 132);
    moneyLabel->setPosition(10, 148);

    add(selectButton);
    add(cancelButton);
    add(newCharButton);
    add(delCharButton);
    add(nameLabel);
    add(levelLabel);
    add(jobLevelLabel);
    add(moneyLabel);

    // Set up event listener
    selectButton->addActionListener(this);
    cancelButton->addActionListener(this);
    newCharButton->addActionListener(this);
    delCharButton->addActionListener(this);

    setSize(240, 216);
    setLocationRelativeTo(getParent());
}

CharSelectDialog::~CharSelectDialog()
{
    delete selectButton;
    delete cancelButton;
    delete newCharButton;
}

void CharSelectDialog::action(const std::string& eventId)
{
    if (eventId == "ok") {
        // Start game
        state = GAME;
    }
    else if (eventId == "cancel") {
        state = EXIT;
    }
    else if (eventId == "new") {
        // Create new character
        if (n_character > 0)
            return;
        // Start new character dialog...
        charCreate();
    } else if (eventId == "delete") {
        // Delete character
        if (n_character <= 0)
            return;
        serverCharDelete();
    }
}

#define MAX_HAIR_STYLE 3
#define MAX_HAIR_COLOR 9
int curHairColor = 0;
int curHairStyle = 0;
std::string curName;

CharCreateDialog::CharCreateDialog(gcn::Container *parent)
    : Window(parent, "Create Character")
{
    nameField = new gcn::TextField("");
    nameLabel = new gcn::Label("Name:");
    nextHairColorButton = new Button(">");
    prevHairColorButton = new Button("<");
    hairColorLabel = new gcn::Label("Hair Color:");
    nextHairStyleButton = new Button(">");
    prevHairStyleButton = new Button("<");
    hairStyleLabel = new gcn::Label("Hair Style:");
    createButton = new Button("Create");

    nextHairColorButton->setEventId("nextcolor");
    prevHairColorButton->setEventId("prevcolor");
    nextHairStyleButton->setEventId("nextstyle");
    prevHairStyleButton->setEventId("prevstyle");
    createButton->setEventId("create");

    nameField->setDimension(gcn::Rectangle(0, 0, 96, 16));

    nameField->setPosition(40, 4);
    nameLabel->setPosition(4, 4);
    nextHairColorButton->setPosition(160, 32);
    prevHairColorButton->setPosition(96, 32);
    hairColorLabel->setPosition(4, 34);
    nextHairStyleButton->setPosition(160, 64);
    prevHairStyleButton->setPosition(96, 64);
    hairStyleLabel->setPosition(4, 66);
    createButton->setPosition(125, 96);

    nextHairColorButton->addActionListener(this);
    prevHairColorButton->addActionListener(this);
    nextHairStyleButton->addActionListener(this);
    prevHairStyleButton->addActionListener(this);
    createButton->addActionListener(this);

    add(nameField);
    add(nameLabel);
    add(nextHairColorButton);
    add(prevHairColorButton);
    add(hairColorLabel);
    add(nextHairStyleButton);
    add(prevHairStyleButton);
    add(hairStyleLabel);
    add(createButton);

    setSize(200, 128);
    setLocationRelativeTo(getParent());
}

CharCreateDialog::~CharCreateDialog()
{
    delete nameField;
    delete nameLabel;
    delete nextHairColorButton;
    delete prevHairColorButton;
    delete hairColorLabel;
    delete nextHairStyleButton;
    delete prevHairStyleButton;
    delete hairStyleLabel;
}

void CharCreateDialog::action(const std::string& eventId)
{
    if (eventId == "create") {
        // Create character (used to exit create dialog loop)
        state = CHAR_SELECT;
    }
    else if (eventId == "nextcolor") {
        curHairColor++;
    }
    else if (eventId == "prevcolor") {
        curHairColor--;
    }
    else if (eventId == "nextstyle") {
        curHairStyle++;
    }
    else if (eventId == "prevstyle") {
        curHairStyle--;
    }

    if (curHairColor < 0)
        curHairColor = 0;
    if (curHairColor > MAX_HAIR_COLOR)
        curHairColor = MAX_HAIR_COLOR;
    if (curHairStyle < 0)
        curHairStyle = 0;
    if (curHairStyle > MAX_HAIR_STYLE)
        curHairStyle = MAX_HAIR_STYLE;
}


void charSelect()
{
    CharSelectDialog *sel;
    sel = new CharSelectDialog(guiTop);

    state = LOGIN;

    while (!key[KEY_ESC] && !key[KEY_ENTER] && state != EXIT && state == LOGIN)
    {
        if (n_character > 0) {
            char tmpBuf[64];
            sel->setName(char_info->name);
            sprintf(tmpBuf, "Lvl: %d", char_info->lv);
            sel->setLevel(tmpBuf);
            sprintf(tmpBuf, "Job Lvl: %d", char_info->job_lv);
            sel->setJobLevel(tmpBuf);
            sprintf(tmpBuf, "Gold: %d", char_info->gp);
            sel->setMoney(tmpBuf);
        }

        // Draw background
        blit((BITMAP *)graphic[LOGIN_BMP].dat, buffer, 0, 0, 0, 0, 800, 600);

        gui_update(NULL);

        // Draw character
        const int pX = 16, pY = 32;
        if (n_character > 0) {
            masked_blit(playerset, buffer, 0, 0,
                    pX + sel->getDimension().x,
                    pY + sel->getDimension().y, 80, 60);
            masked_blit(hairset, buffer,
                    20 * (char_info->hair_color - 1),
                    20 * 4 * (char_info->hair_style - 1),
                    pX + 31 + sel->getDimension().x,
                    pY + 15 + sel->getDimension().y, 20, 20);
        }

        // Draw to screen
        blit(buffer, screen, 0, 0, 0, 0, 800, 600);

    }

    if (state == GAME)
    {
        serverCharSelect();
        close_session();
    }

    delete sel;
}

void serverCharSelect()
{
    // Request character selection
    WFIFOW(0) = net_w_value(0x0066);
    WFIFOB(2) = net_b_value(0);
    WFIFOSET(3);

    while ((in_size < 3) || (out_size > 0)) {
        flush();
    }

    log("CharSelect", "Packet ID: %x, Length: %d, Packet_in_size %d",
            RFIFOW(0),
            get_length(RFIFOW(0)),
            RFIFOW(2));
    log("CharSelect", "In_size: %d", in_size);

    if (RFIFOW(0) == 0x0071) {
        while (in_size < 28) {
            flush();
        }
        char_ID = RFIFOL(2);
        memset(map_path, '\0', 480);
        append_filename(map_path, "./data/map/", RFIFOP(6), 480);
        map_address = RFIFOL(22);
        map_port = RFIFOW(26);
        state = GAME;

        log("CharSelect", "Map: %s", map_name);
        log("CharSelect", "Server: %s:%d", iptostring(map_address), map_port);
        RFIFOSKIP(28);
        close_session();
    } else if (RFIFOW(0) == 0x006c) {
        switch (RFIFOB(2)) {
            case 0:
                ok("Error", "Access denied");
                break;
            case 1:
                ok("Error", "Cannot use this ID");
                break;
        }
        state = CHAR_SELECT;
        RFIFOSKIP(3);
    }
    // Todo: add other packets
}

void charCreate()
{
    CharCreateDialog *create = new CharCreateDialog(guiTop);

    state = LOGIN;
    while (!key[KEY_ESC] && !key[KEY_ENTER] && state != EXIT &&
            state != CHAR_SELECT)
    {
        // Draw background
        blit((BITMAP *)graphic[LOGIN_BMP].dat, buffer, 0, 0, 0, 0, 800, 600);

        gui_update(NULL);

        // Draw character
        const int pX = 96, pY = 40;
        masked_blit(playerset, buffer, 0, 0,
                pX + create->getDimension().x,
                pY + create->getDimension().y, 80, 60);
        masked_blit(hairset, buffer, 20 * curHairColor, 20 * 4 * curHairStyle,
                pX + 31 + create->getDimension().x,
                pY + 15 + create->getDimension().y, 20, 20);

        // Draw to screen
        blit(buffer, screen, 0, 0, 0, 0, 800, 600);

        curName = create->getName();
    }

    if (state == CHAR_SELECT) {
        serverCharCreate();
    }

    delete create;
}

void serverCharDelete() {
    state = CHAR_SELECT;
    // Delete a character
    if (yes_no("Confirm", "Are you sure?")==0) {
        // Request character deletion
        WFIFOW(0) = net_w_value(0x0068);
        WFIFOL(2) = net_l_value(char_info->id);
        WFIFOSET(46);

        while ((in_size < 2) || (out_size > 0)) flush();
        if (RFIFOW(0) == 0x006f) {
            RFIFOSKIP(2);
            ok("Info", "Player deleted");
            free(char_info);
            n_character = 0;
        } else if (RFIFOW(0) == 0x006c) {
            switch (RFIFOB(2)) {
                case 0:
                    ok("Error", "Access denied");
                    break;
                case 1:
                    ok("Error", "Cannot use this ID");
                    break;
            }
            RFIFOSKIP(3);
        } else ok("Error", "Unknown error");
    }
}

void serverCharCreate()
{
    n_character = 1;

    WFIFOW(0) = net_w_value(0x0067);
    strcpy(WFIFOP(2), curName.c_str());
    WFIFOB(26) = net_b_value(5);
    WFIFOB(27) = net_b_value(5);
    WFIFOB(28) = net_b_value(5);
    WFIFOB(29) = net_b_value(5);
    WFIFOB(30) = net_b_value(5);
    WFIFOB(31) = net_b_value(5);
    WFIFOB(32) = net_b_value(0);
    WFIFOW(33) = net_w_value(curHairColor + 1);
    WFIFOW(35) = net_w_value(curHairStyle + 1);
    WFIFOSET(37);

    while ((in_size < 3) || (out_size > 0)) flush();
    if (RFIFOW(0) == 0x006d) {
        while (in_size < 108) flush();
        char_info = (PLAYER_INFO *)malloc(sizeof(PLAYER_INFO));
        char_info->id = RFIFOL(2);//account_ID;
        memset(char_info->name, '\0', 24);
        strcpy(char_info[0].name, RFIFOP(2+74));
        char_info->hp = RFIFOW(2+42);
        char_info->max_hp = RFIFOW(2+44);
        char_info->sp = RFIFOW(2+46);
        char_info->max_sp = RFIFOW(2+48);
        char_info->job_lv = RFIFOL(2+16);
        char_info->job_xp = RFIFOL(2+12);
        char_info->lv = RFIFOW(2+58);
        char_info->xp = RFIFOL(2+4);
        char_info->gp = RFIFOL(2+8);
        char_info->STR = RFIFOB(2+98);
        char_info->AGI = RFIFOB(2+99);
        char_info->VIT = RFIFOB(2+100);
        char_info->INT = RFIFOB(2+101);
        char_info->DEX = RFIFOB(2+102);
        char_info->LUK = RFIFOB(2+103);
        char_info->hair_style = RFIFOW(2+54);
        char_info->hair_color = RFIFOW(2+70);
        char_info->weapon = RFIFOW(2+56);
        RFIFOSKIP(108);
        //n_character++;
    } else if (RFIFOW(0) == 0x006c) {
        switch (RFIFOB(2)) {
            case 0:
                ok("Error", "Access denied");
                break;
            case 1:
                ok("Error", "Cannot use this ID");
                break;
        }
        RFIFOSKIP(3);
        n_character = 0;
    } else {
        ok("Error", "Unknown error");
        n_character = 0;
    }
}
