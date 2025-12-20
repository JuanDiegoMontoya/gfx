#pragma once
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif


typedef int gfx_error_t;

typedef struct gfx_byte_span
{
  const void* ptr;
  size_t size;
} gfx_byte_span;

typedef struct gfx_compute_pipeline_t* gfx_compute_pipeline;

gfx_compute_pipeline gfx_create_compute_pipeline(gfx_byte_span code);
void gfx_free_compute_pipeline(gfx_compute_pipeline pipeline);

void* gfx_malloc(size_t bytes);
void gfx_free(void* ptr);
void* gfx_host_to_device_ptr(void* ptr);


#ifdef __cplusplus
}
#endif