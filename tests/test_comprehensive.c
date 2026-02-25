/* Comprehensive msgpack test suite */
#include "msgpack/msgpack.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

static int tests_passed = 0;
static int tests_failed = 0;

void test(const char *name, int passed) {
    if (passed) {
        printf("[PASS] %s\n", name);
        tests_passed++;
    } else {
        printf("[FAIL] %s\n", name);
        tests_failed++;
    }
}

int main(void) {
    printf("=== Comprehensive MessagePack Tests ===\n\n");
    
    /* ========== NIL ========== */
    printf("--- NIL ---\n");
    {
        msgpack_serializer ser;
        msgpack_serializer_init(&ser, 64);
        msgpack_object obj = {.type = MSGPACK_TYPE_NIL};
        
        int ret = msgpack_serialize(&ser, &obj);
        test("nil serialize", ret == 0 && ser.buffer.length == 1 && ser.buffer.data[0] == 0xC0);
        
        msgpack_reader reader;
        msgpack_reader_init(&reader, ser.buffer.data, ser.buffer.length);
        msgpack_object out = {0};
        ret = msgpack_read_object(&reader, &out);
        test("nil deserialize", ret == 0 && out.type == MSGPACK_TYPE_NIL);
        
        msgpack_serializer_free(&ser);
    }
    
    /* ========== BOOL ========== */
    printf("\n--- BOOL ---\n");
    {
        msgpack_serializer ser;
        msgpack_serializer_init(&ser, 64);
        msgpack_object obj = {.type = MSGPACK_TYPE_BOOL, .as.b = true};
        
        int ret = msgpack_serialize(&ser, &obj);
        test("bool true serialize", ret == 0 && ser.buffer.length == 1 && ser.buffer.data[0] == 0xC3);
        
        msgpack_reader reader;
        msgpack_reader_init(&reader, ser.buffer.data, ser.buffer.length);
        msgpack_object out = {0};
        ret = msgpack_read_object(&reader, &out);
        test("bool true deserialize", ret == 0 && out.type == MSGPACK_TYPE_BOOL && out.as.b == true);
        
        msgpack_serializer_free(&ser);
        
        /* bool false */
        msgpack_serializer_init(&ser, 64);
        obj.as.b = false;
        ret = msgpack_serialize(&ser, &obj);
        test("bool false serialize", ret == 0 && ser.buffer.length == 1 && ser.buffer.data[0] == 0xC2);
        
        msgpack_reader_init(&reader, ser.buffer.data, ser.buffer.length);
        out = (msgpack_object){0};
        ret = msgpack_read_object(&reader, &out);
        test("bool false deserialize", ret == 0 && out.type == MSGPACK_TYPE_BOOL && out.as.b == false);
        
        msgpack_serializer_free(&ser);
    }
    
    /* ========== POSITIVE FIXINT ========== */
    printf("\n--- POSITIVE FIXINT ---\n");
    {
        msgpack_serializer ser;
        msgpack_reader reader;
        msgpack_object out;
        int ret;
        
        /* Test 0 */
        msgpack_serializer_init(&ser, 64);
        msgpack_object obj = {.type = MSGPACK_TYPE_POSITIVE_FIXINT, .as.u = 0};
        ret = msgpack_serialize(&ser, &obj);
        test("fixint 0 serialize", ret == 0 && ser.buffer.length == 1 && ser.buffer.data[0] == 0x00);
        msgpack_reader_init(&reader, ser.buffer.data, ser.buffer.length);
        out = (msgpack_object){0};
        ret = msgpack_read_object(&reader, &out);
        test("fixint 0 deserialize", ret == 0 && out.as.u == 0);
        msgpack_serializer_free(&ser);
        
        /* Test 127 */
        msgpack_serializer_init(&ser, 64);
        obj.as.u = 127;
        ret = msgpack_serialize(&ser, &obj);
        test("fixint 127 serialize", ret == 0 && ser.buffer.data[0] == 0x7F);
        msgpack_reader_init(&reader, ser.buffer.data, ser.buffer.length);
        out = (msgpack_object){0};
        ret = msgpack_read_object(&reader, &out);
        test("fixint 127 deserialize", ret == 0 && out.as.u == 127);
        msgpack_serializer_free(&ser);
    }
    
    /* ========== NEGATIVE FIXINT ========== */
    printf("\n--- NEGATIVE FIXINT ---\n");
    {
        msgpack_serializer ser;
        msgpack_reader reader;
        msgpack_object out;
        int ret;
        
        /* Test -1 */
        msgpack_serializer_init(&ser, 64);
        msgpack_object obj = {.type = MSGPACK_TYPE_NEGATIVE_FIXINT, .as.i = -1};
        ret = msgpack_serialize(&ser, &obj);
        test("fixint -1 serialize", ret == 0 && ser.buffer.data[0] == 0xFF);
        msgpack_reader_init(&reader, ser.buffer.data, ser.buffer.length);
        out = (msgpack_object){0};
        ret = msgpack_read_object(&reader, &out);
        test("fixint -1 deserialize", ret == 0 && out.as.i == -1);
        msgpack_serializer_free(&ser);
        
        /* Test -32 */
        msgpack_serializer_init(&ser, 64);
        obj.as.i = -32;
        ret = msgpack_serialize(&ser, &obj);
        test("fixint -32 serialize", ret == 0 && ser.buffer.data[0] == 0xE0);
        msgpack_reader_init(&reader, ser.buffer.data, ser.buffer.length);
        out = (msgpack_object){0};
        ret = msgpack_read_object(&reader, &out);
        test("fixint -32 deserialize", ret == 0 && out.as.i == -32);
        msgpack_serializer_free(&ser);
    }
    
    /* ========== UINT8 ========== */
    printf("\n--- UINT8 ---\n");
    {
        msgpack_serializer ser;
        msgpack_serializer_init(&ser, 64);
        msgpack_object obj = {.type = MSGPACK_TYPE_UINT8, .as.u = 128};
        
        int ret = msgpack_serialize(&ser, &obj);
        test("uint8 128 serialize", ret == 0 && ser.buffer.data[0] == 0xCC && ser.buffer.data[1] == 128);
        
        msgpack_reader reader;
        msgpack_reader_init(&reader, ser.buffer.data, ser.buffer.length);
        msgpack_object out = {0};
        ret = msgpack_read_object(&reader, &out);
        test("uint8 128 deserialize", ret == 0 && out.as.u == 128);
        
        msgpack_serializer_free(&ser);
        
        /* Max uint8 */
        msgpack_serializer_init(&ser, 64);
        obj.as.u = 255;
        ret = msgpack_serialize(&ser, &obj);
        test("uint8 255 serialize", ret == 0 && ser.buffer.data[1] == 255);
        
        msgpack_reader_init(&reader, ser.buffer.data, ser.buffer.length);
        out = (msgpack_object){0};
        ret = msgpack_read_object(&reader, &out);
        test("uint8 255 deserialize", ret == 0 && out.as.u == 255);
        
        msgpack_serializer_free(&ser);
    }
    
    /* ========== UINT16 ========== */
    printf("\n--- UINT16 ---\n");
    {
        msgpack_serializer ser;
        msgpack_serializer_init(&ser, 64);
        msgpack_object obj = {.type = MSGPACK_TYPE_UINT16, .as.u = 256};
        
        int ret = msgpack_serialize(&ser, &obj);
        test("uint16 256 serialize", ret == 0 && ser.buffer.data[0] == 0xCD);
        
        msgpack_reader reader;
        msgpack_reader_init(&reader, ser.buffer.data, ser.buffer.length);
        msgpack_object out = {0};
        ret = msgpack_read_object(&reader, &out);
        test("uint16 256 deserialize", ret == 0 && out.as.u == 256);
        
        msgpack_serializer_free(&ser);
        
        /* Max uint16 */
        msgpack_serializer_init(&ser, 64);
        obj.as.u = 65535;
        ret = msgpack_serialize(&ser, &obj);
        test("uint16 65535 deserialize", ret == 0);
        
        msgpack_reader_init(&reader, ser.buffer.data, ser.buffer.length);
        out = (msgpack_object){0};
        ret = msgpack_read_object(&reader, &out);
        test("uint16 65535 value", ret == 0 && out.as.u == 65535);
        
        msgpack_serializer_free(&ser);
    }
    
    /* ========== UINT32 ========== */
    printf("\n--- UINT32 ---\n");
    {
        msgpack_serializer ser;
        msgpack_serializer_init(&ser, 64);
        msgpack_object obj = {.type = MSGPACK_TYPE_UINT32, .as.u = 65536};
        
        int ret = msgpack_serialize(&ser, &obj);
        test("uint32 65536 serialize", ret == 0 && ser.buffer.data[0] == 0xCE);
        
        msgpack_reader reader;
        msgpack_reader_init(&reader, ser.buffer.data, ser.buffer.length);
        msgpack_object out = {0};
        ret = msgpack_read_object(&reader, &out);
        test("uint32 65536 deserialize", ret == 0 && out.as.u == 65536);
        
        msgpack_serializer_free(&ser);
    }
    
    /* ========== INT8 ========== */
    printf("\n--- INT8 ---\n");
    {
        msgpack_serializer ser;
        msgpack_serializer_init(&ser, 64);
        msgpack_object obj = {.type = MSGPACK_TYPE_INT8, .as.i = -33};
        
        int ret = msgpack_serialize(&ser, &obj);
        test("int8 -33 serialize", ret == 0 && ser.buffer.data[0] == 0xD0);
        
        msgpack_reader reader;
        msgpack_reader_init(&reader, ser.buffer.data, ser.buffer.length);
        msgpack_object out = {0};
        ret = msgpack_read_object(&reader, &out);
        test("int8 -33 deserialize", ret == 0 && out.as.i == -33);
        
        msgpack_serializer_free(&ser);
        
        /* Min int8 */
        msgpack_serializer_init(&ser, 64);
        obj.as.i = -128;
        ret = msgpack_serialize(&ser, &obj);
        
        msgpack_reader_init(&reader, ser.buffer.data, ser.buffer.length);
        out = (msgpack_object){0};
        ret = msgpack_read_object(&reader, &out);
        test("int8 -128 deserialize", ret == 0 && out.as.i == -128);
        
        msgpack_serializer_free(&ser);
    }
    
    /* ========== INT16 ========== */
    printf("\n--- INT16 ---\n");
    {
        msgpack_serializer ser;
        msgpack_serializer_init(&ser, 64);
        msgpack_object obj = {.type = MSGPACK_TYPE_INT16, .as.i = -129};
        
        int ret = msgpack_serialize(&ser, &obj);
        test("int16 -129 serialize", ret == 0 && ser.buffer.data[0] == 0xD1);
        
        msgpack_reader reader;
        msgpack_reader_init(&reader, ser.buffer.data, ser.buffer.length);
        msgpack_object out = {0};
        ret = msgpack_read_object(&reader, &out);
        test("int16 -129 deserialize", ret == 0 && out.as.i == -129);
        
        msgpack_serializer_free(&ser);
    }
    
    /* ========== FLOAT32 ========== */
    printf("\n--- FLOAT32 ---\n");
    {
        msgpack_serializer ser;
        msgpack_serializer_init(&ser, 64);
        msgpack_object obj = {.type = MSGPACK_TYPE_FLOAT32, .as.f = 1.5f};
        
        int ret = msgpack_serialize(&ser, &obj);
        test("float32 1.5 serialize", ret == 0 && ser.buffer.data[0] == 0xCA);
        
        msgpack_reader reader;
        msgpack_reader_init(&reader, ser.buffer.data, ser.buffer.length);
        msgpack_object out = {0};
        ret = msgpack_read_object(&reader, &out);
        test("float32 1.5 deserialize", ret == 0 && fabs(out.as.f - 1.5f) < 0.001);
        
        msgpack_serializer_free(&ser);
    }
    
    /* ========== FLOAT64 ========== */
    printf("\n--- FLOAT64 ---\n");
    {
        msgpack_serializer ser;
        msgpack_serializer_init(&ser, 64);
        msgpack_object obj = {.type = MSGPACK_TYPE_FLOAT64, .as.f = 3.14159265358979};
        
        int ret = msgpack_serialize(&ser, &obj);
        test("float64 pi serialize", ret == 0 && ser.buffer.data[0] == 0xCB);
        
        msgpack_reader reader;
        msgpack_reader_init(&reader, ser.buffer.data, ser.buffer.length);
        msgpack_object out = {0};
        ret = msgpack_read_object(&reader, &out);
        test("float64 pi deserialize", ret == 0 && fabs(out.as.f - 3.14159265358979) < 0.0000001);
        
        msgpack_serializer_free(&ser);
    }
    
    /* ========== FIXSTR ========== */
    printf("\n--- FIXSTR ---\n");
    {
        msgpack_serializer ser;
        msgpack_serializer_init(&ser, 64);
        const char *str = "hello";
        msgpack_object obj = {.type = MSGPACK_TYPE_STR, .as.str.ptr = str, .as.str.size = 5};
        
        int ret = msgpack_serialize(&ser, &obj);
        test("fixstr 5 bytes serialize", ret == 0 && ser.buffer.data[0] == 0xA5);
        
        msgpack_reader reader;
        msgpack_reader_init(&reader, ser.buffer.data, ser.buffer.length);
        msgpack_object out = {0};
        ret = msgpack_read_object(&reader, &out);
        test("fixstr 5 bytes deserialize", ret == 0 && out.as.str.size == 5 && memcmp(out.as.str.ptr, "hello", 5) == 0);
        
        msgpack_serializer_free(&ser);
        
        /* Max fixstr (31 bytes) */
        msgpack_serializer_init(&ser, 64);
        char *str31 = malloc(31);
        memset(str31, 'x', 31);
        obj.as.str.ptr = str31;
        obj.as.str.size = 31;
        ret = msgpack_serialize(&ser, &obj);
        test("fixstr 31 bytes serialize", ret == 0 && ser.buffer.data[0] == 0xBF);
        
        msgpack_reader_init(&reader, ser.buffer.data, ser.buffer.length);
        out = (msgpack_object){0};
        ret = msgpack_read_object(&reader, &out);
        test("fixstr 31 bytes deserialize", ret == 0 && out.as.str.size == 31);
        
        free(str31);
        msgpack_serializer_free(&ser);
    }
    
    /* ========== STR8 ========== */
    printf("\n--- STR8 ---\n");
    {
        msgpack_serializer ser;
        msgpack_serializer_init(&ser, 256);
        char *str32 = malloc(32);
        memset(str32, 'y', 32);
        msgpack_object obj = {.type = MSGPACK_TYPE_STR, .as.str.ptr = str32, .as.str.size = 32};
        
        int ret = msgpack_serialize(&ser, &obj);
        test("str8 32 bytes serialize", ret == 0 && ser.buffer.data[0] == 0xD9 && ser.buffer.data[1] == 32);
        
        msgpack_reader reader;
        msgpack_reader_init(&reader, ser.buffer.data, ser.buffer.length);
        msgpack_object out = {0};
        ret = msgpack_read_object(&reader, &out);
        test("str8 32 bytes deserialize", ret == 0 && out.as.str.size == 32);
        
        free(str32);
        msgpack_serializer_free(&ser);
        
        /* Boundary: 255 bytes */
        msgpack_serializer_init(&ser, 300);
        char *str255 = malloc(255);
        memset(str255, 'z', 255);
        obj.as.str.ptr = str255;
        obj.as.str.size = 255;
        ret = msgpack_serialize(&ser, &obj);
        
        msgpack_reader_init(&reader, ser.buffer.data, ser.buffer.length);
        out = (msgpack_object){0};
        ret = msgpack_read_object(&reader, &out);
        test("str8 255 bytes deserialize", ret == 0 && out.as.str.size == 255);
        
        free(str255);
        msgpack_serializer_free(&ser);
    }
    
    /* ========== STR16 ========== */
    printf("\n--- STR16 ---\n");
    {
        msgpack_serializer ser;
        msgpack_serializer_init(&ser, 512);
        char *str300 = malloc(300);
        memset(str300, 'w', 300);
        msgpack_object obj = {.type = MSGPACK_TYPE_STR, .as.str.ptr = str300, .as.str.size = 300};
        
        int ret = msgpack_serialize(&ser, &obj);
        test("str16 300 bytes serialize", ret == 0 && ser.buffer.data[0] == 0xDA);
        
        msgpack_reader reader;
        msgpack_reader_init(&reader, ser.buffer.data, ser.buffer.length);
        msgpack_object out = {0};
        ret = msgpack_read_object(&reader, &out);
        test("str16 300 bytes deserialize", ret == 0 && out.as.str.size == 300);
        
        free(str300);
        msgpack_serializer_free(&ser);
    }
    
    /* ========== FIXARRAY ========== */
    printf("\n--- FIXARRAY ---\n");
    {
        msgpack_serializer ser;
        msgpack_serializer_init(&ser, 64);
        
        msgpack_object arr = {.type = MSGPACK_TYPE_ARRAY};
        arr.as.array.size = 3;
        arr.as.array.ptr = malloc(3 * sizeof(msgpack_object));
        arr.as.array.ptr[0].type = MSGPACK_TYPE_INT64; arr.as.array.ptr[0].as.i = 1;
        arr.as.array.ptr[1].type = MSGPACK_TYPE_INT64; arr.as.array.ptr[1].as.i = 2;
        arr.as.array.ptr[2].type = MSGPACK_TYPE_INT64; arr.as.array.ptr[2].as.i = 3;
        
        int ret = msgpack_serialize(&ser, &arr);
        test("fixarray 3 elements serialize", ret == 0 && ser.buffer.data[0] == 0x93);
        
        msgpack_reader reader;
        msgpack_reader_init(&reader, ser.buffer.data, ser.buffer.length);
        msgpack_object out = {0};
        ret = msgpack_read_object(&reader, &out);
        test("fixarray 3 elements deserialize", ret == 0 && out.as.array.size == 3);
        
        free(arr.as.array.ptr);
        msgpack_serializer_free(&ser);
        
        /* Max fixarray (15 elements) */
        msgpack_serializer_init(&ser, 256);
        arr.as.array.size = 15;
        arr.as.array.ptr = malloc(15 * sizeof(msgpack_object));
        for (int i = 0; i < 15; i++) {
            arr.as.array.ptr[i].type = MSGPACK_TYPE_INT64;
            arr.as.array.ptr[i].as.i = i;
        }
        ret = msgpack_serialize(&ser, &arr);
        test("fixarray 15 elements serialize", ret == 0 && ser.buffer.data[0] == 0x9F);
        
        msgpack_reader_init(&reader, ser.buffer.data, ser.buffer.length);
        out = (msgpack_object){0};
        ret = msgpack_read_object(&reader, &out);
        test("fixarray 15 elements deserialize", ret == 0 && out.as.array.size == 15);
        
        free(arr.as.array.ptr);
        msgpack_serializer_free(&ser);
    }
    
    /* ========== ARRAY16 ========== */
    printf("\n--- ARRAY16 ---\n");
    {
        msgpack_serializer ser;
        msgpack_serializer_init(&ser, 256);
        
        msgpack_object arr = {.type = MSGPACK_TYPE_ARRAY};
        arr.as.array.size = 16;
        arr.as.array.ptr = malloc(16 * sizeof(msgpack_object));
        for (int i = 0; i < 16; i++) {
            arr.as.array.ptr[i].type = MSGPACK_TYPE_INT64;
            arr.as.array.ptr[i].as.i = i;
        }
        
        int ret = msgpack_serialize(&ser, &arr);
        test("array16 16 elements serialize", ret == 0 && ser.buffer.data[0] == 0xDC);
        
        msgpack_reader reader;
        msgpack_reader_init(&reader, ser.buffer.data, ser.buffer.length);
        msgpack_object out = {0};
        ret = msgpack_read_object(&reader, &out);
        test("array16 16 elements deserialize", ret == 0 && out.as.array.size == 16);
        
        free(arr.as.array.ptr);
        msgpack_serializer_free(&ser);
    }
    
    /* ========== FIXMAP ========== */
    printf("\n--- FIXMAP ---\n");
    {
        msgpack_serializer ser;
        msgpack_serializer_init(&ser, 64);
        
        msgpack_object map = {.type = MSGPACK_TYPE_MAP};
        map.as.map.size = 2;
        map.as.map.ptr = malloc(2 * sizeof(msgpack_object_kv));
        
        map.as.map.ptr[0].key.type = MSGPACK_TYPE_STR;
        map.as.map.ptr[0].key.as.str.ptr = "a";
        map.as.map.ptr[0].key.as.str.size = 1;
        map.as.map.ptr[0].value.type = MSGPACK_TYPE_INT64;
        map.as.map.ptr[0].value.as.i = 1;
        
        map.as.map.ptr[1].key.type = MSGPACK_TYPE_STR;
        map.as.map.ptr[1].key.as.str.ptr = "b";
        map.as.map.ptr[1].key.as.str.size = 1;
        map.as.map.ptr[1].value.type = MSGPACK_TYPE_INT64;
        map.as.map.ptr[1].value.as.i = 2;
        
        int ret = msgpack_serialize(&ser, &map);
        test("fixmap 2 entries serialize", ret == 0 && ser.buffer.data[0] == 0x82);
        
        msgpack_reader reader;
        msgpack_reader_init(&reader, ser.buffer.data, ser.buffer.length);
        msgpack_object out = {0};
        ret = msgpack_read_object(&reader, &out);
        test("fixmap 2 entries deserialize", ret == 0 && out.as.map.size == 2);
        
        free(map.as.map.ptr);
        msgpack_serializer_free(&ser);
        
        /* Max fixmap (15 entries) */
        msgpack_serializer_init(&ser, 512);
        map.as.map.size = 15;
        map.as.map.ptr = malloc(15 * sizeof(msgpack_object_kv));
        for (int i = 0; i < 15; i++) {
            map.as.map.ptr[i].key.type = MSGPACK_TYPE_STR;
            map.as.map.ptr[i].key.as.str.ptr = "x";
            map.as.map.ptr[i].key.as.str.size = 1;
            map.as.map.ptr[i].value.type = MSGPACK_TYPE_INT64;
            map.as.map.ptr[i].value.as.i = i;
        }
        ret = msgpack_serialize(&ser, &map);
        test("fixmap 15 entries serialize", ret == 0 && ser.buffer.data[0] == 0x8F);
        
        msgpack_reader_init(&reader, ser.buffer.data, ser.buffer.length);
        out = (msgpack_object){0};
        ret = msgpack_read_object(&reader, &out);
        test("fixmap 15 entries deserialize", ret == 0 && out.as.map.size == 15);
        
        free(map.as.map.ptr);
        msgpack_serializer_free(&ser);
    }
    
    /* ========== MAP16 ========== */
    printf("\n--- MAP16 ---\n");
    {
        msgpack_serializer ser;
        msgpack_serializer_init(&ser, 512);
        
        msgpack_object map = {.type = MSGPACK_TYPE_MAP};
        map.as.map.size = 16;
        map.as.map.ptr = malloc(16 * sizeof(msgpack_object_kv));
        for (int i = 0; i < 16; i++) {
            map.as.map.ptr[i].key.type = MSGPACK_TYPE_STR;
            map.as.map.ptr[i].key.as.str.ptr = "k";
            map.as.map.ptr[i].key.as.str.size = 1;
            map.as.map.ptr[i].value.type = MSGPACK_TYPE_INT64;
            map.as.map.ptr[i].value.as.i = i;
        }
        
        int ret = msgpack_serialize(&ser, &map);
        test("map16 16 entries serialize", ret == 0 && ser.buffer.data[0] == 0xDE);
        
        msgpack_reader reader;
        msgpack_reader_init(&reader, ser.buffer.data, ser.buffer.length);
        msgpack_object out = {0};
        ret = msgpack_read_object(&reader, &out);
        test("map16 16 entries deserialize", ret == 0 && out.as.map.size == 16);
        
        free(map.as.map.ptr);
        msgpack_serializer_free(&ser);
    }
    
    /* ========== BINARY ========== */
    printf("\n--- BINARY ---\n");
    {
        msgpack_serializer ser;
        msgpack_serializer_init(&ser, 256);
        
        char *data = malloc(10);
        memset(data, 0xAB, 10);
        msgpack_object obj = {.type = MSGPACK_TYPE_BIN8, .as.bin.ptr = data, .as.bin.size = 10};
        
        int ret = msgpack_serialize(&ser, &obj);
        test("bin8 10 bytes serialize", ret == 0 && ser.buffer.data[0] == 0xC4);
        
        msgpack_reader reader;
        msgpack_reader_init(&reader, ser.buffer.data, ser.buffer.length);
        msgpack_object out = {0};
        ret = msgpack_read_object(&reader, &out);
        test("bin8 10 bytes deserialize", ret == 0 && out.as.bin.size == 10);
        
        free(data);
        msgpack_serializer_free(&ser);
    }
    
    /* ========== TIMESTAMP ========== */
    printf("\n--- TIMESTAMP ---\n");
    {
        msgpack_serializer ser;
        msgpack_serializer_init(&ser, 64);
        msgpack_object obj = {.type = MSGPACK_TYPE_TIMESTAMP, .as.timestamp = 1704067200};
        
        int ret = msgpack_serialize(&ser, &obj);
        test("timestamp serialize", ret == 0 && ser.buffer.data[0] == 0xD6);
        
        msgpack_reader reader;
        msgpack_reader_init(&reader, ser.buffer.data, ser.buffer.length);
        msgpack_object out = {0};
        ret = msgpack_read_object(&reader, &out);
        test("timestamp deserialize", ret == 0 && out.as.timestamp == 1704067200);
        
        msgpack_serializer_free(&ser);
        
        /* Zero timestamp */
        msgpack_serializer_init(&ser, 64);
        obj.as.timestamp = 0;
        ret = msgpack_serialize(&ser, &obj);
        
        msgpack_reader_init(&reader, ser.buffer.data, ser.buffer.length);
        out = (msgpack_object){0};
        ret = msgpack_read_object(&reader, &out);
        test("timestamp 0 deserialize", ret == 0 && out.as.timestamp == 0);
        
        msgpack_serializer_free(&ser);
    }
    
    /* ========== NESTED STRUCTURES ========== */
    printf("\n--- NESTED STRUCTURES ---\n");
    {
        msgpack_serializer ser;
        msgpack_serializer_init(&ser, 512);
        
        /* { "arr": [1, 2, { "x": 3 }], "str": "hello" } */
        msgpack_object map = {.type = MSGPACK_TYPE_MAP};
        map.as.map.size = 2;
        map.as.map.ptr = malloc(2 * sizeof(msgpack_object_kv));
        
        /* "arr" -> [1, 2, { "x": 3 }] */
        map.as.map.ptr[0].key.type = MSGPACK_TYPE_STR;
        map.as.map.ptr[0].key.as.str.ptr = "arr";
        map.as.map.ptr[0].key.as.str.size = 3;
        
        msgpack_object inner_arr = {.type = MSGPACK_TYPE_ARRAY};
        inner_arr.as.array.size = 3;
        inner_arr.as.array.ptr = malloc(3 * sizeof(msgpack_object));
        inner_arr.as.array.ptr[0].type = MSGPACK_TYPE_INT64; inner_arr.as.array.ptr[0].as.i = 1;
        inner_arr.as.array.ptr[1].type = MSGPACK_TYPE_INT64; inner_arr.as.array.ptr[1].as.i = 2;
        
        /* { "x": 3 } */
        inner_arr.as.array.ptr[2].type = MSGPACK_TYPE_MAP;
        inner_arr.as.array.ptr[2].as.map.size = 1;
        inner_arr.as.array.ptr[2].as.map.ptr = malloc(1 * sizeof(msgpack_object_kv));
        inner_arr.as.array.ptr[2].as.map.ptr[0].key.type = MSGPACK_TYPE_STR;
        inner_arr.as.map.ptr[0].key.as.str.ptr = "x";
        inner_arr.as.map.ptr[0].key.as.str.size = 1;
        inner_arr.as.map.ptr[0].value.type = MSGPACK_TYPE_INT64;
        inner_arr.as.map.ptr[0].value.as.i = 3;
        
        map.as.map.ptr[0].value = inner_arr;
        
        /* "str" -> "hello" */
        map.as.map.ptr[1].key.type = MSGPACK_TYPE_STR;
        map.as.map.ptr[1].key.as.str.ptr = "str";
        map.as.map.ptr[1].key.as.str.size = 3;
        map.as.map.ptr[1].value.type = MSGPACK_TYPE_STR;
        map.as.map.ptr[1].value.as.str.ptr = "hello";
        map.as.map.ptr[1].value.as.str.size = 5;
        
        int ret = msgpack_serialize(&ser, &map);
        test("nested complex serialize", ret == 0);
        
        msgpack_reader reader;
        msgpack_reader_init(&reader, ser.buffer.data, ser.buffer.length);
        msgpack_object out = {0};
        ret = msgpack_read_object(&reader, &out);
        test("nested complex deserialize", ret == 0 && out.as.map.size == 2);
        
        free(inner_arr.as.array.ptr[2].as.map.ptr);
        free(inner_arr.as.array.ptr);
        free(map.as.map.ptr);
        msgpack_serializer_free(&ser);
    }
    
    /* ========== EMPTY STRUCTURES ========== */
    printf("\n--- EMPTY STRUCTURES ---\n");
    {
        msgpack_serializer ser;
        msgpack_reader reader;
        msgpack_object out;
        int ret;
        
        /* Empty array */
        msgpack_serializer_init(&ser, 64);
        msgpack_object arr = {.type = MSGPACK_TYPE_ARRAY, .as.array.size = 0, .as.array.ptr = NULL};
        ret = msgpack_serialize(&ser, &arr);
        test("empty array serialize", ret == 0 && ser.buffer.data[0] == 0x90);
        
        msgpack_reader_init(&reader, ser.buffer.data, ser.buffer.length);
        out = (msgpack_object){0};
        ret = msgpack_read_object(&reader, &out);
        test("empty array deserialize", ret == 0 && out.as.array.size == 0);
        msgpack_serializer_free(&ser);
        
        /* Empty map */
        msgpack_serializer_init(&ser, 64);
        msgpack_object map = {.type = MSGPACK_TYPE_MAP, .as.map.size = 0, .as.map.ptr = NULL};
        ret = msgpack_serialize(&ser, &map);
        test("empty map serialize", ret == 0 && ser.buffer.data[0] == 0x80);
        
        msgpack_reader_init(&reader, ser.buffer.data, ser.buffer.length);
        out = (msgpack_object){0};
        ret = msgpack_read_object(&reader, &out);
        test("empty map deserialize", ret == 0 && out.as.map.size == 0);
        msgpack_serializer_free(&ser);
        
        /* Empty string */
        msgpack_serializer_init(&ser, 64);
        msgpack_object str = {.type = MSGPACK_TYPE_STR, .as.str.ptr = "", .as.str.size = 0};
        ret = msgpack_serialize(&ser, &str);
        test("empty string serialize", ret == 0 && ser.buffer.data[0] == 0xA0);
        
        msgpack_reader_init(&reader, ser.buffer.data, ser.buffer.length);
        out = (msgpack_object){0};
        ret = msgpack_read_object(&reader, &out);
        test("empty string deserialize", ret == 0 && out.as.str.size == 0);
        msgpack_serializer_free(&ser);
    }
    
    /* ========== EDGE CASES ========== */
    printf("\n--- EDGE CASES ---\n");
    {
        msgpack_serializer ser;
        msgpack_reader reader;
        msgpack_object out;
        int ret;
        
        /* Large array */
        msgpack_serializer_init(&ser, 4096);
        msgpack_object arr = {.type = MSGPACK_TYPE_ARRAY};
        arr.as.array.size = 100;
        arr.as.array.ptr = malloc(100 * sizeof(msgpack_object));
        for (int i = 0; i < 100; i++) {
            arr.as.array.ptr[i].type = MSGPACK_TYPE_INT64;
            arr.as.array.ptr[i].as.i = i;
        }
        ret = msgpack_serialize(&ser, &arr);
        test("large array (100 elements) serialize", ret == 0);
        
        msgpack_reader_init(&reader, ser.buffer.data, ser.buffer.length);
        out = (msgpack_object){0};
        ret = msgpack_read_object(&reader, &out);
        test("large array (100 elements) deserialize", ret == 0 && out.as.array.size == 100);
        
        free(arr.as.array.ptr);
        msgpack_serializer_free(&ser);
        
        /* Large string (1000 bytes) */
        msgpack_serializer_init(&ser, 2048);
        char *str1000 = malloc(1000);
        memset(str1000, 'L', 1000);
        msgpack_object str = {.type = MSGPACK_TYPE_STR, .as.str.ptr = str1000, .as.str.size = 1000};
        ret = msgpack_serialize(&ser, &str);
        test("large string (1000 bytes) serialize", ret == 0);
        
        msgpack_reader_init(&reader, ser.buffer.data, ser.buffer.length);
        out = (msgpack_object){0};
        ret = msgpack_read_object(&reader, &out);
        test("large string (1000 bytes) deserialize", ret == 0 && out.as.str.size == 1000);
        
        free(str1000);
        msgpack_serializer_free(&ser);
    }
    
    /* ========== SUMMARY ========== */
    printf("\n=== Results: %d passed, %d failed ===\n", tests_passed, tests_failed);
    
    return tests_failed;
}
