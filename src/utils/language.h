/*
 *  The Mana Client
 *  Copyright (C) 2026  The Mana Developers
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

#include <string>
#include <vector>

struct LanguageInfo
{
    std::string code;   /**< Language code as used by gettext. */
    std::string name;   /**< Name of the language, in that language. */
};

/**
 * Sets up gettext and locates the message catalogs.
 */
void initInternationalization();

/**
 * Returns the languages the client can be displayed in, based on the message
 * catalogs that are installed. Sorted by name.
 */
const std::vector<LanguageInfo> &getAvailableLanguages();

/**
 * Returns the language that is used when no override is set. This is the
 * language configured in the system when a translation is available for it,
 * and English otherwise.
 */
const LanguageInfo &getSystemLanguage();

/**
 * Overrides the language used for translations. An empty code means the
 * language configured in the system is used.
 *
 * Must be called after initInternationalization and before any translated
 * string is looked up, since gettext caches the message catalogs it has
 * loaded.
 */
void setLanguageOverride(const std::string &code);

/**
 * Logs where the message catalogs were looked up, which languages were found
 * and which language is used. Meant to be called once the log file is set up.
 */
void logLanguageSetup();
