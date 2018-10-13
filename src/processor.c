#include <processor.h>
size_t count = 1;

void write_header(FILE* f, frame_header_t frame){
    size_t written = fwrite(&frame, 1, sizeof(frame_header_t), f);
    if(written < sizeof(frame_header_t))
        error("Wrote %u of %u frame header bytes\n", written,
                                            sizeof(frame_header_t));
}

void process_raw(frame_header_t frame, char* raw, size_t size){
    //Dump frame to an individual file in the order we receive them
    char fname[30];
    char* dirname = "output";
    FILE* f;
    size_t written;
    int res;

    struct stat s;
    res = stat(dirname, &s);
    if(res && errno == ENOENT){
        //create the output dir    
        res = mkdir(dirname, 0777);
        if(res){
            error("mkdir failed with errno: %d\n", errno);
            return;
        }
    }

    snprintf(fname, 30, "./%s/frame%lu.mp3", dirname, count++);
    f = fopen(fname, "w");
    if(f == NULL){
        error("unable to open file %s\n", fname);
        return;
    }

    write_header(f, frame);
    written = fwrite(raw, 1, size, f);
    if(written < size)
        error("Wrote %u of %u raw bytes to %s", written, size, fname);
    fclose(f);
}
