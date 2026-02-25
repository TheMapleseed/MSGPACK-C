#include "msgpack/msgpack.h"
#include <stdlib.h>
#include <string.h>
#include <math.h>

#define MSGPACK_BUFFER_GROW_SIZE 256

int msgpack_buffer_init(msgpack_buffer *buf, size_t initial_capacity) {
    if (initial_capacity == 0) {
        initial_capacity = MSGPACK_BUFFER_GROW_SIZE;
    }
    buf->data = (uint8_t *)malloc(initial_capacity);
    if (!buf->data) {
        return -1;
    }
    buf->capacity = initial_capacity;
    buf->position = 0;
    buf->length = 0;
    return 0;
}

void msgpack_buffer_free(msgpack_buffer *buf) {
    if (buf->data) {
        free(buf->data);
        buf->data = NULL;
    }
    buf->capacity = 0;
    buf->length = 0;
    buf->position = 0;
}

int msgpack_buffer_append(msgpack_buffer *buf, const void *data, size_t len) {
    if (buf->length + len > buf->capacity) {
        size_t new_capacity = buf->capacity + len + MSGPACK_BUFFER_GROW_SIZE;
        uint8_t *new_data = (uint8_t *)realloc(buf->data, new_capacity);
        if (!new_data) {
            return -1;
        }
        buf->data = new_data;
        buf->capacity = new_capacity;
    }
    memcpy(buf->data + buf->length, data, len);
    buf->length += len;
    return 0;
}

void msgpack_buffer_clear(msgpack_buffer *buf) {
    buf->position = 0;
    buf->length = 0;
}

int msgpack_serializer_init(msgpack_serializer *serializer, size_t initial_capacity) {
    return msgpack_buffer_init(&serializer->buffer, initial_capacity);
}

void msgpack_serializer_free(msgpack_serializer *serializer) {
    msgpack_buffer_free(&serializer->buffer);
}

int msgpack_pack_nil(msgpack_buffer *buf) {
    uint8_t byte = 0xC0;
    return msgpack_buffer_append(buf, &byte, 1);
}

int msgpack_pack_bool(msgpack_buffer *buf, bool b) {
    uint8_t byte = b ? 0xC3 : 0xC2;
    return msgpack_buffer_append(buf, &byte, 1);
}

int msgpack_pack_uint(msgpack_buffer *buf, uint64_t u) {
    if (u <= 127) {
        uint8_t byte = (uint8_t)u;
        return msgpack_buffer_append(buf, &byte, 1);
    } else if (u <= 0xFF) {
        uint8_t bytes[2] = {0xCC, (uint8_t)u};
        return msgpack_buffer_append(buf, bytes, 2);
    } else if (u <= 0xFFFF) {
        uint8_t bytes[3] = {0xCD, (uint8_t)(u >> 8), (uint8_t)u};
        return msgpack_buffer_append(buf, bytes, 3);
    } else if (u <= 0xFFFFFFFF) {
        uint8_t bytes[5] = {0xCE, (uint8_t)(u >> 24), (uint8_t)(u >> 16), (uint8_t)(u >> 8), (uint8_t)u};
        return msgpack_buffer_append(buf, bytes, 5);
    } else {
        uint8_t bytes[9] = {0xCF, (uint8_t)(u >> 56), (uint8_t)(u >> 48), (uint8_t)(u >> 40), (uint8_t)(u >> 32), (uint8_t)(u >> 24), (uint8_t)(u >> 16), (uint8_t)(u >> 8), (uint8_t)u};
        return msgpack_buffer_append(buf, bytes, 9);
    }
}

int msgpack_pack_int(msgpack_buffer *buf, int64_t i) {
    if (i >= 0) {
        return msgpack_pack_uint(buf, (uint64_t)i);
    }
    if (i >= -32) {
        uint8_t byte = msgpack_format_negfixint((int8_t)i);
        return msgpack_buffer_append(buf, &byte, 1);
    } else if (i >= -128) {
        int8_t val = (int8_t)i;
        uint8_t bytes[2] = {0xD0, (uint8_t)val};
        return msgpack_buffer_append(buf, bytes, 2);
    } else if (i >= -32768) {
        int16_t val = (int16_t)i;
        uint8_t bytes[3] = {0xD1, (uint8_t)((uint16_t)val >> 8), (uint8_t)((uint16_t)val & 0xFF)};
        return msgpack_buffer_append(buf, bytes, 3);
    } else if (i >= -2147483648) {
        int32_t val = (int32_t)i;
        uint8_t bytes[5] = {0xD2, (uint8_t)((uint32_t)val >> 24), (uint8_t)(((uint32_t)val >> 16) & 0xFF), (uint8_t)(((uint32_t)val >> 8) & 0xFF), (uint8_t)((uint32_t)val & 0xFF)};
        return msgpack_buffer_append(buf, bytes, 5);
    } else {
        int64_t val = i;
        uint8_t bytes[9] = {0xD3, (uint8_t)((uint64_t)val >> 56), (uint8_t)(((uint64_t)val >> 48) & 0xFF), (uint8_t)(((uint64_t)val >> 40) & 0xFF), (uint8_t)(((uint64_t)val >> 32) & 0xFF), (uint8_t)(((uint64_t)val >> 24) & 0xFF), (uint8_t)(((uint64_t)val >> 16) & 0xFF), (uint8_t)(((uint64_t)val >> 8) & 0xFF), (uint8_t)((uint64_t)val & 0xFF)};
        return msgpack_buffer_append(buf, bytes, 9);
    }
}

