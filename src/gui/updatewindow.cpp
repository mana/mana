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
#include "gui.h"
#include "../main.h"
#include "../log.h"
#include <curl/curl.h>
#include <sstream>
#include <iostream>
#include <cstdio>
#include "SDL_thread.h"

UpdaterWindow *updaterWindow;
float progress = 0.0f;
SDL_Thread *thread;
std::string updateHost = "themanaworld.org/files";
bool downloadComplete = false;

UpdaterWindow::UpdaterWindow()
    : Window("Updating...")
{
    int h = 100;
    int w = 320;
    setContentSize(w, h);

    label = new gcn::Label("Connecting...");
    label->setPosition(5,5);
    progressBar = new ProgressBar(0.0, 5, 25, w - 10, 40, 37, 70, 23);
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
}

void UpdaterWindow::draw(gcn::Graphics *graphics)
{
    Window::draw(graphics);
}

void UpdaterWindow::action(const std::string& eventId)
{
    if (eventId == "cancel") {
        state = EXIT;
    }
    else if (eventId == "play") {
        state = LOGIN;
    }
}

int updateProgress(void *ptr,
                      double t, /* dltotal */
                      double d, /* dlnow */
                      double ultotal,
                      double ulnow)
{
    std::stringstream labelString;
    progress = d/t;
    labelString << (char *)ptr << " (" << (int)(progress*100) << "%)";
    updaterWindow->setLabel(labelString.str());
    updaterWindow->setProgress(progress);

    if (state != UPDATE) {
        // If the action was canceled return an error code to stop the thread
        return -1;
    }

    return 0;
}

int downloadThread(void *ptr)
{
    CURL *curl;
    CURLcode res;
    FILE *outfile;
    std::string fileName((char *)ptr);
    std::string url(updateHost);
    url += "/" + fileName;
    logger->log("Downloading: %s", url.c_str());

    curl = curl_easy_init();
    if (curl)
    {
        downloadComplete = false;
        progress = 0.0f;
        // TODO: download in the proper folder (data?)
        outfile = fopen(fileName.c_str(), "wb");
        curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, outfile);
        curl_easy_setopt(curl, CURLOPT_NOPROGRESS, 0);
        curl_easy_setopt(curl, CURLOPT_PROGRESSFUNCTION, updateProgress);
        curl_easy_setopt(curl, CURLOPT_PROGRESSDATA, ptr);

        res = curl_easy_perform(curl);

        fclose(outfile);
        curl_easy_cleanup(curl);
        downloadComplete = true;
    }

    return 0;
}

int download(std::string url)
{
    thread = SDL_CreateThread(downloadThread, (void *)url.c_str());

    if (thread == NULL) {
        logger->log("Unable to create thread");
    }

    return 0;
}

void updateData()
{
    updaterWindow = new UpdaterWindow();
    state = UPDATE;
    
    std::string updateHost =
            config.getValue("updatehost", "themanaworld.org/files");
    // Try to download the updates list
    download("resources.txt");
    std::ifstream in;

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
        
        // If not alredy downloading another file
        if (downloadComplete) {
            // Try to open resources.txt
            if (!in.is_open())
            {
                in.open("resources.txt");
                if (!in.is_open())
                {
                    logger->error("Unable to open resources.txt");
                }
                // TODO: check for error 404
            }
            else {
                if (!in.eof())
                {
                    // Download each update
                    std::string line;
                    getline(in, line);
                    download(line);
                }
                else {
                    // All updates downloaded
                    updaterWindow->enable();
                    updaterWindow->setLabel("Completed");
                }
            }
        }
        
        gui->logic();

        login_wallpaper->draw(screen, 0, 0);
        gui->draw();
        guiGraphics->updateScreen();
    }
    
    in.close();
    
    SDL_WaitThread(thread, NULL);

    delete updaterWindow;
}
