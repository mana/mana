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

#include "being.h"

#include "animatedsprite.h"
#include "configuration.h"
#include "effectmanager.h"
#include "game.h"
#include "graphics.h"
#include "localplayer.h"
#include "log.h"
#include "map.h"
#include "particle.h"
#include "simpleanimation.h"
#include "sound.h"
#include "text.h"
#include "statuseffect.h"

#include "gui/speechbubble.h"

#include "resources/colordb.h"
#include "resources/emotedb.h"
#include "resources/image.h"
#include "resources/itemdb.h"
#include "resources/iteminfo.h"
#include "resources/resourcemanager.h"

#include "gui/gui.h"
#include "gui/palette.h"
#include "gui/speechbubble.h"

#include "utils/dtor.h"
#include "utils/gettext.h"
#include "utils/stringutils.h"
#include "utils/xml.h"

#include <cassert>
#include <cmath>

namespace {
const bool debug_movement = true;
}


#define BEING_EFFECTS_FILE "effects.xml"
#define HAIR_FILE "hair.xml"

int Being::mNumberOfHairColors = 1;
int Being::mNumberOfHairstyles = 1;
std::vector<std::string> Being::hairColors;

static const int DEFAULT_WIDTH = 32;
static const int DEFAULT_HEIGHT = 32;

Being::Being(int id, int job, Map *map):
#ifdef EATHENA_SUPPORT
    mX(0), mY(0),
    mWalkTime(0),
#endif
    mEmotion(0), mEmotionTime(0),
    mSpeechTime(0),
    mAttackSpeed(350),
    mAction(STAND),
    mJob(job),
    mId(id),
    mDirection(DOWN),
#ifdef TMWSERV_SUPPORT
    mSpriteDirection(DIRECTION_DOWN),
#endif
    mMap(NULL),
    mParticleEffects(config.getValue("particleeffects", 1)),
    mEquippedWeapon(NULL),
#ifdef TMWSERV_SUPPORT
    mHairStyle(0),
#else
    mHairStyle(1),
#endif
    mHairColor(0),
    mGender(GENDER_UNSPECIFIED),
    mStunMode(0),
    mSprites(VECTOREND_SPRITE, NULL),
    mSpriteIDs(VECTOREND_SPRITE, 0),
    mSpriteColors(VECTOREND_SPRITE, ""),
    mStatusParticleEffects(&mStunParticleEffects, false),
    mChildParticleEffects(&mStatusParticleEffects, false),
    mMustResetParticles(false),
#ifdef TMWSERV_SUPPORT
    mWalkSpeed(100),
#else
    mWalkSpeed(150),
#endif
    mPx(0), mPy(0),
    mUsedTargetCursor(NULL)
{
    setMap(map);

    mSpeechBubble = new SpeechBubble;

    mNameColor = &guiPalette->getColor(Palette::CHAT);
    mText = 0;
}

Being::~Being()
{
    mUsedTargetCursor = NULL;
    delete_all(mSprites);
    clearPath();

    if (player_node && player_node->getTarget() == this)
        player_node->setTarget(NULL);

    setMap(NULL);

    delete mSpeechBubble;
    delete mText;
}

void Being::setPosition(const Vector &pos)
{
    mPos = pos;

    // Update pixel coordinates (convert once, for performance reasons)
    mPx = (int) pos.x;
    mPy = (int) pos.y;

    updateCoords();

    if (mText)
        mText->adviseXY(mPx,
                        mPy - getHeight() - mText->getHeight());
}

#ifdef EATHENA_SUPPORT
void Being::setDestination(Uint16 destX, Uint16 destY)
{
    if (mMap)
        setPath(mMap->findPath(mX, mY, destX, destY, getWalkMask()));
}
#endif

