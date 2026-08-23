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

#include "utils/language.h"

#include "log.h"
#include "utils/gettext.h"

#include <SDL.h>

#include <algorithm>
#include <clocale>
#include <cstdlib>
#include <cstring>
#include <filesystem>
#include <map>

#ifdef _WIN32
#include <windows.h>
#include <winnls.h>
#endif

#ifdef __APPLE__
#include "utils/specialfolder.h"
#endif

/**
 * The names of the languages we have translations for, in the language
 * itself. Languages without an entry here are listed by their code.
 */
static const std::map<std::string, std::string> languageNames = {
    { "ar",    "العربية" },
    { "bg",    "Български" },
    { "ca",    "Català" },
    { "cs",    "Čeština" },
    { "da",    "Dansk" },
    { "de",    "Deutsch" },
    { "en",    "English" },
    { "en_GB", "English (United Kingdom)" },
    { "eo",    "Esperanto" },
    { "es",    "Español" },
    { "et",    "Eesti" },
    { "fa",    "فارسی" },
    { "fi",    "Suomi" },
    { "fr",    "Français" },
    { "he",    "עברית" },
    { "hr",    "Hrvatski" },
    { "hu",    "Magyar" },
    { "id",    "Bahasa Indonesia" },
    { "it",    "Italiano" },
    { "ja",    "日本語" },
    { "ka",    "ქართული" },
    { "nb",    "Norsk bokmål" },
    { "nds",   "Plattdüütsch" },
    { "nl",    "Nederlands" },
    { "pl",    "Polski" },
    { "pt",    "Português" },
    { "pt_BR", "Português (Brasil)" },
    { "ru",    "Русский" },
    { "sk",    "Slovenčina" },
    { "sq",    "Shqip" },
    { "sv",    "Svenska" },
    { "ta",    "தமிழ்" },
    { "te",    "తెలుగు" },
    { "th",    "ไทย" },
    { "tr",    "Türkçe" },
    { "uk",    "Українська" },
    { "zh_CN", "简体中文" },
    { "zh_HK", "繁體中文 (香港)" },
    { "zh_TW", "繁體中文 (台灣)" },
};

/**
 * The directory the message catalogs were looked up in, as passed to
 * bindtextdomain.
 */
static std::string localeDir;

/**
 * The language gettext resolved from the system configuration, as determined
 * before any override was applied. Empty when no message catalog is used.
 */
static std::string systemLanguage;

static void setEnvironmentVariable(const char *name, const std::string &value)
{
#ifdef _WIN32
    _putenv_s(name, value.c_str());
#else
    setenv(name, value.c_str(), 1);
#endif
}

#ifdef _WIN32
/**
 * Returns the languages the user prefers for the interface, in order and in
 * the form gettext expects.
 *
 * gettext can query these itself, but only when the GETTEXT_MUI environment
 * variable is set. Otherwise it derives the language from the Windows locale,
 * which it turns into "C" whenever that locale is not one it knows.
 */
static std::vector<std::string> getPreferredUILanguages()
{
    ULONG numLanguages = 0;
    ULONG bufferSize = 0;
    if (!GetUserPreferredUILanguages(MUI_LANGUAGE_NAME, &numLanguages, nullptr,
                                     &bufferSize))
        return {};

    std::wstring buffer(bufferSize, L'\0');
    if (!GetUserPreferredUILanguages(MUI_LANGUAGE_NAME, &numLanguages,
                                     buffer.data(), &bufferSize))
        return {};

    // The names are separated by null characters and the list ends with an
    // empty one. They are ASCII, but use de-DE where gettext wants de_DE.
    std::vector<std::string> languages;
    size_t pos = 0;

    while (pos < buffer.size() && buffer[pos] != L'\0')
    {
        std::string language;

        for (; pos < buffer.size() && buffer[pos] != L'\0'; ++pos)
            language += buffer[pos] == L'-' ? '_'
                                            : static_cast<char>(buffer[pos]);
        ++pos;

        languages.push_back(language);

        // Windows may name the script as well, as in zh-Hans-CN, from which
        // gettext only falls back to the bare language. Add the form without
        // the script, so that a catalog like zh_CN is still matched.
        const size_t script = language.find('_');
        const size_t territory = language.rfind('_');
        if (script != territory)
            languages.push_back(language.substr(0, script)
                                + language.substr(territory));
    }

    return languages;
}
#endif // _WIN32

