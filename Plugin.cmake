# ~~~
# Summary:      Local, non-generic plugin setup
# Copyright (c) 2020-2024 Mike Rossiter
# License:      GPLv3+
# ~~~

# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 3 of the License, or
# (at your option) any later version.


# -------- Options ----------

set(OCPN_TEST_REPO
    "finsar/finsar_edit-alpha"
    CACHE STRING "Default repository for untagged builds"
)
set(OCPN_BETA_REPO
    "finsar/finsar_edit-beta"
    CACHE STRING
    "Default repository for tagged builds matching 'beta'"
)
set(OCPN_RELEASE_REPO
    "finsar/finsar_edit-prod"
    CACHE STRING
    "Default repository for tagged builds not matching 'beta'"
)

#
#
# -------  Plugin setup --------
#
set(PKG_NAME finSAR_edit_pi)
set(PKG_VERSION  0.2.0)
set(PKG_PRERELEASE "")  # Empty, or a tag like 'beta'

set(DISPLAY_NAME finSAR_edit)    # Dialogs, installer artifacts, ...
set(PLUGIN_API_NAME finSAR_edit) # As of GetCommonName() in plugin API
set(PKG_SUMMARY "Finland SAR")
set(PKG_DESCRIPTION [=[
For Finland SAR.
]=])

set(PKG_AUTHOR "Mike Rossiter")
set(PKG_IS_OPEN_SOURCE "yes")
set(PKG_HOMEPAGE https://github.com/Rasbats/finSAR_edit_pi)
set(PKG_INFO_URL https://opencpn.org/OpenCPN/plugins/finSAR_edit.html)

SET(SRC
        src/finSAR_edit_pi.h
        src/finSAR_edit_pi.cpp
        src/finSAR_editOverlayFactory.cpp
        src/finSAR_editOverlayFactory.h
        src/finSAR_editUIDialogBase.cpp
        src/finSAR_editUIDialogBase.h
        src/finSAR_editUIDialog.cpp
        src/finSAR_editUIDialog.h
        src/icons.h
        src/icons.cpp
        src/NavFunc.cpp
        src/NavFunc.h
        src/pugixml.hpp
    )

set(PKG_API_LIB api-18)  #  A directory in libs/ e. g., api-17 or api-16

macro(late_init)
  # Perform initialization after the PACKAGE_NAME library, compilers
  # and ocpn::api is available.
endmacro ()

macro(add_plugin_libraries)
  # Add libraries required by this plugin
  add_subdirectory("${CMAKE_SOURCE_DIR}/opencpn-libs/pugixml")
  target_link_libraries(${PACKAGE_NAME} ocpn::pugixml)

  add_subdirectory("${CMAKE_SOURCE_DIR}/opencpn-libs/wxJSON")
  target_link_libraries(${PACKAGE_NAME} ocpn::wxjson)

  add_subdirectory("${CMAKE_SOURCE_DIR}/opencpn-libs/plugin_dc")
  target_link_libraries(${PACKAGE_NAME} ocpn::plugin-dc)

  add_subdirectory("${CMAKE_SOURCE_DIR}/opencpn-libs/jsoncpp")
  target_link_libraries(${PACKAGE_NAME} ocpn::jsoncpp)

  add_subdirectory("${CMAKE_SOURCE_DIR}/opencpn-libs/sqlite")
  target_link_libraries(${PACKAGE_NAME} ocpn::sqlite)


  # The wxsvg library enables SVG overall in the plugin
  add_subdirectory("${CMAKE_SOURCE_DIR}/opencpn-libs/wxsvg")
  target_link_libraries(${PACKAGE_NAME} ocpn::wxsvg)
endmacro ()
