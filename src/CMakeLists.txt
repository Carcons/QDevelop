project( QDevelop )

# CMake 3.* necessario per Qt
cmake_minimum_required(VERSION 3.0.0 FATAL_ERROR)

# include cmake scripts
set(CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/../cmake)
include(Utils)
include(Setup)
include(Version)
include(InstallLocation)

# programmiamo in C++ 11
set (CMAKE_CXX_STANDARD 11)

# ottengo tutti i sorgenti
file(GLOB my_sources *.h *.cpp *.qrc *.ui)

# attivo funzionalita' specifiche di CMake per Qt
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

# set platform-specific bundle icons
if(APPLE)
  list(APPEND my_sources icons/app.icns)
  set_source_files_properties(icons/app.icns PROPERTIES
    MACOSX_PACKAGE_LOCATION Resources)
#elseif(UNIX)
  #install(FILES "icons/app.desktop" DESTINATION "${INSTALL_XDG_APPS_DIR}")
  #install(FILES "icons/app.png" DESTINATION "${INSTALL_XDG_ICON_DIR}")
elseif(WIN32)
  list(APPEND my_sources icons/app.rc)
endif()

# aggiungo eseguibile
add_executable(QDevelop ${my_sources})

# verifico l'esistenza delle librerie Qt necessarie
find_package(Qt5Widgets REQUIRED)
find_package(Qt5PrintSupport REQUIRED)

# collego le librerie Qt con l'eseguibile
target_link_libraries(QDevelop Qt5::Widgets Qt5::PrintSupport)

# set CPack
include(QDevelopCPack)

# installation
install(TARGETS QDevelop
  RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
  BUNDLE DESTINATION ${CMAKE_INSTALL_BINDIR}
  )

# make sure bundle is standalone and it can be drag-and-dropped
if(APPLE)
	set_target_properties(QDevelop PROPERTIES MACOSX_BUNDLE_INFO_PLIST ${CMAKE_CURRENT_SOURCE_DIR}/osinstall/Info.plist)
endif()
if((APPLE OR WIN32))
	set(pfx ${CMAKE_INSTALL_BINDIR}/)
	set(sfx "")
	if(APPLE)
	set(sfx ".app")
	elseif(WIN32)
	set(sfx ".exe")
	endif()

	get_target_property(output_name QDevelop NAME)
	set(exe "${pfx}${output_name}${sfx}")

	include(InstallRequiredSystemLibraries)
	include(DeployQt5)
	install_qt5_executable(${exe})
endif()

# DeployQt5 does not support Linux
# so we just copy the (symlink-resolved) Qt libs in the installation folder
if(UNIX AND NOT APPLE)
	FOREACH(qt_lib_file ${qt_lib_files})
		get_filename_component(qt_lib_real_file ${qt_lib_file} REALPATH)
		get_filename_component(qt_lib_fname "${qt_lib_file}" NAME)
		if(WITH_QT4)
			set (qt_lib_fname ${qt_lib_fname}.4)
		elseif(WITH_QT5)
			set (qt_lib_fname ${qt_lib_fname}.5)
		endif()
		install(FILES ${qt_lib_real_file} DESTINATION ${CMAKE_INSTALL_BINDIR} RENAME ${qt_lib_fname})
		message(STATUS "Qt lib " ${qt_lib_real_file} " will be installed to " ${CMAKE_INSTALL_BINDIR} " and renamed to " ${qt_lib_fname})
	ENDFOREACH()
	# also copy a launch script that sets LD_LIBRARY_PATH to local path
	install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/osinstall/start_qdevelop.sh DESTINATION ${CMAKE_INSTALL_BINDIR})
endif()