#ifdef TMWSERV_SUPPORT
void Being::adjustCourse(int srcX, int srcY, int dstX, int dstY)
{
    if (debug_movement)
        printf("%p adjustCourse(%d, %d, %d, %d)\n",
                (void*) this, srcX, srcY, dstX, dstY);

    mDest.x = dstX;
    mDest.y = dstY;

    // Find a path to the destination when it is at least a tile away
    if (mMap && fabsf((mDest - mPos).length()) > 32) {
        setPath(mMap->findPath((int) mPos.x / 32, (int) mPos.y / 32,
                               dstX / 32, dstY / 32, getWalkMask()));
    } else {
        setPath(Path());
    }

    // TODO: Evaluate the implementation of this method
    /*
    if (mX / 32 == dstX / 32 && mY / 32 == dstY / 32)
    {
        // The being is already on the last tile of the path.
        Path p;
        p.push_back(Position(dstX, dstY));
        setPath(p);
        return;
    }

    Path p1;
    int p1_size, p1_length;
    Uint16 *p1_dist;
    int onPath = -1;
    if (srcX / 32 == dstX / 32 && srcY / 32 == dstY / 32)
    {
        p1_dist = new Uint16[1];
        p1_size = 1;
        p1_dist[0] = 0;
        p1_length = 0;
    }
    else
    {
        p1 = mMap->findPath(srcX / 32, srcY / 32, dstX / 32, dstY / 32, getWalkMask());
        if (p1.empty())
        {
            // No path, but don't teleport since it could be user input.
            setPath(p1);
            return;
        }
        p1_size = p1.size();
        p1_dist = new Uint16[p1_size];
        int j = 0;
        // Remove last tile so that it can be replaced by the exact destination.
        p1.pop_back();
        for (Path::iterator i = p1.begin(), i_end = p1.end(); i != i_end; ++i)
        {
            // Get distance from source to tile i.
            p1_dist[j] = mMap->getMetaTile(i->x, i->y)->Gcost;
            // Check if the being is already walking on the path.
            if (i->x == mX / 32 && i->y == mY / 32)
            {
                onPath = j;
            }
            // Do not set any offset for intermediate steps.
            i->x = i->x * 32;
            i->y = i->y * 32;
            ++j;
        }
        p1_length = mMap->getMetaTile(dstX / 32, dstY / 32)->Gcost;
        p1_dist[p1_size - 1] = p1_length;
    }
    p1.push_back(Position(dstX, dstY));

    if (mX / 32 == srcX / 32 && mY / 32 == srcY / 32)
    {
        // The being is at the start of the path.
        setPath(p1);
        delete[] p1_dist;
        return;
    }

    if (onPath >= 0)
    {
        // The being is already on the path, but it needs to be slowed down.
        for (int j = onPath; j >= 0; --j)
        {
            p1.pop_front();
        }
        int r = p1_length - p1_dist[onPath];  // remaining length
        assert(r > 0);
        setPath(p1, p1_length * 1024 / r);
        delete[] p1_dist;
        return;
    }

    Path bestPath;
    int bestRating = -1, bestStart = 0, bestLength = 0;
    int j = 0;

    for (Path::iterator i = p1.begin(), i_end = p1.end(); i != i_end; ++i)
    {
        // Look if it is worth passing by tile i.
        Path p2 = mMap->findPath(mX / 32, mY / 32, i->x / 32, i->y / 32, getWalkMask());
        if (!p2.empty())
        {
            int l1 = mMap->getMetaTile(i->x / 32, i->y / 32)->Gcost;
            int l2 = p1_length - p1_dist[j];
            int r = l1 + l2 / 2; // TODO: tune rating formula
            assert(r > 0);
            if (bestRating < 0 || r < bestRating)
            {
                bestPath.swap(p2);
                bestRating = r;
                bestStart = j;
                bestLength = l1 + l2;
            }
        }
        ++j;
    }

    if (bestRating < 0)
    {
        // Unable to reach the path? Still, don't teleport since it could be
        // user input instead of server command.
        setPath(p1);
        delete[] p1_dist;
        return;
    }

    bestPath.pop_back();
    for (Path::iterator i = bestPath.begin(), i_end = bestPath.end(); i != i_end; ++i)
    {
        i->x = i->x * 32;
        i->y = i->y * 32;
    }

    // Concatenate paths.
    for (int j = bestStart; j > 0; --j)
    {
        p1.pop_front();
    }
    p1.splice(p1.begin(), bestPath);

    assert(bestLength > 0);
    setPath(p1, p1_length * 1024 / bestLength);
    delete[] p1_dist;
    */
}

