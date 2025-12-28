//? #version 460 core
#ifndef GFX2_GLSL_H
#define GFX2_GLSL_H

#if defined(__cplusplus) || defined(__STDC__)

  #include <stdint.h>

  #define GFX_GLSL_UINT32 uint32_t

  #define GFX_GLSL_DECLARE_BUFFER_REFERENCE_4(type_name) \
    typedef const void* type_name;                       \
    struct type_name##_t

  #define GFX_GLSL_DECLARE_BUFFER_REFERENCE_8(name) GFX_GLSL_DECLARE_BUFFER_REFERENCE_4(name)

#else // GLSL

  #define GFX_GLSL_UINT32 uint

  #extension GL_GOOGLE_include_directive : enable
  #extension GL_EXT_nonuniform_qualifier : require          // descriptor indexing
  #extension GL_EXT_scalar_block_layout : require           // sane buffer layout
  #extension GL_EXT_buffer_reference : require              // BDA
  #extension GL_EXT_buffer_reference2 : require             // BDA pointer-to-array indexing
  #extension GL_EXT_shader_image_load_formatted : require   // readable images without explicit format
  #extension GL_EXT_samplerless_texture_functions : require // texelFetch on sampled images

  #define NonUniformIndex nonuniformEXT

  #define GFX_GLSL_STORAGE_IMAGE_BINDING        0
  #define GFX_GLSL_SAMPLED_IMAGE_BINDING        1
  #define GFX_GLSL_SAMPLER_BINDING              2

  // TODO: the bindings should come from a shared header
  #define GFX_GLSL_DECLARE_SAMPLED_IMAGES(type) layout(set = 0, binding = GFX_GLSL_SAMPLED_IMAGE_BINDING) uniform type t_sampledImages_##type[]

  #define GFX_GLSL_DECLARE_SAMPLERS layout(set = 0, binding = GFX_GLSL_SAMPLER_BINDING) uniform sampler s_samplers[]

  #define GFX_GLSL_DECLARE_STORAGE_IMAGES(type) layout(set = 0, binding = GFX_GLSL_STORAGE_IMAGE_BINDING) uniform type i_storageImages_##type[]

  #define GFX_GLSL_DECLARE_BUFFER_REFERENCE_4(typename) layout(buffer_reference, buffer_reference_align = 4, scalar) buffer typename
  #define GFX_GLSL_DECLARE_BUFFER_REFERENCE_8(typename) layout(buffer_reference, buffer_reference_align = 8, scalar) buffer typename

  // Qualifiers can be put in the block name
  #define gfx_glsl_get_sampled_image(type, index)       t_sampledImages_##type[index]

  #define gfx_glsl_get_sampler(index) s_samplers[index]

  #define gfx_glsl_get_storage_image(name, index) i_storageImages_##name[index]

  #define gfx_glsl_sampler1D(textureIndex, samplerIndex) \
    NonUniformIndex(sampler1D(gfx_glsl_get_sampled_image(texture1D, textureIndex), gfx_glsl_get_sampler(samplerIndex)))

  #define gfx_glsl_sampler2D(textureIndex, samplerIndex) \
    NonUniformIndex(sampler2D(gfx_glsl_get_sampled_image(texture2D, textureIndex), gfx_glsl_get_sampler(samplerIndex)))

  #define gfx_glsl_sampler2D_array(textureIndex, samplerIndex) \
    NonUniformIndex(sampler2DArray(gfx_glsl_get_sampled_image(texture2DArray, textureIndex), gfx_glsl_get_sampler(samplerIndex)))

  #define gfx_glsl_usampler2D(textureIndex, samplerIndex) \
    NonUniformIndex(usampler2D(gfx_glsl_get_sampled_image(utexture2D, textureIndex), gfx_glsl_get_sampler(samplerIndex)))

  #define gfx_glsl_usampler2D_array(textureIndex, samplerIndex) \
    NonUniformIndex(usampler2DArray(gfx_glsl_get_sampled_image(utexture2DArray, textureIndex), gfx_glsl_get_sampler(samplerIndex)))

  #define gfx_glsl_sampler3D(textureIndex, samplerIndex) NonUniformIndex(sampler3D(gfx_glsl_texture3D(textureIndex), gfx_glsl_get_sampler(samplerIndex)))

  #define gfx_glsl_image2D(imageIndex) gfx_glsl_get_storage_image(image2D, imageIndex)

  #define gfx_glsl_image2D_array(imageIndex) gfx_glsl_get_storage_image(image2DArray, imageIndex)

  #define gfx_glsl_image3D(imageIndex) gfx_glsl_get_storage_image(image3D, imageIndex)

  #define gfx_glsl_uimage2D(imageIndex) gfx_glsl_get_storage_image(uimage2D, imageIndex)

  #define gfx_glsl_uimage2D_array(imageIndex) gfx_glsl_get_storage_image(uimage2DArray, imageIndex)

  #define gfx_glsl_utexture2D(textureIndex) gfx_glsl_get_sampled_image(utexture2D, textureIndex)

  #define gfx_glsl_texture2D(textureIndex) gfx_glsl_get_sampled_image(texture2D, textureIndex)

  #define gfx_glsl_texture1D(textureIndex) gfx_glsl_get_sampled_image(texture1D, textureIndex)

  #define gfx_glsl_texture2D_array(textureIndex) gfx_glsl_get_sampled_image(texture2DArray, textureIndex)

  #define gfx_glsl_texture3D(textureIndex) gfx_glsl_get_sampled_image(texture3D, textureIndex)

