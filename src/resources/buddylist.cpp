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

#include "buddylist.h"
#include <iostream>

BuddyList::BuddyList()
{
    // Open buddy file
    buddyXmlwriterFilename("buddy.xml");

    // Load buddy from file
    // TODO
}

BuddyList::~BuddyList()
{
    int rc;

    /* Close last element. */
    rc = xmlTextWriterEndElement(writer);
    if (rc < 0) {
         std::cerr << "buddyXmlwriterFilename: Error at xmlTextWriterEndElement" << std::endl;
        return;
    }

    xmlFreeTextWriter(writer);
}
void BuddyList::buddyXmlwriterFilename(const char *uri)
{
    int rc;

    // Create a new XmlWriter for uri, with no compression
    writer = xmlNewTextWriterFilename(uri, 0);
    if (writer == NULL) {
         std::cerr << "buddyXmlwriterFilename: Error creating the xml writer" << std::endl;
        return;
    }

    // Start with the xml default version and UTF-8 encoding
    rc = xmlTextWriterStartDocument(writer, NULL, "UTF-8", NULL);
    if (rc < 0) {
         std::cerr << "buddyXmlwriterFilename: Error at xmlTextWriterStartDocument" << std::endl;
        return;
    }

    // Start with the root named "LIST"
    rc = xmlTextWriterStartElement(writer, BAD_CAST "LIST");
    if (rc < 0) {
             std::cerr << "buddyXmlwriterFilename: Error at xmlTextWriterStartElement" << std::endl;
        return;
    }
}

bool BuddyList::addBuddy(const std::string buddy)
{
    int rc;

    for (buddyit = buddylist.begin(); buddyit != buddylist.end(); buddyit++)
    {
        // Buddy already exist
        if (*buddyit == buddy) return false;
    }

    // Buddy doesnt exist, add it
    buddylist.push_back(buddy);

    // Store buddy as a child of "PEOPLE"
    rc = xmlTextWriterWriteElement(writer, BAD_CAST "PEOPLE", (xmlChar *) buddy.c_str());
    if (rc < 0) {
        std::cerr << "buddyXmlwriterFilename: Error at xmlTextWriterWriteElement" << std::endl;
        return false;
    }

    return true;
}

bool BuddyList::removeBuddy(const std::string buddy)
{
    if (buddylist.size() > 0) {
        for (buddyit = buddylist.begin(); buddyit != buddylist.end(); buddyit++)
        {
            // Buddy exist, remove it
            if (*buddyit == buddy) {
                buddylist.remove(buddy);
                return true;
            }
        }
    }

    // Buddy doesnt exist
    return false;
}

int  BuddyList::getNumberOfElements(void)
{
    return buddylist.size();
}

std::string BuddyList::getElementAt(int number)
{
    if (number <= (int) buddylist.size() - 1)
    {
        buddyit = buddylist.begin();
        std::advance(buddyit, number);
        return *buddyit;
    }

    return "";
}
