#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <errno.h>
#include <mp3fun.h>
#include <mp3fun_util.h>

#define SIZE 20

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
    fprintf(out,"========= Begin Header ========\n");
    //TODO: add verbosity check
    //fprintf(out, "%s:\t\t\t0x%.3x\n", "sync", h.sync);
    fprintf(out, "%s:\t\t%s\n", "mpeg version", \
        get_mpeg_version_string(h.mpeg_version));
    fprintf(out, "%s:\t\t\t%.1d\n", "layer", \
        get_layer(h.layer));
    fprintf(out, "%s:\t\t%s\n", "crc enabled", \
        h.crc_disabled ? "false" : "true");
    fprintf(out, "%s:\t\t%hu\n", "bitrate", \
        get_bitrate(h.mpeg_version, h.layer, h.bitrate));
    fprintf(out, "%s:\t%s\n", "sample frequency", \
        get_sample_frequency_string(h.mpeg_version, h.sample_frequency));
    fprintf(out, "%s:\t\t%s\n", "padding", \
        h.is_padded ? "true" : "false");
    //TODO add verbosity check
    //fprintf(out, "%s:\t\t%.1d\n", "private", h.private);
    fprintf(out, "%s:\t\t%s\n", "channel", \
        get_channel_mode_string(h.channel_mode));
    if(h.channel_mode == JOINT_STEREO)
        fprintf(out, "%s:\t\t\t%s\n", "mode extension", \
            get_mode_ext_string(h.layer, h.mode_ext));
    fprintf(out, "%s:\t\t%s\n", "copyright", \
        h.has_copyright ? "true" : "false");
    fprintf(out, "%s:\t\t%s\n", "original", \
        h.is_original ? "true" : "false");
    fprintf(out, "%s:\t\t%s\n", "emphasis", \
        get_emphasis_string(h.emphasis));
    fprintf(out,"========= End Header ==========\n");
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

    while((read = fread(&frame_ref, 1, sizeof(frame_header_t), f)) ==  \
            sizeof(frame_header_t)){
        if(is_frame_valid(frame_ref)){
            dump_frame_header(frame_ref);
        } else {
            //TODO add verbosity check here
            //fprintf(stderr, "invalid frame, seeking next match\n");
            fseek(f, -sizeof(frame_header_t) + 1, SEEK_CUR);
        }
        if(!seek_to_sync(f))
            break;
    }
    
    printf("\n");

    fclose(f);
    return 0;
}