#endif // !(__cplusplus || __STDC__)

// Shared GLSL and C definitions
struct gfx_glsl_sampler
{
  GFX_GLSL_UINT32 samplerIdx;
};

struct gfx_glsl_texture1D
{
  GFX_GLSL_UINT32 texIdx;
};

struct gfx_glsl_texture2D
{
  GFX_GLSL_UINT32 texIdx;
};

struct gfx_glsl_utexture2D
{
  GFX_GLSL_UINT32 texIdx;
};

struct gfx_glsl_texture2D_array
{
  GFX_GLSL_UINT32 texIdx;
};

struct gfx_glsl_texture3D
{
  GFX_GLSL_UINT32 texIdx;
};

struct gfx_glsl_image2D
{
  GFX_GLSL_UINT32 imgIdx;
};

struct gfx_glsl_uimage2D
{
  GFX_GLSL_UINT32 imgIdx;
};

struct gfx_glsl_image2D_array
{
  GFX_GLSL_UINT32 imgIdx;
};

struct gfx_glsl_image3D
{
  GFX_GLSL_UINT32 imgIdx;
};

#ifndef __cplusplus

// Resource declarations
GFX_GLSL_DECLARE_SAMPLERS;

GFX_GLSL_DECLARE_SAMPLED_IMAGES(utexture2D);
GFX_GLSL_DECLARE_SAMPLED_IMAGES(texture1D);
GFX_GLSL_DECLARE_SAMPLED_IMAGES(texture2D);
GFX_GLSL_DECLARE_SAMPLED_IMAGES(texture3D);
GFX_GLSL_DECLARE_SAMPLED_IMAGES(texture2DArray);
GFX_GLSL_DECLARE_SAMPLED_IMAGES(utexture2DArray);

GFX_GLSL_DECLARE_STORAGE_IMAGES(uimage2D);
GFX_GLSL_DECLARE_STORAGE_IMAGES(image2D);
GFX_GLSL_DECLARE_STORAGE_IMAGES(image3D);
GFX_GLSL_DECLARE_STORAGE_IMAGES(image2DArray);
GFX_GLSL_DECLARE_STORAGE_IMAGES(uimage2DArray);

vec4 texture(gfx_glsl_texture2D tex, gfx_glsl_sampler sam, vec2 uv)
{
  return texture(gfx_glsl_sampler2D(tex.texIdx, sam.samplerIdx), uv);
}

vec4 texelFetch(gfx_glsl_texture2D tex, ivec2 coord, int level)
{
  return texelFetch(gfx_glsl_texture2D(tex.texIdx), coord, level);
}

