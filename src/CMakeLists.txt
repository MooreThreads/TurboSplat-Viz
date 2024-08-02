project(Binrary)

#modules
add_subdirectory("launch")
add_subdirectory("render")

file(GLOB SOURCE_FILES "*.cpp")
add_executable(${PROJECT_NAME} WIN32 ${SOURCE_FILES})

#define
target_compile_definitions(${PROJECT_NAME} PRIVATE "DLLIMPORT")
#add dll library
target_link_libraries(${PROJECT_NAME} PUBLIC LaunchModule)
target_link_libraries(${PROJECT_NAME} PUBLIC RenderModule)