/**
 * The language override that was applied, if any.
 */
static std::string languageOverride;

/**
 * Returns the language of the message catalog gettext is currently using,
 * which is named in its header. Returns an empty string when no catalog is
 * loaded, which means the untranslated strings are used.
 */
static std::string currentLanguage()
{
    // Looking up the empty string returns the header of the loaded catalog
    const std::string header = '\n' + std::string(gettext(""));
    const std::string field = "\nLanguage: ";

    const size_t pos = header.find(field);
    if (pos == std::string::npos)
        return std::string();

    const size_t start = pos + field.size();
    return header.substr(start, header.find('\n', start) - start);
}

void initInternationalization()
{
#if ENABLE_NLS
#ifdef _WIN32
    // Tell gettext which languages the user prefers, since on Windows it does
    // not look them up by itself. LC_MESSAGES takes a single locale name and
    // keeps gettext out of the "C" locale, in which it ignores LANGUAGE, while
    // LANGUAGE takes the whole list.
    if (const auto languages = getPreferredUILanguages(); !languages.empty())
    {
        std::string list;
        for (auto &language : languages)
        {
            if (!list.empty())
                list += ':';
            list += language;
        }

        setEnvironmentVariable("LC_MESSAGES", languages.front());
        setEnvironmentVariable("LANGUAGE", list);
    }
#endif // _WIN32

    setlocale(LC_MESSAGES, "");

    std::string basePath;
    if (char *sdlBasePath = SDL_GetBasePath())
    {
        basePath = sdlBasePath;
        SDL_free(sdlBasePath);
    }

#ifdef __APPLE__
    localeDir = getResourcesLocation() + "/Translations";
#elif defined __linux__
    // When running from an AppImage the compiled-in absolute path needs to
    // be resolved against the mount point, which the runtime sets as APPDIR.
    localeDir = LOCALEDIR;
    if (const char *appDir = getenv("APPDIR"))
        localeDir = appDir + localeDir;
#elif defined _WIN32
    // LOCALEDIR is relative to the installation directory on Windows.
    localeDir = basePath + LOCALEDIR;
#else
    localeDir = LOCALEDIR;
#endif

    // When running the client from the build directory, use the catalogs
    // compiled there rather than any that may be installed system-wide.
    const auto buildDir = std::filesystem::path(BUILD_DIR).generic_string();
    if (std::filesystem::path(basePath).generic_string().rfind(buildDir, 0) == 0)
        localeDir = buildDir + "po/locale";

    bindtextdomain("mana", localeDir.c_str());
    bind_textdomain_codeset("mana", "UTF-8");
    textdomain("mana");

    // Needs to happen before any language override is applied, since gettext
    // caches the catalog it looked up a message in.
    systemLanguage = currentLanguage();
#endif // ENABLE_NLS
}

/**
 * Collects the languages for which a message catalog is installed. English is
 * always available, since it needs no catalog.
 */
