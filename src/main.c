#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <mp3fun.h>
#include <mp3fun_util.h>

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
    if(verbose >= 2)
        fprintf(out, "%s:\t\t\t0x%.3x\n", "sync", h.sync);
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
    if(verbose >= 2)
        fprintf(out, "%s:\t\t%.1d\n", "private", h.private);
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
    dump_frame_header_to_file(h, outFile);    
}

int main(int argc, char** argv){
    FILE* f;
    size_t read;
    char file_name[300], out_file_name[300], err_file_name[300];
    int success = false, option;
    frame_header_t frame_ref;
    file_name[0] = out_file_name[0] = err_file_name[0] = '\0';

    if(argc < 2){
        print_usage();
        return -1;
    }

    while((option = getopt(argc, argv, "vi:o:e:")) != -1){
        switch(option){
        case 'i':
            strncpy(file_name, optarg, 299);
            file_name[299] = '\0';
            break;
        case 'o':
            strncpy(out_file_name, optarg, 299);
            out_file_name[299]= '\0';
            break;
        case 'e':
            strncpy(err_file_name, optarg, 299);
            err_file_name[299]= '\0';
            break;
        case 'v':
            verbose++;
            break;
        default:
            fprintf(stderr, "invalid option -%c\n", (char) option);
            print_usage();
            return -1;
        }
    }

    if(file_name[0] == '\0'){
        fprintf(stderr, "Error: no input file specified. Specify with -i\n");
        return -1;
    }

    f = fopen(file_name, "r");

    if(f == NULL){
        fprintf(stderr, "Failed to open %s. errno:%d \n", argv[1], errno);
        return -1;
    }

    if(out_file_name[0] != '\0'){
        outFile = fopen(out_file_name, "w+");
        if(outFile == NULL){
            fprintf(stderr, "Error: unable to open %s for writing\n", \
            out_file_name);
            fprintf(stderr, "\tOutput will be sent to stdout\n");
            outFile = stdout;
        }
    } else outFile = stdout;

    if(err_file_name[0] != '\0'){
        errFile = fopen(err_file_name, "w+");
        if(errFile == NULL){
            fprintf(stderr, "Error: unable to open %s for writing\n", \
            err_file_name);
            fprintf(stderr, "\tErrors will be sent to stderr\n");
            errFile = stderr;
        }
    } else errFile = stderr;

    //Seek to frame sync pattern
    success = seek_to_sync(f);
    
    if(!success){
        fprintf(errFile, "unable to locate sync\n");
        return -1;
    }

    while((read = fread(&frame_ref, 1, sizeof(frame_header_t), f)) ==  \
            sizeof(frame_header_t)){
        if(is_frame_valid(frame_ref)){
            dump_frame_header(frame_ref);
        } else {
            if(verbose >= 1)
                fprintf(errFile, "invalid frame, seeking next match\n");
            fseek(f, -sizeof(frame_header_t) + 1, SEEK_CUR);
        }
        if(!seek_to_sync(f))
            break;
    }
    
    fprintf(outFile, "\n");

    fclose(f);
    fclose(outFile);
    fclose(errFile);
    return 0;
}
