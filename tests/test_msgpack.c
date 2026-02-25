#include "msgpack/msgpack.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static int tests_passed = 0;
static int tests_failed = 0;

void test_case(const char *name, int result) {
    if (result == 0) {
        printf("[PASS] %s\n", name);
        tests_passed++;
    } else {
        printf("[FAIL] %s\n", name);
        tests_failed++;
    }
}

int test_nil(void) {
    msgpack_serializer serializer;
    msgpack_serializer_init(&serializer, 64);
    
    msgpack_object obj = {.type = MSGPACK_TYPE_NIL};
    int ret = msgpack_serialize(&serializer, &obj);
    if (ret != 0) return ret;
    
    msgpack_reader reader;
    msgpack_reader_init(&reader, serializer.buffer.data, serializer.buffer.length);
    
    msgpack_object out = {0};
    ret = msgpack_read_object(&reader, &out);
    if (ret != 0) return ret;
    if (out.type != MSGPACK_TYPE_NIL) return -1;
    
    msgpack_serializer_free(&serializer);
    return 0;
}

int test_bool_true(void) {
    msgpack_serializer serializer;
    msgpack_serializer_init(&serializer, 64);
    
    msgpack_object obj = {.type = MSGPACK_TYPE_BOOL, .as.b = true};
    int ret = msgpack_serialize(&serializer, &obj);
    if (ret != 0) return ret;
    if (serializer.buffer.length != 1) return -1;
    if (serializer.buffer.data[0] != 0xC3) return -1;
    
    msgpack_reader reader;
    msgpack_reader_init(&reader, serializer.buffer.data, serializer.buffer.length);
    msgpack_object out = {0};
    ret = msgpack_read_object(&reader, &out);
    if (ret != 0) return ret;
    if (out.type != MSGPACK_TYPE_BOOL || out.as.b != true) return -1;
    
    msgpack_serializer_free(&serializer);
    return 0;
}

int test_bool_false(void) {
    msgpack_serializer serializer;
    msgpack_serializer_init(&serializer, 64);
    
    msgpack_object obj = {.type = MSGPACK_TYPE_BOOL, .as.b = false};
    int ret = msgpack_serialize(&serializer, &obj);
    if (ret != 0) return ret;
    if (serializer.buffer.length != 1) return -1;
    if (serializer.buffer.data[0] != 0xC2) return -1;
    
    msgpack_reader reader;
    msgpack_reader_init(&reader, serializer.buffer.data, serializer.buffer.length);
    msgpack_object out = {0};
    ret = msgpack_read_object(&reader, &out);
    if (ret != 0) return ret;
    if (out.type != MSGPACK_TYPE_BOOL || out.as.b != false) return -1;
    
    msgpack_serializer_free(&serializer);
    return 0;
}

int test_positive_fixint(void) {
    for (uint64_t i = 0; i <= 127; i++) {
        msgpack_serializer serializer;
        msgpack_serializer_init(&serializer, 64);
        
        msgpack_object obj = {.type = MSGPACK_TYPE_POSITIVE_FIXINT, .as.u = i};
        int ret = msgpack_serialize(&serializer, &obj);
        if (ret != 0) return ret;
        if (serializer.buffer.length != 1) return -1;
        if (serializer.buffer.data[0] != (uint8_t)i) return -1;
        
        msgpack_reader reader;
        msgpack_reader_init(&reader, serializer.buffer.data, serializer.buffer.length);
        msgpack_object out = {0};
        ret = msgpack_read_object(&reader, &out);
        if (ret != 0) return ret;
        if (out.as.u != i) return -1;
        
        msgpack_serializer_free(&serializer);
    }
    return 0;
}

int test_negative_fixint(void) {
    for (int64_t i = -32; i < 0; i++) {
        msgpack_serializer serializer;
        msgpack_serializer_init(&serializer, 64);
        
        msgpack_object obj = {.type = MSGPACK_TYPE_INT64, .as.i = i};
        int ret = msgpack_serialize(&serializer, &obj);
        if (ret != 0) return ret;
        
        msgpack_reader reader;
        msgpack_reader_init(&reader, serializer.buffer.data, serializer.buffer.length);
        msgpack_object out = {0};
        ret = msgpack_read_object(&reader, &out);
        if (ret != 0) return ret;
        if (out.as.i != i) return -1;
        
        msgpack_serializer_free(&serializer);
    }
    return 0;
}

