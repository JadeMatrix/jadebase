################################################################################
# 
# Variables to configure:
# 
# CC 		C compiler with std flags (CC++ is C++ compiler)
# FFBUILD	Platform-specific, for example gcc40.mac.x64 for OS X
# 
################################################################################

# Making FastFormat assumes you have FASTFORMAT_ROOT and STLSOFT set as speci-
# fied in the FastFormat INSTALL.txt
ifndef STLSOFT
$(error STLSOFT must be defined)
endif
ifndef FASTFORMAT_ROOT
$(error FASTFORMAT_ROOT must be defined)
endif

# Compiler(s)
CC = clang
CPPC = ${CC}++ -std=c++11 -stdlib=libstdc++
OBJCC = ${CC}

# Directories
SOURCEDIR = src
RESOURCEDIR = resources
MAKEDIR = make
OBJDIR = ${MAKEDIR}/object
BUILDDIR = ${MAKEDIR}/build

# Fast format build version folder
# TODO: set using Autotools
# FFBUILD = gcc40.mac.x64
FFBUILD = gcc47.unix
FFOBJDIR = ${FASTFORMAT_ROOT}/build/${FFBUILD}

# Headers, links, flags, etc.
INCLUDE = -I${FASTFORMAT_ROOT}/include -I${STLSOFT}/include `pkg-config --cflags lua5.2 libpng gl glew pangocairo`
LINKS = -lm -lpthread `pkg-config --libs lua5.2 libpng gl glew pangocairo`
FRAMEWORKS = -framework Foundation -framework AppKit
DEFINES = -g -DDEBUG -DPLATFORM_XWS_GNUPOSIX
WARNS = -Wall -Wno-unused-local-typedef

PROJNAME = jadebase

INSTALL_LOC = /usr/local


# Utilities ####################################################################

clean:
	@rm -rf ${MAKEDIR}

# TODO: Consider using a perl script for nicer output
#| awk -F: '{ print $$1":"$$2":\n    "; for(i=3;i<NF;i++){printf " %s", $$i} printf "\n" }'
todo:
	@grep -nr --include \* --exclude-dir=make "\(TODO\|WARNING\|FIXME\):[ ]\+" .  # Using '[ ]' so the grep line is ignored by grep

linecount:
	@wc -l `find ./src -type f`


# FastFormat ###################################################################

# FastFormat is statically linked due to the non-standard build methods the
# project uses.  Until there is an official dynamic installation it should
# remain statically linked for ease of (precompiled binary) distribution.

# Single rule to build all FastFormat object files
${FFOBJDIR}/%:
	@echo Trying to automatically build FastFormat\; makefile may need manual editing to compile on some platforms
	cd ${FFOBJDIR}; make build.libs.core

# Not sure how many of these we need, so include all of them
FF_OBJECTS =	${FFOBJDIR}/core.api.o \
				${FFOBJDIR}/core.fmt_cache.o \
				${FFOBJDIR}/core.fmt_spec_defect_handlers.o \
				${FFOBJDIR}/core.init_code_strings.o \
				${FFOBJDIR}/core.mempool.o \
				${FFOBJDIR}/core.replacements.o \
				${FFOBJDIR}/core.snprintf.o


# Core jadebase header chains ##################################################

JADEBASE_PNG_HPP = src/filetypes/jb_png.hpp

