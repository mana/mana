/*
 *  The Mana World
 *  Copyright 2008 The Mana World Development Team
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

#include <map>

#include "gui/chat.h"
#include "statuseffect.h"

#include "log.h"
#include "utils/xml.h"


#define STATUS_EFFECTS_FILE "status-effects.xml"

StatusEffect::StatusEffect()
{}

StatusEffect::~StatusEffect()
{}

void StatusEffect::playSFX()
{
    if (mSFXEffect != "")
        sound.playSfx(mSFXEffect);
}

void StatusEffect::deliverMessage()
{
    if (mMessage != "")
        chatWindow->chatLog(mMessage, BY_SERVER);
}

Particle *StatusEffect::getParticle()
{
    if (mParticleEffect == "")
        return NULL;
    else
        return particleEngine->addEffect(mParticleEffect, 0, 0);
}

AnimatedSprite *StatusEffect::getIcon()
{
    if (mIcon == "")
        return NULL;
    else {
        AnimatedSprite *sprite = AnimatedSprite::load(
                "graphics/sprites/" + mIcon);
        if (false && sprite) {
            sprite->play(ACTION_DEFAULT);
            sprite->reset();
        }
        return sprite;
    }
}

SpriteAction StatusEffect::getAction()
{
    if (mAction == "")
        return ACTION_INVALID;
    else
        return SpriteDef::makeSpriteAction(mAction);
}


// -- initialisation and static parts --


typedef std::map<int, StatusEffect *> status_effect_map[2];

static status_effect_map statusEffects;
static status_effect_map stunEffects;
static std::map<int, int> blockEffectIndexMap;

int StatusEffect::blockEffectIndexToEffectIndex(int blockIndex)
{
    if (blockEffectIndexMap.find(blockIndex) == blockEffectIndexMap.end())
        return -1;
    return blockEffectIndexMap[blockIndex];
}

StatusEffect *StatusEffect::getStatusEffect(int index, bool enabling)
{
    load();
    return statusEffects[enabling][index];
}

StatusEffect *StatusEffect::getStunEffect(int index, bool enabling)
{
    load();
    return stunEffects[enabling][index];
}

static bool status_effects_loaded = false;



void StatusEffect::load()
{
    if (status_effects_loaded)
        return;

    status_effects_loaded = true;

    XML::Document doc(STATUS_EFFECTS_FILE);
    xmlNodePtr rootNode = doc.rootNode();

    if (!rootNode || !xmlStrEqual(rootNode->name, BAD_CAST "status-effects"))
    {
        logger->log("Error loading status effects file: "
                    STATUS_EFFECTS_FILE);
        return;
    }

    for_each_xml_child_node(node, rootNode)
    {
        status_effect_map *the_map = NULL;

        int index = atoi(XML::getProperty(node, "id", "-1").c_str());

        if (xmlStrEqual(node->name, BAD_CAST "status-effect"))
        {
            the_map = &statusEffects;
            int block_index = atoi(XML::getProperty(node, "block-id", "-1").c_str());

            if (index >= 0 && block_index >= 0)
                blockEffectIndexMap[block_index] = index;
        } else if (xmlStrEqual(node->name, BAD_CAST "stun-effect"))
            the_map = &stunEffects;

        if (the_map) {
            StatusEffect *startEffect = new StatusEffect();
            StatusEffect *endEffect = new StatusEffect();

            startEffect->mMessage = XML::getProperty(node, "start-message", "");
            startEffect->mSFXEffect = XML::getProperty(node, "start-audio", "");
            startEffect->mParticleEffect = XML::getProperty(node, "start-particle", "");
            startEffect->mIcon = XML::getProperty(node, "icon", "");
            startEffect->mAction = XML::getProperty(node, "action", "");

            endEffect->mMessage = XML::getProperty(node, "end-message", "");
            endEffect->mSFXEffect = XML::getProperty(node, "end-audio", "");
            endEffect->mParticleEffect = XML::getProperty(node, "end-particle", "");

            (*the_map)[1][index] = startEffect;
            (*the_map)[0][index] = endEffect;
        }
    }
}
