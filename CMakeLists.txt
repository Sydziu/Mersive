cmake_minimum_required(VERSION 3.14.6)


project(httpServer)
file(GLOB_RECURSE SRC_FILES LIST_DIRECTORIES false src/*.cpp src/*.c src/*.cxx src/*.s)

add_executable ( httpServer ${SRC_FILES})

find_package(Threads REQUIRED)
if(THREADS_HAVE_PTHREAD_ARG)
  set_property(TARGET httpServer PROPERTY COMPILE_OPTIONS "-pthread")
  set_property(TARGET httpServer PROPERTY INTERFACE_COMPILE_OPTIONS "-pthread")
endif()
if(CMAKE_THREAD_LIBS_INIT)
  target_link_libraries(httpServer "${CMAKE_THREAD_LIBS_INIT}")
endif()


IF((NOT DEFINED CMAKE_INSTALL_PREFIX) OR CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
  SET(CMAKE_INSTALL_PREFIX "${CMAKE_BINARY_DIR}/dist" CACHE PATH "Default install path" FORCE)
ENDIF()


# asan
option(asan "Adress sanitizer" OFF)
if(asan)
message(INFO " Adress sanitizer")
   set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -g -fsanitize=address -fsanitize=leak")
   set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -fsanitize=address -fsanitize=leak")
   set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} ")
endif()


install(TARGETS httpServer   
    RUNTIME       DESTINATION "bin"
    LIBRARY       DESTINATION "lib" 
    ARCHIVE       DESTINATION "lib"
    PERMISSIONS OWNER_EXECUTE OWNER_WRITE OWNER_READ GROUP_EXECUTE GROUP_READ  GROUP_EXECUTE GROUP_READ WORLD_READ WORLD_EXECUTE
    )