void Being::adjustCourse(int srcX, int srcY)
{
    if (debug_movement)
        printf("%p adjustCourse(%d, %d)\n", (void*) this, srcX, srcY);

    if (!mPath.empty())
        adjustCourse(srcX, srcY, mPath.back().x * 32, mPath.back().y * 32);
}

void Being::setDestination(int destX, int destY)
{
    if (debug_movement)
        printf("%p setDestination(%d, %d)\n", (void*) this, destX, destY);

    adjustCourse((int) mPos.x, (int) mPos.y, destX, destY);
}
#endif  // TMWSERV_SUPPORT

void Being::clearPath()
{
    mPath.clear();
}

void Being::setPath(const Path &path)
{
    mPath = path;
#ifdef TMWSERV_SUPPORT
    std::cout << this << " New path: " << path << std::endl;
#else
    if (mAction != WALK && mAction != DEAD)
    {
        nextStep();
        mWalkTime = tick_time;
    }
#endif
}

void Being::setHairStyle(int style, int color)
{
    mHairStyle = style < 0 ? mHairStyle : style % mNumberOfHairstyles;
    mHairColor = color < 0 ? mHairColor : color % ColorDB::size();
}

void Being::setSprite(int slot, int id, const std::string &color)
{
    assert(slot >= BASE_SPRITE && slot < VECTOREND_SPRITE);
    mSpriteIDs[slot] = id;
    mSpriteColors[slot] = color;
}

void Being::setSpeech(const std::string &text, int time)
{
    mSpeech = text;

    // Trim whitespace
    trim(mSpeech);

    // check for links
    std::string::size_type start = mSpeech.find('[');
    std::string::size_type end = mSpeech.find(']', start);

    while (start != std::string::npos && end != std::string::npos)
    {
        // Catch multiple embeds and ignore them so it doesn't crash the client.
        while ((mSpeech.find('[', start + 1) != std::string::npos) &&
               (mSpeech.find('[', start + 1) < end))
        {
            start = mSpeech.find('[', start + 1);
        }

        std::string::size_type position = mSpeech.find('|');
        if (mSpeech[start + 1] == '@' && mSpeech[start + 2] == '@')
        {
            mSpeech.erase(end, 1);
            mSpeech.erase(start, (position - start) + 1);
        }
        position = mSpeech.find('@');

        while (position != std::string::npos)
        {
            mSpeech.erase(position, 2);
            position = mSpeech.find('@');
        }

        start = mSpeech.find('[', start + 1);
        end = mSpeech.find(']', start);
    }

    if (!mSpeech.empty())
        mSpeechTime = time <= SPEECH_MAX_TIME ? time : SPEECH_MAX_TIME;

    const int speech = (int) config.getValue("speech", NAME_IN_BUBBLE);
    if (speech == TEXT_OVERHEAD) {
        if (mText)
            delete mText;

        mText = new Text(mSpeech,
                         mPx, mPy - getHeight(),
                         gcn::Graphics::CENTER,
                         &guiPalette->getColor(Palette::PARTICLE),
                         true);
    }
}

void Being::takeDamage(Being *attacker, int amount, AttackType type)
{
    gcn::Font *font;
    std::string damage = amount ? toString(amount) : type == FLEE ?
            "dodge" : "miss";
    const gcn::Color* color;

    font = gui->getInfoParticleFont();

    // Selecting the right color
    if (type == CRITICAL || type == FLEE)
    {
        color = &guiPalette->getColor(Palette::HIT_CRITICAL);
    }
    else if (!amount)
    {
        if (attacker == player_node)
        {
            // This is intended to be the wrong direction to visually
            // differentiate between hits and misses
            color = &guiPalette->getColor(Palette::HIT_MONSTER_PLAYER);
        }
        else
        {
            color = &guiPalette->getColor(Palette::MISS);
        }
    }
    else if (getType() == MONSTER)
    {
        color = &guiPalette->getColor(Palette::HIT_PLAYER_MONSTER);
    }
    else
    {
        color = &guiPalette->getColor(Palette::HIT_MONSTER_PLAYER);
    }

    // Show damage number
    particleEngine->addTextSplashEffect(damage,
                                        mPx + 16, mPy + 16,
                                        color, font, true);

    if (amount > 0)
    {
        if (type != CRITICAL)
        {
            effectManager->trigger(26, this);
        }
        else
        {
            effectManager->trigger(28, this);
        }
    }
}

