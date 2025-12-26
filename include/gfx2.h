#pragma once
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif


typedef int gfx_error_t;
typedef uint32_t gfx_flags_t;

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

typedef enum gfx_stage_flag_bits
{
  GFX_STAGE_ALL                    = 0x7FFFFFFF,
  GFX_STAGE_VERTEX_SHADER          = 1 << 0,
  GFX_STAGE_EARLY_FRAGMENT_TESTS   = 1 << 1,
  GFX_STAGE_FRAGMENT_SHADER        = 1 << 2,
  GFX_STAGE_LATE_FRAGMENT_TESTS    = 1 << 3,
  GFX_STAGE_FRAGMENT_SHADER_OUTPUT = 1 << 4,

  GFX_STAGE_COMPUTE = 1 << 5,

  GFX_STAGE_TRANSFER = 1 << 6,
} gfx_stage_flag_bits;
typedef gfx_flags_t gfx_stage_flags;

typedef enum gfx_access_flag_bits
{
  GFX_ACCESS_ALL   = 0x7FFFFFFF,
  GFX_ACCESS_READ  = 1 << 0,
  GFX_ACCESS_WRITE = 1 << 1,
} gfx_access_flag_bits;
typedef gfx_flags_t gfx_access_flags;

typedef struct gfx_command_buffer_t* gfx_command_buffer;
typedef struct gfx_semaphore_t* gfx_semaphore;
typedef struct gfx_compute_pipeline_t* gfx_compute_pipeline;

typedef struct gfx_submit_token
{
  gfx_semaphore semaphore;
  uint64_t value;
} gfx_submit_token;

gfx_command_buffer gfx_create_command_buffer(gfx_queue queue);
void gfx_destroy_command_buffer(gfx_command_buffer command_buffer);

gfx_submit_token gfx_submit(gfx_command_buffer command_buffer, const gfx_submit_token* wait_tokens, uint32_t num_wait_tokens);

void gfx_wait_token(gfx_submit_token token);

gfx_compute_pipeline gfx_create_compute_pipeline(gfx_byte_span code);
void gfx_destroy_compute_pipeline(gfx_compute_pipeline pipeline);

void gfx_cmd_barrier(gfx_command_buffer command_buffer, gfx_stage_flags srcStage, gfx_access_flags srcAccess, gfx_stage_flags dstStage, gfx_access_flags dstAccess);
void gfx_cmd_dispatch(gfx_command_buffer command_buffer, gfx_compute_pipeline pipeline, uint32_t x, uint32_t y, uint32_t z, const void* args);

void* gfx_malloc(size_t bytes);
void gfx_free(void* ptr);
void* gfx_host_to_device_ptr(void* ptr);


#ifdef __cplusplus
}
#endif