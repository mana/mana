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

#include "updatewindow.h"
#include "ok_dialog.h"
#include "gui.h"
#include "../main.h"
#include "../log.h"
#include "../resources/resourcemanager.h"
#include <curl/curl.h>
#include <sstream>
#include <iostream>
#include <cstdio>
#include <SDL_thread.h>
#include <SDL_mutex.h>


UpdaterWindow::UpdaterWindow():
    Window("Updating...")
{
    m_thread = NULL;
    m_mutex = NULL;
    m_downloadStatus = UPDATE_NEWS;
    m_updateHost = "";
    m_currentFile = "news.txt";
    m_downloadComplete = true;
    m_basePath = "";
    m_storeInMemory = true;
    m_downloadedBytes = 0;
    m_memoryBuffer = NULL;
    m_curlError = new char[CURL_ERROR_SIZE];
    m_curlError[0] = 0;

    int h = 300;
    int w = 320;
    setContentSize(w, h);

    browserBox = new BrowserBox();
    browserBox->setOpaque(false);
    scrollArea = new ScrollArea(browserBox);
    scrollArea->setDimension(gcn::Rectangle(5, 5, 310, 190));
    label = new gcn::Label("Connecting...");
    label->setPosition(5,205);
    progressBar = new ProgressBar(0.0, 5, 225, w - 10, 40, 37, 70, 23);
    cancelButton = new Button("Cancel");
    cancelButton->setPosition(5, h - 5 - cancelButton->getHeight());
    cancelButton->setEventId("cancel");
    cancelButton->addActionListener(this);
    playButton = new Button("Play");
    playButton->setPosition(cancelButton->getX() + cancelButton->getWidth() + 5,
                            h - 5 - playButton->getHeight());
    playButton->setEventId("play");
    playButton->setEnabled(false);
    playButton->addActionListener(this);

    add(scrollArea);
    add(label);
    add(progressBar);
    add(cancelButton);
    add(playButton);

    cancelButton->requestFocus();
    setLocationRelativeTo(getParent());
}

UpdaterWindow::~UpdaterWindow()
{
    delete m_curlError;
    delete label;
    delete progressBar;
    delete cancelButton;
    delete playButton;
}

void UpdaterWindow::setProgress(float p)
{
    progressBar->setProgress(p);
}

void UpdaterWindow::setLabel(const std::string &str)
{
    label->setCaption(str);
    label->adjustSize();
}

void UpdaterWindow::enable()
{
    playButton->setEnabled(true);
    playButton->requestFocus();
}

void UpdaterWindow::action(const std::string& eventId)
{
    if (eventId == "cancel") {
        // Skip the updating process
        if (m_downloadStatus == UPDATE_COMPLETE)
        {
            state = EXIT;
        }
        else {
            m_downloadStatus = UPDATE_ERROR;
        }
    }
    else if (eventId == "play") {
        state = LOGIN;
    }
}

void UpdaterWindow::loadNews()
{
    int contentsLength = m_downloadedBytes;
    char *fileContents = m_memoryBuffer;

    if (!fileContents)
    {
        logger->log("Couldn't load news");
        return;
    }

    // Reallocate and include terminating 0 character
    fileContents = (char*)realloc(fileContents, contentsLength + 1);
    fileContents[contentsLength] = '\0';

    browserBox->clearRows();

    // Tokenize and add each line separately
    char *line = strtok(fileContents, "\n");
    while (line != NULL)
    {
        browserBox->addRow(line);
        line = strtok(NULL, "\n");
    }

    //free(fileContents);

    scrollArea->setVerticalScrollAmount(0);
    setVisible(true);
}

void UpdaterWindow::addRow(const std::string &row)
{
    browserBox->addRow(row);
    scrollArea->setVerticalScrollAmount(scrollArea->getVerticalMaxScroll());
}

int UpdaterWindow::updateProgress(void *ptr, double dt, double dn, double ut, double un)
{
    float progress = dn/dt;
    UpdaterWindow *uw = reinterpret_cast<UpdaterWindow *>(ptr);

    if (progress < 0)
    {
        progress = 0.0f;
    }
    std::stringstream progressString;
    progressString << uw->m_currentFile << " (" << ((int)(progress*100)) << "%)";
    uw->setLabel(progressString.str().c_str());
    uw->setProgress(progress);

    if (state != UPDATE || uw->m_downloadStatus == UPDATE_ERROR) {
        // If the action was canceled return an error code to stop the m_thread
        return -1;
    }

    return 0;
}

size_t UpdaterWindow::memoryWrite(void *ptr, size_t size, size_t nmemb, FILE *stream)
{
    UpdaterWindow *uw = reinterpret_cast<UpdaterWindow *>(stream);
    uw->m_memoryBuffer = (char *)realloc(uw->m_memoryBuffer, uw->m_downloadedBytes + nmemb * size + 1);
    if (uw->m_memoryBuffer)
    {
        memcpy(&(uw->m_memoryBuffer[uw->m_downloadedBytes]), ptr, nmemb * size);
        uw->m_downloadedBytes += nmemb;
        uw->m_memoryBuffer[uw->m_downloadedBytes] = 0;
    }
    return nmemb;
}

