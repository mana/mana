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
 *  By ElvenProgrammer aka Eugenio Favalli (umperio@users.sourceforge.net)
 */

#ifndef _IMAGE_H
#define _IMAGE_H

#include <allegro.h>
#include <string>
#include <vector>
#include <iostream>
#include "../log.h"

class Image {
  protected:
    int offset_x, offset_y;
  public:
    Image(int offset_x, int offset_y) {
      this->offset_x = offset_x;
      this->offset_y = offset_y;
    }  
    virtual void draw(BITMAP *dest, int x, int y) = 0;
};

class RleImage : public Image {
  private:
    RLE_SPRITE *src;
  public:
    RleImage(RLE_SPRITE *src, int offset_x, int offset_y) : Image(offset_x, offset_y) {
      this->src = src;
    }
    virtual ~RleImage() {
      destroy_rle_sprite(src);
    }  
    void draw(BITMAP *dest, int x, int y) {
      draw_rle_sprite(dest, src, x+offset_x, y+offset_y);
    }    
};

class VideoImage : public Image {
  private:
    BITMAP *src;
  public:
    VideoImage(BITMAP *src, int offset_x, int offset_y) : Image(offset_x, offset_y) {
      this->src = src;
    }
    virtual ~VideoImage() {
      destroy_bitmap(src);
    }
    void draw(BITMAP *dest, int x, int y) {
      masked_blit(src, dest, 0, 0, x+offset_x, y+offset_y, src->w, src->h);
    }
};

class Spriteset {
    private:
        int get_property(DATAFILE *datafile, int type) {
            return atoi(get_datafile_property(datafile, type));
        }  
    public:
        std::vector<Image *> spriteset; 
         
    Spriteset(std::string filename) {
      DATAFILE *datafile = load_datafile(filename.c_str());
      if(!datafile)error("Unable to load graphic file: " + filename);
      int i = 0;
      while(datafile[i].type!=DAT_END) {
        IMAGE *temp_image;

        if(gfx_capabilities & GFX_HW_VRAM_BLIT) {
          BITMAP *temp_video_bitmap = create_video_bitmap(((RLE_SPRITE *)datafile[i].dat)->w, ((RLE_SPRITE *)datafile[i].dat)->h);
          if(temp_video_bitmap) {
            clear_to_color(temp_video_bitmap, makecol(255,0,255));
            draw_rle_sprite(temp_video_bitmap, (RLE_SPRITE *)datafile[i].dat, 0, 0);
            temp_image = new VideoImage(temp_video_bitmap, get_property(&datafile[i], DAT_ID('X','C','R','P')), get_property(&datafile[i], DAT_ID('Y','C','R','P')));
          } else {
            warning("You ran out of video memory!");
            temp_image = new RleImage((RLE_SPRITE*)datafile[i].dat, get_property(&datafile[i], DAT_ID('X','C','R','P')), get_property(&datafile[i], DAT_ID('Y','C','R','P')));
          }          
        } else {
          temp_image = new RleImage((RLE_SPRITE*)datafile[i].dat, get_property(&datafile[i], DAT_ID('X','C','R','P')), get_property(&datafile[i], DAT_ID('Y','C','R','P')));
        }
        spriteset.push_back(temp_image);
        i++;
      }  
    }
};

#endif
