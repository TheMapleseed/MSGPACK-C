#include "msgpack/msgpack.h"
#include <stdio.h>
#include <stdlib.h>

int main(void) {
    msgpack_serializer serializer;
    msgpack_serializer_init(&serializer, 1024);
    
    msgpack_object obj = {0};
    obj.type = MSGPACK_TYPE_MAP;
    obj.as.map.size = 3;
    obj.as.map.ptr = (msgpack_object_kv *)malloc(3 * sizeof(msgpack_object_kv));
    
    obj.as.map.ptr[0].key.type = MSGPACK_TYPE_STR;
    obj.as.map.ptr[0].key.as.str.ptr = "name";
    obj.as.map.ptr[0].key.as.str.size = 4;
    obj.as.map.ptr[0].value.type = MSGPACK_TYPE_STR;
    obj.as.map.ptr[0].value.as.str.ptr = "Alice";
    obj.as.map.ptr[0].value.as.str.size = 5;
    
    obj.as.map.ptr[1].key.type = MSGPACK_TYPE_STR;
    obj.as.map.ptr[1].key.as.str.ptr = "age";
    obj.as.map.ptr[1].key.as.str.size = 3;
    obj.as.map.ptr[1].value.type = MSGPACK_TYPE_INT;
    obj.as.map.ptr[1].value.as.i = 30;
    
    obj.as.map.ptr[2].key.type = MSGPACK_TYPE_STR;
    obj.as.map.ptr[2].key.as.str.ptr = "active";
    obj.as.map.ptr[2].key.as.str.size = 6;
    obj.as.map.ptr[2].value.type = MSGPACK_TYPE_BOOL;
    obj.as.map.ptr[2].value.as.b = true;
    
    int ret = msgpack_serialize(&serializer, &obj);
    if (ret != 0) {
        printf("Serialization failed\n");
        return 1;
    }
    
    printf("Serialized %zu bytes\n", serializer.buffer.length);
    
    msgpack_reader reader;
    msgpack_reader_init(&reader, serializer.buffer.data, serializer.buffer.length);
    
    msgpack_object read_obj = {0};
    ret = msgpack_read_object(&reader, &read_obj);
    if (ret != 0) {
        printf("Deserialization failed\n");
        return 1;
    }
    
    printf("Deserialized: type=%d\n", read_obj.type);
    
    msgpack_object_free(&obj);
    msgpack_object_free(&read_obj);
    msgpack_serializer_free(&serializer);
    
    printf("Test passed!\n");
    return 0;
}