JADEBASE_BUTTON_HPP = src/gui/jb_button.hpp ${JADEBASE_ELEMENT_HPP} ${JADEBASE_RESOURCE_HPP} ${JADEBASE_CALLBACK_HPP}
JADEBASE_DIAL_HPP = src/gui/jb_dial.hpp ${JADEBASE_ELEMENT_HPP} ${JADEBASE_CALLBACK_HPP}
JADEBASE_ELEMENT_HPP = src/gui/jb_element.hpp ${JADEBASE_MUTEX_HPP} ${JADEBASE_CALLBACK_HPP} ${JADEBASE_DPI_HPP} ${JADEBASE_PLATFORM_H} ${JADEBASE_WINDOWEVENT_HPP}
JADEBASE_GROUP_HPP = src/gui/jb_group.hpp ${JADEBASE_ELEMENT_HPP} ${JADEBASE_SCROLLABLE_HPP} ${JADEBASE_CALLBACK_HPP}
JADEBASE_IMAGE_RSRC_HPP = src/gui/jb_image_rsrc.hpp ${JADEBASE_RESOURCE_HPP} ${JADEBASE_TEXTURE_HPP}
JADEBASE_NAMED_RESOURCES = src/gui/jb_named_resources.hpp ${JADEBASE_RESOURCE_HPP} ${JADEBASE_RESOURCE_NAMES_HPP}
JADEBASE_RESOURCE_HPP = src/gui/jb_resource.hpp ${JADEBASE_DPI_HPP}
JADEBASE_RESOURCE_NAMES_HPP = src/gui/jb_resource_names.hpp
JADEBASE_SCROLLABLE_HPP = src/gui/jb_scrollable.hpp ${JADEBASE_ELEMENT_HPP}
JADEBASE_SCROLLSET_HPP = src/gui/jb_scrollset.hpp ${JADEBASE_ELEMENT_HPP} ${JADEBASE_SCROLLABLE_HPP}
JADEBASE_TABSET_HPP = src/gui/jb_tabset.hpp ${JADEBASE_ELEMENT_HPP} ${JADEBASE_GROUP_HPP} ${JADEBASE_TEXT_RSRC_HPP}
JADEBASE_TEXT_RSRC_HPP = src/gui/jb_text_rsrc.hpp ${JADEBASE_RESOURCE_HPP} ${JADEBASE_MUTEX_HPP}
JADEBASE_TEXTURE_HPP = src/gui/jb_texture.hpp ${JADEBASE_GL_HPP}
JADEBASE_WINDOWVIEW_HPP = src/gui/jb_windowview.hpp ${JADEBASE_GROUP_HPP}

JADEBASE_LUA_HPP = src/scripting/jb_lua.hpp ${JADEBASE_MUTEX_HPP}
JADEBASE_LUAAPI_HPP = src/scripting/jb_luaapi.hpp ${JADEBASE_LUA_HPP} ${JADEBASE_ELEMENT_HPP} ${JADEBASE_GROUP_HPP} ${JADEBASE_TASK_HPP}

JADEBASE_TASK_HPP = src/tasking/jb_task.hpp ${JADEBASE_TASKUTIL_HPP}
JADEBASE_TASKEXEC_HPP = src/tasking/jb_taskexec.hpp ${JADEBASE_TASKUTIL_HPP} ${JADEBASE_TASK_HPP} ${JADEBASE_THREADUTIL_HPP}
JADEBASE_TASKQUEUE_HPP = src/tasking/jb_taskqueue.hpp ${JADEBASE_TASK_HPP} ${JADEBASE_TASKUTIL_HPP} ${JADEBASE_CONDITION_HPP} ${JADEBASE_MUTEX_HPP}
JADEBASE_TASKUTIL_HPP = src/tasking/jb_taskutil.hpp

JADEBASE_CONDITION_HPP = src/threading/jb_condition.hpp ${JADEBASE_MUTEX_HPP} ${JADEBASE_THREADUTIL_HPP} ${JADEBASE_PLATFORM_H}
JADEBASE_MUTEX_HPP = src/threading/jb_mutex.hpp ${JADEBASE_SCOPEDLOCK_HPP} ${JADEBASE_THREADUTIL_HPP} ${JADEBASE_PLATFORM_H}
JADEBASE_SCOPEDLOCK_HPP = src/threading/jb_scopedlock.hpp
JADEBASE_SEMAPHORE_HPP = src/threading/jb_semaphore.hpp ${JADEBASE_CONDITION_HPP} ${JADEBASE_MUTEX_HPP} ${JADEBASE_SCOPEDLOCK_HPP} ${JADEBASE_THREADUTIL_HPP}
JADEBASE_THREAD_HPP = src/threading/jb_thread.hpp ${JADEBASE_THREADUTIL_HPP}
JADEBASE_THREADUTIL_HPP = src/threading/jb_threadutil.hpp ${JADEBASE_PLATFORM_H}

