# CMAKE generated file: DO NOT EDIT!
# Generated by "Unix Makefiles" Generator, CMake Version 3.16

# Delete rule output on recipe failure.
.DELETE_ON_ERROR:


#=============================================================================
# Special targets provided by cmake.

# Disable implicit rules so canonical targets will work.
.SUFFIXES:


# Remove some rules from gmake that .SUFFIXES does not remove.
SUFFIXES =

.SUFFIXES: .hpux_make_needs_suffix_list


# Suppress display of executed commands.
$(VERBOSE).SILENT:


# A target that is always out of date.
cmake_force:

.PHONY : cmake_force

#=============================================================================
# Set environment variables for the build.

# The shell in which to execute make rules.
SHELL = /bin/sh

# The CMake executable.
CMAKE_COMMAND = /usr/bin/cmake

# The command to remove a file.
RM = /usr/bin/cmake -E remove -f

# Escaping for special characters.
EQUALS = =

# The top-level source directory on which CMake was run.
CMAKE_SOURCE_DIR = /home/evgeniya/Desktop/refactored/WSRTI/syncqueue

# The top-level build directory on which CMake was run.
CMAKE_BINARY_DIR = /home/evgeniya/Desktop/refactored/WSRTI/syncqueue/build

# Include any dependencies generated for this target.
include CMakeFiles/syncqueue-test.dir/depend.make

# Include the progress variables for this target.
include CMakeFiles/syncqueue-test.dir/progress.make

# Include the compile flags for this target's objects.
include CMakeFiles/syncqueue-test.dir/flags.make

CMakeFiles/syncqueue-test.dir/test/SyncQueueTest.cpp.o: CMakeFiles/syncqueue-test.dir/flags.make
CMakeFiles/syncqueue-test.dir/test/SyncQueueTest.cpp.o: ../test/SyncQueueTest.cpp
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/evgeniya/Desktop/refactored/WSRTI/syncqueue/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_1) "Building CXX object CMakeFiles/syncqueue-test.dir/test/SyncQueueTest.cpp.o"
	/usr/bin/c++  $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -o CMakeFiles/syncqueue-test.dir/test/SyncQueueTest.cpp.o -c /home/evgeniya/Desktop/refactored/WSRTI/syncqueue/test/SyncQueueTest.cpp

CMakeFiles/syncqueue-test.dir/test/SyncQueueTest.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/syncqueue-test.dir/test/SyncQueueTest.cpp.i"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /home/evgeniya/Desktop/refactored/WSRTI/syncqueue/test/SyncQueueTest.cpp > CMakeFiles/syncqueue-test.dir/test/SyncQueueTest.cpp.i

CMakeFiles/syncqueue-test.dir/test/SyncQueueTest.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/syncqueue-test.dir/test/SyncQueueTest.cpp.s"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /home/evgeniya/Desktop/refactored/WSRTI/syncqueue/test/SyncQueueTest.cpp -o CMakeFiles/syncqueue-test.dir/test/SyncQueueTest.cpp.s

# Object files for target syncqueue-test
syncqueue__test_OBJECTS = \
"CMakeFiles/syncqueue-test.dir/test/SyncQueueTest.cpp.o"

# External object files for target syncqueue-test
syncqueue__test_EXTERNAL_OBJECTS =

syncqueue-test: CMakeFiles/syncqueue-test.dir/test/SyncQueueTest.cpp.o
syncqueue-test: CMakeFiles/syncqueue-test.dir/build.make
syncqueue-test: CMakeFiles/syncqueue-test.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --bold --progress-dir=/home/evgeniya/Desktop/refactored/WSRTI/syncqueue/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_2) "Linking CXX executable syncqueue-test"
	$(CMAKE_COMMAND) -E cmake_link_script CMakeFiles/syncqueue-test.dir/link.txt --verbose=$(VERBOSE)

# Rule to build all files generated by this target.
CMakeFiles/syncqueue-test.dir/build: syncqueue-test

.PHONY : CMakeFiles/syncqueue-test.dir/build

CMakeFiles/syncqueue-test.dir/clean:
	$(CMAKE_COMMAND) -P CMakeFiles/syncqueue-test.dir/cmake_clean.cmake
.PHONY : CMakeFiles/syncqueue-test.dir/clean

CMakeFiles/syncqueue-test.dir/depend:
	cd /home/evgeniya/Desktop/refactored/WSRTI/syncqueue/build && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /home/evgeniya/Desktop/refactored/WSRTI/syncqueue /home/evgeniya/Desktop/refactored/WSRTI/syncqueue /home/evgeniya/Desktop/refactored/WSRTI/syncqueue/build /home/evgeniya/Desktop/refactored/WSRTI/syncqueue/build /home/evgeniya/Desktop/refactored/WSRTI/syncqueue/build/CMakeFiles/syncqueue-test.dir/DependInfo.cmake --color=$(COLOR)
.PHONY : CMakeFiles/syncqueue-test.dir/depend
