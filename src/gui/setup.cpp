/*----------------------------------------------------------------
 * setup.c -- setup routines
 *----------------------------------------------------------------
 *  This module takes care of everything relating to the
 *  setup dialog.
 */

#include <allegro.h>
#include "gui.h"

#define CONFIG_FILE "tmw.ini"

extern unsigned int screen_mode;

DIALOG_PLAYER *player_setup;    /* Pointer to player's dialog */
bool show_player_setup;         /* Switch for setup dialog */

typedef struct { 
  int width, height;
  char* desc; 
} LIST;                         /* New type for holding video modes */

int apply_setup(int msg, DIALOG *d, int c);  /* Need this here in order to access dialog directly */

/* 
 * Hard coded list cuz get_gfx_mode_list fails with AUTODETECT 
 */
const LIST mode_list[] = {           
  { 1024,768, "1024x768" },
  { 800,600, "800x600" }, 
  { 640,480, "640x480"} 
};

/*
 * Get video modes list for setup
 */
char *get_listbox_video_modes(int index, int *list_size)
{ 
  if (index < 0 & index < 4) {
    *list_size = 3;
    return NULL;
  } else
    return mode_list[index].desc;					  
}

/*
 * Calls orginal tmw_button_proc and If D_O_K was cliked calls function in dp3
 */
int tmw_ok_button_proc(int msg, DIALOG *d, int c)
{
  int ret;
  
  ret = tmw_button_proc(msg, d, c); // returns D_CLOSE when D_O_K was clicked otherwise D_O_K
  
  if(ret == D_CLOSE && d->dp3 != NULL)
    return ((int(*)(void))d->dp3)();
  
  return ret;
}

/* 
 * Array for dialog objects and callbacks 
 */
DIALOG setup_dialog[] =
  {
    /* Dialog proc*/
    { tmw_dialog_proc,     300, 300,  300,  200,   0,  0,    0,      0,       0,   0,    (char *)"Setup",                NULL, NULL  },
    
    /* Descriptions */
    { tmw_text_proc,       305, 320,    0,    0,   0,  0,    0,      0,       0,   0,    (char *)"Display",              NULL, NULL  },
    { tmw_text_proc,       435, 320,    0,    0,   0,  0,    0,      0,       0,   0,    (char *)"Strech modes",         NULL, NULL  },
    
    /* List */
    { tmw_list_proc,	   305, 345,  100,  100,   0,  0,    0,      0,       0,   0,    (char *)get_listbox_video_modes, NULL, NULL  },
    
    /* Radio buttons */
    //{ tmw_radio_proc,      435, 339,  160,   19,   0,  0,    0,      0,       0,   0,    (char *)"Normal",               NULL, NULL  },
    //{ tmw_radio_proc,      435, 358,  160,   19,   0,  0,    0,      0,       0,   0,    (char *)"2xSaI",                NULL, NULL  },
    //{ tmw_radio_proc,      435, 377,  160,   19,   0,  0,    0,      0,       0,   0,    (char *)"SuperEagle",           NULL, NULL  },
    
    /* Buttons */
    { tmw_ok_button_proc,  315, 470,   40,   20,   0,  0,    0,      D_EXIT,       0,   0,    (char *)"&Ok",             NULL, (int*)apply_setup },
    { tmw_ok_button_proc,  535, 470,   40,   20,   0,  0,    0,      D_EXIT,       0,   0,    (char *)"&Cancel",         NULL, NULL  },
    { NULL,                  0,   0,    0,    0,   0,  0,    0,      0,            0,   0,                 NULL,         NULL, NULL  }
  };

/*
 * Apply setup settings
 */
int apply_setup(int msg, DIALOG *d, int c) {
  
  int index = 0;
  int index_loaded = 0, value;
  
  /* Load settings from file */
  if(get_config_int("[settings]","selected_vmode",value) == 1)
    index_loaded = value;
  
  /* Check if settings changed if so use and save them */
  if((index = setup_dialog[3].d1) != index_loaded) {
    set_config_int("[settings]","selected_vmode",index);
    flush_config_file();
  }
  
  /* 
     if(set_gfx_mode(screen_mode, list[index].width, list[index].height, 0, 0)) 
     error(allegro_error);
  */
  return D_CLOSE;                   // closes the dialog
}

/*
 * Initialize setup dialog
 */
void init_setup(void) {
  int value = 0;
  
  show_player_setup = false;        // setup starts as hidden
  
  /* Initialize */
  player_setup = init_dialog(setup_dialog, -1);
  position_dialog(setup_dialog, 300, 200);
  set_config_file(CONFIG_FILE);
  
  /* Set previous selections */
  value = get_config_int("[settings]","selected_vmode",0);
  setup_dialog[3].d1 = value;
}

/*
 * Update setup dialog 
 */
void update_setup(void) {
  if(show_player_setup) {
    if(gui_update(player_setup) == 0) show_player_setup = false;
  }    
}
