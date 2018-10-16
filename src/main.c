#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <mp3fun.h>
#include <mp3fun_util.h>
#include <processor.h>

int seek_to_sync(FILE* f){
    size_t read = 0, buf_size = 1000;
    int bytes_read = 0;
    char buf[buf_size];
    int i, j, diff;
    frame_header_t h;

    while((read = fread(buf, 1, buf_size, f)) == buf_size){
        //maintain bytes_read
        bytes_read += read;
        //Search for the pattern
        for(i = 0; i < read-3; i++){
            read_header_bytes(buf, i, &h);
            if(h.sync == SYNC_VAL){
                //found!! sync seek back f to i    
                fseek(f, -buf_size + i, SEEK_CUR);
                bytes_read += (-buf_size + i);
                return bytes_read;
            }
            if((h.sync & 0x00f) < 0x00f){
                //next step and the one after won't pass either
                i += 2; //factors in increment of for loop
                continue;
            }
            if((h.sync & 0x070) < 0x070){
                //next step won't pass
                i++; //factors in increment of for loop
            }
        }//end for

        //test this with chosen input
        //make sure we don't miss a header that spans
        //buffer boundary
        diff = buf_size - i;
        if(diff){
            fseek(f, -diff, SEEK_CUR);
            bytes_read -= diff;
        }
    }//end while
    bytes_read += read;
    i = 0;
    while(read-i >= sizeof(frame_header_t)){
        read_header_bytes(buf, i, &h);
        if(h.sync == SYNC_VAL){
            //found!! sync seek back f to i    
            fseek(f, -read + i, SEEK_CUR);
            bytes_read += (-read + i);
            return bytes_read;
        }
        if((h.sync & 0x00f) < 0x00f){
            //next step and the one after won't pass either
            i += 3;
            continue;
        }
        if((h.sync & 0x070) < 0x070){
            //next step won't pass
            i++; //factors in increment of loop
        }
        i++; 
    }//end while
    return ERR_NO_HEADER;
}

int seek_to_valid_sync(FILE* f, frame_header_t* out_frame){
    int bytes_read = 0;
    char buf[sizeof(frame_header_t)];
    size_t read;

    while((read = seek_to_sync(f)) != ERR_NO_HEADER){
        bytes_read += read;
        read = fread(buf, 1, sizeof(frame_header_t), f);
        if(read < sizeof(frame_header_t)){
            error("Short read on frame header\n");
            return ERR_SHORT_READ;
        }
        bytes_read += read;
        read_header_bytes(buf, 0, out_frame);
        if(is_frame_valid(*out_frame))
            return bytes_read;
        if(verbose >= 2)
            error("invalid frame, seeking next match\n");
        fseek(f, -sizeof(frame_header_t) + 1, SEEK_CUR);
    }
    return ERR_NO_HEADER;
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
    fprintf(out, "%s:\t%u Hz\n", "sample frequency", \
        get_sample_frequency(h.mpeg_version, h.sample_frequency));
    fprintf(out, "%s:\t\t%s\n", "padding", \
        h.is_padded ? "true" : "false");
    if(verbose >= 2)
        fprintf(out, "%s:\t\t%.1d\n", "private", h.private);
    fprintf(out, "%s:\t\t%s\n", "channel", \
        get_channel_mode_string(h.channel_mode));
    if(h.channel_mode == JOINT_STEREO)
        fprintf(out, "\t%s:\t%s\n", "mode extension", \
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

long get_bytes_to_EOF(FILE* f){
    long current = ftell(f);
    fseek(f, 0, SEEK_END);
    long end = ftell(f);
    fseek(f, current, SEEK_SET); //undo side effect
    return end-current;
}

int main(int argc, char** argv){
    FILE* f;
    size_t read, frame_size;
    char file_name[300], out_file_name[300], err_file_name[300];
    char* frame_buf = NULL;
    int offset, option;
    unsigned short crc;
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
            error("invalid option -%c\n", (char) option);
            print_usage();
            return -1;
        }
    }

    if(file_name[0] == '\0'){
        error("Error: no input file specified. Specify with -i\n");
        return -1;
    }

    f = fopen(file_name, "rb");

    if(f == NULL){
        fprintf(stderr, "Failed to open %s. errno:%d \n", argv[1], errno);
        return -1;
    }

    if(out_file_name[0] != '\0'){
        outFile = fopen(out_file_name, "w+");
        if(outFile == NULL){
            error("Error: unable to open %s for writing\n", \
                    out_file_name);
            error("\tOutput will be sent to stdout\n");
            outFile = stdout;
        }
    } else outFile = stdout;

    if(err_file_name[0] != '\0'){
        errFile = fopen(err_file_name, "w+");
        if(errFile == NULL){
            error("Error: unable to open %s for writing\n", \
                    err_file_name);
            error("\tErrors will be sent to stderr\n");
            errFile = stderr;
        }
    } else errFile = stderr;

    //main loop
    while((offset = seek_to_valid_sync(f, &frame_ref)) >= 0){
        if(verbose > 0)
            dump_frame_header(frame_ref);

        crc = 0;
        if( !frame_ref.crc_disabled ){
            read = fread(&crc, 1, sizeof(short), f);
            if(read < sizeof(short)){
                error("Error: short read on crc\n");
                dump_frame_header_to_file(frame_ref, errFile);
                break;
            }
            if(verbose > 0)
                output("Frame crc:\t0x%.4x\n", crc);
        }

        frame_size = calculate_frame_size(frame_ref);

        if(frame_size == 0 && frame_ref.bitrate == BITRATE_FREE){
            //NOTE: normally we need to use get_bitrate() but 
            //this is a special case thanks to (0 == BITRATE_FREE)

            //Determine frame size by locating next valid frame
            //and taking the offset
            frame_header_t tmp_frame;
            frame_size = seek_to_valid_sync(f, &tmp_frame);
            if(frame_size == ERR_NO_HEADER || 
                frame_size == ERR_SHORT_READ){
                //assume frame spans to end of file    
                frame_size = get_bytes_to_EOF(f);
            } else fseek(f, -frame_size, SEEK_CUR);
        }

        if(verbose > 0)
            output("frame size: %lu\n", frame_size);

        frame_buf = malloc(frame_size);
        read = fread(frame_buf, 1, frame_size, f);

        if(read < frame_size){
            error("Error: short read on frame\n");
            dump_frame_header_to_file(frame_ref, errFile);
            free(frame_buf);
            break;
        }

        process_raw(frame_ref, crc, frame_buf, frame_size);

        free(frame_buf);
    }
    if(offset == ERR_SHORT_READ)
        error("Short read while seeking header\n");
    
    output("\n");

    fclose(f);
    fclose(outFile);
    fclose(errFile);
    close_output();
    return 0;
}