static std::vector<LanguageInfo> collectAvailableLanguages()
{
    std::vector<LanguageInfo> languages;

    auto addLanguage = [&](const std::string &code) {
        auto it = languageNames.find(code);
        languages.push_back({ code, it != languageNames.end() ? it->second
                                                              : code });
    };

    addLanguage("en");

    // Message catalogs are installed as <localedir>/<code>/LC_MESSAGES/mana.mo.
    // Without translations enabled the directory is empty and no languages get
    // added here.
    std::error_code error;
    std::filesystem::directory_iterator it { localeDir, error };
    const std::filesystem::directory_iterator end;

    for (; !error && it != end; it.increment(error))
    {
        const std::string code = it->path().filename().string();
        if (code == "en")
            continue;

        const auto catalog = it->path() / "LC_MESSAGES" / "mana.mo";
        std::error_code fileError;
        if (std::filesystem::is_regular_file(catalog, fileError))
            addLanguage(code);
    }

    std::sort(languages.begin(), languages.end(),
              [](const LanguageInfo &a, const LanguageInfo &b) {
                  return a.name < b.name;
              });

    return languages;
}

static const LanguageInfo *findLanguage(const std::string &code)
{
    auto &languages = getAvailableLanguages();
    auto it = std::find_if(languages.begin(), languages.end(),
                           [&](const LanguageInfo &language) {
                               return language.code == code;
                           });
    return it != languages.end() ? &*it : nullptr;
}

const LanguageInfo &getSystemLanguage()
{
    if (auto *language = findLanguage(systemLanguage))
        return *language;

    // Without a catalog the untranslated strings are used, which are English
    return *findLanguage("en");
}

const std::vector<LanguageInfo> &getAvailableLanguages()
{
    static const std::vector<LanguageInfo> languages =
            collectAvailableLanguages();
    return languages;
}

void setLanguageOverride(const std::string &code)
{
#if ENABLE_NLS
    if (code.empty())
        return;

    languageOverride = code;

    // The LANGUAGE variable takes precedence over the messages locale
    setEnvironmentVariable("LANGUAGE", code);

#ifdef _WIN32
    // On Windows gettext derives the locale from the environment as well, and
    // it ignores LANGUAGE while that locale is "C". Setting the messages
    // locale to the chosen language keeps it out of that case.
    setEnvironmentVariable("LC_MESSAGES", code);
#else
    // LANGUAGE is ignored while the messages locale is "C", "C.<encoding>" or
    // "POSIX", so in that case activate another locale. Which one hardly
    // matters, since LANGUAGE determines the message catalog that gets used.
    const char *messagesLocale = setlocale(LC_MESSAGES, nullptr);
    if (!messagesLocale
            || (messagesLocale[0] == 'C'
                && (messagesLocale[1] == '\0' || messagesLocale[1] == '.'))
            || strcmp(messagesLocale, "POSIX") == 0)
    {
        const std::string candidates[] = {
            code + ".UTF-8",    // matches when the code includes the territory
            code,
            "en_US.UTF-8",
        };

        if (std::none_of(std::begin(candidates), std::end(candidates),
                         [](const std::string &candidate) {
                             return setlocale(LC_MESSAGES, candidate.c_str());
                         }))
        {
            Log::warn("Failed to activate a locale, the language override "
                      "'%s' will not be applied", code.c_str());
        }
    }
#endif // _WIN32
#endif // ENABLE_NLS
}

void logLanguageSetup()
{
    Log::info("Message catalogs: %s", localeDir.empty() ? "none"
                                                        : localeDir.c_str());

    std::string codes;
    for (auto &language : getAvailableLanguages())
    {
        if (!codes.empty())
            codes += ' ';
        codes += language.code;
    }
    Log::info("Available languages: %s", codes.c_str());

    Log::info("System language: %s, override: %s",
              systemLanguage.empty() ? "English (no catalog loaded)"
                                     : systemLanguage.c_str(),
              languageOverride.empty() ? "none" : languageOverride.c_str());

    // These are what gettext derives the language from
    for (const char *variable : { "LANGUAGE", "LC_ALL", "LC_MESSAGES", "LANG" })
        if (const char *value = getenv(variable); value && *value)
            Log::info("  %s=%s", variable, value);

    // LANGUAGE only applies while this is not a "C" locale
    if (const char *messagesLocale = setlocale(LC_MESSAGES, nullptr))
        Log::info("  messages locale: %s", messagesLocale);
}
