#ifndef _IMAGE_H
#define _IMAGE_H

#include <string>
#include <vector>
#include <iostream>
#include <allegro.h>
#include "../log.h"
using namespace std;

class IMAGE {
  protected:
    int offset_x, offset_y;
  public:
    IMAGE(int offset_x, int offset_y) {
      this->offset_x = offset_x;
      this->offset_y = offset_y;
    }  
    virtual void draw(BITMAP *dest, int x, int y) = 0;
};

class RLE_IMAGE : public IMAGE {
  private:
    RLE_SPRITE *src;
  public:
    RLE_IMAGE(RLE_SPRITE *src, int offset_x, int offset_y) : IMAGE(offset_x, offset_y) {
      this->src = src;
    }
    ~RLE_IMAGE() {
      destroy_rle_sprite(src);
    }  
    void draw(BITMAP *dest, int x, int y) {
      draw_rle_sprite(dest, src, x+offset_x, y+offset_y);
    }    
};

class VIDEO_IMAGE : public IMAGE {
  private:
    BITMAP *src;
  public:
    VIDEO_IMAGE(BITMAP *src, int offset_x, int offset_y) : IMAGE(offset_x, offset_y) {
      this->src = src;
    }
    ~VIDEO_IMAGE() {
      destroy_bitmap(src);
    }
    void draw(BITMAP *dest, int x, int y) {
      masked_blit(src, dest, 0, 0, x+offset_x, y+offset_y, src->w, src->h);
    }
};

class SPRITESET {
  private:
    int get_property(DATAFILE *datafile, int type) {
      return atoi(get_datafile_property(datafile, type));
    }  
  public:
    vector<IMAGE *> spriteset; 
         
    SPRITESET(string filename) {
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
            temp_image = new VIDEO_IMAGE(temp_video_bitmap, get_property(&datafile[i], DAT_ID('X','C','R','P')), get_property(&datafile[i], DAT_ID('Y','C','R','P')));
          } else {
            warning("You ran out of video memory!");
            temp_image = new RLE_IMAGE((RLE_SPRITE*)datafile[i].dat, get_property(&datafile[i], DAT_ID('X','C','R','P')), get_property(&datafile[i], DAT_ID('Y','C','R','P')));
          }          
        } else {
          temp_image = new RLE_IMAGE((RLE_SPRITE*)datafile[i].dat, get_property(&datafile[i], DAT_ID('X','C','R','P')), get_property(&datafile[i], DAT_ID('Y','C','R','P')));
        }
        spriteset.push_back(temp_image);
        i++;
      }  
      // Do not unload the datafile, instead just use the allocated memory
      // directly without making copies.
      //unload_datafile(datafile);
    }
};

#endif