JADEBASE_CALLBACK_HPP = src/utility/jb_callback.hpp
JADEBASE_CONTAINER_HPP = src/utility/jb_container.hpp ${JADEBASE_MUTEX_HPP} ${JADEBASE_SCOPEDLOCK_HPP} ${JADEBASE_EXCEPTION_HPP}
JADEBASE_DPI_HPP = src/utility/jb_dpi.hpp
JADEBASE_EXCEPTION_HPP = src/utility/jb_exception.hpp
JADEBASE_GL_HPP = src/utility/jb_gl.hpp ${JADEBASE_PLATFORM_H}
JADEBASE_LAUNCHARGS_HPP = src/utility/jb_launchargs.hpp
JADEBASE_LOG_HPP = src/utility/jb_log.hpp
JADEBASE_PLATFORM_H = src/utility/jb_platform.h
JADEBASE_SETTINGS_HPP = src/utility/jb_settings.hpp
JADEBASE_TIMESTAMP_HPP = src/utility/jb_timestamp.hpp
JADEBASE_TRACKABLE_HPP = src/utility/jb_trackable.hpp ${JADEBASE_TIMESTAMP_HPP}
JADEBASE_VERSION_HPP = src/utility/jb_version.hpp

JADEBASE_EVENTS_HPP = src/windowsys/jb_events.hpp ${JADEBASE_TASK_HPP} ${JADEBASE_LOG_HPP}
JADEBASE_KEYCODE_HPP = src/windowsys/jb_keycode.hpp ${JADEBASE_PLATFORM_H}
JADEBASE_WINDOW_HPP =	src/windowsys/jb_window.hpp ${JADEBASE_WINDOWEVENT_HPP} ${JADEBASE_WINDOWVIEW_HPP} ${JADEBASE_TASK_HPP} ${JADEBASE_MUTEX_HPP} ${JADEBASE_CONTAINER_HPP} ${JADEBASE_PLATFORM_H} ${JADEBASE_VERSION_HPP} \
						${X_INPUTDEVICES_HPP}
						# ^^^ TODO: Make that platform-agnostic or the include platform-specific
JADEBASE_WINDOWEVENT_HPP = src/windowsys/jb_windowevent.hpp ${JADEBASE_KEYCODE_HPP} ${JADEBASE_PLATFORM_H}
JADEBASE_WINDOWMANAGEMENT_HPP = src/windowsys/jb_windowmanagement.hpp ${JADEBASE_WINDOW_HPP} ${JADEBASE_PLATFORM_H}
X_INPUTDEVICES_HPP = src/windowsys/x_inputdevices.hpp ${JADEBASE_PLATFORM_H}


# Core jadebase objects ########################################################

FILETYPES_OBJECTS =	${OBJDIR}/filetypes.jb_png.o

GUI_OBJECTS =	${OBJDIR}/gui.jb_button.o \
				${OBJDIR}/gui.jb_dial.o \
				${OBJDIR}/gui.jb_element.o \
				${OBJDIR}/gui.jb_group.o \
				${OBJDIR}/gui.jb_image_rsrc.o \
				${OBJDIR}/gui.jb_resource.o \
				${OBJDIR}/gui.jb_scrollset.o \
				${OBJDIR}/gui.jb_tabset.o \
				${OBJDIR}/gui.jb_text_rsrc.o \
				${OBJDIR}/gui.jb_named_resources.o \
				${OBJDIR}/gui.jb_windowview.o

