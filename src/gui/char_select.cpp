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
 *
 *  $Id$
 */

#include "../main.h"
#include "char_select.h"
#include "textfield.h"
#include "button.h"
#include "ok_dialog.h"
#include "../graphics.h"
#include "../net/protocol.h"
#include "../resources/resourcemanager.h"
#include <sstream>

#define NR_HAIR_STYLES 4
#define NR_HAIR_COLORS 10

CharSelectDialog::CharDeleteConfirm::CharDeleteConfirm(CharSelectDialog *m):
    ConfirmDialog(m,
            "Confirm", "Are you sure you want to delete this character?"),
    master(m)
{
}

void CharSelectDialog::CharDeleteConfirm::action(const std::string &eventId)
{
    ConfirmDialog::action(eventId);
    if (eventId == "yes") {
        master->serverCharDelete();
    }
}

CharSelectDialog::CharSelectDialog():
    Window("Select Character")
{
    selectButton = new Button("OK");
    cancelButton = new Button("Cancel");
    newCharButton = new Button("New");
    delCharButton = new Button("Delete");
    nameLabel = new gcn::Label("Name");
    levelLabel = new gcn::Label("Level");
    jobLevelLabel = new gcn::Label("Job Level");
    moneyLabel = new gcn::Label("Money");
    playerBox = new PlayerBox();

    selectButton->setEventId("ok");
    newCharButton->setEventId("new");
    cancelButton->setEventId("cancel");
    delCharButton->setEventId("delete");

    int w = 195;
    int h = 195;
    setContentSize(w, h);
    playerBox->setDimension(gcn::Rectangle(5, 5, w - 10, 90));
    nameLabel->setDimension(gcn::Rectangle(10, 100, 128, 16));
    levelLabel->setDimension(gcn::Rectangle(10, 116, 128, 16));
    jobLevelLabel->setDimension(gcn::Rectangle(10, 132, 128, 16));
    moneyLabel->setDimension(gcn::Rectangle(10, 148, 128, 16));
    newCharButton->setPosition(5, h - 5 - newCharButton->getHeight());
    delCharButton->setPosition(
            5 + newCharButton->getWidth() + 5,
            newCharButton->getY());
    cancelButton->setPosition(
            w - 5 - cancelButton->getWidth(),
            newCharButton->getY());
    selectButton->setPosition(
            cancelButton->getX() - 5 - selectButton->getWidth(),
            newCharButton->getY());

    add(playerBox);
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

    selectButton->requestFocus();
    setLocationRelativeTo(getParent());
    setPlayerInfo(NULL);
}

CharSelectDialog::~CharSelectDialog()
{
    delete selectButton;
    delete cancelButton;
    delete newCharButton;
    delete delCharButton;
    delete nameLabel;
    delete levelLabel;
    delete jobLevelLabel;
    delete moneyLabel;
    delete playerBox;
}

void CharSelectDialog::action(const std::string& eventId)
{
    if (eventId == "ok" && n_character > 0) {
        // Start game
        serverCharSelect();
        close_session();
    }
    else if (eventId == "cancel") {
        state = EXIT;
    }
    else if (eventId == "new") {
        if (n_character == 0) {
            // Start new character dialog
            new CharCreateDialog(this);
        }
    } else if (eventId == "delete") {
        // Delete character
        if (n_character > 0) {
            new CharDeleteConfirm(this);
        }
    }
}

void CharSelectDialog::setPlayerInfo(PLAYER_INFO *pi)
{
    if (pi) {
        std::stringstream nameCaption, levelCaption, jobCaption, moneyCaption;

        nameCaption << pi->name;
        levelCaption << "Lvl: " << pi->lv;
        jobCaption << "Job Lvl: " << pi->job_lv;
        moneyCaption << "Gold: " << pi->gp;

        nameLabel->setCaption(nameCaption.str());
        levelLabel->setCaption(levelCaption.str());
        jobLevelLabel->setCaption(jobCaption.str());
        moneyLabel->setCaption(moneyCaption.str());
        newCharButton->setEnabled(false);
        delCharButton->setEnabled(true);
        selectButton->setEnabled(true);

        playerBox->hairStyle = pi->hair_style - 1;
        playerBox->hairColor = pi->hair_color - 1;
        playerBox->showPlayer = true;
    }
    else {
        nameLabel->setCaption("Name");
        levelLabel->setCaption("Level");
        jobLevelLabel->setCaption("Job Level");
        moneyLabel->setCaption("Money");
        newCharButton->setEnabled(true);
        delCharButton->setEnabled(false);
        selectButton->setEnabled(false);

        playerBox->hairStyle = 0;
        playerBox->hairColor = 0;
        playerBox->showPlayer = false;
    }
}

