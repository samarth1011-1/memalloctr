# Custom Memory Allocator

A memory allocator which uses

- `malloc()`
- `free()`
- `realloc()`
- `calloc()`

custom written in C.

*(This uses `sbrk()` to allocate/deallocate memory from the OS, which is normally not available in Windows environments.)*
