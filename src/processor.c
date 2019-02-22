#include <processor.h>
FILE* processed = NULL;
size_t frame_count = 1;
size_t count = 0;
size_t bcount = 1;

void write_header(FILE* f, frame_header_t frame){
    char buf[sizeof(frame_header_t)];
    write_header_bytes(frame, buf);
    size_t written = fwrite(buf, 1, sizeof(frame_header_t), f);
    if(written < sizeof(frame_header_t))
        error("Wrote %u of %u frame header bytes\n", written,
                                            sizeof(frame_header_t));
}

void write_crc(FILE* f, unsigned short crc){
    size_t written = fwrite(&crc, 1, sizeof(short), f);
    if(written < sizeof(short))
        error("Short write on crc\n");
}

void dump_frame_to_blob(frame_header_t frame, unsigned short crc, char* raw, size_t size){
    //Dump frame to an individual file in the order we receive them
    char fname[30];
    char* dirname = "blobs";
    FILE* f;
    size_t written;
    int res;
    size_t blob_size = 5000;

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

    if((count++ % blob_size) == 0){
    	snprintf(fname, 30, "./%s/blob%lu.mp3", dirname, bcount++);
	count = 1; //reset count to avoid integer overflow
    }
    f = fopen(fname, "a");
    if(f == NULL){
        error("unable to open file %s\n", fname);
        return;
    }

    write_header(f, frame);
    if(!frame.crc_disabled)
        write_crc(f, crc);
    written = fwrite(raw, 1, size, f);
    if(written < size)
        error("Wrote %u of %u raw bytes to %s", written, size, fname);
    fclose(f);
}

void dump_frame(frame_header_t frame, unsigned short crc, char* raw, size_t size){
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

    snprintf(fname, 30, "./%s/frame%lu.mp3", dirname, frame_count++);
    f = fopen(fname, "w");
    if(f == NULL){
        error("unable to open file %s\n", fname);
        return;
    }

    write_header(f, frame);
    if(!frame.crc_disabled)
        write_crc(f, crc);
    written = fwrite(raw, 1, size, f);
    if(written < size)
        error("Wrote %u of %u raw bytes to %s", written, size, fname);
    fclose(f);
}

void append_to_output(frame_header_t frame, unsigned short crc, char* raw, size_t size){
    if(processed == NULL){
        processed = fopen("output.mp3", "w");
        if(processed == NULL){
            error("unable to open output.mp3\n");
            return;
        }
    }
    write_header(processed, frame);
    if(!frame.crc_disabled)
        write_crc(processed, crc);
    size_t written = fwrite(raw, 1, size, processed);
    if(written < size)
        error("Wrote %u of %u raw bytes to output.mp3\n", written, size);
}

void process_raw(frame_header_t frame, unsigned short crc, char* raw, size_t size){
    dump_frame_to_blob(frame, crc, raw, size);
    //dump_frame(frame, crc, raw, size);
    //append_to_output(frame, crc, raw, size);
}

void close_output(){
    if(processed != NULL)
        fclose(processed);
}