int test_uint8(void) {
    msgpack_serializer serializer;
    msgpack_serializer_init(&serializer, 64);
    
    msgpack_object obj = {.type = MSGPACK_TYPE_UINT8, .as.u = 128};
    int ret = msgpack_serialize(&serializer, &obj);
    if (ret != 0) return ret;
    if (serializer.buffer.length != 2) return -1;
    if (serializer.buffer.data[0] != 0xCC) return -1;
    if (serializer.buffer.data[1] != 128) return -1;
    
    msgpack_reader reader;
    msgpack_reader_init(&reader, serializer.buffer.data, serializer.buffer.length);
    msgpack_object out = {0};
    ret = msgpack_read_object(&reader, &out);
    if (ret != 0) return ret;
    if (out.as.u != 128) return -1;
    
    msgpack_serializer_free(&serializer);
    return 0;
}

int test_uint32(void) {
    msgpack_serializer serializer;
    msgpack_serializer_init(&serializer, 64);
    
    msgpack_object obj = {.type = MSGPACK_TYPE_UINT32, .as.u = 0x12345678};
    int ret = msgpack_serialize(&serializer, &obj);
    if (ret != 0) return ret;
    if (serializer.buffer.length != 5) return -1;
    if (serializer.buffer.data[0] != 0xCE) return -1;
    
    msgpack_reader reader;
    msgpack_reader_init(&reader, serializer.buffer.data, serializer.buffer.length);
    msgpack_object out = {0};
    ret = msgpack_read_object(&reader, &out);
    if (ret != 0) return ret;
    if (out.as.u != 0x12345678) return -1;
    
    msgpack_serializer_free(&serializer);
    return 0;
}

int test_int64(void) {
    int64_t test_vals[] = {-1, -128, -32768, -2147483648, -9223372036854775807LL};
    
    for (size_t i = 0; i < sizeof(test_vals) / sizeof(test_vals[0]); i++) {
        msgpack_serializer serializer;
        msgpack_serializer_init(&serializer, 64);
        
        msgpack_object obj = {.type = MSGPACK_TYPE_INT64, .as.i = test_vals[i]};
        int ret = msgpack_serialize(&serializer, &obj);
        if (ret != 0) return ret;
        
        msgpack_reader reader;
        msgpack_reader_init(&reader, serializer.buffer.data, serializer.buffer.length);
        msgpack_object out = {0};
        ret = msgpack_read_object(&reader, &out);
        if (ret != 0) return ret;
        if (out.as.i != test_vals[i]) return -1;
        
        msgpack_serializer_free(&serializer);
    }
    return 0;
}

int test_float32(void) {
    msgpack_serializer serializer;
    msgpack_serializer_init(&serializer, 64);
    
    float f = 3.14f;
    msgpack_object obj = {.type = MSGPACK_TYPE_FLOAT32, .as.f = f};
    int ret = msgpack_serialize(&serializer, &obj);
    if (ret != 0) return ret;
    if (serializer.buffer.data[0] != 0xCA) return -1;
    
    msgpack_reader reader;
    msgpack_reader_init(&reader, serializer.buffer.data, serializer.buffer.length);
    msgpack_object out = {0};
    ret = msgpack_read_object(&reader, &out);
    if (ret != 0) return ret;
    
    msgpack_serializer_free(&serializer);
    return 0;
}

int test_float64(void) {
    msgpack_serializer serializer;
    msgpack_serializer_init(&serializer, 64);
    
    double f = 3.14159265358979;
    msgpack_object obj = {.type = MSGPACK_TYPE_FLOAT64, .as.f = f};
    int ret = msgpack_serialize(&serializer, &obj);
    if (ret != 0) return ret;
    if (serializer.buffer.data[0] != 0xCB) return -1;
    
    msgpack_reader reader;
    msgpack_reader_init(&reader, serializer.buffer.data, serializer.buffer.length);
    msgpack_object out = {0};
    ret = msgpack_read_object(&reader, &out);
    if (ret != 0) return ret;
    
    msgpack_serializer_free(&serializer);
    return 0;
}

int test_fixstr(void) {
    const char *test = "hello";
    
    msgpack_serializer serializer;
    msgpack_serializer_init(&serializer, 64);
    
    msgpack_object obj = {.type = MSGPACK_TYPE_STR, .as.str.ptr = test, .as.str.size = 5};
    int ret = msgpack_serialize(&serializer, &obj);
    if (ret != 0) return ret;
    if (serializer.buffer.length != 6) return -1;
    if ((serializer.buffer.data[0] & 0xE0) != 0xA0) return -1;
    if (serializer.buffer.data[0] != 0xA5) return -1;
    
    msgpack_reader reader;
    msgpack_reader_init(&reader, serializer.buffer.data, serializer.buffer.length);
    msgpack_object out = {0};
    ret = msgpack_read_object(&reader, &out);
    if (ret != 0) return ret;
    if (out.as.str.size != 5) return -1;
    if (memcmp(out.as.str.ptr, test, 5) != 0) return -1;
    
    msgpack_serializer_free(&serializer);
    return 0;
}