int msgpack_pack_float(msgpack_buffer *buf, double f) {
    float f32 = (float)f;
    if ((double)f32 == f) {
        uint8_t bytes[5];
        bytes[0] = 0xCA;
        uint8_t *fbytes = (uint8_t *)&f32;
        bytes[1] = fbytes[3];
        bytes[2] = fbytes[2];
        bytes[3] = fbytes[1];
        bytes[4] = fbytes[0];
        return msgpack_buffer_append(buf, bytes, 5);
    } else {
        uint8_t bytes[9];
        bytes[0] = 0xCB;
        uint8_t *fbytes = (uint8_t *)&f;
        bytes[1] = fbytes[7];
        bytes[2] = fbytes[6];
        bytes[3] = fbytes[5];
        bytes[4] = fbytes[4];
        bytes[5] = fbytes[3];
        bytes[6] = fbytes[2];
        bytes[7] = fbytes[1];
        bytes[8] = fbytes[0];
        return msgpack_buffer_append(buf, bytes, 9);
    }
}

int msgpack_pack_str(msgpack_buffer *buf, const char *str, size_t len) {
    if (len <= 31) {
        uint8_t byte = (uint8_t)(0xA0 | len);
        if (msgpack_buffer_append(buf, &byte, 1) != 0) return -1;
    } else if (len <= 255) {
        uint8_t bytes[2] = {0xD9, (uint8_t)len};
        if (msgpack_buffer_append(buf, bytes, 2) != 0) return -1;
    } else if (len <= 0xFFFF) {
        uint8_t bytes[3] = {0xDA, (uint8_t)(len >> 8), (uint8_t)len};
        if (msgpack_buffer_append(buf, bytes, 3) != 0) return -1;
    } else {
        uint8_t bytes[5] = {0xDB, (uint8_t)(len >> 24), (uint8_t)(len >> 16), (uint8_t)(len >> 8), (uint8_t)len};
        if (msgpack_buffer_append(buf, bytes, 5) != 0) return -1;
    }
    return msgpack_buffer_append(buf, str, len);
}

int msgpack_pack_bin(msgpack_buffer *buf, const uint8_t *bin, size_t len) {
    if (len <= 255) {
        uint8_t bytes[2] = {0xC4, (uint8_t)len};
        if (msgpack_buffer_append(buf, bytes, 2) != 0) return -1;
    } else if (len <= 0xFFFF) {
        uint8_t bytes[3] = {0xC5, (uint8_t)(len >> 8), (uint8_t)len};
        if (msgpack_buffer_append(buf, bytes, 3) != 0) return -1;
    } else {
        uint8_t bytes[5] = {0xC6, (uint8_t)(len >> 24), (uint8_t)(len >> 16), (uint8_t)(len >> 8), (uint8_t)len};
        if (msgpack_buffer_append(buf, bytes, 5) != 0) return -1;
    }
    return msgpack_buffer_append(buf, bin, len);
}

int msgpack_pack_array(msgpack_buffer *buf, uint32_t size) {
    if (size <= 15) {
        uint8_t byte = (uint8_t)(0x90 | size);
        return msgpack_buffer_append(buf, &byte, 1);
    } else if (size <= 0xFFFF) {
        uint8_t bytes[3] = {0xDC, (uint8_t)(size >> 8), (uint8_t)size};
        return msgpack_buffer_append(buf, bytes, 3);
    } else {
        uint8_t bytes[5] = {0xDD, (uint8_t)(size >> 24), (uint8_t)(size >> 16), (uint8_t)(size >> 8), (uint8_t)size};
        return msgpack_buffer_append(buf, bytes, 5);
    }
}

