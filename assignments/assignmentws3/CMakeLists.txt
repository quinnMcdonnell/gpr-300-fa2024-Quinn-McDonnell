file(
 GLOB_RECURSE ASSIGNMENTWS3_INC CONFIGURE_DEPENDS
 RELATIVE ${CMAKE_CURRENT_SOURCE_DIR}
 *.h *.hpp
)

file(
 GLOB_RECURSE ASSIGNMENTWS3_SRC CONFIGURE_DEPENDS
 RELATIVE ${CMAKE_CURRENT_SOURCE_DIR}
 *.c *.cpp
)
#Copies this assignmentws3's asset folder to bin when it is built
add_custom_target(copyAssetsAWS3 ALL COMMAND ${CMAKE_COMMAND} -E copy_directory
${CMAKE_CURRENT_SOURCE_DIR}/assets/
${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/assets/)

install(FILES ${ASSIGNMENTWS3_INC} DESTINATION include/assignmentws3)
add_executable(assignmentws3 ${ASSIGNMENTWS3_SRC} ${ASSIGNMENTWS3_INC})
target_link_libraries(assignmentws3 PUBLIC core IMGUI assimp)
target_include_directories(assignmentws3 PUBLIC ${CORE_INC_DIR} ${stb_INCLUDE_DIR})

#Trigger asset copy when assignmentws3 is built
add_dependencies(assignmentws3 copyAssetsAWS3)