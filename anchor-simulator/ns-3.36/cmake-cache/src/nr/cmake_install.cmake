# Install script for directory: /home/francis-mutetwa/Documents/Main FIle/trial1/anchor-simulator/ns-3.36/src/nr

# Set the install prefix
if(NOT DEFINED CMAKE_INSTALL_PREFIX)
  set(CMAKE_INSTALL_PREFIX "/usr/local")
endif()
string(REGEX REPLACE "/$" "" CMAKE_INSTALL_PREFIX "${CMAKE_INSTALL_PREFIX}")

# Set the install configuration name.
if(NOT DEFINED CMAKE_INSTALL_CONFIG_NAME)
  if(BUILD_TYPE)
    string(REGEX REPLACE "^[^A-Za-z0-9_]+" ""
           CMAKE_INSTALL_CONFIG_NAME "${BUILD_TYPE}")
  else()
    set(CMAKE_INSTALL_CONFIG_NAME "default")
  endif()
  message(STATUS "Install configuration: \"${CMAKE_INSTALL_CONFIG_NAME}\"")
endif()

# Set the component getting installed.
if(NOT CMAKE_INSTALL_COMPONENT)
  if(COMPONENT)
    message(STATUS "Install component: \"${COMPONENT}\"")
    set(CMAKE_INSTALL_COMPONENT "${COMPONENT}")
  else()
    set(CMAKE_INSTALL_COMPONENT)
  endif()
endif()

# Install shared libraries without execute permission?
if(NOT DEFINED CMAKE_INSTALL_SO_NO_EXE)
  set(CMAKE_INSTALL_SO_NO_EXE "1")
endif()

# Is this installation the result of a crosscompile?
if(NOT DEFINED CMAKE_CROSSCOMPILING)
  set(CMAKE_CROSSCOMPILING "FALSE")
endif()

