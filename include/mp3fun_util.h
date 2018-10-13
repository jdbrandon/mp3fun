#ifndef MP3UTIL_HEADER
#define MP3UTIL_HEADER
#include <stdio.h>
#include <stdarg.h>
#include <mp3fun.h>

extern unsigned verbose;
extern FILE* outFile;
extern FILE* errFile;

void error(char* fmt, ...);
void output(char* fmt, ...);
void print_usage(void);
int is_frame_valid(const frame_header_t frame);
int is_mode_and_l2_bitrate_valid(unsigned mode, unsigned l2_bitrate);
char* get_mpeg_version_string(unsigned mpeg_version);
unsigned get_layer(unsigned layer_code);
unsigned short get_bitrate(unsigned version, unsigned layer, unsigned bitrate_idx);
unsigned get_sample_frequency(unsigned version, unsigned freq);
char* get_channel_mode_string(unsigned mode);
char* get_mode_ext_string(unsigned layer, unsigned ext); 
char* get_emphasis_string(unsigned emph);
size_t calculate_frame_size(frame_header_t frame);

#endif