int msgpack_pack_map(msgpack_buffer *buf, uint32_t size) {
    if (size <= 15) {
        uint8_t byte = (uint8_t)(0x80 | size);
        return msgpack_buffer_append(buf, &byte, 1);
    } else if (size <= 0xFFFF) {
        uint8_t bytes[3] = {0xDE, (uint8_t)(size >> 8), (uint8_t)size};
        return msgpack_buffer_append(buf, bytes, 3);
    } else {
        uint8_t bytes[5] = {0xDF, (uint8_t)(size >> 24), (uint8_t)(size >> 16), (uint8_t)(size >> 8), (uint8_t)size};
        return msgpack_buffer_append(buf, bytes, 5);
    }
}

int msgpack_pack_ext(msgpack_buffer *buf, int8_t type, const uint8_t *data, size_t len) {
    if (len == 1) {
        uint8_t bytes[2] = {0xD4, (uint8_t)type};
        if (msgpack_buffer_append(buf, bytes, 2) != 0) return -1;
    } else if (len == 2) {
        uint8_t bytes[2] = {0xD5, (uint8_t)type};
        if (msgpack_buffer_append(buf, bytes, 2) != 0) return -1;
    } else if (len == 4) {
        uint8_t bytes[2] = {0xD6, (uint8_t)type};
        if (msgpack_buffer_append(buf, bytes, 2) != 0) return -1;
    } else if (len == 8) {
        uint8_t bytes[2] = {0xD7, (uint8_t)type};
        if (msgpack_buffer_append(buf, bytes, 2) != 0) return -1;
    } else if (len == 16) {
        uint8_t bytes[2] = {0xD8, (uint8_t)type};
        if (msgpack_buffer_append(buf, bytes, 2) != 0) return -1;
    } else if (len <= 255) {
        uint8_t bytes[2] = {0xC7, (uint8_t)len};
        if (msgpack_buffer_append(buf, bytes, 2) != 0) return -1;
        if (msgpack_buffer_append(buf, (uint8_t *)&type, 1) != 0) return -1;
    } else if (len <= 0xFFFF) {
        uint8_t bytes[3] = {0xC8, (uint8_t)(len >> 8), (uint8_t)len};
        if (msgpack_buffer_append(buf, bytes, 3) != 0) return -1;
        if (msgpack_buffer_append(buf, (uint8_t *)&type, 1) != 0) return -1;
    } else {
        uint8_t bytes[5] = {0xC9, (uint8_t)(len >> 24), (uint8_t)(len >> 16), (uint8_t)(len >> 8), (uint8_t)len};
        if (msgpack_buffer_append(buf, bytes, 5) != 0) return -1;
        if (msgpack_buffer_append(buf, (uint8_t *)&type, 1) != 0) return -1;
    }
    return msgpack_buffer_append(buf, data, len);
}

int msgpack_pack_timestamp(msgpack_buffer *buf, int64_t seconds, uint32_t nanoseconds) {
    if (nanoseconds == 0 && seconds >= 0 && seconds <= 0xFFFFFFFFLL) {
        uint8_t bytes[6];
        bytes[0] = 0xD6;
        bytes[1] = (int8_t)-1;
        uint32_t sec = (uint32_t)seconds;
        bytes[2] = (uint8_t)((sec >> 24) & 0xFF);
        bytes[3] = (uint8_t)((sec >> 16) & 0xFF);
        bytes[4] = (uint8_t)((sec >> 8) & 0xFF);
        bytes[5] = (uint8_t)(sec & 0xFF);
        return msgpack_buffer_append(buf, bytes, 6);
    } else if ((uint64_t)seconds <= 0xFFFFFFFF) {
        uint8_t bytes[10];
        bytes[0] = 0xD7;
        bytes[1] = (int8_t)-1;
        uint32_t ns = nanoseconds;
        uint32_t sec = (uint32_t)seconds;
        bytes[2] = (uint8_t)((ns >> 24) & 0xFF);
        bytes[3] = (uint8_t)((ns >> 16) & 0xFF);
        bytes[4] = (uint8_t)((ns >> 8) & 0xFF);
        bytes[5] = (uint8_t)(ns & 0xFF);
        bytes[6] = (uint8_t)((sec >> 24) & 0xFF);
        bytes[7] = (uint8_t)((sec >> 16) & 0xFF);
        bytes[8] = (uint8_t)((sec >> 8) & 0xFF);
        bytes[9] = (uint8_t)(sec & 0xFF);
        return msgpack_buffer_append(buf, bytes, 10);
    } else {
        uint8_t bytes[15];
        bytes[0] = 0xC7;
        bytes[1] = 12;
        bytes[2] = (int8_t)-1;
        uint32_t ns = nanoseconds;
        bytes[3] = (uint8_t)((ns >> 24) & 0xFF);
        bytes[4] = (uint8_t)((ns >> 16) & 0xFF);
        bytes[5] = (uint8_t)((ns >> 8) & 0xFF);
        bytes[6] = (uint8_t)(ns & 0xFF);
        int64_t sec = seconds;
        bytes[7] = (uint8_t)((sec >> 56) & 0xFF);
        bytes[8] = (uint8_t)((sec >> 48) & 0xFF);
        bytes[9] = (uint8_t)((sec >> 40) & 0xFF);
        bytes[10] = (uint8_t)((sec >> 32) & 0xFF);
        bytes[11] = (uint8_t)((sec >> 24) & 0xFF);
        bytes[12] = (uint8_t)((sec >> 16) & 0xFF);
        bytes[13] = (uint8_t)((sec >> 8) & 0xFF);
        bytes[14] = (uint8_t)(sec & 0xFF);
        return msgpack_buffer_append(buf, bytes, 15);
    }
}

