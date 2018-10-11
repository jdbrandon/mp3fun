#include <stdlib.h>
#include <stdio.h>
#include <mp3fun.h>
#include <mp3fun_util.h>
#include <stdbool.h>

const unsigned char bitrate_table[BR_TABLE_ROW][BR_TABLE_COL] = {
    {    BRFR,     BRFR,     BRFR,     BRFR,     BRFR},
    { 32 >> 1,  32 >> 1,  32 >> 1,  32 >> 1,   8 >> 1},
    { 64 >> 1,  48 >> 1,  40 >> 1,  48 >> 1,  16 >> 1},
    { 96 >> 1,  56 >> 1,  48 >> 1,  56 >> 1,  24 >> 1},
    {128 >> 1,  64 >> 1,  56 >> 1,  64 >> 1,  32 >> 1},
    {160 >> 1,  80 >> 1,  64 >> 1,  80 >> 1,  40 >> 1},
    {192 >> 1,  96 >> 1,  80 >> 1,  96 >> 1,  48 >> 1},
    {224 >> 1, 112 >> 1,  96 >> 1, 112 >> 1,  56 >> 1},
    {256 >> 1, 128 >> 1, 112 >> 1, 128 >> 1,  64 >> 1},
    {288 >> 1, 160 >> 1, 128 >> 1, 144 >> 1,  80 >> 1},
    {320 >> 1, 192 >> 1, 160 >> 1, 160 >> 1,  96 >> 1},
    {352 >> 1, 224 >> 1, 192 >> 1, 176 >> 1, 112 >> 1},
    {384 >> 1, 256 >> 1, 224 >> 1, 192 >> 1, 128 >> 1},
    {416 >> 1, 320 >> 1, 256 >> 1, 224 >> 1, 144 >> 1},
    {448 >> 1, 384 >> 1, 320 >> 1, 256 >> 1, 160 >> 1},
    { BRBA>>1,  BRBA>>1,  BRBA>>1,  BRBA>>1,  BRBA>>1}
};

unsigned verbose = 0;
FILE* outFile = NULL;
FILE* errFile = NULL;

void print_usage(){
    printf("mp3fun usage:\n\t");
    printf("\t./mp3fun [-v] -i input_file [-o output_file] [-e error_file]\n");
}

int is_frame_valid(const frame_header_t frame){
    unsigned version = frame.mpeg_version;
    if(version == MPEG_RESERVED)
        return false;

    unsigned layer = frame.layer;
    if(layer == LAYER_RESERVED)
        return false;

    unsigned short bitrate = get_bitrate(version, layer, frame.bitrate);

    if(bitrate == BITRATE_BAD)
        return false;

    unsigned freq = frame.sample_frequency;
    if(freq == FREQ_RESERVED)
        return false;

    unsigned emph = frame.emphasis;
    if(emph == EMPHASIS_RESERVED)
        return false;

    if(layer == LAYER_II)
        return is_mode_and_l2_bitrate_valid(frame.channel_mode, bitrate);
    return true;
}

int is_mode_and_l2_bitrate_valid(unsigned mode, unsigned l2_bitrate){
    int res;

    switch(l2_bitrate){
    case 32:
    case 48:
    case 56:
    case 80:
        res = mode == SINGLE_CHANNEL;
        break;
    case 224:
    case 256:
    case 320:
    case 384:
        res = (mode == STEREO) || \
                (mode == INTENSITY_STEREO) || \
                (mode == DUAL_CHANNEL);
        break;
    case BITRATE_FREE:
    case 64:
    case 96:
    case 112:
    case 128:
    case 160:
    case 192:
        res = true;
        break;
    default:
        res = false;
    }
    return res;
}

char* get_mpeg_version_string(unsigned mpeg_version){
    char* res;
    switch(mpeg_version){
    case MPEG1:
        res = "MPEG Version 1 (ISO/IEC 11172-3)";
        break;
    case MPEG2:
        res = "MPEG Version 2 (ISO/IEC 13818-3)";
        break;
    case MPEG2_5:
        res = "MPEG Version 2.5 (MPEG2 Low Bitrate Extension)";
        break;
    case MPEG_RESERVED:
        res = "Reserved for future use";
        break;
    default:
        res = "Error: unknown MPEG version";
    }
    return res;
}

unsigned get_layer(unsigned layer_code){
    unsigned res;
    switch(layer_code){
    case LAYER_I:
        res = 1;
        break;
    case LAYER_II:
        res = 2;
        break;
    case LAYER_III:
        res = 3;
        break;
    default:
        res = ERR_BAD_LAYER;
    }
    return res;
}