#ifdef TMWSERV_SUPPORT
void Being::handleAttack()
#else
void Being::handleAttack(Being *victim, int damage, AttackType type)
#endif
{
    if (this != player_node)
        setAction(Being::ATTACK);
#ifdef EATHENA_SUPPORT
    if (getType() == PLAYER)
    {
        if (mEquippedWeapon && mEquippedWeapon->getAttackType() == ACTION_ATTACK_BOW)
        {
            Particle *p = new Particle(NULL);
            p->setLifetime(1000);
            p->moveBy(Vector(0.0f, 0.0f, 32.0f));
            victim->controlParticle(p);

            Particle *p2 = particleEngine->addEffect("graphics/particles/arrow.particle.xml", mPx, mPy);
            p2->setLifetime(900);
            p2->setDestination(p, 7, 0);
            p2->setDieDistance(8);
        }
    }
    mFrame = 0;
    mWalkTime = tick_time;
#endif
}

void Being::setMap(Map *map)
{
    // Remove sprite from potential previous map
    if (mMap)
        mMap->removeSprite(mSpriteIterator);

    mMap = map;

    // Add sprite to potential new map
    if (mMap)
        mSpriteIterator = mMap->addSprite(this);

    // Clear particle effect list because child particles became invalid
    mChildParticleEffects.clear();
    mMustResetParticles = true; // Reset status particles on next redraw
}

void Being::controlParticle(Particle *particle)
{
    mChildParticleEffects.addLocally(particle);
}

void Being::setAction(Action action, int attackType)
{
    SpriteAction currentAction = ACTION_INVALID;

    switch (action)
    {
        case WALK:
            currentAction = ACTION_WALK;
            break;
        case SIT:
            currentAction = ACTION_SIT;
            break;
        case ATTACK:
            if (mEquippedWeapon)
                currentAction = mEquippedWeapon->getAttackType();
            else
                currentAction = ACTION_ATTACK;

            for (int i = 0; i < VECTOREND_SPRITE; i++)
            {
                if (mSprites[i])
                    mSprites[i]->reset();
            }
            break;
        case HURT:
            //currentAction = ACTION_HURT;  // Buggy: makes the player stop
                                            // attacking and unable to attack
                                            // again until he moves
            break;
        case DEAD:
            currentAction = ACTION_DEAD;
            break;
        case STAND:
            currentAction = ACTION_STAND;
            break;
    }

    if (currentAction != ACTION_INVALID)
    {
        for (int i = 0; i < VECTOREND_SPRITE; i++)
        {
            if (mSprites[i])
                mSprites[i]->play(currentAction);
        }
        mAction = action;
    }
}

void Being::setDirection(Uint8 direction)
{
    if (mDirection == direction)
        return;

#ifdef TMWSERV_SUPPORT
    // if the direction does not change much, keep the common component
    int mFaceDirection = mDirection & direction;
    if (!mFaceDirection)
        mFaceDirection = direction;
    mDirection = direction;

    SpriteDirection dir;
    if (mFaceDirection & UP)
        dir = DIRECTION_UP;
    else if (mFaceDirection & DOWN)
        dir = DIRECTION_DOWN;
    else if (mFaceDirection & RIGHT)
        dir = DIRECTION_RIGHT;
    else
        dir = DIRECTION_LEFT;
    mSpriteDirection = dir;
#else
    mDirection = direction;
    SpriteDirection dir = getSpriteDirection();
#endif

    for (int i = 0; i < VECTOREND_SPRITE; i++)
    {
       if (mSprites[i])
           mSprites[i]->setDirection(dir);
    }
}

#ifdef EATHENA_SUPPORT
SpriteDirection Being::getSpriteDirection() const
{
    SpriteDirection dir;

    if (mDirection & UP)
        dir = DIRECTION_UP;
    else if (mDirection & DOWN)
        dir = DIRECTION_DOWN;
    else if (mDirection & RIGHT)
        dir = DIRECTION_RIGHT;
    else
        dir = DIRECTION_LEFT;

    return dir;
}

