#ifndef MSGPACK_H
#define MSGPACK_H

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>

#ifdef __cplusplus
extern "C" {
#endif

#define MSGPACK_VERSION_MAJOR 1
#define MSGPACK_VERSION_MINOR 0
#define MSGPACK_VERSION_PATCH 0

typedef enum msgpack_type {
    MSGPACK_TYPE_NIL = 0,
    MSGPACK_TYPE_BOOL,
    MSGPACK_TYPE_POSITIVE_FIXINT,
    MSGPACK_TYPE_NEGATIVE_FIXINT,
    MSGPACK_TYPE_UINT8,
    MSGPACK_TYPE_UINT16,
    MSGPACK_TYPE_UINT32,
    MSGPACK_TYPE_UINT64,
    MSGPACK_TYPE_INT8,
    MSGPACK_TYPE_INT16,
    MSGPACK_TYPE_INT32,
    MSGPACK_TYPE_INT64,
    MSGPACK_TYPE_FLOAT32,
    MSGPACK_TYPE_FLOAT64,
    MSGPACK_TYPE_FIXSTR,
    MSGPACK_TYPE_STR8,
    MSGPACK_TYPE_STR16,
    MSGPACK_TYPE_STR32,
    MSGPACK_TYPE_FIXARRAY,
    MSGPACK_TYPE_ARRAY16,
    MSGPACK_TYPE_ARRAY32,
    MSGPACK_TYPE_FIXMAP,
    MSGPACK_TYPE_MAP16,
    MSGPACK_TYPE_MAP32,
    MSGPACK_TYPE_BIN8,
    MSGPACK_TYPE_BIN16,
    MSGPACK_TYPE_BIN32,
    MSGPACK_TYPE_FIXEXT1,
    MSGPACK_TYPE_FIXEXT2,
    MSGPACK_TYPE_FIXEXT4,
    MSGPACK_TYPE_FIXEXT8,
    MSGPACK_TYPE_FIXEXT16,
    MSGPACK_TYPE_EXT8,
    MSGPACK_TYPE_EXT16,
    MSGPACK_TYPE_EXT32,
    MSGPACK_TYPE_TIMESTAMP,
    MSGPACK_TYPE_STR = MSGPACK_TYPE_FIXSTR,
    MSGPACK_TYPE_INT = MSGPACK_TYPE_INT64,
    MSGPACK_TYPE_MAP = MSGPACK_TYPE_FIXMAP,
    MSGPACK_TYPE_ARRAY = MSGPACK_TYPE_FIXARRAY,
} msgpack_type;

typedef struct msgpack_buffer {
    uint8_t *data;
    size_t capacity;
    size_t position;
    size_t length;
} msgpack_buffer;

typedef struct msgpack_object {
    msgpack_type type;
    union {
        bool b;
        uint64_t u;
        int64_t i;
        double f;
        struct {
            uint32_t size;
            const char *ptr;
        } str;
        struct {
            uint32_t size;
            const uint8_t *ptr;
        } bin;
        struct {
            uint32_t size;
            struct msgpack_object *ptr;
        } array;
        struct {
            uint32_t size;
            struct msgpack_object_kv *ptr;
        } map;
        struct {
            int8_t type;
            uint32_t size;
            const uint8_t *ptr;
        } ext;
        int64_t timestamp;
    } as;
} msgpack_object;

typedef struct msgpack_object_kv {
    msgpack_object key;
    msgpack_object value;
} msgpack_object_kv;

typedef struct msgpack_reader {
    const uint8_t *data;
    size_t length;
    size_t position;
} msgpack_reader;

typedef struct msgpack_serializer msgpack_serializer;

typedef int (*msgpack_serialize_func)(msgpack_serializer *serializer, const msgpack_object *obj, msgpack_buffer *buf);

typedef struct msgpack_serializer {
    msgpack_buffer buffer;
    msgpack_serialize_func serialize;
    void *user_data;
} msgpack_serializer;

int msgpack_buffer_init(msgpack_buffer *buf, size_t initial_capacity);
void msgpack_buffer_free(msgpack_buffer *buf);
int msgpack_buffer_append(msgpack_buffer *buf, const void *data, size_t len);
void msgpack_buffer_clear(msgpack_buffer *buf);

int msgpack_serializer_init(msgpack_serializer *serializer, size_t initial_capacity);
void msgpack_serializer_free(msgpack_serializer *serializer);
int msgpack_serialize(msgpack_serializer *serializer, const msgpack_object *obj);

int msgpack_reader_init(msgpack_reader *reader, const void *data, size_t len);
int msgpack_read_object(msgpack_reader *reader, msgpack_object *obj);
void msgpack_object_free(msgpack_object *obj);

int msgpack_pack_nil(msgpack_buffer *buf);
int msgpack_pack_bool(msgpack_buffer *buf, bool b);
int msgpack_pack_uint(msgpack_buffer *buf, uint64_t u);
int msgpack_pack_int(msgpack_buffer *buf, int64_t i);
int msgpack_pack_float(msgpack_buffer *buf, double f);
int msgpack_pack_str(msgpack_buffer *buf, const char *str, size_t len);
int msgpack_pack_bin(msgpack_buffer *buf, const uint8_t *bin, size_t len);
int msgpack_pack_array(msgpack_buffer *buf, uint32_t size);
int msgpack_pack_map(msgpack_buffer *buf, uint32_t size);
int msgpack_pack_ext(msgpack_buffer *buf, int8_t type, const uint8_t *data, size_t len);
int msgpack_pack_timestamp(msgpack_buffer *buf, int64_t seconds, uint32_t nanoseconds);

static inline uint8_t msgpack_format_posfixint(uint8_t value) {
    return value & 0x7F;
}

static inline uint8_t msgpack_format_negfixint(int8_t value) {
    return (uint8_t)(0xE0 | (value & 0x1F));
}

static inline bool msgpack_is_posfixint(uint8_t b) {
    return (b & 0x80) == 0x00;
}

static inline bool msgpack_is_negfixint(uint8_t b) {
    return (b & 0xE0) == 0xE0;
}

static inline bool msgpack_is_fixstr(uint8_t b) {
    return (b & 0xE0) == 0xA0;
}

static inline bool msgpack_is_fixarray(uint8_t b) {
    return (b & 0xF0) == 0x90;
}

static inline bool msgpack_is_fixmap(uint8_t b) {
    return (b & 0xF0) == 0x80;
}

static inline bool msgpack_is_fixext(uint8_t b) {
    return (b & 0xF0) == 0xD0 && b >= 0xD4 && b <= 0xD8;
}

static inline uint8_t msgpack_fixstr_size(uint8_t b) {
    return b & 0x1F;
}

static inline uint8_t msgpack_fixarray_size(uint8_t b) {
    return b & 0x0F;
}

static inline uint8_t msgpack_fixmap_size(uint8_t b) {
    return b & 0x0F;
}

static inline int8_t msgpack_fixext_type(uint8_t b) {
    return (int8_t)(b & 0x0F);
}

static inline uint8_t msgpack_fixext_size(uint8_t b) {
    return (b & 0x0F);
}

#ifdef __cplusplus
}
#endif

#endif
