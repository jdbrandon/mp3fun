#ifndef PROCESSOR_HFILE
#define PROCESSOR_HFILE
#include <mp3fun.h>
#include <mp3fun_util.h>
#include <errno.h>
#include <sys/stat.h>
#include <sys/types.h>
void process_raw(frame_header_t frame, unsigned short crc, char* raw, size_t size);
void close_output();
#endif //PROCESSOR_HFILE