SCRIPTING_OBJECTS =	${OBJDIR}/scripting.jb_lua.o \
					${OBJDIR}/scripting.jb_lua_initapi.o \
					${OBJDIR}/scripting.jb_luaapi.o \
					${OBJDIR}/scripting.jb_luaapi_filetypes.o \
					${OBJDIR}/scripting.jb_luaapi_gui_imgrsrc.o \
					${OBJDIR}/scripting.jb_luaapi_gui_textrsrc.o \
					${OBJDIR}/scripting.jb_luaapi_gui_button.o \
					${OBJDIR}/scripting.jb_luaapi_gui_dial.o \
					${OBJDIR}/scripting.jb_luaapi_gui_group.o \
					${OBJDIR}/scripting.jb_luaapi_gui_scrollset.o \
					${OBJDIR}/scripting.jb_luaapi_gui_tabset.o \
					${OBJDIR}/scripting.jb_luaapi_tasking.o \
					${OBJDIR}/scripting.jb_luaapi_utility.o \
					${OBJDIR}/scripting.jb_luaapi_windowsys.o

TASKING_OBJECTS =	${OBJDIR}/tasking.jb_taskexec.o \
					${OBJDIR}/tasking.jb_taskqueue.o

THREADING_OBJECTS =	${OBJDIR}/threading.jb_condition.o \
					${OBJDIR}/threading.jb_mutex.o \
					${OBJDIR}/threading.jb_semaphore.o \
					${OBJDIR}/threading.jb_threadutil.o \
					${OBJDIR}/threading.jb_thread.o

UTILITY_OBJECTS =	${OBJDIR}/utility.jb_exception.o \
					${OBJDIR}/utility.jb_gl.o \
					${OBJDIR}/utility.jb_launchargs.o \
					${OBJDIR}/utility.jb_platform.c.o \
					${OBJDIR}/utility.jb_settings.o \
					${OBJDIR}/utility.jb_timestamp.o \
					${OBJDIR}/utility.jb_trackable.o \
					${OBJDIR}/utility.jb_version.o

WINDOWSYS_OBJECTS =	${OBJDIR}/windowsys.jb_events.o \
					${OBJDIR}/windowsys.jb_keycode.o \
					${OBJDIR}/windowsys.jb_window.o \
					${OBJDIR}/windowsys.jb_windowevent.o \
					${OBJDIR}/windowsys.jb_windowmanagement.o

CORE_OBJECTS =	${FILETYPES_OBJECTS} \
				${GUI_OBJECTS} \
				${SCRIPTING_OBJECTS} \
				${TASKING_OBJECTS} \
				${THREADING_OBJECTS} \
				${UTILITY_OBJECTS} \
				${WINDOWSYS_OBJECTS} \
				${OBJDIR}/main.jb_main.o

${OBJDIR}/jb_%.o: ${SOURCEDIR}/jb_%.cpp
	@mkdir -p ${OBJDIR}
	${CPPC} ${DEFINES} -Wall -Wno-unused-local-typedef -fPIC -c ${INCLUDE} $^ -o ${OBJDIR}/jb_$*.o

${OBJDIR}/filetypes.jb_%.o: ${SOURCEDIR}/filetypes/jb_%.cpp
	@mkdir -p ${OBJDIR}
	${CPPC} ${DEFINES} -Wall -Wno-unused-local-typedef -fPIC -c ${INCLUDE} $^ -o ${OBJDIR}/filetypes.jb_$*.o

${OBJDIR}/gui.jb_%.o: ${SOURCEDIR}/gui/jb_%.cpp
	@mkdir -p ${OBJDIR}
	${CPPC} ${DEFINES} -Wall -Wno-unused-local-typedef -fPIC -c ${INCLUDE} $^ -o ${OBJDIR}/gui.jb_$*.o

${OBJDIR}/main.jb_%.o: ${SOURCEDIR}/main/jb_%.cpp
	@mkdir -p ${OBJDIR}
	${CPPC} ${DEFINES} -Wall -Wno-unused-local-typedef -fPIC -c ${INCLUDE} $^ -o ${OBJDIR}/main.jb_$*.o

