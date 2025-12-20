#pragma once
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif


typedef int gfx_error_t;

typedef struct gfx_byte_span
{
  const void* ptr;
  size_t size;
} gfx_byte_span;

typedef enum gfx_queue
{
  GFX_QUEUE_GRAPHICS,
  GFX_QUEUE_COMPUTE,
  GFX_QUEUE_TRANSFER,

  GFX_NUM_QUEUES,
} gfx_queue;

typedef struct gfx_command_buffer_t* gfx_command_buffer;
typedef struct gfx_semaphore_t* gfx_semaphore;
typedef struct gfx_compute_pipeline_t* gfx_compute_pipeline;

gfx_command_buffer gfx_create_command_buffer(gfx_queue queue);
void gfx_destroy_command_buffer(gfx_command_buffer command_buffer);

void gfx_submit(gfx_command_buffer command_buffer, gfx_semaphore semaphore, uint64_t signal);

gfx_semaphore gfx_create_semaphore(uint64_t initial_value);
void gfx_wait_semaphore(gfx_semaphore semaphore, uint64_t value);
void gfx_destroy_semaphore(gfx_semaphore semaphore);

gfx_compute_pipeline gfx_create_compute_pipeline(gfx_byte_span code);
void gfx_destroy_compute_pipeline(gfx_compute_pipeline pipeline);

void gfx_cmd_dispatch(gfx_command_buffer command_buffer, gfx_compute_pipeline pipeline, uint32_t x, uint32_t y, uint32_t z, const void* args);

void* gfx_malloc(size_t bytes);
void gfx_free(void* ptr);
void* gfx_host_to_device_ptr(void* ptr);


#ifdef __cplusplus
}
#endif