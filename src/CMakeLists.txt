project(Binrary)

#modules
add_subdirectory("render")
add_subdirectory("game")

file(GLOB SOURCE_FILES "*.cpp")
add_executable(${PROJECT_NAME} WIN32 ${SOURCE_FILES})

#define
target_compile_definitions(${PROJECT_NAME} PRIVATE "DLLIMPORT")
#add dll library
target_link_libraries(${PROJECT_NAME} PUBLIC RenderModule)
target_link_libraries(${PROJECT_NAME} PUBLIC GameModule)