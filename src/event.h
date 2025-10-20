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

#pragma once

#include <any>
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
class VariableData;

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
        StorageChannel,
        QuestsChannel
    };

    enum Type
    {
        Announcement,
        Being,
        ClearDialog,
        Close,
        CloseAll,
        CloseDialog,
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
        DoUnequip,
        DoUse,
        EnginesInitialized,
        EnginesInitializing,
        GuiWindowsLoaded,
        GuiWindowsLoading,
        GuiWindowsUnloaded,
        GuiWindowsUnloading,
        IntegerInput,
        ItemInput,
        MapLoaded,
        Menu,
        Message,
        Next,
        NpcCount,
        Player,
        Post,
        PostCount,
        ServerNotice,
        StateChange,
        StorageCount,
        StringInput,
        UpdateAttribute,
        UpdateStat,
        UpdateStatusEffect,
        Whisper,
        WhisperError,
        QuestVarsChanged,
    };

    /**
     * Makes an event with the given name.
     */
    Event(Type type)
        : mType(type)
    {}

    /**
     * Makes an event with the given name and value.
     */
    template<typename T>
    Event(Type type, const T &value)
        : mType(type)
        , mValue(value)
    {}

    ~Event();

    /**
     * Returns the name of the event.
     */
    Type getType() const
    { return mType; }

    /**
     * Sets the value of the event.
     */
    template<typename T>
    void setValue(const T &value)
    { mValue = value; }

    /**
     * Returns the value of the event. Throws an exception if the event has no
     * value of the given type.
     */
    template<typename T>
    const T &value() const
    { return std::any_cast<const T &>(mValue); }

    /**
     * Returns whether the event has the given the value.
     */
    template<typename T>
    bool hasValue(const T &value) const
    { return mValue.type() == typeid(T) && Event::value<T>() == value; }

// Integers

    /**
     * Sets the given variable to the given integer, if it isn't already set.
     */
    void setInt(const std::string &key, int value);

    /**
     * Returns the given variable if it is set and an integer.
     */
    int getInt(const std::string &key) const;

    /**
     * Returns the given variable if it is set and an integer, returning the
     * given default otherwise.
     */
    int getInt(const std::string &key, int defaultValue) const
    { try { return getInt(key); } catch (BadEvent) { return defaultValue; }}

// Strings

    /**
     * Sets the given variable to the given string, if it isn't already set.
     */
    void setString(const std::string &key, const std::string &value);

    /**
     * Returns the given variable if it is set and a string.
     */
    const std::string &getString(const std::string &key) const;

    /**
     * Returns the given variable if it is set and a string, returning the
     * given default otherwise.
     */
    std::string getString(const std::string &key,
                          const std::string &defaultValue) const
    { try { return getString(key); } catch (BadEvent) { return defaultValue; }}

// Floats

    /**
     * Sets the given variable to the given floating-point, if it isn't already
     * set.
     */
    void setFloat(const std::string &key, double value);

    /**
     * Returns the given variable if it is set and a floating-point.
     */
    double getFloat(const std::string &key) const;

    /**
     * Returns the given variable if it is set and a floating-point, returning
     * the given default otherwise.
     */
    double getFloat(const std::string &key, float defaultValue) const
    { try { return getFloat(key); } catch (BadEvent) { return defaultValue; }}

// Booleans

    /**
     * Sets the given variable to the given boolean, if it isn't already set.
     */
    void setBool(const std::string &key, bool value);

    /**
     * Returns the given variable if it is set and a boolean.
     */
    bool getBool(const std::string &key) const;

    /**
     * Returns the given variable if it is set and a boolean, returning the
     * given default otherwise.
     */
    bool getBool(const std::string &key, bool defaultValue) const
    { try { return getBool(key); } catch (BadEvent) { return defaultValue; }}

// Items

    /**
     * Sets the given variable to the given Item, if it isn't already set.
     */
    void setItem(const std::string &key, Item *value);

    /**
     * Returns the given variable if it is set and an Item.
     */
    Item *getItem(const std::string &key) const;

    /**
     * Returns the given variable if it is set and an Item, returning the
     * given default otherwise.
     */
    Item *getItem(const std::string &key, Item *defaultValue) const
    { try { return getItem(key); } catch (BadEvent) { return defaultValue; }}

// ActorSprites

    /**
     * Sets the given variable to the given actor, if it isn't already set.
     */
    void setActor(const std::string &key, ActorSprite *value);

    /**
     * Returns the given variable if it is set and an actor.
     */
    ActorSprite *getActor(const std::string &key) const;

    /**
     * Returns the given variable if it is set and an actor, returning the
     * given default otherwise.
     */
    ActorSprite *getActor(const std::string &key,
                          ActorSprite *defaultValue) const
    { try { return getActor(key); } catch (BadEvent) { return defaultValue; }}

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
    using ListenMap = std::map<Channel, std::set<EventListener *>>;
    static ListenMap mBindings;

    const Type mType;
    std::map<std::string, VariableData *> mData;
    std::any mValue;
};

inline void serverNotice(const std::string &message)
{
    Event event(Event::ServerNotice);
    event.setString("message", message);
    event.trigger(Event::NoticesChannel);
}
