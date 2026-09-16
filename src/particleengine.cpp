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

#include "particleengine.h"

#include "textparticle.h"

#include "resources/particleeffectdef.h"
#include "resources/resourcemanager.h"

#include "utils/dtor.h"

ParticleEngine *particleEngine = nullptr;

ParticleEngine::~ParticleEngine()
{
    clear();
}

void ParticleEngine::update()
{
    if (!mMap)
        return;

    Particle::updateParticles(mParticles, Vector());
}

void ParticleEngine::clear()
{
    delete_all(mParticles);
    mParticles.clear();
}

Particle *ParticleEngine::createParticle()
{
    auto *newParticle = new Particle;
    newParticle->setMap(mMap);
    mParticles.push_back(newParticle);
    return newParticle;
}

Particle *ParticleEngine::addEffect(const std::string &particleEffectFile,
                                    int pixelX, int pixelY, int rotation)
{
    auto effect = ResourceManager::getInstance()->getParticleEffect(particleEffectFile);
    if (!effect)
        return nullptr;

    Particle *newParticle = nullptr;

    for (const ParticleDef &def : effect->particles())
    {
        newParticle = Particle::create(effect, def, mMap, rotation);
        newParticle->moveTo(Vector((float)pixelX + def.offset.x,
                                   (float)pixelY + def.offset.y,
                                   def.offset.z));
        newParticle->setLifetime(def.lifetime);
        newParticle->setAllowSizeAdjust(def.sizeAdjustable);

        mParticles.push_back(newParticle);
    }

    return newParticle;
}

Particle *ParticleEngine::addTextSplashEffect(const std::string &text,
                                              int x, int y,
                                              const gcn::Color *color,
                                              gcn::Font *font, bool outline)
{
    Particle *newParticle = new TextParticle(text, color, font, outline);
    newParticle->setMap(mMap);
    newParticle->moveTo(x, y);
    newParticle->setVelocity(((rand() % 100) - 50) / 200.0f,    // X
                             ((rand() % 100) - 50) / 200.0f,    // Y
                             ((rand() % 100) / 200.0f) + 4.0f); // Z
    newParticle->setGravity(0.1f);
    newParticle->setBounce(0.5f);
    newParticle->setLifetime(200);
    newParticle->setFadeOut(100);

    mParticles.push_back(newParticle);

    return newParticle;
}

Particle *ParticleEngine::addTextRiseFadeOutEffect(const std::string &text,
                                                   int x, int y,
                                                   const gcn::Color *color,
                                                   gcn::Font *font,
                                                   bool outline)
{
    Particle *newParticle = new TextParticle(text, color, font, outline);
    newParticle->setMap(mMap);
    newParticle->moveTo(x, y);
    newParticle->setVelocity(0.0f, 0.0f, 0.5f);
    newParticle->setGravity(0.0015f);
    newParticle->setLifetime(300);
    newParticle->setFadeOut(50);
    newParticle->setFadeIn(200);

    mParticles.push_back(newParticle);

    return newParticle;
}