int test_str8(void) {
    char buf[300];
    memset(buf, 'a', 300);
    
    msgpack_serializer serializer;
    msgpack_serializer_init(&serializer, 1024);
    
    msgpack_object obj = {.type = MSGPACK_TYPE_STR, .as.str.ptr = buf, .as.str.size = 300};
    int ret = msgpack_serialize(&serializer, &obj);
    if (ret != 0) return ret;
    if (serializer.buffer.data[0] != 0xDA) return -1;  // STR16 for 300 bytes
    
    msgpack_reader reader;
    msgpack_reader_init(&reader, serializer.buffer.data, serializer.buffer.length);
    msgpack_object out = {0};
    ret = msgpack_read_object(&reader, &out);
    if (ret != 0) return ret;
    if (out.as.str.size != 300) return -1;
    
    msgpack_serializer_free(&serializer);
    return 0;
}

int test_fixarray(void) {
    msgpack_serializer serializer;
    msgpack_serializer_init(&serializer, 64);
    
    msgpack_object arr_obj = {.type = MSGPACK_TYPE_ARRAY};
    arr_obj.as.array.size = 3;
    arr_obj.as.array.ptr = (msgpack_object *)malloc(3 * sizeof(msgpack_object));
    
    arr_obj.as.array.ptr[0].type = MSGPACK_TYPE_INT;
    arr_obj.as.array.ptr[0].as.i = 1;
    arr_obj.as.array.ptr[1].type = MSGPACK_TYPE_INT;
    arr_obj.as.array.ptr[1].as.i = 2;
    arr_obj.as.array.ptr[2].type = MSGPACK_TYPE_INT;
    arr_obj.as.array.ptr[2].as.i = 3;
    
    int ret = msgpack_serialize(&serializer, &arr_obj);
    if (ret != 0) return ret;
    if (serializer.buffer.length != 4) return -1;
    if (serializer.buffer.data[0] != 0x93) return -1;
    
    msgpack_reader reader;
    msgpack_reader_init(&reader, serializer.buffer.data, serializer.buffer.length);
    msgpack_object out = {0};
    ret = msgpack_read_object(&reader, &out);
    if (ret != 0) return ret;
    if (out.as.array.size != 3) return -1;
    if (out.as.array.ptr[0].as.i != 1) return -1;
    if (out.as.array.ptr[1].as.i != 2) return -1;
    if (out.as.array.ptr[2].as.i != 3) return -1;
    
    free(arr_obj.as.array.ptr);
    msgpack_object_free(&out);
    msgpack_serializer_free(&serializer);
    return 0;
}

int test_fixmap(void) {
    msgpack_serializer serializer;
    msgpack_serializer_init(&serializer, 128);
    
    msgpack_object map_obj = {.type = MSGPACK_TYPE_MAP};
    map_obj.as.map.size = 2;
    map_obj.as.map.ptr = (msgpack_object_kv *)malloc(2 * sizeof(msgpack_object_kv));
    
    map_obj.as.map.ptr[0].key.type = MSGPACK_TYPE_STR;
    map_obj.as.map.ptr[0].key.as.str.ptr = "a";
    map_obj.as.map.ptr[0].key.as.str.size = 1;
    map_obj.as.map.ptr[0].value.type = MSGPACK_TYPE_INT;
    map_obj.as.map.ptr[0].value.as.i = 1;
    
    map_obj.as.map.ptr[1].key.type = MSGPACK_TYPE_STR;
    map_obj.as.map.ptr[1].key.as.str.ptr = "b";
    map_obj.as.map.ptr[1].key.as.str.size = 1;
    map_obj.as.map.ptr[1].value.type = MSGPACK_TYPE_INT;
    map_obj.as.map.ptr[1].value.as.i = 2;
    
    int ret = msgpack_serialize(&serializer, &map_obj);
    if (ret != 0) return ret;
    if (serializer.buffer.data[0] != 0x82) return -1;
    
    msgpack_reader reader;
    msgpack_reader_init(&reader, serializer.buffer.data, serializer.buffer.length);
    msgpack_object out = {0};
    ret = msgpack_read_object(&reader, &out);
    if (ret != 0) return ret;
    if (out.as.map.size != 2) return -1;
    
    free(map_obj.as.map.ptr);
    msgpack_object_free(&out);
    msgpack_serializer_free(&serializer);
    return 0;
}

