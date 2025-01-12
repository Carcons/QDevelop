#pragma once
#include <QColor>



namespace qdev {

	// Source utils
	static const QColor SRC_WARNING_COLOR = QColor(255, 255, 102);
	static const QColor SRC_ERROR_COLOR = QColor(255, 51, 51);
	static const QColor SRC_NEXT_LINE_COLOR = QColor(255, 102, 102);

	// Project utils
	enum class ProjectType { EXE, STATIC, SHARED, MAKEFILE, CMAKE};


	// Icons path
	namespace icons {
		static const char* APP(":icons/app.png"); // ok
		static const char* APP_ICO(":icons/appicon.png"); // ok
		static const char* SAVE(":icons/save.png"); //ok
		static const char* SAVE_ALL(":icons/saveall.png"); //ok
		static const char* CLOSE(":icons/close.png"); //ok
		static const char* CLOSE_ALL(":icons/closeall.png"); //ok
		static const char* PRINTER(":icons/printer.png");
		static const char* CANCEL(":icons/cancel.png");
		static const char* UNDO(":icons/undo.png"); //ok
		static const char* REDO(":icons/redo.png"); //ok
		static const char* SETTINGS(":icons/settings.png");
		static const char* ADD_SOURCE(":icons/sourceadd.png"); // ok
		static const char* RENAME_SOURCE(":icons/sourcerename.png"); // ok
		static const char* REMOVE_SOURCE(":icons/sourceremove.png"); // ok
		static const char* BUILD(":icons/build.png"); // ok
		static const char* RUN(":icons/run.png"); // ok
		static const char* STOP(":icons/stop.png"); // ok
		static const char* ABOUT(":icons/about.png");
		static const char* EXE(":icons/exe.png"); // ok
		static const char* STATIC_LIB(":icons/static.png");
		static const char* SHARED_LIB(":icons/shared.png");
		static const char* DIR(":icons/dir.png"); // ok
		static const char* DIR_NEW(":icons/dirnew.png"); //ok
		static const char* DIR_OPEN(":icons/diropen.png"); // ok
		static const char* DIR_RENAME(":icons/dirrename.png"); //ok
		static const char* DIR_DELETE(":icons/dirdelete.png"); // ok
		static const char* CPP_FILE(":icons/cppfile.png"); //ok
		static const char* H_FILE(":icons/hfile.png"); //ok
		static const char* C_FILE(":icons/cfile.png"); //ok 
		static const char* MAKEFILE_FILE(":icons/makefile.png"); //ok
		static const char* MAKEFILE_PROJ(":icons/makefileproj.png"); // ok
		static const char* CMAKE_PROJ(":icons/cmakeproj.png");
		static const char* CMAKE_FILE(":icons/cmake.png"); //ok
		static const char* GENERIC_FILE(":icons/genericfile.png"); //ok
		static const char* DEBUGGER_START(":icons/debuggerstart.png"); // ok
		static const char* DEBUGGER_STOP(":icons/debuggerstop.png"); // ok
		static const char* DEBUGGER_CONTINUE(":icons/debuggercontinue.png"); // ok
		static const char* DEBUGGER_NEXT_LINE(":icons/debuggernextline.png"); // ok
		static const char* DEBUGGER_BUILD(":icons/builddebug.png"); // ok
		static const char* INFO_LOCAL(":icons/infolocal.png");
	}

	namespace runconstants {
		static const QString MINGW32_MAKE = "mingw32-make"; // Make for Windows
		static const QString MAKE = "make"; // Make for *unix* derived systems
		static const QString XTERM = "xterm"; // xterm as default terminal for linux based systems
		static const QString XTERM_EXEC = "-e"; // xterm "execute" argument
		static const QString XTERM_VERSION = "-v"; // xterm "version" argument
		static const QString MAC_TERMINAL = "Terminal"; // MacOS terminal app
		static const QString MAC_OPEN = "open"; // MacOS app launcher command
		static const QString MAC_OPEN_ARGS = "-a"; // MacOS open "arguments"
	}


	namespace debuggerconstants {
		static const QString DEBUGGER = "gdb"; // Start gdb
		static const QString BREAK_CMD = "break"; // Set new breakpoint
		static const QString FILE_CMD = "file"; // Specify executable
		static const QString DELETE_CMD = "delete"; // Clear all setted breakpoints
		static const QString NEW_CONSOLE_CMD = "set new-console on"; // Run executable on new window
		static const QString RUN_CMD = "run"; // Run gdb specified file
		static const QString CONTINUE_CMD = "continue"; // Continue program flow after a stop
		static const QString QUIT = "quit"; // Exit
		static const QString CONFIRM_OFF = "set confirm off"; // Confirm off
		static const QString STEP = "step"; // Go next line
		static const QString NORMAL_EXIT = "exited normally]"; // Program has finish
		static const QString WORKING_SRC_PATH = "set substitute-path"; // Change sources path
		static const QString VERSION = "-v"; // Print gdb version
		static const QString INFO_LOCALS = "info locals"; // Ask for local variables values
	}

	namespace buildconstants {
		static const QString CXX = "g++"; // GNU C/C++ compiler
		static const QString FPIC = "-fPIC"; // Position Independence for shared libraries
		static const QString COMPILE = "-c"; // Compile sources into objects
		static const QString OUTPUT = "-o"; // Ouput command
		static const QString SHARED = "-shared"; // Specify that g++ must collect objects to create a shared lib
		static const QString ARCHIVE = "ar"; // Archive Program, collect object files
		static const QString RVS = "rvs"; // Replace/Verbose/Symbol for Archive program
		static const QString INCLUDE = "-I"; // Include directive for g++
		static const QString PREPROCESSOR_DEFS = "-D"; // Enable/Disable preprocessor defs
		static const QString DEBUGGING = "-g"; // Insert debugging symbols
		static const QString MINGW32_MAKE = "mingw32-make"; // Make for Windows
		static const QString MAKE = "make"; // Make for *unix* derived systems
		static const QString CXX_VER = "-v"; // Ask for g++ CMake
		static const QString CMAKE = "cmake"; // CMake needed for CMake project import
		static const QString CMAKE_SOURCE = "-S"; // Source's path for CMake
		static const QString CMAKE_BUILD = "-B"; // Build directive for CMake
		static const QString CMAKE_DEBUG = "-D"; // Debug directive for CMake
		static const QString CMAKE_BUILD_DEBUG = "CMAKE_BUILD_TYPE=Debug"; // Debug when generating CMake Makefile
		static const QString CMAKE_VERSION = "--version"; // CMake version check
		enum class BuildMode { NORMAL, DEBUG }; // Debug or not when compiling
	}






}
