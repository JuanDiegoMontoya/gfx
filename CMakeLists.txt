cmake_minimum_required(VERSION 3.25)

project(GFX2)

find_package(Vulkan REQUIRED)

option(GFX2_BUILD_EXAMPLES "Compile example executable for GFX2." FALSE)

add_library(gfx2
	"src/memory.cpp"
	"src/context.cpp"
	"src/gfx2_vulkan.cpp"
	"src/pipeline.cpp"
)

target_compile_features(gfx2 PUBLIC cxx_std_23)

target_compile_options(gfx2
	PRIVATE
	$<$<OR:$<CXX_COMPILER_ID:AppleClang>,$<CXX_COMPILER_ID:GNU>,$<CXX_COMPILER_ID:Clang>>:
		-Wall
		-Wextra
	>
	$<$<CXX_COMPILER_ID:MSVC>:
		/W4
		/permissive-
		/Zc:preprocessor # Enable preprocessor compliance
	>
)

target_compile_definitions(gfx2 PUBLIC
	VMA_VULKAN_VERSION=1003000 # Allow VMA to use Vulkan 1.2 and 1.3 features (BDA and maintenance4)
)

target_include_directories(gfx2
	PUBLIC
	${Vulkan_INCLUDE_DIRS}
	src
	include
)

add_subdirectory(external/VulkanMemoryAllocator)

target_link_libraries(gfx2
	PRIVATE
	Vulkan::Vulkan
	VulkanMemoryAllocator
)

if (${GFX2_BUILD_EXAMPLES})
	add_subdirectory(examples)
endif()