# Set default install directory permissions.
if(NOT DEFINED CMAKE_OBJDUMP)
  set(CMAKE_OBJDUMP "/usr/bin/objdump")
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  if(EXISTS "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/libns3.36-nr-default.so" AND
     NOT IS_SYMLINK "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/libns3.36-nr-default.so")
    file(RPATH_CHECK
         FILE "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/libns3.36-nr-default.so"
         RPATH "")
  endif()
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib" TYPE SHARED_LIBRARY FILES "/home/francis-mutetwa/Documents/Main FIle/trial1/anchor-simulator/ns-3.36/build/lib/libns3.36-nr-default.so")
  if(EXISTS "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/libns3.36-nr-default.so" AND
     NOT IS_SYMLINK "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/libns3.36-nr-default.so")
    file(RPATH_CHANGE
         FILE "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/libns3.36-nr-default.so"
         OLD_RPATH "/home/francis-mutetwa/Documents/Main FIle/trial1/anchor-simulator/ns-3.36/build/lib:"
         NEW_RPATH "")
    if(CMAKE_INSTALL_DO_STRIP)
      execute_process(COMMAND "/usr/bin/strip" "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/libns3.36-nr-default.so")
    endif()
  endif()
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include/ns3" TYPE FILE FILES
    "/home/francis-mutetwa/Documents/Main FIle/trial1/anchor-simulator/ns-3.36/src/nr/helper/nr-helper.h"
    "/home/francis-mutetwa/Documents/Main FIle/trial1/anchor-simulator/ns-3.36/src/nr/helper/nr-phy-rx-trace.h"
    "/home/francis-mutetwa/Documents/Main FIle/trial1/anchor-simulator/ns-3.36/src/nr/helper/nr-mac-rx-trace.h"
    "/home/francis-mutetwa/Documents/Main FIle/trial1/anchor-simulator/ns-3.36/src/nr/helper/nr-point-to-point-epc-helper.h"
    "/home/francis-mutetwa/Documents/Main FIle/trial1/anchor-simulator/ns-3.36/src/nr/helper/nr-bearer-stats-calculator.h"
    "/home/francis-mutetwa/Documents/Main FIle/trial1/anchor-simulator/ns-3.36/src/nr/helper/nr-bearer-stats-connector.h"
    "/home/francis-mutetwa/Documents/Main FIle/trial1/anchor-simulator/ns-3.36/src/nr/helper/nr-bearer-stats-simple.h"
    "/home/francis-mutetwa/Documents/Main FIle/trial1/anchor-simulator/ns-3.36/src/nr/helper/beamforming-helper-base.h"
    "/home/francis-mutetwa/Documents/Main FIle/trial1/anchor-simulator/ns-3.36/src/nr/helper/ideal-beamforming-helper.h"
    "/home/francis-mutetwa/Documents/Main FIle/trial1/anchor-simulator/ns-3.36/src/nr/helper/realistic-beamforming-helper.h"
    "/home/francis-mutetwa/Documents/Main FIle/trial1/anchor-simulator/ns-3.36/src/nr/helper/node-distribution-scenario-interface.h"
    "/home/francis-mutetwa/Documents/Main FIle/trial1/anchor-simulator/ns-3.36/src/nr/helper/grid-scenario-helper.h"
    "/home/francis-mutetwa/Documents/Main FIle/trial1/anchor-simulator/ns-3.36/src/nr/helper/hexagonal-grid-scenario-helper.h"
    "/home/francis-mutetwa/Documents/Main FIle/trial1/anchor-simulator/ns-3.36/src/nr/helper/file-scenario-helper.h"
    "/home/francis-mutetwa/Documents/Main FIle/trial1/anchor-simulator/ns-3.36/src/nr/helper/cc-bwp-helper.h"
    "/home/francis-mutetwa/Documents/Main FIle/trial1/anchor-simulator/ns-3.36/src/nr/helper/nr-radio-environment-map-helper.h"
    "/home/francis-mutetwa/Documents/Main FIle/trial1/anchor-simulator/ns-3.36/src/nr/helper/nr-spectrum-value-helper.h"
    "/home/francis-mutetwa/Documents/Main FIle/trial1/anchor-simulator/ns-3.36/src/nr/helper/scenario-parameters.h"
    "/home/francis-mutetwa/Documents/Main FIle/trial1/anchor-simulator/ns-3.36/src/nr/helper/three-gpp-ftp-m1-helper.h"
    "/home/francis-mutetwa/Documents/Main FIle/trial1/anchor-simulator/ns-3.36/src/nr/helper/nr-stats-calculator.h"
    "/home/francis-mutetwa/Documents/Main FIle/trial1/anchor-simulator/ns-3.36/src/nr/helper/nr-mac-scheduling-stats.h"
    "/home/francis-mutetwa/Documents/Main FIle/trial1/anchor-simulator/ns-3.36/src/nr/model/nr-net-device.h"
    "/home/francis-mutetwa/Documents/Main FIle/trial1/anchor-simulator/ns-3.36/src/nr/model/nr-gnb-net-device.h"
    "/home/francis-mutetwa/Documents/Main FIle/trial1/anchor-simulator/ns-3.36/src/nr/model/nr-ue-net-device.h"
    "/home/francis-mutetwa/Documents/Main FIle/trial1/anchor-simulator/ns-3.36/src/nr/model/nr-phy.h"
    "/home/francis-mutetwa/Documents/Main FIle/trial1/anchor-simulator/ns-3.36/src/nr/model/nr-gnb-phy.h"
    "/home/francis-mutetwa/Documents/Main FIle/trial1/anchor-simulator/ns-3.36/src/nr/model/nr-ue-phy.h"
    "/home/francis-mutetwa/Documents/Main FIle/trial1/anchor-simulator/ns-3.36/src/nr/model/nr-spectrum-phy.h"
    "/home/francis-mutetwa/Documents/Main FIle/trial1/anchor-simulator/ns-3.36/src/nr/model/nr-interference.h"
    "/home/francis-mutetwa/Documents/Main FIle/trial1/anchor-simulator/ns-3.36/src/nr/model/nr-mac-pdu-info.h"
    "/home/francis-mutetwa/Documents/Main FIle/trial1/anchor-simulator/ns-3.36/src/nr/model/nr-mac-header-vs.h"
    "/home/francis-mutetwa/Documents/Main FIle/trial1/anchor-simulator/ns-3.36/src/nr/model/nr-mac-header-vs-ul.h"
    "/home/francis-mutetwa/Documents/Main FIle/trial1/anchor-simulator/ns-3.36/src/nr/model/nr-mac-header-vs-dl.h"
    "/home/francis-mutetwa/Documents/Main FIle/trial1/anchor-simulator/ns-3.36/src/nr/model/nr-mac-header-fs.h"
    "/home/francis-mutetwa/Documents/Main FIle/trial1/anchor-simulator/ns-3.36/src/nr/model/nr-mac-header-fs-ul.h"
    "/home/francis-mutetwa/Documents/Main FIle/trial1/anchor-simulator/ns-3.36/src/nr/model/nr-mac-header-fs-dl.h"
    "/home/francis-mutetwa/Documents/Main FIle/trial1/anchor-simulator/ns-3.36/src/nr/model/nr-mac-short-bsr-ce.h"
    "/home/francis-mutetwa/Documents/Main FIle/trial1/anchor-simulator/ns-3.36/src/nr/model/nr-phy-mac-common.h"
    "/home/francis-mutetwa/Documents/Main FIle/trial1/anchor-simulator/ns-3.36/src/nr/model/nr-mac-scheduler.h"
    "/home/francis-mutetwa/Documents/Main FIle/trial1/anchor-simulator/ns-3.36/src/nr/model/nr-mac-scheduler-tdma-rr.h"
    "/home/francis-mutetwa/Documents/Main FIle/trial1/anchor-simulator/ns-3.36/src/nr/model/nr-mac-scheduler-tdma-pf.h"
    "/home/francis-mutetwa/Documents/Main FIle/trial1/anchor-simulator/ns-3.36/src/nr/model/nr-mac-scheduler-ofdma-rr.h"
    "/home/francis-mutetwa/Documents/Main FIle/trial1/anchor-simulator/ns-3.36/src/nr/model/nr-mac-scheduler-ofdma-pf.h"
    "/home/francis-mutetwa/Documents/Main FIle/trial1/anchor-simulator/ns-3.36/src/nr/model/nr-control-messages.h"
    "/home/francis-mutetwa/Documents/Main FIle/trial1/anchor-simulator/ns-3.36/src/nr/model/nr-spectrum-signal-parameters.h"
    "/home/francis-mutetwa/Documents/Main FIle/trial1/anchor-simulator/ns-3.36/src/nr/model/nr-radio-bearer-tag.h"
    "/home/francis-mutetwa/Documents/Main FIle/trial1/anchor-simulator/ns-3.36/src/nr/model/nr-amc.h"
    "/home/francis-mutetwa/Documents/Main FIle/trial1/anchor-simulator/ns-3.36/src/nr/model/nr-mac-sched-sap.h"
    "/home/francis-mutetwa/Documents/Main FIle/trial1/anchor-simulator/ns-3.36/src/nr/model/nr-mac-csched-sap.h"
    "/home/francis-mutetwa/Documents/Main FIle/trial1/anchor-simulator/ns-3.36/src/nr/model/nr-phy-sap.h"
    "/home/francis-mutetwa/Documents/Main FIle/trial1/anchor-simulator/ns-3.36/src/nr/model/nr-lte-mi-error-model.h"
    "/home/francis-mutetwa/Documents/Main FIle/trial1/anchor-simulator/ns-3.36/src/nr/model/nr-gnb-mac.h"
    "/home/francis-mutetwa/Documents/Main FIle/trial1/anchor-simulator/ns-3.36/src/nr/model/nr-ue-mac.h"
    "/home/francis-mutetwa/Documents/Main FIle/trial1/anchor-simulator/ns-3.36/src/nr/model/nr-rrc-protocol-ideal.h"
    "/home/francis-mutetwa/Documents/Main FIle/trial1/anchor-simulator/ns-3.36/src/nr/model/nr-harq-phy.h"
    "/home/francis-mutetwa/Documents/Main FIle/trial1/anchor-simulator/ns-3.36/src/nr/model/bandwidth-part-gnb.h"
    "/home/francis-mutetwa/Documents/Main FIle/trial1/anchor-simulator/ns-3.36/src/nr/model/bandwidth-part-ue.h"
    "/home/francis-mutetwa/Documents/Main FIle/trial1/anchor-simulator/ns-3.36/src/nr/model/bwp-manager-gnb.h"
    "/home/francis-mutetwa/Documents/Main FIle/trial1/anchor-simulator/ns-3.36/src/nr/model/bwp-manager-ue.h"
    "/home/francis-mutetwa/Documents/Main FIle/trial1/anchor-simulator/ns-3.36/src/nr/model/bwp-manager-algorithm.h"
    "/home/francis-mutetwa/Documents/Main FIle/trial1/anchor-simulator/ns-3.36/src/nr/model/nr-mac-harq-process.h"
    "/home/francis-mutetwa/Documents/Main FIle/trial1/anchor-simulator/ns-3.36/src/nr/model/nr-mac-harq-vector.h"
    "/home/francis-mutetwa/Documents/Main FIle/trial1/anchor-simulator/ns-3.36/src/nr/model/nr-mac-scheduler-harq-rr.h"
    "/home/francis-mutetwa/Documents/Main FIle/trial1/anchor-simulator/ns-3.36/src/nr/model/nr-mac-scheduler-cqi-management.h"
    "/home/francis-mutetwa/Documents/Main FIle/trial1/anchor-simulator/ns-3.36/src/nr/model/nr-mac-scheduler-lcg.h"
    "/home/francis-mutetwa/Documents/Main FIle/trial1/anchor-simulator/ns-3.36/src/nr/model/nr-mac-scheduler-ns3.h"
    "/home/francis-mutetwa/Documents/Main FIle/trial1/anchor-simulator/ns-3.36/src/nr/model/nr-mac-scheduler-tdma.h"
    "/home/francis-mutetwa/Documents/Main FIle/trial1/anchor-simulator/ns-3.36/src/nr/model/nr-mac-scheduler-ofdma.h"
    "/home/francis-mutetwa/Documents/Main FIle/trial1/anchor-simulator/ns-3.36/src/nr/model/nr-mac-scheduler-ofdma-mr.h"
    "/home/francis-mutetwa/Documents/Main FIle/trial1/anchor-simulator/ns-3.36/src/nr/model/nr-mac-scheduler-tdma-mr.h"
    "/home/francis-mutetwa/Documents/Main FIle/trial1/anchor-simulator/ns-3.36/src/nr/model/nr-mac-scheduler-ue-info.h"
    "/home/francis-mutetwa/Documents/Main FIle/trial1/anchor-simulator/ns-3.36/src/nr/model/nr-mac-scheduler-ue-info-mr.h"
    "/home/francis-mutetwa/Documents/Main FIle/trial1/anchor-simulator/ns-3.36/src/nr/model/nr-mac-scheduler-ue-info-rr.h"
    "/home/francis-mutetwa/Documents/Main FIle/trial1/anchor-simulator/ns-3.36/src/nr/model/nr-mac-scheduler-ue-info-pf.h"
    "/home/francis-mutetwa/Documents/Main FIle/trial1/anchor-simulator/ns-3.36/src/nr/model/nr-eesm-error-model.h"
    "/home/francis-mutetwa/Documents/Main FIle/trial1/anchor-simulator/ns-3.36/src/nr/model/nr-eesm-t1.h"
    "/home/francis-mutetwa/Documents/Main FIle/trial1/anchor-simulator/ns-3.36/src/nr/model/nr-eesm-t2.h"
    "/home/francis-mutetwa/Documents/Main FIle/trial1/anchor-simulator/ns-3.36/src/nr/model/nr-eesm-ir.h"
    "/home/francis-mutetwa/Documents/Main FIle/trial1/anchor-simulator/ns-3.36/src/nr/model/nr-eesm-cc.h"
    "/home/francis-mutetwa/Documents/Main FIle/trial1/anchor-simulator/ns-3.36/src/nr/model/nr-eesm-ir-t1.h"
    "/home/francis-mutetwa/Documents/Main FIle/trial1/anchor-simulator/ns-3.36/src/nr/model/nr-eesm-ir-t2.h"
    "/home/francis-mutetwa/Documents/Main FIle/trial1/anchor-simulator/ns-3.36/src/nr/model/nr-eesm-cc-t1.h"
    "/home/francis-mutetwa/Documents/Main FIle/trial1/anchor-simulator/ns-3.36/src/nr/model/nr-eesm-cc-t2.h"
    "/home/francis-mutetwa/Documents/Main FIle/trial1/anchor-simulator/ns-3.36/src/nr/model/nr-error-model.h"
    "/home/francis-mutetwa/Documents/Main FIle/trial1/anchor-simulator/ns-3.36/src/nr/model/nr-ch-access-manager.h"
    "/home/francis-mutetwa/Documents/Main FIle/trial1/anchor-simulator/ns-3.36/src/nr/model/beam-id.h"
    "/home/francis-mutetwa/Documents/Main FIle/trial1/anchor-simulator/ns-3.36/src/nr/model/beamforming-vector.h"
    "/home/francis-mutetwa/Documents/Main FIle/trial1/anchor-simulator/ns-3.36/src/nr/model/beam-manager.h"
    "/home/francis-mutetwa/Documents/Main FIle/trial1/anchor-simulator/ns-3.36/src/nr/model/ideal-beamforming-algorithm.h"
    "/home/francis-mutetwa/Documents/Main FIle/trial1/anchor-simulator/ns-3.36/src/nr/model/realistic-beamforming-algorithm.h"
    "/home/francis-mutetwa/Documents/Main FIle/trial1/anchor-simulator/ns-3.36/src/nr/model/sfnsf.h"
    "/home/francis-mutetwa/Documents/Main FIle/trial1/anchor-simulator/ns-3.36/src/nr/model/lena-error-model.h"
    "/home/francis-mutetwa/Documents/Main FIle/trial1/anchor-simulator/ns-3.36/src/nr/model/nr-mac-scheduler-srs.h"
    "/home/francis-mutetwa/Documents/Main FIle/trial1/anchor-simulator/ns-3.36/src/nr/model/nr-mac-scheduler-srs-default.h"
    "/home/francis-mutetwa/Documents/Main FIle/trial1/anchor-simulator/ns-3.36/src/nr/model/nr-ue-power-control.h"
    "/home/francis-mutetwa/Documents/Main FIle/trial1/anchor-simulator/ns-3.36/src/nr/model/realistic-bf-manager.h"
    "/home/francis-mutetwa/Documents/Main FIle/trial1/anchor-simulator/ns-3.36/src/nr/model/beam-conf-id.h"
    "/home/francis-mutetwa/Documents/Main FIle/trial1/anchor-simulator/ns-3.36/src/nr/model/node-mobility.h"
    "/home/francis-mutetwa/Documents/Main FIle/trial1/anchor-simulator/ns-3.36/src/nr/model/sgp4ext.h"
    "/home/francis-mutetwa/Documents/Main FIle/trial1/anchor-simulator/ns-3.36/src/nr/model/sgp4io.h"
    "/home/francis-mutetwa/Documents/Main FIle/trial1/anchor-simulator/ns-3.36/src/nr/model/sgp4unit.h"
    "/home/francis-mutetwa/Documents/Main FIle/trial1/anchor-simulator/ns-3.36/src/nr/utils/file-transfer-helper.h"
    "/home/francis-mutetwa/Documents/Main FIle/trial1/anchor-simulator/ns-3.36/src/nr/utils/file-transfer-application.h"
    "/home/francis-mutetwa/Documents/Main FIle/trial1/anchor-simulator/ns-3.36/src/nr/utils/three-gpp-channel-model-param.h"
    "/home/francis-mutetwa/Documents/Main FIle/trial1/anchor-simulator/ns-3.36/src/nr/utils/distance-based-three-gpp-spectrum-propagation-loss-model.h"
    "/home/francis-mutetwa/Documents/Main FIle/trial1/anchor-simulator/ns-3.36/build/include/ns3/nr-module.h"
    )
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for each subdirectory.
  include("/home/francis-mutetwa/Documents/Main FIle/trial1/anchor-simulator/ns-3.36/cmake-cache/src/nr/examples/cmake_install.cmake")

endif()

