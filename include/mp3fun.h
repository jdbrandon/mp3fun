/*
    mostly written referencing www.mp3-tech.org/programmer/frame_header.html
*/
#ifndef MP3FUN_HEADER
#define MP3FUN_HEADER

#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>

#define ERR_BAD_MPEG_VERSION    (-1)
#define ERR_BAD_LAYER           (-2)
#define ERR_BAD_FREQ            (-3)
#define ERR_NO_HEADER           (-4)
#define ERR_SHORT_READ          (-5)
#define ERR_BAD_HEADER          (-6)

#define SYNC_VAL 0x7ff

#define MPEG2_5         0
#define MPEG_RESERVED   1
#define MPEG2           2
#define MPEG1           3

#define LAYER_RESERVED  0
#define LAYER_III       1
#define LAYER_II        2
#define LAYER_I         3

#define CRC_ENABLED     0
#define CRC_DISABLED    1

#define BITRATE_FREE    0
#define BRFR            BITRATE_FREE
#define BITRATE_BAD     2
#define BRBA            BITRATE_BAD
#define BR_TABLE_ROW    16
#define BR_TABLE_COL    5

#define V1_L1( index ) (bitrate_table[index][0] << 1)
#define V1_L2( index ) (bitrate_table[index][1] << 1)
#define V1_L3( index ) (bitrate_table[index][2] << 1)
#define V2_L1( index ) (bitrate_table[index][3] << 1)
#define V2_L2( index ) (bitrate_table[index][4] << 1)
#define V2_L3( index ) V2_L2( index )

#define FREQ_MPEG1_44KHZ    0
#define FREQ_MPEG1_48KHZ    1
#define FREQ_MPEG1_32KHZ    2
#define FREQ_MPEG2_22KHZ    0
#define FREQ_MPEG2_24KHZ    1
#define FREQ_MPEG2_16KHZ    2
#define FREQ_MPEG2_5_11KHZ  0
#define FREQ_MPEG2_5_12KHZ  1
#define FREQ_MPEG2_5_8KHZ   2
#define FREQ_RESERVED       3

#define PADDING_LAYER_I     4
#define PADDING_LAYER_II    1
#define PADDING_LAYER_III   1

#define STEREO              0
#define INTENSITY_STEREO    1
#define JOINT_STEREO        INTENSITY_STEREO
#define DUAL_CHANNEL        2
#define SINGLE_CHANNEL      3

#define MODE_EXT_4_31       0
#define MODE_EXT_8_31       1
#define MODE_EXT_12_31      2
#define MODE_EXT_16_31      3
#define LAYER_III_INTENSITY_BIT (0x1 << 1)
#define LAYER_III_MS_BIT        (0x1 << 0)

#define EMPHASIS_NONE       0
#define EMPHASIS_50_15      1
#define EMPHASIS_RESERVED   2
#define EMPHASIS_CCIT_J_17  3

extern const unsigned char bitrate_table[][BR_TABLE_COL];

typedef struct{
#if __BYTE_ORDER == __BIG_ENDIAN
    unsigned sync:11;
    unsigned mpeg_version:2;
    unsigned layer:2;
    unsigned crc_disabled:1;
    
    unsigned bitrate:4;
    unsigned sample_frequency:2;
    unsigned is_padded:1;
    unsigned private:1;
    
    unsigned channel_mode:2;
    unsigned mode_ext:2;
    unsigned has_copyright:1;
    unsigned is_original:1;
    unsigned emphasis:2;
#elif __BYTE_ORDER == __LITTLE_ENDIAN
    unsigned emphasis:2;
    unsigned is_original:1;
    unsigned has_copyright:1;
    unsigned mode_ext:2;
    unsigned channel_mode:2;
    unsigned private:1;
    unsigned is_padded:1;
    unsigned sample_frequency:2;
    unsigned bitrate:4;
    unsigned crc_disabled:1;
    unsigned layer:2;
    unsigned mpeg_version:2;
    unsigned sync:11;
#endif
} frame_header_t;

typedef union {
    frame_header_t frame;
    char bytes[4];
} test_t;
#endif