void Being::nextStep()
{
    if (mPath.empty())
    {
        setAction(STAND);
        return;
    }

    Position pos = mPath.front();
    mPath.pop_front();

    int dir = 0;
    if (pos.x > mX)
        dir |= RIGHT;
    else if (pos.x < mX)
        dir |= LEFT;
    if (pos.y > mY)
        dir |= DOWN;
    else if (pos.y < mY)
        dir |= UP;

    setDirection(dir);

    if (!mMap->getWalk(pos.x, pos.y, getWalkMask()))
    {
        setAction(STAND);
        return;
    }

    mX = pos.x;
    mY = pos.y;
    setAction(WALK);
    mWalkTime += mWalkSpeed / 10;
}
#endif

void Being::logic()
{
    // Reduce the time that speech is still displayed
    if (mSpeechTime > 0)
        mSpeechTime--;

    // Remove text and speechbubbles if speech boxes aren't being used
    if (mSpeechTime == 0 && mText)
    {
        delete mText;
        mText = 0;
    }

#ifdef TMWSERV_SUPPORT
    const Vector dest = (mPath.empty()) ?
        mDest : Vector(mPath.front().x * 32 + 16,
                       mPath.front().y * 32 + 16);

    Vector dir = dest - mPos;
    const float length = dir.length();

    // When we're over 2 pixels from our destination, move to it
    // TODO: Should be possible to make it even pixel exact, but this solves
    //       the jigger caused by moving too far.
    if (length > 2.0f) {
        const float speed = mWalkSpeed / 100.0f;
        setPosition(mPos + (dir / (length / speed)));

        if (mAction != WALK)
            setAction(WALK);

        // Update the player sprite direction
        int direction = 0;
        const float dx = std::abs(dir.x);
        const float dy = std::abs(dir.y);
        if (dx > dy)
            direction |= (dir.x > 0) ? RIGHT : LEFT;
        else
            direction |= (dir.y > 0) ? DOWN : UP;
        setDirection(direction);
    }
    else if (!mPath.empty()) {
        // TODO: Pop as soon as there is a direct unblocked line to the next
        //       point on the path.
        mPath.pop_front();
    } else if (mAction == WALK) {
        setAction(STAND);
    }
#else
    // Update pixel coordinates
    setPosition(mX * 32 + 16 + getXOffset(),
                mY * 32 + 32 + getYOffset());
#endif

    if (mEmotion != 0)
    {
        mEmotionTime--;
        if (mEmotionTime == 0)
            mEmotion = 0;
    }

    // Update sprite animations
    if (mUsedTargetCursor)
        mUsedTargetCursor->update(tick_time * 10);

    for (int i = 0; i < VECTOREND_SPRITE; i++)
    {
        if (mSprites[i])
            mSprites[i]->update(tick_time * 10);
    }

    // Restart status/particle effects, if needed
    if (mMustResetParticles) {
        mMustResetParticles = false;
        for (std::set<int>::iterator it = mStatusEffects.begin();
             it != mStatusEffects.end(); it++) {
            const StatusEffect *effect = StatusEffect::getStatusEffect(*it, true);
            if (effect && effect->particleEffectIsPersistent())
                updateStatusEffect(*it, true);
        }
    }

    // Update particle effects
    mChildParticleEffects.moveTo(mPos.x, mPos.y);
}

void Being::draw(Graphics *graphics, int offsetX, int offsetY) const
{
    // TODO: Eventually, we probably should fix all sprite offsets so that
    //       these translations aren't necessary anymore. The sprites know
    //       best where their centerpoint should be.
    const int px = mPx + offsetX - 16;
    const int py = mPy + offsetY - 32;

    if (mUsedTargetCursor)
        mUsedTargetCursor->draw(graphics, px, py);

    for (int i = 0; i < VECTOREND_SPRITE; i++)
    {
        if (mSprites[i])
        {
            mSprites[i]->draw(graphics, px, py);
        }
    }
}

void Being::drawEmotion(Graphics *graphics, int offsetX, int offsetY)
{
    if (!mEmotion)
        return;

    const int px = mPx - offsetX - 16;
    const int py = mPy - offsetY - 64 - 32;
    const int emotionIndex = mEmotion - 1;

    if (emotionIndex >= 0 && emotionIndex <= EmoteDB::getLast())
        EmoteDB::getAnimation(emotionIndex)->draw(graphics, px, py);
}