vec4 texelFetch(gfx_glsl_texture3D tex, ivec3 coord, int level)
{
  return texelFetch(gfx_glsl_texture3D(tex.texIdx), coord, level);
}

vec4 texelFetch(gfx_glsl_texture2D_array tex, ivec3 coord, int level)
{
  return texelFetch(gfx_glsl_texture2D_array(tex.texIdx), coord, level);
}

uvec4 texelFetch(gfx_glsl_utexture2D tex, ivec2 coord, int level)
{
  return texelFetch(gfx_glsl_utexture2D(tex.texIdx), coord, level);
}

vec4 textureLod(gfx_glsl_texture2D tex, gfx_glsl_sampler sam, vec2 uv, float lod)
{
  return textureLod(gfx_glsl_sampler2D(tex.texIdx, sam.samplerIdx), uv, lod);
}

vec4 textureLod(gfx_glsl_texture1D tex, gfx_glsl_sampler sam, float uv, float lod)
{
  return textureLod(gfx_glsl_sampler1D(tex.texIdx, sam.samplerIdx), uv, lod);
}

vec4 textureLod(gfx_glsl_texture2D_array tex, gfx_glsl_sampler sam, vec3 uvw, float lod)
{
  return textureLod(gfx_glsl_sampler2D_array(tex.texIdx, sam.samplerIdx), uvw, lod);
}

vec4 textureLod(gfx_glsl_texture3D tex, gfx_glsl_sampler sam, vec3 uvw, float lod)
{
  return textureLod(gfx_glsl_sampler3D(tex.texIdx, sam.samplerIdx), uvw, lod);
}

ivec2 textureSize(gfx_glsl_texture2D tex, int level)
{
  return textureSize(gfx_glsl_texture2D(tex.texIdx), level);
}

ivec3 textureSize(gfx_glsl_texture2D_array tex, int level)
{
  return textureSize(gfx_glsl_texture2D_array(tex.texIdx), level);
}

ivec3 textureSize(gfx_glsl_texture3D tex, int level)
{
  return textureSize(gfx_glsl_texture3D(tex.texIdx), level);
}

ivec2 imageSize(gfx_glsl_image2D img)
{
  return imageSize(gfx_glsl_image2D(img.imgIdx));
}

ivec2 imageSize(gfx_glsl_uimage2D img)
{
  return imageSize(gfx_glsl_uimage2D(img.imgIdx));
}

ivec3 imageSize(gfx_glsl_image2D_array img)
{
  return imageSize(gfx_glsl_image2D_array(img.imgIdx));
}

ivec3 imageSize(gfx_glsl_image3D img)
{
  return imageSize(gfx_glsl_image3D(img.imgIdx));
}

void imageStore(gfx_glsl_image2D img, ivec2 coord, vec4 data)
{
  imageStore(gfx_glsl_image2D(img.imgIdx), coord, data);
}

void imageStore(gfx_glsl_image3D img, ivec3 coord, vec4 data)
{
  imageStore(gfx_glsl_image3D(img.imgIdx), coord, data);
}

void imageStore(gfx_glsl_uimage2D img, ivec2 coord, uvec4 data)
{
  imageStore(gfx_glsl_uimage2D(img.imgIdx), coord, data);
}

void imageStore(gfx_glsl_image2D_array img, ivec3 coord, vec4 data)
{
  imageStore(gfx_glsl_image2D_array(img.imgIdx), coord, data);
}

vec4 imageLoad(gfx_glsl_image2D img, ivec2 coord)
{
  return imageLoad(gfx_glsl_image2D(img.imgIdx), coord);
}

uvec4 imageLoad(gfx_glsl_uimage2D img, ivec2 coord)
{
  return imageLoad(gfx_glsl_uimage2D(img.imgIdx), coord);
}

vec4 imageLoad(gfx_glsl_image2D_array img, ivec3 coord)
{
  return imageLoad(gfx_glsl_image2D_array(img.imgIdx), coord);
}

#endif // !__cplusplus

#endif // GFX2_GLSL_H
