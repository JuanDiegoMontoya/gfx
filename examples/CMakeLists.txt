add_executable(example
	main.cpp
)

find_package(Vulkan COMPONENTS shaderc_combined)

target_link_libraries(example
	PRIVATE 
	gfx2
	Vulkan::shaderc_combined
)