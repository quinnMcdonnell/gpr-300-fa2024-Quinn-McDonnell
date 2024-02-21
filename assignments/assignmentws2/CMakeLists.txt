file(
 GLOB_RECURSE ASSIGNMENTWS2_INC CONFIGURE_DEPENDS
 RELATIVE ${CMAKE_CURRENT_SOURCE_DIR}
 *.h *.hpp
)

file(
 GLOB_RECURSE ASSIGNMENTWS2_SRC CONFIGURE_DEPENDS
 RELATIVE ${CMAKE_CURRENT_SOURCE_DIR}
 *.c *.cpp
)
#Copies this assignmentws2's asset folder to bin when it is built
add_custom_target(copyAssetsAWS2 ALL COMMAND ${CMAKE_COMMAND} -E copy_directory
${CMAKE_CURRENT_SOURCE_DIR}/assets/
${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/assets/)

install(FILES ${ASSIGNMENTWS2_INC} DESTINATION include/assignmentws2)
add_executable(assignmentws2 ${ASSIGNMENTWS2_SRC} ${ASSIGNMENTWS2_INC})
target_link_libraries(assignmentws2 PUBLIC core IMGUI assimp)
target_include_directories(assignmentws2 PUBLIC ${CORE_INC_DIR} ${stb_INCLUDE_DIR})

#Trigger asset copy when assignmentws2 is built
add_dependencies(assignmentws2 copyAssetsAWS2)