${OBJDIR}/scripting.jb_%.o: ${SOURCEDIR}/scripting/jb_%.cpp
	@mkdir -p ${OBJDIR}
	${CPPC} ${DEFINES} -Wall -Wno-unused-local-typedef -fPIC -c ${INCLUDE} $^ -o ${OBJDIR}/scripting.jb_$*.o

${OBJDIR}/tasking.jb_%.o: ${SOURCEDIR}/tasking/jb_%.cpp
	@mkdir -p ${OBJDIR}
	${CPPC} ${DEFINES} -Wall -Wno-unused-local-typedef -fPIC -c ${INCLUDE} $^ -o ${OBJDIR}/tasking.jb_$*.o

${OBJDIR}/threading.jb_%.o: ${SOURCEDIR}/threading/jb_%.cpp
	@mkdir -p ${OBJDIR}
	${CPPC} ${DEFINES} -Wall -Wno-unused-local-typedef -fPIC -c ${INCLUDE} $^ -o ${OBJDIR}/threading.jb_$*.o

${OBJDIR}/utility.jb_%.o: ${SOURCEDIR}/utility/jb_%.cpp
	@mkdir -p ${OBJDIR}
	${CPPC} ${DEFINES} -Wall -Wno-unused-local-typedef -fPIC -c ${INCLUDE} $^ -o ${OBJDIR}/utility.jb_$*.o

${OBJDIR}/utility.jb_%.c.o: ${SOURCEDIR}/utility/jb_%.c
	@mkdir -p ${OBJDIR}
	${CC} ${DEFINES} -Wall -Wno-unused-local-typedef -fPIC -c ${INCLUDE} $^ -o ${OBJDIR}/utility.jb_$*.c.o

${OBJDIR}/windowsys.jb_%.o: ${SOURCEDIR}/windowsys/jb_%.cpp
	@mkdir -p ${OBJDIR}
	${CPPC} ${DEFINES} -Wall -Wno-unused-local-typedef -fPIC -c ${INCLUDE} $^ -o ${OBJDIR}/windowsys.jb_$*.o


# Linux ########################################################################

X_INPUTDEVICES_HPP = ${SOURCEDIR}/windowsys/x_inputdevices.hpp ${JADEBASE_PLATFORM_H}

LINUX_OBJECTS = ${OBJDIR}/main.x_main.o \
				${OBJDIR}/windowsys.x_inputdevices.o

${OBJDIR}/main.x_main.o: ${SOURCEDIR}/main/x_main.cpp ${JADEBASE_MAIN_H} ${JADEBASE_LAUNCHARGS_HPP} ${JADEBASE_LOG_HPP} ${JADEBASE_PLATFORM_H}
	@mkdir -p ${OBJDIR}
	${CPPC} ${DEFINES} -Wall -Wno-unused-local-typedef -fPIC -c ${INCLUDE} $^ -o ${OBJDIR}/main.x_main.o

${OBJDIR}/windowsys.x_inputdevices.o: ${SOURCEDIR}/windowsys/x_inputdevices.cpp ${X_INPUTDEVICES_HPP} ${JADEBASE_WINDOWMANAGEMENT_HPP} ${JADEBASE_WINDOWEVENT_HPP} ${JADEBASE_MUTEX_HPP} ${JADEBASE_EXCEPTION_HPP} ${JADEBASE_LAUNCHARGS_HPP} ${JADEBASE_LOG_HPP} ${JADEBASE_SETTINGS_HPP}
	@mkdir -p ${OBJDIR}
	${CPPC} ${DEFINES} -Wall -Wno-unused-local-typedef -fPIC -c ${INCLUDE} $^ -o ${OBJDIR}/windowsys.x_inputdevices.o

${BUILDDIR}/lib${PROJNAME}-${CC}.so.0.1: ${CORE_OBJECTS} ${LINUX_OBJECTS} ${FF_OBJECTS}
	@mkdir -p ${BUILDDIR}
	${CPPC} -shared -Wl,-soname,lib${PROJNAME}-${CC}.so.0 -o "${BUILDDIR}/lib${PROJNAME}-${CC}.so.0.1" ${LINKS} -lX11 -lXext -lXi $^