unsigned short
get_bitrate(unsigned version, unsigned layer, unsigned bitrate_idx){
    short res;
    switch(layer){
    case LAYER_I:
        if(version == MPEG1)
            res = V1_L1(bitrate_idx);
        else if(version == MPEG2 || version == MPEG2_5)
            res = V2_L1(bitrate_idx);
        else res = ERR_BAD_MPEG_VERSION;
        break;
    case LAYER_II:
        if(version == MPEG1)
            res = V1_L2(bitrate_idx);
        else if(version == MPEG2 || version == MPEG2_5)
            res = V2_L2(bitrate_idx);
        else res = ERR_BAD_MPEG_VERSION;
        break;
    case LAYER_III:
        if(version == MPEG1)
            res = V1_L3(bitrate_idx);
        else if(version == MPEG2 || version == MPEG2_5)
            res = V2_L3(bitrate_idx);
        else res = ERR_BAD_MPEG_VERSION;
        break;
    default:
        res = ERR_BAD_LAYER;
    }
    return res;
}

char* get_sample_frequency_string(unsigned version, unsigned freq){
    char* res;
    char* bad_freq = "Error: bad frequency";
    char* bad_version = "Error: bad MPEG Version";
    switch(version){
    case MPEG1:
        if(freq == FREQ_MPEG1_44KHZ)
            res = "44100 Hz";
        else if(freq == FREQ_MPEG1_48KHZ)
            res = "48000 Hz";
        else if(freq == FREQ_MPEG1_32KHZ)
            res = "32000 Hz";
        else res = bad_freq;
        break;
    case MPEG2:
        if(freq == FREQ_MPEG2_22KHZ)
            res = "22050 Hz";
        else if(freq == FREQ_MPEG2_24KHZ)
            res = "24000 Hz";
        else if(freq == FREQ_MPEG2_16KHZ)
            res = "16000 Hz";
        else res = bad_freq;
        break;
    case MPEG2_5:
        if(freq == FREQ_MPEG2_5_11KHZ)
            res = "11025 Hz";
        else if(freq == FREQ_MPEG2_5_12KHZ)
            res = "12000 Hz";
        else if(freq == FREQ_MPEG2_5_8KHZ)
            res = "8000 Hz";
        else res = bad_freq;
        break;
    default:
        res = bad_version;
    }
    return res;
}

char* get_channel_mode_string(unsigned mode){
    char* res;
    char* bad_mode = "Error: bad Mode (also it should be impossible to reach"
    " this because the 2 bits only represent the 4 modes)";
    switch(mode){
    case STEREO:
        res = "Stereo";
        break;
    case JOINT_STEREO:
        res = "Joint Stereo";
        break;
    case DUAL_CHANNEL:
        res = "Dual Channel";
        break;
    case SINGLE_CHANNEL:
        res = "Single Channel";
        break;
    default:
        res = bad_mode;
    }
    return res;
}

char* get_mode_ext_string(unsigned layer, unsigned ext){
    char* res;
    char* bad_ext = "Impossible ext";
    switch(layer){
    case LAYER_I:
    case LAYER_II:
        if(ext == MODE_EXT_4_31)
            res = "bands 4 thru 31";
        else if(ext == MODE_EXT_8_31)
            res = "bands 8 thru 31";
        else if(ext == MODE_EXT_12_31)
            res = "bands 12 thru 31";
        else if(ext == MODE_EXT_16_31)
            res = "bands 16 thru 31";
        else res = bad_ext;
        break;
    case LAYER_III:
        if(ext & LAYER_III_INTENSITY_BIT){
            if(ext & LAYER_III_MS_BIT)
                res = "Intensity Stereo: ON\tMS Stereo: ON";
            else res = "Intensity Stereo: ON\tMS Stereo: OFF";
        } else {
            if(ext & LAYER_III_MS_BIT)
                res = "Intensity Stereo: OFF\tMS Stereo: ON";
            else res = "Intensity Stereo: OFF\tMS Stereo: OFF";
        }
        break;
    default:
        res = "Error: bad layer";
    }
    return res;
}

char* get_emphasis_string(unsigned emph){
    char* res;
    switch(emph){
    case EMPHASIS_NONE:
        res = "No Emphasis";
        break;
    case EMPHASIS_50_15:
        res = "50/15 ms";
        break;
    case EMPHASIS_RESERVED:
        res = "Error: reserved empahsis";
        break;
    case EMPHASIS_CCIT_J_17:
        res = "CCIT J.17";
        break;
    default:
        res = "Error: bad emphasis";
    }
    return res;
}
