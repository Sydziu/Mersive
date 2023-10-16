cmake_minimum_required(VERSION 3.14.6)



#*******************************************************************************
#                        Main Target
#*******************************************************************************
project(httpServer)

file(GLOB_RECURSE SRC_FILES LIST_DIRECTORIES false src/*.cpp src/*.c src/*.cxx src/*.s)

add_executable ( httpServer ${SRC_FILES})
target_link_libraries(httpServer PRIVATE  pthread)

#if(THREADS_HAVE_PTHREAD_ARG)
#  set_property(TARGET httpServer PROPERTY COMPILE_OPTIONS "-pthread")
#  set_property(TARGET httpServer PROPERTY INTERFACE_COMPILE_OPTIONS "-pthread")
#endif()
#if(CMAKE_THREAD_LIBS_INIT)
#  target_link_libraries(httpServer "${CMAKE_THREAD_LIBS_INIT}")
#endif()


IF((NOT DEFINED CMAKE_INSTALL_PREFIX) OR CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
  SET(CMAKE_INSTALL_PREFIX "${CMAKE_BINARY_DIR}/dist" CACHE PATH "Default install path" FORCE)
ENDIF()

install(TARGETS httpServer   
    RUNTIME       DESTINATION "bin"
    LIBRARY       DESTINATION "lib" 
    ARCHIVE       DESTINATION "lib"
    PERMISSIONS OWNER_EXECUTE OWNER_WRITE OWNER_READ GROUP_EXECUTE GROUP_READ  GROUP_EXECUTE GROUP_READ WORLD_READ WORLD_EXECUTE
    )

#*******************************************************************************
#                        Static Analyzer 
#*******************************************************************************
option(sta "Static Analyzer" OFF)
if(sta)
    message(INFO " Static Analyzer is on")
    find_program(CMAKE_CXX_CLANG_TIDY NAMES clang-tidy)
    list( APPEND CMAKE_CXX_CLANG_TIDY "-checks=*,-cppcoreguidelines-no-malloc,-hicpp-no-malloc, -altera-unroll-loops, -hicpp-signed-bitwise,-cppcoreguidelines-avoid-magic-numbers,-readability-magic-numbers,-llvm*,-fuchsia*,-android*,-boost*,-mpi*,-objc*,-zircon*,-abseil*,-clang-analyzer-osx*,-clang-analyzer-optin.osx*,-clang-analyzer-optin.mpi*,-cert-*,-clang-diagnostic-unused-function,-cppcoreguidelines-pro-type-union-access,-cppcoreguidelines-pro-bounds-constant-array-index,-cppcoreguidelines-pro-type-vararg,-hicpp-vararg,-cppcoreguidelines-pro-bounds-pointer-arithmetic,-hicpp-no-array-decay,-modernize-avoid-c-arrays,-hicpp-avoid-c-arrays,-cppcoreguidelines-avoid-c-arrays,-cppcoreguidelines-pro-bounds-array-to-pointer-decay,-hicpp-use-nullptr,-modernize-use-nullptr,-cppcoreguidelines-pro-type-cstyle-cast,-cppcoreguidelines-owning-memory,-modernize-use-trailing-return-type") 
    find_program(CMAKE_CXX_CPPCHECK NAMES cppcheck)
    list( APPEND CMAKE_CXX_CPPCHECK "")
endif()

#*******************************************************************************
#                        Code Coverage 
#*******************************************************************************
option(cov "Code coverage" OFF)
if(cov)
message(INFO " Code coverage is on")
   set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -coverage -g")
   set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -coverage -g")
   set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -lgcov")
endif()

#*******************************************************************************
#                        WALL
#*******************************************************************************
option(wall "Warning all" OFF)
if(wall)
message(INFO " Warning all is on")
   set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -Wextra")
   set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra")
   set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} ")
endif()


#*******************************************************************************
#                        ASAN
#*******************************************************************************
option(asan "Adress sanitizer" OFF)
if(asan)
message(INFO " Adress sanitizer is on")
   set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -g -fsanitize=address -fsanitize=leak")
   set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -fsanitize=address -fsanitize=leak")
   set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} ")
endif()

#*******************************************************************************
#                        TSAN
#*******************************************************************************
# tsan
option(tsan "Thread sanitizer" OFF)
if(tsan)
message(INFO " Thread sanitizer is on")
   set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -g -fsanitize=thread")
   set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -fsanitize=thread")
   set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} ")
endif()

#*******************************************************************************
#                        GTESTs
#*******************************************************************************
find_package(GTest QUIET)

if(GTest_FOUND)
   enable_testing()
   file(GLOB_RECURSE GSRC_FILES LIST_DIRECTORIES false test/*.cpp test/*.c test/*.cxx test/*.s)
   
   foreach (fileName ${GSRC_FILES})
      get_filename_component(testName ${fileName} NAME_WLE)
      add_executable(${testName} ${fileName})
      add_test(NAME ${testName} COMMAND ${testName})
      target_link_libraries(${testName} PUBLIC  GTest::GTest gtest_main)
  
      install(TARGETS ${testName}   
         RUNTIME       DESTINATION "test"
         LIBRARY       DESTINATION "test" 
         ARCHIVE       DESTINATION "test"
         PERMISSIONS OWNER_EXECUTE OWNER_WRITE OWNER_READ GROUP_EXECUTE GROUP_READ  GROUP_EXECUTE GROUP_READ WORLD_READ WORLD_EXECUTE
         )
   endforeach()
   

    
else()
   message(WARNING "GTest library has not been found. Unit tests will not be avaiable.")
endif()

#*******************************************************************************
#                               Product test
#*******************************************************************************
find_program (BASH_PROGRAM bash)
if (BASH_PROGRAM)
  add_test(
     NAME productTest 
     COMMAND ${BASH_PROGRAM} ${CMAKE_CURRENT_SOURCE_DIR}/productTest/testScript.sh
     WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/productTest/
    )
endif (BASH_PROGRAM)

#*******************************************************************************
#                   Code wich dump all the CMAKE's variables
#*******************************************************************************
#get_cmake_property(_variableNames VARIABLES)
#foreach (_variableName ${_variableNames})
#    message(STATUS "${_variableName}=${${_variableName}}")
#endforeach()


