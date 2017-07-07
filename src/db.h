#ifndef DB_H
#define DB_H

int db_add_stream(
    const char * filename, 
    float time, 
    int vehicle_id);

void db_insert_detections(
    int width,
    int height,
    int num, 
    float thresh, 
    box *boxes, 
    float **probs, 
    char **names,  
    int classes,
    int stream_id,
    int frame_index);
    
#endif