static int msgpack_serialize_recursive(msgpack_serializer *serializer, const msgpack_object *obj);

int msgpack_serialize(msgpack_serializer *serializer, const msgpack_object *obj) {
    msgpack_buffer_clear(&serializer->buffer);
    return msgpack_serialize_recursive(serializer, obj);
}

static int msgpack_serialize_recursive(msgpack_serializer *serializer, const msgpack_object *obj) {
    // Note: buffer is NOT cleared here - it's already cleared by the top-level call
    
    switch (obj->type) {
        case MSGPACK_TYPE_NIL:
            return msgpack_pack_nil(&serializer->buffer);
        case MSGPACK_TYPE_BOOL:
            return msgpack_pack_bool(&serializer->buffer, obj->as.b);
        case MSGPACK_TYPE_POSITIVE_FIXINT:
        case MSGPACK_TYPE_UINT8:
        case MSGPACK_TYPE_UINT16:
        case MSGPACK_TYPE_UINT32:
        case MSGPACK_TYPE_UINT64:
            return msgpack_pack_uint(&serializer->buffer, obj->as.u);
        case MSGPACK_TYPE_NEGATIVE_FIXINT:
        case MSGPACK_TYPE_INT8:
        case MSGPACK_TYPE_INT16:
        case MSGPACK_TYPE_INT32:
        case MSGPACK_TYPE_INT64:
            return msgpack_pack_int(&serializer->buffer, obj->as.i);
        case MSGPACK_TYPE_FLOAT32:
        case MSGPACK_TYPE_FLOAT64:
            return msgpack_pack_float(&serializer->buffer, obj->as.f);
        case MSGPACK_TYPE_FIXSTR:
        case MSGPACK_TYPE_STR8:
        case MSGPACK_TYPE_STR16:
        case MSGPACK_TYPE_STR32:
            return msgpack_pack_str(&serializer->buffer, obj->as.str.ptr, obj->as.str.size);
        case MSGPACK_TYPE_BIN8:
        case MSGPACK_TYPE_BIN16:
        case MSGPACK_TYPE_BIN32:
            return msgpack_pack_bin(&serializer->buffer, obj->as.bin.ptr, obj->as.bin.size);
        case MSGPACK_TYPE_FIXARRAY:
        case MSGPACK_TYPE_ARRAY16:
        case MSGPACK_TYPE_ARRAY32: {
            int ret = msgpack_pack_array(&serializer->buffer, obj->as.array.size);
            if (ret != 0) return ret;
            for (uint32_t i = 0; i < obj->as.array.size; i++) {
                ret = msgpack_serialize_recursive(serializer, &obj->as.array.ptr[i]);
                if (ret != 0) return ret;
            }
            return 0;
        }
        case MSGPACK_TYPE_FIXMAP:
        case MSGPACK_TYPE_MAP16:
        case MSGPACK_TYPE_MAP32: {
            int ret = msgpack_pack_map(&serializer->buffer, obj->as.map.size);
            if (ret != 0) return ret;
            for (uint32_t i = 0; i < obj->as.map.size; i++) {
                ret = msgpack_serialize_recursive(serializer, &obj->as.map.ptr[i].key);
                if (ret != 0) return ret;
                ret = msgpack_serialize_recursive(serializer, &obj->as.map.ptr[i].value);
                if (ret != 0) return ret;
            }
            return 0;
        }
        case MSGPACK_TYPE_FIXEXT1:
        case MSGPACK_TYPE_FIXEXT2:
        case MSGPACK_TYPE_FIXEXT4:
        case MSGPACK_TYPE_FIXEXT8:
        case MSGPACK_TYPE_FIXEXT16:
        case MSGPACK_TYPE_EXT8:
        case MSGPACK_TYPE_EXT16:
        case MSGPACK_TYPE_EXT32:
            return msgpack_pack_ext(&serializer->buffer, obj->as.ext.type, obj->as.ext.ptr, obj->as.ext.size);
        case MSGPACK_TYPE_TIMESTAMP:
            return msgpack_pack_timestamp(&serializer->buffer, obj->as.timestamp, 0);
        default:
            return -1;
    }
}
