/*
 *  The Mana Client
 *  Copyright (C) 2006-2009  The Mana World Development Team
 *  Copyright (C) 2009-2026  The Mana Developers
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

#include "resources/particleeffectdef.h"

#include "log.h"
#include "particle.h"

#include "resources/dye.h"
#include "resources/image.h"
#include "resources/resourcemanager.h"

#include "utils/xml.h"

#define SIN45 0.707106781f
#define DEG_RAD_FACTOR 0.017453293f

template <typename T>
static ParticleEmitterProp<T> readParticleEmitterProp(XML::Node propertyNode,
                                                      T def)
{
    def = propertyNode.getFloatProperty("value", (double) def);
    const T min = (T) propertyNode.getFloatProperty("min", (double) def);
    const T max = (T) propertyNode.getFloatProperty("max", (double) def);

    ParticleEmitterProp<T> retval(min, max);

    std::string change = propertyNode.getProperty("change-func", "none");
    T amplitude = (T) propertyNode.getFloatProperty("change-amplitude", 0.0);
    int period = propertyNode.getProperty("change-period", 0);
    int phase = propertyNode.getProperty("change-phase", 0);
    if (change == "saw" || change == "sawtooth")
        retval.setFunction(FUNC_SAW, amplitude, period, phase);
    else if (change == "sine" || change == "sinewave")
        retval.setFunction(FUNC_SINE, amplitude, period, phase);
    else if (change == "triangle")
        retval.setFunction(FUNC_TRIANGLE, amplitude, period, phase);
    else if (change == "square")
        retval.setFunction(FUNC_SQUARE, amplitude, period, phase);

    return retval;
}

static DeathEffect readDeathEffect(XML::Node node)
{
    DeathEffect deathEffect;
    deathEffect.effect = node.textContent();

    if (node.getBoolProperty("on-floor", true))
        deathEffect.conditions |= Particle::DEAD_FLOOR;
    if (node.getBoolProperty("on-sky", true))
        deathEffect.conditions |= Particle::DEAD_SKY;
    if (node.getBoolProperty("on-other", false))
        deathEffect.conditions |= Particle::DEAD_OTHER;
    if (node.getBoolProperty("on-impact", true))
        deathEffect.conditions |= Particle::DEAD_IMPACT;
    if (node.getBoolProperty("on-timeout", true))
        deathEffect.conditions |= Particle::DEAD_TIMEOUT;

    return deathEffect;
}

static ResourceRef<Image> loadImage(std::string imagePath,
                                    const std::string &dyePalettes)
{
    if (imagePath.empty())
        return nullptr;

    Dye::instantiate(imagePath, dyePalettes);
    return ResourceManager::getInstance()->getImage(imagePath);
}

static ParticleEmitterDef readEmitter(XML::Node emitterNode,
                                      const std::string &dyePalettes)
{
    ParticleEmitterDef def;

    for (auto propertyNode : emitterNode.children())
    {
        if (propertyNode.name() == "property")
        {
            std::string name = propertyNode.getProperty("name", "");

            if (name == "position-x")
            {
                def.posX = readParticleEmitterProp(propertyNode, 0.0f);
            }
            else if (name == "position-y")
            {
                def.posY = readParticleEmitterProp(propertyNode, 0.0f);
                def.posY.minVal *= SIN45;
                def.posY.maxVal *= SIN45;
                def.posY.changeAmplitude *= SIN45;
            }
            else if (name == "position-z")
            {
                def.posZ = readParticleEmitterProp(propertyNode, 0.0f);
                def.posZ.minVal *= SIN45;
                def.posZ.maxVal *= SIN45;
                def.posZ.changeAmplitude *= SIN45;
            }
            else if (name == "image")
            {
                if (!def.image)
                {
                    def.image = loadImage(propertyNode.getProperty("value", ""),
                                          dyePalettes);
                }
            }
            else if (name == "horizontal-angle")
            {
                def.angleHorizontal = readParticleEmitterProp(propertyNode, 0.0f);
                def.angleHorizontal.minVal *= DEG_RAD_FACTOR;
                def.angleHorizontal.maxVal *= DEG_RAD_FACTOR;
                def.angleHorizontal.changeAmplitude *= DEG_RAD_FACTOR;
            }
            else if (name == "vertical-angle")
            {
                def.angleVertical = readParticleEmitterProp(propertyNode, 0.0f);
                def.angleVertical.minVal *= DEG_RAD_FACTOR;
                def.angleVertical.maxVal *= DEG_RAD_FACTOR;
                def.angleVertical.changeAmplitude *= DEG_RAD_FACTOR;
            }
            else if (name == "power")
            {
                def.power = readParticleEmitterProp(propertyNode, 0.0f);
            }
            else if (name == "gravity")
            {
                def.gravity = readParticleEmitterProp(propertyNode, 0.0f);
            }
            else if (name == "randomnes" || name == "randomness") // legacy bug
            {
                def.randomness = readParticleEmitterProp(propertyNode, 0);
            }
            else if (name == "bounce")
            {
                def.bounce = readParticleEmitterProp(propertyNode, 0.0f);
            }
            else if (name == "lifetime")
            {
                def.lifetime = readParticleEmitterProp(propertyNode, 0);
                def.lifetime.minVal += 1;
            }
            else if (name == "output")
            {
                def.output = readParticleEmitterProp(propertyNode, 0);
                def.output.maxVal += 1;
            }
            else if (name == "output-pause")
            {
                def.outputPause = readParticleEmitterProp(propertyNode, 0);
            }
            else if (name == "acceleration")
            {
                def.acceleration = readParticleEmitterProp(propertyNode, 0.0f);
            }
            else if (name == "die-distance")
            {
                def.dieDistance = readParticleEmitterProp(propertyNode, 0.0f);
            }
            else if (name == "momentum")
            {
                def.momentum = readParticleEmitterProp(propertyNode, 1.0f);
            }
            else if (name == "fade-out")
            {
                def.fadeOut = readParticleEmitterProp(propertyNode, 0);
            }
            else if (name == "fade-in")
            {
                def.fadeIn = readParticleEmitterProp(propertyNode, 0);
            }
            else if (name == "alpha")
            {
                def.alpha = readParticleEmitterProp(propertyNode, 1.0f);
            }
            else if (name == "follow-parent")
            {
                def.follow = propertyNode.getBoolProperty("value", true);
            }
            else
            {
                Log::info("Particle Engine: Warning, unknown emitter property \"%s\"",
                          name.c_str());
            }
        }
        else if (propertyNode.name() == "emitter")
        {
            def.emitters.push_back(readEmitter(propertyNode, dyePalettes));
        }
        else if (propertyNode.name() == "rotation")
        {
            def.rotation = Animation::fromXML(propertyNode, dyePalettes);
        }
        else if (propertyNode.name() == "animation")
        {
            def.animation = Animation::fromXML(propertyNode, dyePalettes);
        }
        else if (propertyNode.name() == "deatheffect")
        {
            def.deathEffect = readDeathEffect(propertyNode);
        }
    }

    return def;
}

static ParticleDef readParticle(XML::Node particleNode,
                                const std::string &dyePalettes)
{
    ParticleDef def;

    def.offset.x = particleNode.getFloatProperty("position-x", 0);
    def.offset.y = particleNode.getFloatProperty("position-y", 0);
    def.offset.z = particleNode.getFloatProperty("position-z", 0);
    def.lifetime = particleNode.getProperty("lifetime", -1);
    def.sizeAdjustable = "false" != particleNode.getProperty("size-adjustable", "false");

    for (auto childNode : particleNode.children())
    {
        if (childNode.name() == "image")
            def.image = loadImage(std::string(childNode.textContent()), dyePalettes);
        else if (childNode.name() == "animation")
            def.animation = Animation::fromXML(childNode, dyePalettes);
        else if (childNode.name() == "rotation")
            def.rotation = Animation::fromXML(childNode, dyePalettes);
        else if (childNode.name() == "emitter")
            def.emitters.push_back(readEmitter(childNode, dyePalettes));
        else if (childNode.name() == "deatheffect")
            def.deathEffect = readDeathEffect(childNode);
    }

    return def;
}

ParticleEffectDef *ParticleEffectDef::load(const std::string &effectFile)
{
    std::string::size_type pos = effectFile.find('|');
    std::string dyePalettes;
    if (pos != std::string::npos)
        dyePalettes = effectFile.substr(pos + 1);

    XML::Document doc(effectFile.substr(0, pos));
    XML::Node rootNode = doc.rootNode();

    if (!rootNode || rootNode.name() != "effect")
    {
        Log::info("Error loading particle: %s", effectFile.c_str());
        return nullptr;
    }

    auto *effect = new ParticleEffectDef;

    for (auto particleNode : rootNode.children())
    {
        if (particleNode.name() == "particle")
            effect->mParticles.push_back(readParticle(particleNode, dyePalettes));
    }

    return effect;
}
