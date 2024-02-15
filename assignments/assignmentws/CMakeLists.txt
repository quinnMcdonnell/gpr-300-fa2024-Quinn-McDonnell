file(
 GLOB_RECURSE ASSIGNMENTWS_INC CONFIGURE_DEPENDS
 RELATIVE ${CMAKE_CURRENT_SOURCE_DIR}
 *.h *.hpp
)

file(
 GLOB_RECURSE ASSIGNMENTWS_SRC CONFIGURE_DEPENDS
 RELATIVE ${CMAKE_CURRENT_SOURCE_DIR}
 *.c *.cpp
)
#Copies this assignmentws's asset folder to bin when it is built
add_custom_target(copyAssetsAws ALL COMMAND ${CMAKE_COMMAND} -E copy_directory
${CMAKE_CURRENT_SOURCE_DIR}/assets/
${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/assets/)

install(FILES ${ASSIGNMENTWS_INC} DESTINATION include/assignmentws)
add_executable(assignmentws ${ASSIGNMENTWS_SRC} ${ASSIGNMENTWS_INC})
target_link_libraries(assignmentws PUBLIC core IMGUI assimp)
target_include_directories(assignmentws PUBLIC ${CORE_INC_DIR} ${stb_INCLUDE_DIR})

#Trigger asset copy when assignmentws is built
add_dependencies(assignmentws copyAssetsAws)