linux_install: ${BUILDDIR}/lib${PROJNAME}-${CC}.so.0.1
	@sudo mkdir -p ${INSTALL_LOC}/lib
	sudo cp "${BUILDDIR}/lib${PROJNAME}-${CC}.so.0.1" ${INSTALL_LOC}/lib/
	sudo ln -f "${INSTALL_LOC}/lib/lib${PROJNAME}-${CC}.so.0.1" "${INSTALL_LOC}/lib/lib${PROJNAME}-${CC}.so.0"
	sudo ln -f "${INSTALL_LOC}/lib/lib${PROJNAME}-${CC}.so.0" "${INSTALL_LOC}/lib/lib${PROJNAME}-${CC}.so"
	sudo mkdir -p ${INSTALL_LOC}/include/${PROJNAME}
	cd ${SOURCEDIR}; sudo find -type f -name "*.h*" -exec cp --parents {} ${INSTALL_LOC}/include/${PROJNAME}/ \;

linux_uninstall:
	sudo rm -f "${INSTALL_LOC}/lib/lib${PROJNAME}-${CC}.so.0.1"
	sudo rm -f "${INSTALL_LOC}/lib/lib${PROJNAME}-${CC}.so.0"
	sudo rm -f "${INSTALL_LOC}/lib/lib${PROJNAME}-${CC}.so"
	sudo rm -rf "${INSTALL_LOC}/include/jadebase"

# linux_profile: linux
# 	valgrind --tool=callgrind "${MAKEDIR}/${PROJNAME}/Linux/${PROJNAME}" -d


# OS X #########################################################################

# TOREDO:

COCOA_APPDELEGATE_H = ${SOURCEDIR}/main/cocoa_appdelegate.h

OSX_OBJECTS =	${OBJDIR}/main.cocoa_appdelegate.o \
				${OBJDIR}/main.cocoa_main.o

${OBJDIR}/main.cocoa_appdelegate.o: ${SOURCEDIR}/main/cocoa_appdelegate.m ${COCOA_APPDELEGATE_H} ${JADEBASE_MAIN_H}
	@mkdir -p ${OBJDIR}
	${OBJCC} ${DEFINES} -Wall -Wno-unused-local-typedef -fPIC -c ${INCLUDE} $^ -o ${OBJDIR}/main.cocoa_appdelegate.o

${OBJDIR}/main.cocoa_main.o: ${SOURCEDIR}/main/cocoa_main.m ${COCOA_APPDELEGATE_H} ${JADEBASE_MAIN_H}
	@mkdir -p ${OBJDIR}
	${OBJCC} ${DEFINES} -Wall -Wno-unused-local-typedef -fPIC -c ${INCLUDE} $^ -o ${OBJDIR}/main.cocoa_main.o

# Bleh...
# osx_build: ${CORE_OBJECTS} ${OSX_OBJECTS} ${FF_OBJECTS}
# 	mkdir -p ${BUILDDIR}
# 	${CPPC} -o "${BUILDDIR}/${PROJNAME}" ${FRAMEWORKS} ${LINKS} -lobjc $^

osx_install:
	@echo "No working OS X build yet"

osx_uninstall:
	@echo "No working OS X build yet"


# Windows ######################################################################

# TODO:


# Test program(s) ##############################################################

test:
	@mkdir -p ${OBJDIR}
	${CPPC} ${DEFINES} ${WARNS} -c ${INCLUDE} "${SOURCEDIR}/jb_test.cpp" -o "${OBJDIR}/jb_test.o"
	@mkdir -p ${BUILDDIR}
	${CPPC} -o "${BUILDDIR}/jb_test" "${OBJDIR}/jb_test.o" -l${PROJNAME}-${CC} ${LINKS}

# PHONY ########################################################################

.PHONY:	clean \
		fastformat \
		linecount \
		linux_install \
		linux_uninstall \
		osx_install \
		osx_uninstall \
		todo \
		windows