void Being::drawSpeech(int offsetX, int offsetY)
{
    const int px = mPx - offsetX;
    const int py = mPy - offsetY;
    const int speech = (int) config.getValue("speech", NAME_IN_BUBBLE);

    // Draw speech above this being
    if (mSpeechTime == 0)
    {
        if (mSpeechBubble->isVisible())
            mSpeechBubble->setVisible(false);
    }
    else if (mSpeechTime > 0 && (speech == NAME_IN_BUBBLE ||
             speech == NO_NAME_IN_BUBBLE))
    {
        const bool showName = (speech == NAME_IN_BUBBLE);

        if (mText)
        {
            delete mText;
            mText = NULL;
        }

        mSpeechBubble->setCaption(showName ? mName : "", mNameColor);

        mSpeechBubble->setText(mSpeech, showName);
        mSpeechBubble->setPosition(px - (mSpeechBubble->getWidth() / 2),
                                   py - getHeight() - (mSpeechBubble->getHeight()));
        mSpeechBubble->setVisible(true);
    }
    else if (mSpeechTime > 0 && speech == TEXT_OVERHEAD)
    {
        mSpeechBubble->setVisible(false);

        if (! mText) {
            mText = new Text(mSpeech,
                             mPx, mPy - getHeight(),
                             gcn::Graphics::CENTER,
                             &guiPalette->getColor(Palette::PARTICLE),
                             true);
        }
    }
    else if (speech == NO_SPEECH)
    {
        mSpeechBubble->setVisible(false);

        if (mText)
            delete mText;

        mText = NULL;
    }
}

Being::Type Being::getType() const
{
    return UNKNOWN;
}

void Being::setStatusEffectBlock(int offset, Uint16 newEffects)
{
    for (int i = 0; i < STATUS_EFFECTS; i++) {
        int index = StatusEffect::blockEffectIndexToEffectIndex(offset + i);

        if (index != -1)
            setStatusEffect(index, (newEffects & (1 << i)) > 0);
    }
}

void Being::handleStatusEffect(StatusEffect *effect, int effectId)
{
    if (!effect)
        return;

    // TODO: Find out how this is meant to be used
    // (SpriteAction != Being::Action)
    //SpriteAction action = effect->getAction();
    //if (action != ACTION_INVALID)
    //    setAction(action);

    Particle *particle = effect->getParticle();

    if (effectId >= 0)
        mStatusParticleEffects.setLocally(effectId, particle);
    else {
        mStunParticleEffects.clearLocally();
        if (particle)
            mStunParticleEffects.addLocally(particle);
    }
}

void Being::updateStunMode(int oldMode, int newMode)
{
    handleStatusEffect(StatusEffect::getStatusEffect(oldMode, false), -1);
    handleStatusEffect(StatusEffect::getStatusEffect(newMode, true), -1);
}

void Being::updateStatusEffect(int index, bool newStatus)
{
    handleStatusEffect(StatusEffect::getStatusEffect(index, newStatus), index);
}

void Being::setStatusEffect(int index, bool active)
{
    const bool wasActive = mStatusEffects.find(index) != mStatusEffects.end();

    if (active != wasActive) {
        updateStatusEffect(index, active);
        if (active)
            mStatusEffects.insert(index);
        else
            mStatusEffects.erase(index);
    }
}

#ifdef EATHENA_SUPPORT
int Being::getOffset(char pos, char neg) const
{
    // Check whether we're walking in the requested direction
    if (mAction != WALK ||  !(mDirection & (pos | neg)))
        return 0;

    int offset = (get_elapsed_time(mWalkTime) * 32) / mWalkSpeed;

    // We calculate the offset _from_ the _target_ location
    offset -= 32;
    if (offset > 0)
        offset = 0;

    // Going into negative direction? Invert the offset.
    if (mDirection & pos)
        offset = -offset;

    return offset;
}
#endif

int Being::getWidth() const
{
    if (AnimatedSprite *base = mSprites[BASE_SPRITE])
        return std::max(base->getWidth(), DEFAULT_WIDTH);
    else
        return DEFAULT_WIDTH;
}

