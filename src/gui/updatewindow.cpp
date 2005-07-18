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

UpdaterWindow *updaterWindow;
SDL_Thread *thread = NULL;
std::string updateHost = "themanaworld.org/files";
std::string currentFile = "news.txt";
bool downloadComplete = true;
int downloadStatus = UPDATE_NEWS;
std::string basePath = "";
bool memoryTransfer = true;
int downloadedBytes = 0;
char *memoryBuffer = NULL;
unsigned int fileIndex = 0;

UpdaterWindow::UpdaterWindow():
    Window("Updating...")
{
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
        if (downloadStatus == UPDATE_COMPLETE)
        {
            state = EXIT;
        }
        else {
            downloadStatus = UPDATE_ERROR;
        }
    }
    else if (eventId == "play") {
        state = LOGIN;
    }
}

void UpdaterWindow::loadNews()
{
    int contentsLength = downloadedBytes;
    char *fileContents = memoryBuffer;

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

int updateProgress(void *ptr, double dt, double dn, double ut, double un)
{
    float progress = dn/dt;
    if (progress < 0)
    {
        progress = 0.0f;
    }
    std::stringstream progressString;
    progressString << currentFile << " (" << ((int)(progress*100)) << "%)";
    updaterWindow->setLabel(progressString.str().c_str());
    updaterWindow->setProgress(progress);

    if (state != UPDATE || downloadStatus == UPDATE_ERROR) {
        // If the action was canceled return an error code to stop the thread
        return -1;
    }

    return 0;
}

size_t memoryWrite(void *ptr, size_t size, size_t nmemb, FILE *stream)
{
    if (memoryTransfer) {
        memoryBuffer = (char *)realloc(memoryBuffer, downloadedBytes + nmemb + 1);
        if (memoryBuffer) {
            memcpy(&(memoryBuffer[downloadedBytes]), ptr, nmemb);
            downloadedBytes += nmemb;
            memoryBuffer[downloadedBytes] = 0;
        }
        return nmemb;
    }
    return fwrite(ptr, size, nmemb, stream);
}

int downloadThread(void *ptr)
{
    CURL *curl;
    CURLcode res;
    FILE *outfile;
    std::string outFilename;
    std::string url(updateHost + "/" + currentFile);

    curl = curl_easy_init();
    if (curl)
    {
        // Download current file as a temp file
        logger->log("Downloading: %s", url.c_str());
        // Download in the proper folder : ./data under win,
        // /home/user/.tmw/data for unices
        if (memoryTransfer)
        {
            downloadedBytes = 0;
            curl_easy_setopt(curl, CURLOPT_FAILONERROR, 1);
            curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, memoryWrite);
            curl_easy_setopt(curl, CURLOPT_WRITEDATA, NULL);
        }
        else {
            outFilename =  basePath + "/data/download.temp";
            outfile = fopen(outFilename.c_str(), "wb");
            curl_easy_setopt(curl, CURLOPT_WRITEDATA, outfile);
        }
        curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
        curl_easy_setopt(curl, CURLOPT_NOPROGRESS, 0);
        curl_easy_setopt(curl, CURLOPT_PROGRESSFUNCTION, updateProgress);
        curl_easy_setopt(curl, CURLOPT_PROGRESSDATA, NULL);

        res = curl_easy_perform(curl);

        curl_easy_cleanup(curl);
        downloadComplete = true;
        if (res != 0) {
            downloadStatus = UPDATE_ERROR;
        }
        else if (!memoryTransfer) {
            fclose(outfile);
            // If the download was successful give the file the proper name
            // else it will be deleted later
            std::string newName(basePath + "/data/" + currentFile.c_str());
            rename(outFilename.c_str(), newName.c_str());
        }
    }

    return 0;
}

void download()
{
    downloadComplete = false;
    thread = SDL_CreateThread(downloadThread, NULL);

    if (thread == NULL) {
        logger->log("Unable to create thread");
        downloadStatus = UPDATE_ERROR;
    }
}

void updateData()
{
    std::ifstream in;
    std::vector<std::string> files;

    updaterWindow = new UpdaterWindow();
    state = UPDATE;

    updateHost = config.getValue("updatehost", "themanaworld.org/files");
    basePath = config.getValue("homeDir", ".");

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

        switch (downloadStatus) {
            case UPDATE_ERROR:
                SDL_WaitThread(thread, NULL);
                updaterWindow->addRow("");
                updaterWindow->addRow("##1  The update process is incomplete.");
                updaterWindow->addRow("##1  It is strongly recommended that");
                updaterWindow->addRow("##1  you try again later");
                downloadStatus = UPDATE_COMPLETE;
                break;
            case UPDATE_NEWS:
                if (downloadComplete) {
                    // Try to open news.txt
                    updaterWindow->loadNews();
                    // Doesn't matter if it couldn't find news.txt,
                    // go to the next step
                    currentFile = "resources.txt";
                    if (memoryBuffer != NULL)
                    {
                        free(memoryBuffer);
                        memoryBuffer = NULL;
                    }
                    download();
                    downloadStatus = UPDATE_LIST;
                }
                break;
            case UPDATE_LIST:
                if (downloadComplete) {
                    if (memoryBuffer != NULL)
                    {
                        // Tokenize and add each line separately
                        char *line = strtok(memoryBuffer, "\n");
                        while (line != NULL)
                        {
                            files.push_back(line);
                            line = strtok(NULL, "\n");
                        }
                        memoryTransfer = false;
                        downloadStatus = UPDATE_RESOURCES;
                    }
                    else {
                        logger->log("Unable to download resources.txt");
                        downloadStatus = UPDATE_ERROR;
                    }
                }
                break;
            case UPDATE_RESOURCES:
                if (downloadComplete) {
                    if (thread)
                    {
                        SDL_WaitThread(thread, NULL);
                        thread = NULL;
                    }
                    if (fileIndex < files.size())
                    {
                        currentFile = files[fileIndex];
                        std::ifstream temp(
                                (basePath + "/data/" + currentFile).c_str());
                        if (!temp.is_open()) {
                            temp.close();
                            download();
                        }
                        else {
                            logger->log("%s already here", currentFile.c_str());
                        }
                        fileIndex++;
                    }
                    else {
                        // Download of updates completed
                        downloadStatus = UPDATE_COMPLETE;
                    }
                }
                break;
            case UPDATE_COMPLETE:
                updaterWindow->enable();
                updaterWindow->setLabel("Completed");
                break;
            case UPDATE_IDLE:
                break;
        }

        gui->logic();

        guiGraphics->drawImage(login_wallpaper, 0, 0);
        gui->draw();
        guiGraphics->updateScreen();
    }

    free(memoryBuffer);
    in.close();
    // Remove downloaded files
    remove((basePath + "/data/news.txt").c_str());
    remove((basePath + "/data/resources.txt").c_str());
    remove((basePath + "/data/download.temp").c_str());

    delete updaterWindow;
}