void CharSelectDialog::serverCharDelete() {
    // Request character deletion
    WFIFOW(0) = net_w_value(0x0068);
    WFIFOL(2) = net_l_value(char_info->id);
    WFIFOSET(46);

    while ((in_size < 2) || (out_size > 0)) flush();
    if (RFIFOW(0) == 0x006f) {
        RFIFOSKIP(2);
        free(char_info);
        n_character = 0;
        setPlayerInfo(NULL);
        new OkDialog(this, "Info", "Player deleted");
    }
    else if (RFIFOW(0) == 0x0070) {
        new OkDialog(this, "Error", "Failed to delete character.");
        RFIFOSKIP(3);
    }
    else {
        new OkDialog(this, "Error", "Unknown");
    }
}

void CharSelectDialog::serverCharSelect()
{
    // Request character selection
    WFIFOW(0) = net_w_value(0x0066);
    WFIFOB(2) = net_b_value(0);
    WFIFOSET(3);

    while ((in_size < 3) || (out_size > 0)) {
        flush();
    }

    logger.log("CharSelect: Packet ID: %x, Length: %d, Packet_in_size %d",
            RFIFOW(0),
            get_length(RFIFOW(0)),
            RFIFOW(2));
    logger.log("CharSelect: In_size: %d", in_size);

    if (RFIFOW(0) == 0x0071) {
        while (in_size < 28) {
            flush();
        }
        char_ID = RFIFOL(2);
        memset(map_path, '\0', 480);
        strcat(map_path, "data/maps/");
        strncat(map_path, RFIFOP(6), 479 - strlen(map_path));
        map_address = RFIFOL(22);
        map_port = RFIFOW(26);
        state = GAME;

        logger.log("CharSelect: Map: %s", map_name);
        logger.log("CharSelect: Server: %s:%d", iptostring(map_address), map_port);
        RFIFOSKIP(28);
        close_session();
    }
    else if (RFIFOW(0) == 0x006c) {
        switch (RFIFOB(2)) {
            case 0:
                new OkDialog(this, "Error", "Access denied");
                break;
            case 1:
                new OkDialog(this, "Error", "Cannot use this ID");
                break;
        }
        RFIFOSKIP(3);
    }
    // Todo: add other packets
}

CharCreateDialog::CharCreateDialog(Window *parent):
    Window("Create Character", true, parent)
{
    nameField = new TextField("");
    nameLabel = new gcn::Label("Name:");
    nextHairColorButton = new Button(">");
    prevHairColorButton = new Button("<");
    hairColorLabel = new gcn::Label("Hair Color:");
    nextHairStyleButton = new Button(">");
    prevHairStyleButton = new Button("<");
    hairStyleLabel = new gcn::Label("Hair Style:");
    createButton = new Button("Create");
    cancelButton = new Button("Cancel");
    playerBox = new PlayerBox();
    playerBox->showPlayer = true;

    nameField->setEventId("create");
    nextHairColorButton->setEventId("nextcolor");
    prevHairColorButton->setEventId("prevcolor");
    nextHairStyleButton->setEventId("nextstyle");
    prevHairStyleButton->setEventId("prevstyle");
    createButton->setEventId("create");
    cancelButton->setEventId("cancel");

    int w = 200;
    int h = 150;
    setContentSize(w, h);
    playerBox->setDimension(gcn::Rectangle(80, 30, 110, 85));
    nameLabel->setPosition(5, 5);
    nameField->setDimension(
            gcn::Rectangle(45, 5, w - 45 - 7, nameField->getHeight()));
    prevHairColorButton->setPosition(90, 35);
    nextHairColorButton->setPosition(165, 35);
    hairColorLabel->setPosition(5, 40);
    prevHairStyleButton->setPosition(90, 64);
    nextHairStyleButton->setPosition(165, 64);
    hairStyleLabel->setPosition(5, 70);
    cancelButton->setPosition(
            w - 5 - cancelButton->getWidth(),
            h - 5 - cancelButton->getHeight());
    createButton->setPosition(
            cancelButton->getX() - 5 - createButton->getWidth(),
            h - 5 - cancelButton->getHeight());

    nameField->addActionListener(this);
    nextHairColorButton->addActionListener(this);
    prevHairColorButton->addActionListener(this);
    nextHairStyleButton->addActionListener(this);
    prevHairStyleButton->addActionListener(this);
    createButton->addActionListener(this);
    cancelButton->addActionListener(this);

    add(playerBox);
    add(nameField);
    add(nameLabel);
    add(nextHairColorButton);
    add(prevHairColorButton);
    add(hairColorLabel);
    add(nextHairStyleButton);
    add(prevHairStyleButton);
    add(hairStyleLabel);
    add(createButton);
    add(cancelButton);

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
    delete playerBox;
}