int test_binary(void) {
    uint8_t bin_data[] = {0xDE, 0xAD, 0xBE, 0xEF};
    
    msgpack_serializer serializer;
    msgpack_serializer_init(&serializer, 64);
    
    msgpack_object obj = {.type = MSGPACK_TYPE_BIN8, .as.bin.ptr = bin_data, .as.bin.size = 4};
    int ret = msgpack_serialize(&serializer, &obj);
    if (ret != 0) return ret;
    if (serializer.buffer.data[0] != 0xC4) return -1;
    
    msgpack_reader reader;
    msgpack_reader_init(&reader, serializer.buffer.data, serializer.buffer.length);
    msgpack_object out = {0};
    ret = msgpack_read_object(&reader, &out);
    if (ret != 0) return ret;
    if (out.as.bin.size != 4) return -1;
    if (memcmp(out.as.bin.ptr, bin_data, 4) != 0) return -1;
    
    msgpack_serializer_free(&serializer);
    return 0;
}

int test_timestamp(void) {
    msgpack_serializer serializer;
    msgpack_serializer_init(&serializer, 64);
    
    msgpack_object obj = {.type = MSGPACK_TYPE_TIMESTAMP, .as.timestamp = 1704067200};
    int ret = msgpack_serialize(&serializer, &obj);
    if (ret != 0) return ret;
    
    msgpack_reader reader;
    msgpack_reader_init(&reader, serializer.buffer.data, serializer.buffer.length);
    msgpack_object out = {0};
    ret = msgpack_read_object(&reader, &out);
    if (ret != 0) return ret;
    if (out.type != MSGPACK_TYPE_TIMESTAMP) return -1;
    
    msgpack_serializer_free(&serializer);
    return 0;
}

int test_nested(void) {
    msgpack_serializer serializer;
    msgpack_serializer_init(&serializer, 256);
    
    msgpack_object map = {.type = MSGPACK_TYPE_MAP};
    map.as.map.size = 1;
    map.as.map.ptr = (msgpack_object_kv *)malloc(sizeof(msgpack_object_kv));
    
    map.as.map.ptr[0].key.type = MSGPACK_TYPE_STR;
    map.as.map.ptr[0].key.as.str.ptr = "array";
    map.as.map.ptr[0].key.as.str.size = 5;
    
    map.as.map.ptr[0].value.type = MSGPACK_TYPE_ARRAY;
    map.as.map.ptr[0].value.as.array.size = 2;
    map.as.map.ptr[0].value.as.array.ptr = (msgpack_object *)malloc(2 * sizeof(msgpack_object));
    map.as.map.ptr[0].value.as.array.ptr[0].type = MSGPACK_TYPE_INT;
    map.as.map.ptr[0].value.as.array.ptr[0].as.i = 1;
    map.as.map.ptr[0].value.as.array.ptr[1].type = MSGPACK_TYPE_INT;
    map.as.map.ptr[0].value.as.array.ptr[1].as.i = 2;
    
    int ret = msgpack_serialize(&serializer, &map);
    if (ret != 0) return ret;
    
    msgpack_reader reader;
    msgpack_reader_init(&reader, serializer.buffer.data, serializer.buffer.length);
    msgpack_object out = {0};
    ret = msgpack_read_object(&reader, &out);
    if (ret != 0) return ret;
    if (out.as.map.size != 1) return -1;
    if (out.as.map.ptr[0].value.as.array.size != 2) return -1;
    if (out.as.map.ptr[0].value.as.array.ptr[0].as.i != 1) return -1;
    if (out.as.map.ptr[0].value.as.array.ptr[1].as.i != 2) return -1;
    
    free(map.as.map.ptr[0].value.as.array.ptr);
    free(map.as.map.ptr);
    msgpack_object_free(&out);
    msgpack_serializer_free(&serializer);
    return 0;
}

int main(void) {
    printf("=== msgpack-c Functional Tests ===\n\n");
    
    test_case("nil", test_nil());
    test_case("bool true", test_bool_true());
    test_case("bool false", test_bool_false());
    test_case("positive fixint (0-127)", test_positive_fixint());
    test_case("negative fixint", test_negative_fixint());
    test_case("uint8", test_uint8());
    test_case("uint32", test_uint32());
    test_case("int64", test_int64());
    test_case("float32", test_float32());
    test_case("float64", test_float64());
    test_case("fixstr", test_fixstr());
    test_case("str8", test_str8());
    test_case("fixarray", test_fixarray());
    test_case("fixmap", test_fixmap());
    test_case("binary", test_binary());
    test_case("timestamp", test_timestamp());
    test_case("nested structures", test_nested());
    
    printf("\n=== Results: %d passed, %d failed ===\n", tests_passed, tests_failed);
    return tests_failed > 0 ? 1 : 0;
}
