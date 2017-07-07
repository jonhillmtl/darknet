#include <stdlib.h>
#include <libpq-fe.h>
#include "box.h"
#include "utils.h"

char * db_get_conninfo()
{
    return "host=localhost port=5432 dbname=bob_yolo connect_timeout=10";
}

PGconn * db_connection()
{
    return PQconnectdb(db_get_conninfo());
}

/**************************************************************************************************
*
* adds a stream to the database... does not (yet?) detect for duplicates of the filename
*
* return - the unique id of the stream that we just added
* 
*/ 
int db_add_stream(
    const char * filename, 
    float begin_time, 
    int vehicle_id)
{
    PGconn * conn = db_connection();
    
    char cbegin_time[40];
    char cvehicle_id[20];
    
    sprintf(cbegin_time, "%f", begin_time);
    sprintf(cvehicle_id, "%d", vehicle_id);
    
    const char * values[3] = {
        filename,
        cbegin_time,
        cvehicle_id
    };
    
    PGresult *res = PQexecParams(conn,
        "INSERT INTO streams (filename, begin_time, vehicle_id) VALUES ($1, $2, $3) RETURNING id;",
        3,
        NULL,
        values,
        NULL,
        NULL,
        0);
    
    if(conn)
    {
    	PQfinish(conn);
    } 

    int id = atoi(PQgetvalue(res, 0, 0));
    PQclear(res);    
    return id;
}    

void db_insert_detection(PGconn * conn, char * name, int x1, int y1, int x2, int y2, float prob, int stream_id, int frame_index)
{
    char cx1[10];
    char cy1[10];
    char cx2[10];
    char cy2[10];
    char cprob[20];
    char cstream_id[20];
    char cframe_index[20];
    
    sprintf(cx1, "%d", x1);
    sprintf(cy1, "%d", y1);
    sprintf(cx2, "%d", x2);
    sprintf(cy2, "%d", y2);
    sprintf(cprob, "%f", prob);
    sprintf(cstream_id, "%d", stream_id);
    sprintf(cframe_index, "%d", frame_index);
    
    const char *values[8] = {
        name,
        cx1,
        cy1,
        cx2,
        cy2,
        cprob,
        cstream_id,
        cframe_index
    };
        
    PGresult *res = PQexecParams(conn,
        "INSERT INTO detections (object_name, x1, y1, x2, y2, prob, stream_id, frame_index) VALUES ($1, $2, $3, $4, $5, $6, $7, $8);",
        8,
        NULL,
        values,
        NULL,
        NULL,
        0);
        
    PQclear(res);   
}

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
    int frame_index)
{
    PGconn * conn = db_connection();
    
    for(int i = 0; i < num; i++)
    {
        int class = max_index(probs[i], classes);
        float prob = probs[i][class];
        
        if(prob > thresh)
        {
            char * name = names[class];
            box b = boxes[i];

            int x1  = (b.x - b.w / 2.0) * width;
            int x2 = (b.x + b.w / 2.0) * width;
            int y1   = (b.y - b.h / 2.0) * height;
            int y2   = (b.y + b.h / 2.0) * height;

            if(x1 < 0) 
            {
                x1 = 0;
            }
            
            if(x2 > width - 1)
            {
                x2 = width -1;
            }
            
            if(y1 < 0)
            {
                y1 = 0;
            }
            
            if(y2 > height - 1)
            {
                y2 = height - 1;
            }
            
            db_insert_detection(conn, name, x1, y1, x2, y2, prob, stream_id, frame_index);
        }
    }
    
    if(conn)
    {
    	PQfinish(conn);
    }    
}