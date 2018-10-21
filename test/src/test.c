#include <stdio.h>
#include <stdlib.h>
#include <munit.h>
#include <mp3fun.h>
#include <mp3fun_util.h>
#include <libmpeg123/mpeghead.h>

bool test_equal(frame_header_t f, unsigned long h){
    int res = true;
    if(f.sync != HDR_SYNC_VAL(h)){
        error("sync does not match\n");    
        error("0x%.3x\t0x%.3x\n", f.sync, HDR_SYNC_VAL(h));
        res = false;
    }
    if(f.mpeg_version != HDR_VERSION_VAL(h)){
        error("version does not match\n");
        res = false;
    }
    if(f.layer != HDR_LAYER_VAL(h)){
        error("layer does not match\n");
        res = false;
    }
    if(f.crc_disabled != HDR_CRC_VAL(h)){
        error("crc bit does not match\n");
        res = false;
    }
    if(f.bitrate != HDR_BITRATE_VAL(h)){
        error("bitrate index does not match\n");
        res = false;
    }
    if(f.sample_frequency != HDR_SAMPLERATE_VAL(h)){
        error("sample frequency does not match\n");
        res = false;
    }
    if(f.is_padded != HDR_PADDING_VAL(h)){
        error("padding enable bit does not mathc\n");
        res = false;
    }
    if(f.private != HDR_PRIVATE_VAL(h)){
        error("private bit doesn not match\n");
        res = false;
    }
    if(f.channel_mode != HDR_CHANNEL_VAL(h)){
        error("channel mode does not match\n");
        res = false;
    }
    if(f.mode_ext != HDR_CHANEX_VAL(h)){
        error("channel extension does not match\n");
        res = false;
    }
    if(f.has_copyright != HDR_COPYRIGHT_VAL(h)){
        error("copyright bit does not match\n");
        res = false;
    }
    if(f.is_original != HDR_ORIGINAL_VAL(h)){
        error("original bit does not match\n");
        res = false;
    }
    if(f.emphasis != HDR_EMPHASIS_VAL(h)){
        error("emphasis does not match\n");
        res = false;
    }
    return res;
}

static MunitResult test(const MunitParameter params[], void* data){
    char* v1 = munit_parameters_get(params, "v1");
    char* v2 = munit_parameters_get(params, "v2");

    printf("%s: %s\n", v2, v1);

    return MUNIT_OK;    
}

static char* v1_params[] = {
    "42", "1", "8"
};

static char* v2_params[] = {
    "str1", "str2", "str3"
};

static MunitParameterEnum test_params[] = {
    { "v1", v1_params},
    { "v2", v2_params},
    { NULL, NULL}
};

static void* test_setup(const MunitParameter params[], void* data){
    printf("test setup\n");
    return NULL;
}

static void test_teardown(void* fixture){
    printf("tear down\n");    
}

static MunitResult
test_read_header(const MunitParameter params[], void* buf_ptr){
    char* buf = munit_parameters_get(params, "buf");
    int i = atoi(munit_parameters_get(params, "i"));
    frame_header_t f;
    read_header_bytes(buf, i, &f);
    munit_assert(test_equal(f, *((unsigned long*)&f)));
    return MUNIT_OK;    
}

static char* buf_params[] = {"abcde", "fafaf", "01010", "fghij", NULL};
static char* i_params[] = {"0", "1", NULL};

static MunitParameterEnum rh_params[] = {
    {"buf", buf_params},
    {"i", i_params},
    {NULL, NULL}
};

static MunitTest util_tests[] = {
    //example: {"name", test, test_setup, test_teardown, MUNIT_TEST_OPTION_NONE, test_params},
    {"read_header", test_read_header, NULL, NULL,
    MUNIT_TEST_OPTION_NONE, rh_params},
    {NULL, NULL, NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL} //terminates list
};

static const MunitSuite test_suite = {
    "mp3fun-util/", //name
    util_tests, //test
    NULL, //other test suites to include
    1, //num of iterations to perform on each test
    MUNIT_SUITE_OPTION_NONE //default settings
};

int main(int argc, char** argv){
    printf("hello test\n");    
    return munit_suite_main(&test_suite, (void*) "munit", argc, argv);
}