void CharCreateDialog::action(const std::string& eventId)
{
    if (eventId == "create") {
        if (getName().length() >= 4) {
            // Attempt to create the character
            serverCharCreate();
        }
        else {
            new OkDialog(this, "Error",
                    "Your name needs to be at least 4 characters.");
        }
    }
    else if (eventId == "cancel") {
        windowContainer->scheduleDelete(this);
    }
    else if (eventId == "nextcolor") {
        playerBox->hairColor++;
    }
    else if (eventId == "prevcolor") {
        playerBox->hairColor += NR_HAIR_COLORS - 1;
    }
    else if (eventId == "nextstyle") {
        playerBox->hairStyle++;
    }
    else if (eventId == "prevstyle") {
        playerBox->hairStyle += NR_HAIR_STYLES - 1;
    }

    playerBox->hairColor %= NR_HAIR_COLORS;
    playerBox->hairStyle %= NR_HAIR_STYLES;
}

std::string CharCreateDialog::getName() {
    return nameField->getText();
}

void CharCreateDialog::serverCharCreate()
{
    n_character = 1;

    WFIFOW(0) = net_w_value(0x0067);
    strcpy(WFIFOP(2), getName().c_str());
    WFIFOB(26) = net_b_value(5);
    WFIFOB(27) = net_b_value(5);
    WFIFOB(28) = net_b_value(5);
    WFIFOB(29) = net_b_value(5);
    WFIFOB(30) = net_b_value(5);
    WFIFOB(31) = net_b_value(5);
    WFIFOB(32) = net_b_value(0);
    WFIFOW(33) = net_w_value(playerBox->hairColor + 1);
    WFIFOW(35) = net_w_value(playerBox->hairStyle + 1);
    WFIFOSET(37);

    while ((in_size < 3) || (out_size > 0)) flush();
    if (RFIFOW(0) == 0x006d) {
        while (in_size < 108) flush();
        char_info = (PLAYER_INFO *)malloc(sizeof(PLAYER_INFO));
        char_info->id = RFIFOL(2);//account_ID;
        memset(char_info->name, '\0', 24);
        strcpy(char_info[0].name, RFIFOP(2 + 74));
        char_info->hp = RFIFOW(2 + 42);
        char_info->max_hp = RFIFOW(2 + 44);
        char_info->sp = RFIFOW(2 + 46);
        char_info->max_sp = RFIFOW(2 + 48);
        char_info->job_lv = RFIFOL(2 + 16);
        char_info->job_xp = RFIFOL(2 + 12);
        char_info->lv = RFIFOW(2 + 58);
        char_info->xp = RFIFOL(2 + 4);
        char_info->gp = RFIFOL(2 + 8);
        char_info->STR = RFIFOB(2 + 98);
        char_info->AGI = RFIFOB(2 + 99);
        char_info->VIT = RFIFOB(2 + 100);
        char_info->INT = RFIFOB(2 + 101);
        char_info->DEX = RFIFOB(2 + 102);
        char_info->LUK = RFIFOB(2 + 103);
        char_info->hair_style = RFIFOW(2 + 54);
        char_info->hair_color = RFIFOW(2 + 70);
        char_info->weapon = RFIFOW(2 + 56);
        RFIFOSKIP(108);
        //n_character++;
    } else if (RFIFOW(0) == 0x006e) {
        new OkDialog(this, "Error", "Failed to create character");
        RFIFOSKIP(3);
        n_character = 0;
    } else {
        new OkDialog(this, "Error", "Unknown error");
        n_character = 0;
    }

    // Remove window when succeeded
    if (n_character == 1) {
        windowContainer->scheduleDelete(this);
    }
}

void charSelect()
{
    CharSelectDialog *sel;
    sel = new CharSelectDialog();

    state = CHAR_SELECT;

    while (/*!key[KEY_ESC] && !key[KEY_ENTER] &&*/ state == CHAR_SELECT)
    {
        // Handle SDL events
        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            switch (event.type) {
                case SDL_QUIT:
                    state = EXIT;
                    break;

                case SDL_KEYDOWN:
                    if (event.key.keysym.sym == SDLK_ESCAPE)
                    {
                        state = EXIT;
                    }
                    break;
            }

            guiInput->pushInput(event);
        }

        if (n_character > 0) {
            sel->setPlayerInfo(char_info);
        }

        gui->logic();

        login_wallpaper->draw(screen, 0, 0);
        gui->draw();
        guiGraphics->updateScreen();
    }

    delete sel;
}

