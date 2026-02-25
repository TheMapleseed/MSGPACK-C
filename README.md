# msgpack-c

A small, portable C library for **serializing and deserializing** [MessagePack](https://msgpack.org/) (binary JSON-like format). It supports the full MessagePack type set and can be used as a static library in your project.

## Features

- **Types supported:** nil, bool, integers (fixint, int8–64, uint8–64), float32/64, strings (fixstr, str8/16/32), binary (bin8/16/32), arrays (fixarray, array16/32), maps (fixmap, map16/32), extension types, and timestamps
- **Two usage styles:** high-level object tree + serialize/deserialize, or low-level `msgpack_pack_*` + buffer for streaming/custom encoding
- **C23**, single public header, no external dependencies
- **CMake** build; produces a static library plus optional example and test executables

## Requirements

- CMake 3.16+
- C compiler with C23 support (e.g. GCC 12+, Clang 15+)

## Building

```bash
mkdir build && cd build
cmake ..
make
```

This builds:

- **`libmsgpack.a`** – static library
- **`msgpack_example`** – demo program
- **`msgpack_test`** / **`msgpack_test_comprehensive`** – test suites

Run tests:

```bash
ctest
# or
./msgpack_test
./msgpack_test_comprehensive
```

## How to Use

### 1. Add to your project

- Add this repo as a submodule or copy the `include/` and `src/` trees into your project.
- In CMake, add the subdirectory and link:

  ```cmake
  add_subdirectory(path/to/msgpack-c)
  target_link_libraries(your_target msgpack)
  target_include_directories(your_target PRIVATE path/to/msgpack-c/include)
  ```

- In your C code, include the header:

  ```c
  #include "msgpack/msgpack.h"
  ```

### 2. High-level: object tree → bytes → object tree

Build a **`msgpack_object`** tree (maps, arrays, strings, numbers, etc.), then serialize it to bytes; or take bytes and read them back into a **`msgpack_object`** tree.

**Serializing (object → bytes):**

```c
msgpack_serializer serializer;
msgpack_serializer_init(&serializer, 1024);

msgpack_object obj = {0};
obj.type = MSGPACK_TYPE_MAP;
obj.as.map.size = 2;
obj.as.map.ptr = malloc(2 * sizeof(msgpack_object_kv));

// Key "name" -> value "Alice"
obj.as.map.ptr[0].key.type   = MSGPACK_TYPE_STR;
obj.as.map.ptr[0].key.as.str.ptr = "name";
obj.as.map.ptr[0].key.as.str.size = 4;
obj.as.map.ptr[0].value.type = MSGPACK_TYPE_STR;
obj.as.map.ptr[0].value.as.str.ptr = "Alice";
obj.as.map.ptr[0].value.as.str.size = 5;

// Key "age" -> value 30
obj.as.map.ptr[1].key.type   = MSGPACK_TYPE_STR;
obj.as.map.ptr[1].key.as.str.ptr = "age";
obj.as.map.ptr[1].key.as.str.size = 3;
obj.as.map.ptr[1].value.type = MSGPACK_TYPE_INT;
obj.as.map.ptr[1].value.as.i = 30;

int ret = msgpack_serialize(&serializer, &obj);
if (ret != 0) { /* error */ }

// Result: serializer.buffer.data, length serializer.buffer.length
uint8_t *bytes = serializer.buffer.data;
size_t   len   = serializer.buffer.length;

// ... send or store bytes ...

free(obj.as.map.ptr);
msgpack_serializer_free(&serializer);
```

**Deserializing (bytes → object):**

```c
msgpack_reader reader;
msgpack_reader_init(&reader, bytes, len);

msgpack_object out = {0};
int ret = msgpack_read_object(&reader, &out);
if (ret != 0) { /* error */ }

// Use out (e.g. out.type, out.as.map.size, out.as.map.ptr[i].key/value)
// When done, free nested arrays/maps:
msgpack_object_free(&out);
```

**Important:** For strings and binary, the decoded `msgpack_object` holds **pointers into the buffer** you passed to `msgpack_reader_init`. Keep that buffer valid while using the object, or copy the data.

### 3. Low-level: pack directly into a buffer

You can skip the object tree and encode values one-by-one with the `msgpack_pack_*` functions:

```c
msgpack_buffer buf;
msgpack_buffer_init(&buf, 256);

msgpack_pack_map(&buf, 2);                    // map of 2 pairs
msgpack_pack_str(&buf, "name", 4);           // key "name"
msgpack_pack_str(&buf, "Alice", 5);          // value "Alice"
msgpack_pack_str(&buf, "age", 3);           // key "age"
msgpack_pack_int(&buf, 30);                  // value 30

// Result in buf.data, length buf.length

msgpack_buffer_free(&buf);
```

Available pack functions include: `msgpack_pack_nil`, `msgpack_pack_bool`, `msgpack_pack_uint`, `msgpack_pack_int`, `msgpack_pack_float`, `msgpack_pack_str`, `msgpack_pack_bin`, `msgpack_pack_array`, `msgpack_pack_map`, `msgpack_pack_ext`, `msgpack_pack_timestamp`. See `include/msgpack/msgpack.h` for the full API.

### 4. Run the example

From the build directory:

```bash
./msgpack_example
```

This serializes a small map (name, age, active), deserializes it back, and prints the result.

## API overview

| Area | Functions |
|------|-----------|
| **Buffer** | `msgpack_buffer_init`, `msgpack_buffer_free`, `msgpack_buffer_append`, `msgpack_buffer_clear` |
| **Serializer** | `msgpack_serializer_init`, `msgpack_serializer_free`, `msgpack_serialize` |
| **Reader** | `msgpack_reader_init`, `msgpack_read_object`, `msgpack_object_free` |
| **Packing** | `msgpack_pack_nil`, `msgpack_pack_bool`, `msgpack_pack_uint`, `msgpack_pack_int`, `msgpack_pack_float`, `msgpack_pack_str`, `msgpack_pack_bin`, `msgpack_pack_array`, `msgpack_pack_map`, `msgpack_pack_ext`, `msgpack_pack_timestamp` |

Types and helpers (e.g. `msgpack_is_fixstr`, `msgpack_fixstr_size`) are defined in **`include/msgpack/msgpack.h`**.

## License

See the repository for license information.
