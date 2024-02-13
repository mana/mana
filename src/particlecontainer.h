/*
 *  The Mana Client
 *  Copyright (C) 2008-2009  The Mana World Development Team
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

#ifndef PARTICLE_CONTAINER_H
#define PARTICLE_CONTAINER_H

#include <list>
#include <vector>

class Particle;

/**
 * Set of particle effects.  May be stacked with other ParticleContainers.  All
 * operations herein affect such stacked containers, unless the operations end
 * in `Locally'.
 */
class ParticleContainer
{
public:
    /**
     * Constructs a new particle container and assumes responsibility for
     * its parent (for all operations defined herein, except when ending in `Locally')
     *
     * delParent means that the destructor should also free the parent.
     */
    ParticleContainer(ParticleContainer *parent = nullptr, bool delParent = true);
    virtual ~ParticleContainer();

    /**
     * Kills and removes all particle effects
     */
    void clear();

    /**
     * Kills and removes all particle effects (only in this container)
     */
    virtual void clearLocally() {}

    /**
     * Sets the positions of all elements
     */
    virtual void moveTo(float x, float y);

protected:
    bool mDelParent;                    /**< Delete mNext in destructor */
    ParticleContainer *mNext;           /**< Contained container, if any */
};

/**
 * Linked list of particle effects.
 */
class ParticleList : public ParticleContainer
{
public:
    ParticleList(ParticleContainer *parent = nullptr, bool delParent = true);
    ~ParticleList() override;

    /**
     * Takes control of and adds a particle
     */
    void addLocally(Particle *);

    /**
     * `kills' and removes a particle
     */
    void removeLocally(Particle *);

    void clearLocally() override;

    void moveTo(float x, float y) override;

protected:
    std::list<Particle *> mElements;    /**< Contained particle effects */
};

/**
 * Particle container with indexing facilities
 */
class ParticleVector : public ParticleContainer
{
public:
    ParticleVector(ParticleContainer *parent = nullptr, bool delParent = true);
    ~ParticleVector() override;

    /**
     * Sets a particle at a specified index.  Kills the previous particle
     * there, if needed.
     */
    virtual void setLocally(int index, Particle *particle);

    /**
     * Removes a particle at a specified index
     */
    virtual void delLocally(int index);

    void clearLocally() override;
    void moveTo(float x, float y) override;

protected:
    std::vector<Particle *> mIndexedElements;
};

#endif
