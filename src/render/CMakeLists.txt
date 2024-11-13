project(RenderModule)

#add_subdirectory("DirectX-Headers")

file(GLOB SOURCE_FILES "*.cpp")
file(GLOB HELPER_SOURCE_FILES "d3dhelper/*.cpp")
add_library(${PROJECT_NAME} SHARED ${SOURCE_FILES} ${HELPER_SOURCE_FILES})
target_compile_definitions(${PROJECT_NAME} PUBLIC "DLLEXPORT")
target_include_directories(${PROJECT_NAME} PUBLIC "./include")
target_include_directories(${PROJECT_NAME} PUBLIC "./DirectX-Headers/include/directx")
target_include_directories(${PROJECT_NAME} PUBLIC "./dxc/inc")
target_include_directories(${PROJECT_NAME} PUBLIC "../util/DirectXMath/inc")
target_include_directories(${PROJECT_NAME} PRIVATE "./private_include")
target_include_directories(${PROJECT_NAME} PRIVATE "./d3dhelper/include")
target_link_directories(${PROJECT_NAME} PUBLIC "./dxc/lib/x64")
target_link_libraries(${PROJECT_NAME} PRIVATE "d3d12.lib" "dxgi.lib" "d3dcompiler.lib" "dxguid.lib" "dxcompiler.lib")

add_custom_target(RenderShader)
add_dependencies(${PROJECT_NAME} RenderShader)
add_custom_command(
         TARGET RenderShader POST_BUILD
         COMMAND ${CMAKE_COMMAND} -E copy_directory              
         "${CMAKE_CURRENT_SOURCE_DIR}/shader"
         "${EXECUTABLE_OUTPUT_PATH}/shader"
         COMMENT "Copying shaders" VERBATIM
         )


add_custom_target(AgilityDX12)
add_dependencies(${PROJECT_NAME} AgilityDX12)
add_custom_command(
         TARGET AgilityDX12 POST_BUILD
         COMMAND ${CMAKE_COMMAND} -E copy_directory              
         "${CMAKE_CURRENT_SOURCE_DIR}/D3d12-Agility/bin"
         "${EXECUTABLE_OUTPUT_PATH}/D3D12"
         COMMENT "Copying agility sdk" VERBATIM
         )

add_custom_target(DxCompiler)
add_dependencies(${PROJECT_NAME} DxCompiler)
add_custom_command(
         TARGET DxCompiler POST_BUILD
         COMMAND ${CMAKE_COMMAND} -E copy_directory              
         "${CMAKE_CURRENT_SOURCE_DIR}/dxc/bin/x64"
         "${EXECUTABLE_OUTPUT_PATH}"
         COMMENT "Copying DxCompiler" VERBATIM
         )