int Being::getHeight() const
{
    if (AnimatedSprite *base = mSprites[BASE_SPRITE])
        return std::max(base->getHeight(), DEFAULT_HEIGHT);
    else
        return DEFAULT_HEIGHT;
}

void Being::setTargetAnimation(SimpleAnimation* animation)
{
    mUsedTargetCursor = animation;
    mUsedTargetCursor->reset();
}

struct EffectDescription {
    std::string mGFXEffect;
    std::string mSFXEffect;
};

static EffectDescription *default_effect = NULL;
static std::map<int, EffectDescription *> effects;
static bool effects_initialized = false;

static EffectDescription *getEffectDescription(xmlNodePtr node, int *id)
{
    EffectDescription *ed = new EffectDescription;

    *id = atoi(XML::getProperty(node, "id", "-1").c_str());
    ed->mSFXEffect = XML::getProperty(node, "audio", "");
    ed->mGFXEffect = XML::getProperty(node, "particle", "");

    return ed;
}

static EffectDescription *getEffectDescription(int effectId)
{
    if (!effects_initialized)
    {
        XML::Document doc(BEING_EFFECTS_FILE);
        xmlNodePtr root = doc.rootNode();

        if (!root || !xmlStrEqual(root->name, BAD_CAST "being-effects"))
        {
            logger->log("Error loading being effects file: "
                    BEING_EFFECTS_FILE);
            return NULL;
        }

        for_each_xml_child_node(node, root)
        {
            int id;

            if (xmlStrEqual(node->name, BAD_CAST "effect"))
            {
                EffectDescription *EffectDescription =
                    getEffectDescription(node, &id);
                effects[id] = EffectDescription;
            } else if (xmlStrEqual(node->name, BAD_CAST "default"))
            {
                EffectDescription *EffectDescription =
                    getEffectDescription(node, &id);

                if (default_effect)
                    delete default_effect;

                default_effect = EffectDescription;
            }
        }

        effects_initialized = true;
    } // done initializing

    EffectDescription *ed = effects[effectId];

    if (!ed)
        return default_effect;
    else
        return ed;
}

void Being::internalTriggerEffect(int effectId, bool sfx, bool gfx)
{
    logger->log("Special effect #%d on %s", effectId,
                getId() == player_node->getId() ? "self" : "other");

    EffectDescription *ed = getEffectDescription(effectId);

    if (!ed) {
        logger->log("Unknown special effect and no default recorded");
        return;
    }

    if (gfx && !ed->mGFXEffect.empty()) {
        Particle *selfFX;

        selfFX = particleEngine->addEffect(ed->mGFXEffect, 0, 0);
        controlParticle(selfFX);
    }

    if (sfx && !ed->mSFXEffect.empty()) {
        sound.playSfx(ed->mSFXEffect);
    }
}

int Being::getHairStyleCount()
{
    return mNumberOfHairstyles;
}

int Being::getHairColorCount()
{
    return mNumberOfHairColors;
}

std::string Being::getHairColor(int index)
{
    if (index < 0 || index >= mNumberOfHairColors)
        return "#000000";

    return hairColors[index];
}

void Being::load()
{
    // Hairstyles are encoded as negative numbers. Count how far negative
    // we can go.
    int hairstyles = 1;

    while (ItemDB::get(-hairstyles).getSprite(GENDER_MALE) != "error.xml")
        hairstyles++;

    mNumberOfHairstyles = hairstyles;

    XML::Document doc(HAIR_FILE);
    xmlNodePtr root = doc.rootNode();

    // Add an initial hair color
    hairColors.resize(1, "#000000");

    if (!root || !xmlStrEqual(root->name, BAD_CAST "colors"))
    {
        logger->log("Error loading being hair configuration file");
    } else {
        for_each_xml_child_node(node, root)
        {
            if (xmlStrEqual(node->name, BAD_CAST "color"))
            {
                int index = atoi(XML::getProperty(node, "id", "-1").c_str());
                std::string value = XML::getProperty(node, "value", "");

                if (index >= 0 && !value.empty()) {
                    if (index >= mNumberOfHairColors) {
                        mNumberOfHairColors = index + 1;
                        hairColors.resize(mNumberOfHairColors, "#000000");
                    }
                    hairColors[index] = value;
                }
            }
        }
    }
}
