#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <errno.h>
#include <mp3fun_util.h>

#define SIZE 20

typedef struct{
    unsigned sync:11;
    unsigned mpeg_version:2;
    unsigned layer:2;
    unsigned crc_enabled:1;
    
    unsigned bitrate:4;
    unsigned sample_frequency:2;
    unsigned padding:1;
    unsigned private_bit:1;
    
    unsigned channel:2;
    unsigned mode_ext:2;
    unsigned copyright:1;
    unsigned original:1;
    unsigned emphasis:2;
} frame_header_t;

int seek_to_sync(FILE* f){
    size_t read = 0, buf_size = 1000;
    char buf[buf_size];
    int i = 0;
    frame_header_t* header;

    while((read = fread(buf, 1, buf_size, f)) == buf_size){
        //Search for the pattern
        for(i=0; i < read-2; i++){
            header = (frame_header_t*) &buf[i];
            header->sync &= 0x7ff;
            if(header->sync == 0x7ff){
                //found!! sync seek back f to i    
                fseek(f, -buf_size + i, SEEK_CUR);
                return true;
            }
            if(header->sync & 0x00f < 0x00f){
                //next step and the one after won't pass either
                i += 2; //factors in increment of for loop
                continue;
            }
            if(header->sync & 0x0f0 < 0x0f0){
                //next step won't pass
                i++; //factors in increment of for loop
            }
        }//end for
        if(i == buf_size-2){
            //need to check last 2 bytes of buf
            if((buf[buf_size-1] & buf[buf_size-2]) == 0xff)
                fseek(f, -2, SEEK_CUR);
        }
        if(i == buf_size-1){
            if(buf[buf_size-1] == 0xff)
                fseek(f, -1, SEEK_CUR);
        }
    }//end while
    i = 0;
    while(read-i >= sizeof(frame_header_t)){
        header = (frame_header_t*) &buf[i];
        header->sync &= 0x7ff;
        if(header->sync == 0x7ff){
            //found!! sync seek back f to i    
            fseek(f, -buf_size + i, SEEK_CUR);
            return true;
        }
        if(header->sync & 0x00f < 0x00f){
            //next step and the one after won't pass either
            i += 3;
            continue;
        }
        if(header->sync & 0x0f0 < 0x0f0){
            //next step won't pass
            i++; //factors in increment of loop
        }
        i++; 
    }//end while
    return false;
}

void dump_frame_header_to_file(const frame_header_t h, FILE* out){
    fprintf(out, "%s:\t\t\t0x%.3x\n", "sync", h.sync);
    fprintf(out, "%s:\t\t0x%.1x\n", "mpeg version", h.mpeg_version);
    fprintf(out, "%s:\t\t\t0x%.1x\n", "layer", h.layer);
    fprintf(out, "%s:\t\t0x%.1x\n", "crc enabled", h.crc_enabled);
    fprintf(out, "%s:\t\t0x%.1x\n", "bitrate", h.bitrate);
    fprintf(out, "%s:\t0x%.1x\n", "sample frequency", h.sample_frequency);
    fprintf(out, "%s:\t\t0x%.1x\n", "padding", h.padding);
    fprintf(out, "%s:\t\t0x%.1x\n", "private", h.private_bit);
    fprintf(out, "%s:\t\t0x%.1x\n", "channel", h.channel);
    fprintf(out, "%s:\t\t\t0x%.1x\n", "mode", h.mode_ext);
    fprintf(out, "%s:\t\t0x%.1x\n", "copyright", h.copyright);
    fprintf(out, "%s:\t\t0x%.1x\n", "original", h.original);
    fprintf(out, "%s:\t\t0x%.1x\n", "emphasis", h.emphasis);
}

void dump_frame_header(const frame_header_t h){
    dump_frame_header_to_file(h, stdout);    
}

int main(int argc, char** argv){
    FILE* f;
    size_t read, count;
    unsigned* walker;
    char buf[SIZE];
    int success = false;
    frame_header_t frame_ref;

    if(argc < 2){
        print_usage();
        return -1;
    }
    f = fopen(argv[1], "r");

    if(f == NULL){
        fprintf(stderr, "Failed to open %s. errno:%d \n", argv[1], errno);
        return -1;
    }
    //Seek to frame sync pattern
    success = seek_to_sync(f);
    
    if(!success){
        fprintf(stderr, "unable to locate sync\n");
        return -1;
    }

    fread(&frame_ref, 1, sizeof(frame_header_t), f);
    
    if(frame_ref.sync != 0x7ff){
        fprintf(stderr, "frame_ref is out of sync\n");
        return -1;
    }

    dump_frame_header(frame_ref);

/*
    while((read = fread(buf, 1, SIZE, f)) == SIZE){
        walker = (unsigned*) &buf[0];
        for(count = 0; count < (SIZE>>2); count++)
            printf("0x%.8x\t", walker[count]);
        printf("\n");
    }
    for(count = 0; count < read; count++){
        printf("%d ", buf[count]);    
    }
*/
    printf("\n");

    fclose(f);
    return 0;
}
