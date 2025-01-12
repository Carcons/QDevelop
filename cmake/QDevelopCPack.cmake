# set CPack options

# set package description file (ignored by certain installers)
set (CPACK_PACKAGE_DESCRIPTION_FILE ${CMAKE_CURRENT_SOURCE_DIR}/../README.md)

# set package license (Apple DragNDrop needs a file with UNIX ending)
if(APPLE)
	set (CPACK_RESOURCE_FILE_LICENSE ${CMAKE_CURRENT_SOURCE_DIR}/../LICENSE_APPLE.txt)
else()
	set (CPACK_RESOURCE_FILE_LICENSE ${CMAKE_CURRENT_SOURCE_DIR}/../LICENSE.txt)
endif()

# set package short description (ignored by certain installers)
set (CPACK_PACKAGE_DESCRIPTION "A Qt-based IDE for fast and easy C++ project development")

# set package version
set (CPACK_PACKAGE_VERSION_MAJOR ${QDevelop_VERSION_MAJOR} )
set (CPACK_PACKAGE_VERSION_MINOR ${QDevelop_VERSION_MINOR} )
set (CPACK_PACKAGE_VERSION_PATCH ${QDevelop_VERSION_PATCH} )

# set package contact
set (CPACK_PACKAGE_CONTACT "Alessandro Bria (a.bria@unicas.it)")

# set package name
set(CPACK_PACKAGE_NAME "QDevelop")

# set other O.S. specific packaging options
# APPLE
if(APPLE)
	set(CPACK_GENERATOR "DragNDrop")
	set(CPACK_COMPONENTS_ALL_IN_ONE_PACKAGE ON)
	set(CPACK_PACKAGE_ICON "${CMAKE_CURRENT_SOURCE_DIR}/../src/icons/app.icns")
	set(CPACK_PACKAGING_INSTALL_PREFIX "/${CPACK_PACKAGE_NAME}")
	set(CPACK_BUNDLE_ICON "${CMAKE_CURRENT_SOURCE_DIR}/../src/icons/app.icns")
endif()
# Windows
if(WIN32)
	string (CONCAT DESKTOP_LINK_NAME 
		"QDevelop " 
		${QDevelop_VERSION_MAJOR} 
		"." 
		${QDevelop_VERSION_MINOR} 
		"." 
		${QDevelop_VERSION_PATCH})
	set (CPACK_PACKAGE_EXECUTABLES "QDevelop" "${DESKTOP_LINK_NAME}")
	set (CPACK_CREATE_DESKTOP_LINKS "QDevelop")
endif()

# set other installer specific options
configure_file("${CMAKE_CURRENT_LIST_DIR}/QDevelopCPackOptions.cmake.in"
  "${QDevelop_BINARY_DIR}/QDevelopCPackOptions.cmake" @ONLY)
set(CPACK_PROJECT_CONFIG_FILE
  "${QDevelop_BINARY_DIR}/QDevelopCPackOptions.cmake")

# activate cpack
include (CPack )