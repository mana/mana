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

#include "char_select.h"

#include <sstream>
#include <string>
#include <SDL.h>

#include <guichan/widgets/label.hpp>

#include "button.h"
#include "ok_dialog.h"
#include "playerbox.h"
#include "textfield.h"
#include "windowcontainer.h"
#include "../being.h"
#include "../game.h"
#include "../log.h"
#include "../main.h"
#include "../playerinfo.h"

#include "../net/messagein.h"
#include "../net/messageout.h"
#include "../net/network.h"
#include "../net/protocol.h"


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
    Window("Select Character"), mStatus(0)
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

void CharSelectDialog::action(const std::string& eventId)
{
    if (eventId == "ok" && n_character > 0) {
        // Start game
        attemptCharSelect();
        mStatus = 1;
    }
    else if (eventId == "cancel") {
        state = EXIT_STATE;
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
        levelCaption << "Lvl: " << pi->lvl;
        jobCaption << "Job Lvl: " << pi->jobLvl;
        moneyCaption << "Gold: " << pi->gp;

        nameLabel->setCaption(nameCaption.str());
        levelLabel->setCaption(levelCaption.str());
        jobLevelLabel->setCaption(jobCaption.str());
        moneyLabel->setCaption(moneyCaption.str());
        newCharButton->setEnabled(false);
        delCharButton->setEnabled(true);
        selectButton->setEnabled(true);

        playerBox->hairStyle = pi->hairStyle - 1;
        playerBox->hairColor = pi->hairColor - 1;
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

void CharSelectDialog::serverCharDelete()
{
    // Request character deletion
    MessageOut outMsg;
    outMsg.writeInt16(0x0068);
    outMsg.writeInt32(char_info[0]->id);
    outMsg.writeString("a@a.com", 40);

    MessageIn msg = get_next_message();

    if (msg.getId() == 0x006f)
    {
        skip(msg.getLength());
        delete char_info[0];
        free(char_info);
        n_character = 0;
        setPlayerInfo(NULL);
        new OkDialog(this, "Info", "Player deleted");
    }
    else if (msg.getId() == 0x0070)
    {
        new OkDialog(this, "Error", "Failed to delete character.");
        skip(msg.getLength());
    }
    else {
        new OkDialog(this, "Error", "Unknown");
        skip(msg.getLength());
    }
}

void CharSelectDialog::attemptCharSelect()
{
    // Request character selection
    MessageOut outMsg;
    outMsg.writeInt16(0x0066);
    outMsg.writeInt8(0);
}

void
CharSelectDialog::checkCharSelect()
{
    // Receive reply
    MessageIn msg = get_next_message();
    if (state == ERROR_STATE)
    {
        return;
    }

    logger->log("CharSelect: Packet ID: %x, Length: %d, in_size: %d",
                msg.getId(), msg.getLength(), in_size);

    if (msg.getId() == 0x0071)
    {
        char_ID = msg.readInt32();
        map_path = "maps/" + msg.readString(16);
        map_path = map_path.substr(0, map_path.rfind(".")) + ".tmx.gz";
        map_address = msg.readInt32();
        map_port = msg.readInt16();
        player_info = char_info[0];
        state = CONNECTING_STATE;

        logger->log("CharSelect: Map: %s", map_path.c_str());
        logger->log("CharSelect: Server: %s:%i", iptostring(map_address),
                    map_port);
        closeConnection();
    }
    else if (msg.getId() == 0x006c)
    {
        switch (msg.readInt8()) {
            case 0:
                errorMessage = "Access denied";
                break;
            case 1:
                errorMessage = "Cannot use this ID";
                break;
            default:
                errorMessage = "Unknown failure to select character";
                break;
        }
        skip(msg.getLength());
    }
    else if (msg.getId() == 0x0081)
    {
        switch (msg.readInt8()) {
            case 1:
                errorMessage = "Map server offline";
                break;
            case 3:
                errorMessage = "Speed hack detected";
                break;
            case 8:
                errorMessage = "Duplicated login";
                break;
            default:
                errorMessage = "Unkown error with 0x0081";
                break;
        }
        closeConnection();
        state = ERROR_STATE;
    }

    // Todo: add other packets
}

void CharSelectDialog::logic()
{
    if (n_character > 0) {
        setPlayerInfo(char_info[0]);
    }
    
    if (mStatus == 1)
    {
        if (packetReady())
        {
            checkCharSelect();
        }
        else
        {
            flush();
        }
    }
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

std::string CharCreateDialog::getName()
{
    return nameField->getText();
}

void CharCreateDialog::serverCharCreate()
{
    // Send character infos
    MessageOut outMsg;
    outMsg.writeInt16(0x0067);
    outMsg.writeString(getName(), 24);
    outMsg.writeInt8(5);
    outMsg.writeInt8(5);
    outMsg.writeInt8(5);
    outMsg.writeInt8(5);
    outMsg.writeInt8(5);
    outMsg.writeInt8(5);
    outMsg.writeInt8(0);
    outMsg.writeInt16(playerBox->hairColor + 1);
    outMsg.writeInt16(playerBox->hairStyle + 1);

    MessageIn msg = get_next_message();

    if (msg.getId() == 0x006d)
    {
        char_info = (PLAYER_INFO**)malloc(sizeof(PLAYER_INFO*));
        char_info[0] = new PLAYER_INFO;

        char_info[0]->id = msg.readInt32();
        char_info[0]->xp = msg.readInt32();
        char_info[0]->gp = msg.readInt32();
        char_info[0]->jobXp = msg.readInt32();
        char_info[0]->jobLvl = msg.readInt32();
        msg.skip(8);                          // unknown
        msg.readInt32();                       // option
        msg.readInt32();                       // karma
        msg.readInt32();                       // manner
        msg.skip(2);                          // unknown
        char_info[0]->hp = msg.readInt16();
        char_info[0]->maxHp = msg.readInt16();
        char_info[0]->mp = msg.readInt16();
        char_info[0]->maxMp = msg.readInt16();
        msg.readInt16();                       // speed
        msg.readInt16();                       // class
        char_info[0]->hairStyle = msg.readInt16();
        char_info[0]->weapon = msg.readInt16();
        char_info[0]->lvl = msg.readInt16();
        msg.readInt16();                       // skill point
        msg.readInt16();                       // head bottom
        msg.readInt16();                       // shield
        msg.readInt16();                       // head option top
        msg.readInt16();                       // head option mid
        char_info[0]->hairColor = msg.readInt16();
        msg.readInt16();                       // unknown
        char_info[0]->name = msg.readString(24);
        char_info[0]->STR = msg.readInt8();
        char_info[0]->AGI = msg.readInt8();
        char_info[0]->VIT = msg.readInt8();
        char_info[0]->INT = msg.readInt8();
        char_info[0]->DEX = msg.readInt8();
        char_info[0]->LUK = msg.readInt8();
        char_info[0]->characterNumber = msg.readInt8(); // character number
        msg.readInt8();                        // unknown

        n_character = 1;
    }
    else if (msg.getId() == 0x006e)
    {
        new OkDialog(this, "Error", "Failed to create character");
        n_character = 0;
    }
    else
    {
        new OkDialog(this, "Error", "Unknown error");
        n_character = 0;
    }

    skip(msg.getLength());

    // Remove window when succeeded
    if (n_character == 1) {
        windowContainer->scheduleDelete(this);
    }
}

void charSelectInputHandler(SDL_KeyboardEvent *keyEvent)
{
    if (keyEvent->keysym.sym == SDLK_ESCAPE)
    {
        state = EXIT_STATE;
    }
}
