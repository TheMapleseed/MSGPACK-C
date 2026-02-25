#include "msgpack/msgpack.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int msgpack_reader_init(msgpack_reader *reader, const void *data, size_t len) {
    reader->data = (const uint8_t *)data;
    reader->length = len;
    reader->position = 0;
    return 0;
}

static int msgpack_read_bytes(msgpack_reader *reader, void *out, size_t len) {
    if (reader->position + len > reader->length) {
        return -1;
    }
    memcpy(out, reader->data + reader->position, len);
    reader->position += len;
    return 0;
}

int msgpack_read_object(msgpack_reader *reader, msgpack_object *obj) {
    if (reader->position >= reader->length) {
        return -1;
    }
    
    uint8_t b = reader->data[reader->position++];
    
    if (b == 0xC0) {
        obj->type = MSGPACK_TYPE_NIL;
        return 0;
    }
    
    if (b == 0xC2) {
        obj->type = MSGPACK_TYPE_BOOL;
        obj->as.b = false;
        return 0;
    }
    
    if (b == 0xC3) {
        obj->type = MSGPACK_TYPE_BOOL;
        obj->as.b = true;
        return 0;
    }
    
    if (msgpack_is_posfixint(b)) {
        obj->type = MSGPACK_TYPE_POSITIVE_FIXINT;
        obj->as.u = b;
        return 0;
    }
    
    if (msgpack_is_negfixint(b)) {
        obj->type = MSGPACK_TYPE_NEGATIVE_FIXINT;
        obj->as.i = (int8_t)b;
        return 0;
    }
    
    if ((b & 0xE0) == 0xA0) {
        obj->type = MSGPACK_TYPE_FIXSTR;
        obj->as.str.size = b & 0x1F;
        obj->as.str.ptr = (const char *)(reader->data + reader->position);
        reader->position += obj->as.str.size;
        return 0;
    }
    
    if (msgpack_is_fixarray(b)) {
        obj->type = MSGPACK_TYPE_FIXARRAY;
        obj->as.array.size = b & 0x0F;
        obj->as.array.ptr = (msgpack_object *)malloc(obj->as.array.size * sizeof(msgpack_object));
        if (!obj->as.array.ptr) {
            return -1;
        }
        for (uint32_t i = 0; i < obj->as.array.size; i++) {
            if (msgpack_read_object(reader, &obj->as.array.ptr[i]) != 0) {
                return -1;
            }
        }
        return 0;
    }
    
    if (msgpack_is_fixmap(b)) {
        obj->type = MSGPACK_TYPE_FIXMAP;
        obj->as.map.size = b & 0x0F;
        obj->as.map.ptr = (msgpack_object_kv *)malloc(obj->as.map.size * sizeof(msgpack_object_kv));
        if (!obj->as.map.ptr) {
            return -1;
        }
        for (uint32_t i = 0; i < obj->as.map.size; i++) {
            if (msgpack_read_object(reader, &obj->as.map.ptr[i].key) != 0) {
                return -1;
            }
            if (msgpack_read_object(reader, &obj->as.map.ptr[i].value) != 0) {
                return -1;
            }
        }
        return 0;
    }
    
    if (msgpack_is_fixext(b)) {
        if (b == 0xD6) {
            int8_t ext_type;
            if (msgpack_read_bytes(reader, &ext_type, 1) != 0) return -1;
            if (ext_type != -1) return -1;
            uint32_t sec;
            if (msgpack_read_bytes(reader, &sec, 4) != 0) return -1;
            uint8_t *bytes = (uint8_t *)&sec;
            sec = ((uint32_t)bytes[0] << 24) | ((uint32_t)bytes[1] << 16) | 
                  ((uint32_t)bytes[2] << 8) | (uint32_t)bytes[3];
            obj->type = MSGPACK_TYPE_TIMESTAMP;
            obj->as.timestamp = (int64_t)sec;
            return 0;
        }
        if (b == 0xD7) {
            uint8_t data[8];
            if (msgpack_read_bytes(reader, data, 8) != 0) return -1;
            uint64_t val = ((uint64_t)data[0] << 56) | ((uint64_t)data[1] << 48) |
                           ((uint64_t)data[2] << 40) | ((uint64_t)data[3] << 32) |
                           ((uint64_t)data[4] << 24) | ((uint64_t)data[5] << 16) |
                           ((uint64_t)data[6] << 8) | (uint64_t)data[7];
            obj->type = MSGPACK_TYPE_TIMESTAMP;
            obj->as.timestamp = (int64_t)val;
            return 0;
        }
        obj->type = (msgpack_type)b;
        obj->as.ext.type = msgpack_fixext_type(b);
        obj->as.ext.size = msgpack_fixext_size(b);
        obj->as.ext.ptr = reader->data + reader->position;
        reader->position += obj->as.ext.size;
        return 0;
    }
    
    switch (b) {
        case 0xCC:
            obj->type = MSGPACK_TYPE_UINT8;
            if (msgpack_read_bytes(reader, &obj->as.u, 1) != 0) return -1;
            return 0;
        case 0xCD:
            obj->type = MSGPACK_TYPE_UINT16;
            if (msgpack_read_bytes(reader, &obj->as.u, 2) != 0) return -1;
            {
                uint16_t v = (uint16_t)obj->as.u;
                obj->as.u = (uint16_t)((v >> 8) | ((v & 0xFF) << 8));
            }
            return 0;
        case 0xCE:
            obj->type = MSGPACK_TYPE_UINT32;
            if (msgpack_read_bytes(reader, &obj->as.u, 4) != 0) return -1;
            {
                uint32_t v = (uint32_t)obj->as.u;
                obj->as.u = ((v & 0xFF) << 24) | ((v & 0xFF00) << 8) | ((v >> 8) & 0xFF00) | ((v >> 24) & 0xFF);
            }
            return 0;
        case 0xCF:
            obj->type = MSGPACK_TYPE_UINT64;
            {
                uint64_t v;
                if (msgpack_read_bytes(reader, &v, 8) != 0) return -1;
                uint8_t b[8];
                memcpy(b, &v, 8);
                uint64_t r = ((uint64_t)b[0] << 56) | ((uint64_t)b[1] << 48) | 
                           ((uint64_t)b[2] << 40) | ((uint64_t)b[3] << 32) |
                           ((uint64_t)b[4] << 24) | ((uint64_t)b[5] << 16) |
                           ((uint64_t)b[6] << 8) | (uint64_t)b[7];
                obj->as.u = r;
            }
            return 0;
        case 0xD0:
            obj->type = MSGPACK_TYPE_INT8;
            {
                int8_t v;
                if (msgpack_read_bytes(reader, &v, 1) != 0) return -1;
                obj->as.i = v;
            }
            return 0;
        case 0xD1:
            obj->type = MSGPACK_TYPE_INT16;
            {
                int16_t v;
                if (msgpack_read_bytes(reader, &v, 2) != 0) return -1;
                v = (int16_t)((((uint16_t)v >> 8) & 0xFF) | (((uint16_t)v & 0xFF) << 8));
                obj->as.i = v;
            }
            return 0;
        case 0xD2:
            obj->type = MSGPACK_TYPE_INT32;
            {
                int32_t v;
                if (msgpack_read_bytes(reader, &v, 4) != 0) return -1;
                uint32_t uv = (uint32_t)v;
                uv = ((uv >> 24) & 0xFF) | ((uv >> 8) & 0xFF00) | ((uv & 0xFF00) << 8) | ((uv & 0xFF) << 24);
                obj->as.i = (int32_t)uv;
            }
            return 0;
        case 0xD3:
            obj->type = MSGPACK_TYPE_INT64;
            {
                int64_t v;
                if (msgpack_read_bytes(reader, &v, 8) != 0) return -1;
                uint8_t b[8];
                memcpy(b, &v, 8);
                int64_t r = ((int64_t)b[0] << 56) | ((int64_t)b[1] << 48) | 
                           ((int64_t)b[2] << 40) | ((int64_t)b[3] << 32) |
                           ((int64_t)b[4] << 24) | ((int64_t)b[5] << 16) |
                           ((int64_t)b[6] << 8) | (int64_t)b[7];
                obj->as.i = r;
            }
            return 0;
            return 0;
        case 0xCA: {
            obj->type = MSGPACK_TYPE_FLOAT32;
            uint8_t bytes[4];
            if (msgpack_read_bytes(reader, bytes, 4) != 0) return -1;
            float f32;
            uint8_t *fbytes = (uint8_t *)&f32;
            fbytes[0] = bytes[3];
            fbytes[1] = bytes[2];
            fbytes[2] = bytes[1];
            fbytes[3] = bytes[0];
            obj->as.f = f32;
            return 0;
        }
        case 0xCB: {
            obj->type = MSGPACK_TYPE_FLOAT64;
            uint8_t bytes[8];
            if (msgpack_read_bytes(reader, bytes, 8) != 0) return -1;
            double f64;
            uint8_t *fbytes = (uint8_t *)&f64;
            fbytes[0] = bytes[7];
            fbytes[1] = bytes[6];
            fbytes[2] = bytes[5];
            fbytes[3] = bytes[4];
            fbytes[4] = bytes[3];
            fbytes[5] = bytes[2];
            fbytes[6] = bytes[1];
            fbytes[7] = bytes[0];
            obj->as.f = f64;
            return 0;
        }
        case 0xD9:
            obj->type = MSGPACK_TYPE_STR8;
            if (msgpack_read_bytes(reader, &obj->as.str.size, 1) != 0) return -1;
            obj->as.str.ptr = (const char *)(reader->data + reader->position);
            reader->position += obj->as.str.size;
            return 0;
        case 0xDA: {
            uint16_t size;
            if (msgpack_read_bytes(reader, &size, 2) != 0) return -1;
            obj->type = MSGPACK_TYPE_STR16;
            obj->as.str.size = (uint16_t)((((uint16_t)size >> 8) & 0xFF) | ((((uint16_t)size & 0xFF) << 8)));
            obj->as.str.ptr = (const char *)(reader->data + reader->position);
            reader->position += obj->as.str.size;
            return 0;
        }
        case 0xDB: {
            uint32_t size;
            if (msgpack_read_bytes(reader, &size, 4) != 0) return -1;
            obj->type = MSGPACK_TYPE_STR32;
            obj->as.str.size = ((size >> 24) | ((size >> 8) & 0xFF00) | ((size & 0xFF) << 8) | ((size & 0xFF) << 24));
            obj->as.str.ptr = (const char *)(reader->data + reader->position);
            reader->position += obj->as.str.size;
            return 0;
        }
        case 0xC4:
            obj->type = MSGPACK_TYPE_BIN8;
            if (msgpack_read_bytes(reader, &obj->as.bin.size, 1) != 0) return -1;
            obj->as.bin.ptr = reader->data + reader->position;
            reader->position += obj->as.bin.size;
            return 0;
        case 0xC5: {
            uint16_t size;
            if (msgpack_read_bytes(reader, &size, 2) != 0) return -1;
            obj->type = MSGPACK_TYPE_BIN16;
            obj->as.bin.size = ((size >> 8) | ((size & 0xFF) << 8));
            obj->as.bin.ptr = reader->data + reader->position;
            reader->position += obj->as.bin.size;
            return 0;
        }
        case 0xC6: {
            uint32_t size;
            if (msgpack_read_bytes(reader, &size, 4) != 0) return -1;
            obj->type = MSGPACK_TYPE_BIN32;
            obj->as.bin.size = ((size >> 24) | ((size >> 8) & 0xFF00) | ((size & 0xFF) << 8) | ((size & 0xFF) << 24));
            obj->as.bin.ptr = reader->data + reader->position;
            reader->position += obj->as.bin.size;
            return 0;
        }
        case 0xDC: {
            uint16_t size;
            if (msgpack_read_bytes(reader, &size, 2) != 0) return -1;
            obj->type = MSGPACK_TYPE_ARRAY16;
            obj->as.array.size = ((size >> 8) | ((size & 0xFF) << 8));
            obj->as.array.ptr = (msgpack_object *)malloc(obj->as.array.size * sizeof(msgpack_object));
            if (!obj->as.array.ptr) {
                return -1;
            }
            for (uint32_t i = 0; i < obj->as.array.size; i++) {
                if (msgpack_read_object(reader, &obj->as.array.ptr[i]) != 0) {
                    return -1;
                }
            }
            return 0;
        }
        case 0xDD: {
            uint32_t size;
            if (msgpack_read_bytes(reader, &size, 4) != 0) return -1;
            obj->type = MSGPACK_TYPE_ARRAY32;
            obj->as.array.size = ((size >> 24) | ((size >> 8) & 0xFF00) | ((size & 0xFF) << 8) | ((size & 0xFF) << 24));
            obj->as.array.ptr = (msgpack_object *)malloc(obj->as.array.size * sizeof(msgpack_object));
            if (!obj->as.array.ptr) {
                return -1;
            }
            for (uint32_t i = 0; i < obj->as.array.size; i++) {
                if (msgpack_read_object(reader, &obj->as.array.ptr[i]) != 0) {
                    return -1;
                }
            }
            return 0;
        }
        case 0xDE: {
            uint16_t size;
            if (msgpack_read_bytes(reader, &size, 2) != 0) return -1;
            obj->type = MSGPACK_TYPE_MAP16;
            obj->as.map.size = ((size >> 8) | ((size & 0xFF) << 8));
            obj->as.map.ptr = (msgpack_object_kv *)malloc(obj->as.map.size * sizeof(msgpack_object_kv));
            if (!obj->as.map.ptr) {
                return -1;
            }
            for (uint32_t i = 0; i < obj->as.map.size; i++) {
                if (msgpack_read_object(reader, &obj->as.map.ptr[i].key) != 0) {
                    return -1;
                }
                if (msgpack_read_object(reader, &obj->as.map.ptr[i].value) != 0) {
                    return -1;
                }
            }
            return 0;
        }
        case 0xDF: {
            uint32_t size;
            if (msgpack_read_bytes(reader, &size, 4) != 0) return -1;
            obj->type = MSGPACK_TYPE_MAP32;
            obj->as.map.size = ((size >> 24) | ((size >> 8) & 0xFF00) | ((size & 0xFF) << 8) | ((size & 0xFF) << 24));
            obj->as.map.ptr = (msgpack_object_kv *)malloc(obj->as.map.size * sizeof(msgpack_object_kv));
            if (!obj->as.map.ptr) {
                return -1;
            }
            for (uint32_t i = 0; i < obj->as.map.size; i++) {
                if (msgpack_read_object(reader, &obj->as.map.ptr[i].key) != 0) {
                    return -1;
                }
                if (msgpack_read_object(reader, &obj->as.map.ptr[i].value) != 0) {
                    return -1;
                }
            }
            return 0;
        }
        case 0xC7: {
            int8_t ext_type;
            uint8_t size;
            if (msgpack_read_bytes(reader, &size, 1) != 0) return -1;
            if (msgpack_read_bytes(reader, &ext_type, 1) != 0) return -1;
            if (ext_type == -1) {
                if (size == 4) {
                    uint32_t ns, sec;
                    if (msgpack_read_bytes(reader, &ns, 4) != 0) return -1;
                    if (msgpack_read_bytes(reader, &sec, 4) != 0) return -1;
                    obj->type = MSGPACK_TYPE_TIMESTAMP;
                    obj->as.timestamp = (int64_t)sec;
                    return 0;
                } else if (size == 8) {
                    uint64_t val;
                    if (msgpack_read_bytes(reader, &val, 8) != 0) return -1;
                    obj->type = MSGPACK_TYPE_TIMESTAMP;
                    obj->as.timestamp = (int64_t)val;
                    return 0;
                } else if (size == 12) {
                    uint32_t ns;
                    int64_t sec;
                    if (msgpack_read_bytes(reader, &ns, 4) != 0) return -1;
                    if (msgpack_read_bytes(reader, &sec, 8) != 0) return -1;
                    obj->type = MSGPACK_TYPE_TIMESTAMP;
                    obj->as.timestamp = sec;
                    return 0;
                }
            }
            obj->type = MSGPACK_TYPE_EXT8;
            obj->as.ext.type = ext_type;
            obj->as.ext.size = size;
            obj->as.ext.ptr = reader->data + reader->position;
            reader->position += size;
            return 0;
        }
        case 0xC8: {
            uint16_t size;
            if (msgpack_read_bytes(reader, &size, 2) != 0) return -1;
            int8_t ext_type;
            if (msgpack_read_bytes(reader, &ext_type, 1) != 0) return -1;
            obj->type = MSGPACK_TYPE_EXT16;
            obj->as.ext.size = ((size >> 8) | ((size & 0xFF) << 8));
            obj->as.ext.ptr = reader->data + reader->position;
            reader->position += obj->as.ext.size;
            return 0;
        }
        case 0xC9: {
            uint32_t size;
            if (msgpack_read_bytes(reader, &size, 4) != 0) return -1;
            int8_t ext_type;
            if (msgpack_read_bytes(reader, &ext_type, 1) != 0) return -1;
            obj->type = MSGPACK_TYPE_EXT32;
            obj->as.ext.size = ((size >> 24) | ((size >> 8) & 0xFF00) | ((size & 0xFF) << 8) | ((size & 0xFF) << 24));
            obj->as.ext.ptr = reader->data + reader->position;
            reader->position += obj->as.ext.size;
            return 0;
        }
        case 0xD4:
            obj->type = MSGPACK_TYPE_FIXEXT1;
            if (msgpack_read_bytes(reader, &obj->as.ext.type, 1) != 0) return -1;
            obj->as.ext.size = 1;
            obj->as.ext.ptr = reader->data + reader->position;
            reader->position += 1;
            return 0;
        case 0xD5:
            obj->type = MSGPACK_TYPE_FIXEXT2;
            if (msgpack_read_bytes(reader, &obj->as.ext.type, 1) != 0) return -1;
            obj->as.ext.size = 2;
            obj->as.ext.ptr = reader->data + reader->position;
            reader->position += 2;
            return 0;
        case 0xD6:
            printf("DEBUG: Hit case 0xD6\n");
            obj->type = MSGPACK_TYPE_FIXEXT4;
            if (msgpack_read_bytes(reader, &obj->as.ext.type, 1) != 0) return -1;
            printf("DEBUG: ext.type = %d\n", obj->as.ext.type);
            if (obj->as.ext.type == -1) {
                uint32_t sec;
                if (msgpack_read_bytes(reader, &sec, 4) != 0) return -1;
                printf("DEBUG: sec (raw) = %u\n", sec);
                sec = ((sec >> 24) & 0xFF) | ((sec >> 8) & 0xFF00) | ((sec & 0xFF00) << 8) | ((sec & 0xFF) << 24);
                printf("DEBUG: sec (swapped) = %u\n", sec);
                obj->type = MSGPACK_TYPE_TIMESTAMP;
                obj->as.timestamp = (int64_t)sec;
                printf("DEBUG: Set timestamp to %lld\n", (long long)obj->as.timestamp);
                return 0;
            }
            obj->as.ext.size = 4;
            obj->as.ext.ptr = reader->data + reader->position;
            reader->position += 4;
            return 0;
        case 0xD7:
            obj->type = MSGPACK_TYPE_FIXEXT8;
            if (msgpack_read_bytes(reader, &obj->as.ext.type, 1) != 0) return -1;
            if (obj->as.ext.type == -1) {
                uint8_t data[8];
                if (msgpack_read_bytes(reader, data, 8) != 0) return -1;
                uint64_t val = ((uint64_t)data[0] << 56) | ((uint64_t)data[1] << 48) |
                               ((uint64_t)data[2] << 40) | ((uint64_t)data[3] << 32) |
                               ((uint64_t)data[4] << 24) | ((uint64_t)data[5] << 16) |
                               ((uint64_t)data[6] << 8) | (uint64_t)data[7];
                obj->type = MSGPACK_TYPE_TIMESTAMP;
                obj->as.timestamp = (int64_t)val;
                return 0;
            }
            obj->as.ext.size = 8;
            obj->as.ext.ptr = reader->data + reader->position;
            reader->position += 8;
            return 0;
        case 0xD8:
            obj->type = MSGPACK_TYPE_FIXEXT16;
            if (msgpack_read_bytes(reader, &obj->as.ext.type, 1) != 0) return -1;
            obj->as.ext.size = 16;
            obj->as.ext.ptr = reader->data + reader->position;
            reader->position += 16;
            return 0;
    }
    
    return -1;
}

void msgpack_object_free(msgpack_object *obj) {
    if (!obj) return;
    
    switch (obj->type) {
        case MSGPACK_TYPE_FIXARRAY:
        case MSGPACK_TYPE_ARRAY16:
        case MSGPACK_TYPE_ARRAY32:
            if (obj->as.array.ptr) {
                for (uint32_t i = 0; i < obj->as.array.size; i++) {
                    msgpack_object_free(&obj->as.array.ptr[i]);
                }
                free(obj->as.array.ptr);
            }
            break;
        case MSGPACK_TYPE_FIXMAP:
        case MSGPACK_TYPE_MAP16:
        case MSGPACK_TYPE_MAP32:
            if (obj->as.map.ptr) {
                for (uint32_t i = 0; i < obj->as.map.size; i++) {
                    msgpack_object_free(&obj->as.map.ptr[i].key);
                    msgpack_object_free(&obj->as.map.ptr[i].value);
                }
                free(obj->as.map.ptr);
            }
            break;
        default:
            break;
    }
}
