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
#include <fstream>

BuddyList::BuddyList()
{
    // Find home dir
    findHomeDir();

    // Create buddy file
    buddyXmlwriterMemory();

    // Load buddy from file
    streamFile();
}

BuddyList::~BuddyList()
{
    int rc;

    std::ofstream outputStream(filename->c_str(), std::ios::trunc);
    if( !outputStream ) {
        std::cerr << "Error opening output stream" << std::endl;
        return;
    }

    /* Close last element. */
    rc = xmlTextWriterEndElement(writer);
    if (rc < 0) {
         std::cerr << "buddyXmlwriterFilename: Error at xmlTextWriterEndElement" << std::endl;
        return;
    }

    xmlFreeTextWriter(writer);
    xmlCleanupParser();

    /* Write and close file */
    outputStream << (const char*)buf->content;
    outputStream.close();
    xmlBufferFree(buf);

    delete filename;
}

void BuddyList::findHomeDir(void) {
    //TODO: Find homeDir
    filename = new std::string("buddy.xml");
}

void BuddyList::streamFile(void) {
    int ret;

    reader = xmlReaderForFile(filename->c_str(), NULL, 0);
    if (reader != NULL) {
        ret = xmlTextReaderRead(reader);
        while (ret == 1) {
            processNode();
            ret = xmlTextReaderRead(reader);
        }
        xmlFreeTextReader(reader);
        if (ret != 0) {
            std::cerr << filename << " : failed to parse" << std::endl;
        }
    } else {
         std::cerr << "Unable to open " << filename << std::endl;
    }
}

void BuddyList::processNode(void) {
    const xmlChar *name, *value;

    name = xmlTextReaderConstName(reader);
    if (name == NULL)
        name = BAD_CAST "--";

    value = xmlTextReaderConstValue(reader);

   if(!xmlTextReaderIsEmptyElement(reader) && xmlTextReaderHasValue(reader))
      addBuddy((char *)value);

}
void BuddyList::buddyXmlwriterMemory(void)
{
    int rc;

    /* Create a new XML buffer, to which the XML document will be
     * written */
    buf = xmlBufferCreate();
    if (buf == NULL) {
        std::cerr << "buddyXmlwriterMemory: Error creating the xml buffer" << std::endl;
        return;
    }

    // Create a new XmlWriter for uri, with no compression
    writer = xmlNewTextWriterMemory(buf, 0);
    if (writer == NULL) {
        std::cerr << "buddyXmlwriterMemory: Error creating the xml writer" << std::endl;
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

    // Store buddy in "PEOPLE"
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
    // TODO: Remove from buddy.xml

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