int UpdaterWindow::downloadThread(void *ptr)
{
    CURL *curl;
    CURLcode res;
    FILE *outfile = NULL;
    UpdaterWindow *uw = reinterpret_cast<UpdaterWindow *>(ptr);
    std::string outFilename;
    std::string url(uw->m_updateHost + "/" + uw->m_currentFile);

    curl = curl_easy_init();
    if (curl)
    {
        // Download current file as a temp file
        logger->log("Downloading: %s", url.c_str());
        // Download in the proper folder : ./data under win,
        // /home/user/.tmw/data for unices
        if (uw->m_storeInMemory)
        {
            uw->m_downloadedBytes = 0;
            curl_easy_setopt(curl, CURLOPT_FAILONERROR, 1);
            curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, UpdaterWindow::memoryWrite);
            curl_easy_setopt(curl, CURLOPT_WRITEDATA, ptr);
        }
        else
        {
            outFilename =  uw->m_basePath + "/data/download.temp";
            outfile = fopen(outFilename.c_str(), "wb");
            curl_easy_setopt(curl, CURLOPT_WRITEDATA, outfile);
        }
        curl_easy_setopt(curl, CURLOPT_ERRORBUFFER, uw->m_curlError);
        curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
        curl_easy_setopt(curl, CURLOPT_NOPROGRESS, 0);
        curl_easy_setopt(curl, CURLOPT_PROGRESSFUNCTION, UpdaterWindow::updateProgress);
        curl_easy_setopt(curl, CURLOPT_PROGRESSDATA, ptr);

        if ((res = curl_easy_perform(curl)) != 0)
        {
            uw->m_downloadStatus = UPDATE_ERROR;
            std::cerr << "curl error " << res << " : " << uw->m_curlError << std::endl;
        }

        curl_easy_cleanup(curl);
        uw->m_downloadComplete = true;

        if (!uw->m_storeInMemory)
        {
            fclose(outfile);
            // If the download was successful give the file the proper name
            // else it will be deleted later
            std::string newName(uw->m_basePath + "/data/" + uw->m_currentFile.c_str());
            rename(outFilename.c_str(), newName.c_str());
        }
    }

    return 0;
}

void UpdaterWindow::download()
{
    m_downloadComplete = false;
    m_thread = SDL_CreateThread(UpdaterWindow::downloadThread, this);

    if (m_thread == NULL) {
        logger->log("Unable to create m_thread");
        m_downloadStatus = UPDATE_ERROR;
    }
}

void UpdaterWindow::updateData()
{
    std::ifstream in;
    std::vector<std::string> files;

    state = UPDATE;
    unsigned int fileIndex = 0;

    m_updateHost = config.getValue("updatehost", "themanaworld.org/files");
    m_basePath = config.getValue("homeDir", ".");

    // Try to download the updates list
    download();

    while (state == UPDATE)
    {
        // Handle SDL events
        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            switch (event.type) {
                case SDL_QUIT:
                    state = EXIT;
                    break;

                case SDL_KEYDOWN:
                    if (event.key.keysym.sym == SDLK_ESCAPE)
                    {
                        state = EXIT;
                    }
                    break;
            }

            guiInput->pushInput(event);
        }

        switch (m_downloadStatus) {
            case UPDATE_ERROR:
                if (m_thread)
                {
                    SDL_WaitThread(m_thread, NULL);
                    m_thread = NULL;
                }
                addRow("");
                addRow("##1  The update process is incomplete.");
                addRow("##1  It is strongly recommended that");
                addRow("##1  you try again later");
                addRow(m_curlError);
                m_downloadStatus = UPDATE_COMPLETE;
                break;
            case UPDATE_NEWS:
                if (m_downloadComplete) {
                    // Try to open news.txt
                    loadNews();
                    // Doesn't matter if it couldn't find news.txt,
                    // go to the next step
                    m_currentFile = "resources.txt";
                    if (m_memoryBuffer != NULL)
                    {
                        free(m_memoryBuffer);
                        m_memoryBuffer = NULL;
                    }
                    download();
                    m_downloadStatus = UPDATE_LIST;
                }
                break;
            case UPDATE_LIST:
                if (m_downloadComplete) {
                    if (m_memoryBuffer != NULL)
                    {
                        // Tokenize and add each line separately
                        char *line = strtok(m_memoryBuffer, "\n");
                        while (line != NULL)
                        {
                            files.push_back(line);
                            line = strtok(NULL, "\n");
                        }
                        m_storeInMemory = false;
                        m_downloadStatus = UPDATE_RESOURCES;
                    }
                    else {
                        logger->log("Unable to download resources.txt");
                        m_downloadStatus = UPDATE_ERROR;
                    }
                }
                break;
            case UPDATE_RESOURCES:
                if (m_downloadComplete) {
                    if (m_thread)
                    {
                        SDL_WaitThread(m_thread, NULL);
                        m_thread = NULL;
                    }

                    if (fileIndex < files.size())
                    {
                        m_currentFile = files[fileIndex];
                        std::ifstream temp(
                                (m_basePath + "/data/" + m_currentFile).c_str());
                        if (!temp.is_open()) {
                            temp.close();
                            download();
                        }
                        else {
                            logger->log("%s already here", m_currentFile.c_str());
                        }
                        fileIndex++;
                    }
                    else {
                        // Download of updates completed
                        m_downloadStatus = UPDATE_COMPLETE;
                    }
                }
                break;
            case UPDATE_COMPLETE:
                enable();
                setLabel("Completed");
                break;
            case UPDATE_IDLE:
                break;
        }

        gui->logic();

        guiGraphics->drawImage(login_wallpaper, 0, 0);
        gui->draw();
        guiGraphics->updateScreen();
    }

    if (m_thread)
    {
         SDL_WaitThread(m_thread, NULL);
         m_thread = NULL;
    }

    free(m_memoryBuffer);
    in.close();
    // Remove downloaded files
    remove((m_basePath + "/data/news.txt").c_str());
    remove((m_basePath + "/data/resources.txt").c_str());
    remove((m_basePath + "/data/download.temp").c_str());
}
