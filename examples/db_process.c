#include "network.h"
#include "detection_layer.h"
#include "region_layer.h"
#include "cost_layer.h"
#include "utils.h"
#include "parser.h"
#include "box.h"
#include "image.h"
#include "db.h"
#include <sys/time.h>

#ifdef OPENCV

void db_process(
    char *cfgfile, 
    char *weightfile, 
    float thresh, 
    int cam_index, 
    const char *filename, 
    char **names, 
    int classes, 
    char *prefix, 
    float hier, 
    int w, 
    int h, 
    int frames, 
    int fullscreen)
{
    // configure the network properly...
    network net = parse_network_cfg(cfgfile);

    if(weightfile)
    {
        load_weights(&net, weightfile);
    }
 
    set_batch_network(&net, 1);
    
    printf("\n\n******************************************************************************************\n");
    
    // initiate a new stream in the database
    int stream_id = db_add_stream(filename, 0, 1);    
    printf("stream_id: %d\n", stream_id);
    printf("thresh: %f\n", thresh);
    
    srand(2222222);

    printf("video file: %s\n", filename);
    CvCapture * cap = cvCaptureFromFile(filename);

    if(!cap)
    {
        error("Couldn't connect to webcam.\n");
    }
    
    layer l = net.layers[net.n-1];
    
    float ** predictions = calloc(1, sizeof(float*));
    predictions[0] = (float *) calloc(l.outputs, sizeof(float));
    
    float ** probs = (float **)calloc(l.w * l.h * l.n, sizeof(float *));
    box * boxes = (box *)calloc(l.w * l.h * l.n, sizeof(box));

    for(int j = 0; j < l.w * l.h * l.n; ++j)
    {
        probs[j] = (float *)calloc(l.classes+1, sizeof(float));
    }

    image im = get_image_from_stream(cap);
    int status = 1;
    int frame_index = 0;
    float nms = 0.4;
    
    while(status != 0)
    {
        printf("frame_index: %d\n", frame_index);
        image sized = letterbox_image(im, net.w, net.h);
        layer l = net.layers[net.n-1];

        box *boxes = calloc(l.w*l.h*l.n, sizeof(box));
        float **probs = calloc(l.w*l.h*l.n, sizeof(float *));
        for(int j = 0; j < l.w*l.h*l.n; ++j)
        {
            probs[j] = calloc(l.classes + 1, sizeof(float *));
        }

        float *X = sized.data;
        network_predict(net, X);
        
        get_region_boxes(l, im.w, im.h, net.w, net.h, thresh, probs, boxes, 0, 0, thresh, 1);
        
        db_insert_detections(
            im.w, 
            im.h, 
            l.w * l.h * l.n, 
            thresh, 
            boxes, 
            probs, 
            names, 
            l.classes,
            stream_id,
            frame_index);
                
        status = fill_image_from_stream(cap, im);
        frame_index++;
        
        free_image(sized);
        free(boxes);
        free_ptrs((void **)probs, l.w*l.h*l.n);
    }
    
    free_image(im);
}

#else

void db_process(char *cfgfile, char *weightfile, float thresh, int cam_index, const char *filename, char **names, int classes, char *prefix, float hier, int w, int h, int frames, int fullscreen)
{
    fprintf(stderr, "db_process needs OpenCV for webcam images.\n");
}

#endif

