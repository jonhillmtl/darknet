#ifndef DB_PROCESS_H
#define DB_PROCESS_H

#include "image.h"

void db_process(
    char *cfgfile, 
    char *weightfile, 
    float thresh, 
    int cam_index, 
    const char *filename, 
    char **names, 
    int classes, 
    char *prefix, 
    float hier_thresh, 
    int w, 
    int h, 
    int fps, 
    int fullscreen);

#endif
