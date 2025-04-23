# CMake generated Testfile for 
# Source directory: /home/francis-mutetwa/Documents/Main FIle/trial1/anchor-simulator/ns-3.36/src/fd-net-device
# Build directory: /home/francis-mutetwa/Documents/Main FIle/trial1/anchor-simulator/ns-3.36/cmake-cache/src/fd-net-device
# 
# This file includes the relevant testing commands required for 
# testing this directory and lists subdirectories to be tested as well.
add_test(ctest-raw-sock-creator "ns3.36-raw-sock-creator-default")
set_tests_properties(ctest-raw-sock-creator PROPERTIES  WORKING_DIRECTORY "/home/francis-mutetwa/Documents/Main FIle/trial1/anchor-simulator/ns-3.36/build/src/fd-net-device/" _BACKTRACE_TRIPLES "/home/francis-mutetwa/Documents/Main FIle/trial1/anchor-simulator/ns-3.36/build-support/macros-and-definitions.cmake;1354;add_test;/home/francis-mutetwa/Documents/Main FIle/trial1/anchor-simulator/ns-3.36/src/fd-net-device/CMakeLists.txt;169;set_runtime_outputdirectory;/home/francis-mutetwa/Documents/Main FIle/trial1/anchor-simulator/ns-3.36/src/fd-net-device/CMakeLists.txt;0;")
add_test(ctest-tap-device-creator "ns3.36-tap-device-creator-default")
set_tests_properties(ctest-tap-device-creator PROPERTIES  WORKING_DIRECTORY "/home/francis-mutetwa/Documents/Main FIle/trial1/anchor-simulator/ns-3.36/build/src/fd-net-device/" _BACKTRACE_TRIPLES "/home/francis-mutetwa/Documents/Main FIle/trial1/anchor-simulator/ns-3.36/build-support/macros-and-definitions.cmake;1354;add_test;/home/francis-mutetwa/Documents/Main FIle/trial1/anchor-simulator/ns-3.36/src/fd-net-device/CMakeLists.txt;204;set_runtime_outputdirectory;/home/francis-mutetwa/Documents/Main FIle/trial1/anchor-simulator/ns-3.36/src/fd-net-device/CMakeLists.txt;0;")
subdirs("examples")
