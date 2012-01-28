/*
 *  The Mana Client
 *  Copyright (C) 2010-2012  The Mana Developers
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

#ifndef EVENT_H
#define EVENT_H

#include <map>
#include <set>
#include <string>

class ActorSprite;
class Item;

// Possible exception that can be thrown
enum BadEvent {
    BAD_KEY,
    BAD_VALUE,
    KEY_ALREADY_EXISTS
};

class EventListener;

typedef std::set<EventListener *> ListenerSet;
class VariableData;
typedef std::map<std::string, VariableData *> VariableMap;

class Event
{
public:
    enum Channel
    {
        ActorSpriteChannel,
        AttributesChannel,
        BuySellChannel,
        ChatChannel,
        ClientChannel,
        ConfigChannel,
        GameChannel,
        ItemChannel,
        NoticesChannel,
        NpcChannel,
        StorageChannel
    };

    enum Type
    {
        Announcement,
        Being,
        Close,
        CloseAll,
        CloseSent,
        ConfigOptionChanged,
        Constructed,
        LoadingDatabases,
        Destroyed,
        Destructed,
        Destructing,
        DoCloseInventory,
        DoDrop,
        DoEquip,
        DoMove,
        DoSplit,
        DoUnequip,
        DoUse,
        End,
        EnginesInitialized,
        EnginesInitializing,
        GuiWindowsLoaded,
        GuiWindowsLoading,
        GuiWindowsUnloaded,
        GuiWindowsUnloading,
        IntegerInput,
        IntegerInputSent,
        MapLoaded,
        Menu,
        MenuSent,
        Message,
        Next,
        NextSent,
        NpcCount,
        Player,
        Post,
        PostCount,
        SendLetterSent,
        ServerNotice,
        StateChange,
        StorageCount,
        StringInput,
        StringInputSent,
        Stun,
        TalkSent,
        UpdateAttribute,
        UpdateStat,
        UpdateStatusEffect,
        Whisper,
        WhisperError
    };

    /**
     * Makes an event with the given name.
     */
    Event(Type type)
    { mType = type; }

    ~Event();

    /**
     * Returns the name of the event.
     */
    Type getType() const
    { return mType; }

// Integers

    /**
     * Sets the given variable to the given integer, if it isn't already set.
     */
    void setInt(const std::string &key, int value) throw (BadEvent);

    /**
     * Returns the given variable if it is set and an integer.
     */
    int getInt(const std::string &key) const throw (BadEvent);

    /**
     * Returns the given variable if it is set and an integer, returning the
     * given default otherwise.
     */
    int getInt(const std::string &key, int defaultValue) const
    { try { return getInt(key); } catch (BadEvent) { return defaultValue; }}

    /**
     * Returns true if the given variable exists and is an integer.
     */
    bool hasInt(const std::string &key) const;

// Strings

    /**
     * Sets the given variable to the given string, if it isn't already set.
     */
    void setString(const std::string &key, const std::string &value) throw (BadEvent);

    /**
     * Returns the given variable if it is set and a string.
     */
    const std::string &getString(const std::string &key) const throw (BadEvent);

    /**
     * Returns the given variable if it is set and a string, returning the
     * given default otherwise.
     */
    std::string getString(const std::string &key,
                          const std::string &defaultValue) const
    { try { return getString(key); } catch (BadEvent) { return defaultValue; }}

    /**
     * Returns true if the given variable exists and is a string.
     */
    bool hasString(const std::string &key) const;

// Floats

    /**
     * Sets the given variable to the given floating-point, if it isn't already
     * set.
     */
    void setFloat(const std::string &key, double value) throw (BadEvent);

    /**
     * Returns the given variable if it is set and a floating-point.
     */
    double getFloat(const std::string &key) const throw (BadEvent);

    /**
     * Returns the given variable if it is set and a floating-point, returning
     * the given default otherwise.
     */
    double getFloat(const std::string &key, float defaultValue) const
    { try { return getFloat(key); } catch (BadEvent) { return defaultValue; }}

    /**
     * Returns true if the given variable exists and is a floating-point.
     */
    bool hasFloat(const std::string &key) const;

// Booleans

    /**
     * Sets the given variable to the given boolean, if it isn't already set.
     */
    void setBool(const std::string &key, bool value) throw (BadEvent);

    /**
     * Returns the given variable if it is set and a boolean.
     */
    bool getBool(const std::string &key) const throw (BadEvent);

    /**
     * Returns the given variable if it is set and a boolean, returning the
     * given default otherwise.
     */
    bool getBool(const std::string &key, bool defaultValue) const
    { try { return getBool(key); } catch (BadEvent) { return defaultValue; }}

    /**
     * Returns true if the given variable exists and is a boolean.
     */
    bool hasBool(const std::string &key) const;

// Items

    /**
     * Sets the given variable to the given Item, if it isn't already set.
     */
    void setItem(const std::string &key, Item *value) throw (BadEvent);

    /**
     * Returns the given variable if it is set and an Item.
     */
    Item *getItem(const std::string &key) const throw (BadEvent);

    /**
     * Returns the given variable if it is set and an Item, returning the
     * given default otherwise.
     */
    Item *getItem(const std::string &key, Item *defaultValue) const
    { try { return getItem(key); } catch (BadEvent) { return defaultValue; }}

    /**
     * Returns true if the given variable exists and is an Item.
     */
    bool hasItem(const std::string &key) const;

// ActorSprites

    /**
     * Sets the given variable to the given actor, if it isn't already set.
     */
    void setActor(const std::string &key, ActorSprite *value) throw (BadEvent);

    /**
     * Returns the given variable if it is set and an actor.
     */
    ActorSprite *getActor(const std::string &key) const throw (BadEvent);

    /**
     * Returns the given variable if it is set and an actor, returning the
     * given default otherwise.
     */
    ActorSprite *getActor(const std::string &key,
                          ActorSprite *defaultValue) const
    { try { return getActor(key); } catch (BadEvent) { return defaultValue; }}

    /**
     * Returns true if the given variable exists and is an actor.
     */
    bool hasActor(const std::string &key) const;

// Triggers

    /**
     * Sends this event to all classes listening to the given channel.
     */
    void trigger(Channel channel) const
    { trigger(channel, *this); }

    /**
     * Sends the given event to all classes listening to the given channel.
     */
    static void trigger(Channel channel, const Event &event);

    /**
     * Sends an empty event with the given name to all classes listening to the
     * given channel.
     */
    static void trigger(Channel channel, Type type)
    { trigger(channel, Event(type)); }

protected:
    friend class EventListener;

    /**
     * Binds the given listener to the given channel. The listener will receive
     * all events triggered on the channel.
     */
    static void bind(EventListener *listener, Channel channel);

    /**
     * Unbinds the given listener from the given channel. The listener will no
     * longer receive any events from the channel.
     */
    static void unbind(EventListener *listener, Channel channel);

    /**
     * Unbinds the given listener from all channels.
     */
    static void remove(EventListener *listener);

private:
    typedef std::map<Channel, ListenerSet > ListenMap;
    static ListenMap mBindings;

    Type mType;
    VariableMap mData;
};

#define SERVER_NOTICE(message) { \
Event event(Event::ServerNotice); \
event.setString("message", message); \
event.trigger(Event::NoticesChannel, event); }

#endif // EVENT_H
