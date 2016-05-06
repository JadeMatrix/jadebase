################################################################################
# 
# Makefile designed against GNU Make 3.81
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

# TODO: Configurable
PLATFORM = MACOSX

ifeq (${PLATFORM},XWS_GNUPOSIX)
	# Compilers
	CC = gcc
	CPPC = ${CC}++ -std=c++11 -stdlib=libstdc++
	
	# Headers, links, flags, etc.
	INCLUDE = -I${FASTFORMAT_ROOT}/include -I${STLSOFT}/include `pkg-config --cflags lua5.2 libpng gl glew pangocairo`
	LINUX_LINKS = -lm -lpthread `pkg-config --libs lua5.2 libpng gl glew pango`
	
	# FastFormat version
	FFBUILD = gcc47.unix
else
	ifeq (${PLATFORM},MACOSX)
		# Compilers
		CC = clang
		CPPC = ${CC}++ -std=gnu++11 -stdlib=libc++
		OBJCC = ${CC}++ -Wno-c++11-extensions
		
		# Headers, links, flags, etc.
		INCLUDE = -I${FASTFORMAT_ROOT}/include -I${STLSOFT}/include `pkg-config --cflags lua5.3 libpng glew pangocairo`
		COCOA_LINKS = -lm -lpthread `pkg-config --libs lua5.3 libpng glew` -framework Foundation -framework AppKit -framework OpenGL
		
		# FastFormat version
		FFBUILD = gcc40.mac
	else
		$(error Platform ${PLATFORM} not currently supported)
	endif
endif

# Directories
SOURCEDIR = src
RESOURCEDIR = resources
MAKEDIR = make
OBJDIR = ${MAKEDIR}/object
BUILDDIR = ${MAKEDIR}/build

# FastFormat build version folder
FFOBJDIR = ${FASTFORMAT_ROOT}/build/${FFBUILD}

DEFINES = -g -DDEBUG -DPLATFORM_${PLATFORM}
WARNS = -Wall -Wno-unused-local-typedef

PROJNAME = jadebase

INSTALL_LOC = /usr/local


# Utilities ####################################################################

clean:
	@rm -rf ${MAKEDIR}

# Using '[ ]' so the grep line is ignored by grep
todo:
	@grep -nr --include \* --exclude-dir=make "\(TODO\|WARNING\|FIXME\):[ ]\+" .

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

JADEBASE_BASIC_TENSOR_ENGINE_HPP = src/dynamics/jb_basic_tensor_engine.hpp ${JADEBASE_TENSOR_HPP} ${JADEBASE_MUTEX_HPP} ${JADEBASE_LOG_HPP}
JADEBASE_GUI_DYNAMICS_HPP = src/dynamics/jb_gui_dynamics.hpp ${JADEBASE_TENSOR_HPP} ${JADEBASE_DPI_HPP}
JADEBASE_TENSOR_HPP = src/dynamics/jb_tensor.hpp ${JADEBASE_MUTEX_HPP}

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

JADEBASE_MAIN_H = src/main/jb_main.h

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
JADEBASE_WINDOW_HPP =	src/windowsys/jb_window.hpp ${JADEBASE_WINDOWEVENT_HPP} ${JADEBASE_GUI_DYNAMICS_HPP} ${JADEBASE_WINDOWVIEW_HPP} ${JADEBASE_TASK_HPP} ${JADEBASE_MUTEX_HPP} ${JADEBASE_CONTAINER_HPP} ${JADEBASE_PLATFORM_H} ${JADEBASE_VERSION_HPP} \
						${X_INPUTDEVICES_HPP}
						# ^^^ TODO: Make that platform-agnostic or the include platform-specific
JADEBASE_WINDOWEVENT_HPP = src/windowsys/jb_windowevent.hpp ${JADEBASE_KEYCODE_HPP} ${JADEBASE_DPI_HPP} ${JADEBASE_PLATFORM_H}
JADEBASE_WINDOWMANAGEMENT_HPP = src/windowsys/jb_windowmanagement.hpp ${JADEBASE_WINDOW_HPP} ${JADEBASE_PLATFORM_H}
X_INPUTDEVICES_HPP = src/windowsys/x_inputdevices.hpp ${JADEBASE_PLATFORM_H}

JADEBASE_HPP =	${JADEBASE_TENSOR_HPP} \
				\
				${JADEBASE_PNG_HPP} \
				\
				${JADEBASE_BUTTON_HPP} \
				${JADEBASE_DIAL_HPP} \
				${JADEBASE_ELEMENT_HPP} \
				${JADEBASE_GROUP_HPP} \
				${JADEBASE_IMAGE_RSRC_HPP} \
				${JADEBASE_NAMED_RESOURCES} \
				${JADEBASE_RESOURCE_HPP} \
				${JADEBASE_RESOURCE_NAMES_HPP} \
				${JADEBASE_SCROLLABLE_HPP} \
				${JADEBASE_SCROLLSET_HPP} \
				${JADEBASE_TABSET_HPP} \
				${JADEBASE_TEXT_RSRC_HPP} \
				${JADEBASE_TEXTURE_HPP} \
				\
				${JADEBASE_MAIN_H} \
				\
				${JADEBASE_LUA_HPP} \
				${JADEBASE_LUAAPI_HPP} \
				\
				${JADEBASE_TASK_HPP} \
				${JADEBASE_TASKEXEC_HPP} \
				${JADEBASE_TASKUTIL_HPP} \
				\
				${JADEBASE_CONDITION_HPP} \
				${JADEBASE_MUTEX_HPP} \
				${JADEBASE_SCOPEDLOCK_HPP} \
				${JADEBASE_SEMAPHORE_HPP} \
				${JADEBASE_THREAD_HPP} \
				${JADEBASE_THREADUTIL_HPP} \
				\
				${JADEBASE_CALLBACK_HPP} \
				${JADEBASE_CONTAINER_HPP} \
				${JADEBASE_EXCEPTION_HPP} \
				${JADEBASE_GL_HPP} \
				${JADEBASE_LAUNCHARGS_HPP} \
				${JADEBASE_LOG_HPP} \
				${JADEBASE_PLATFORM_H} \
				${JADEBASE_SETTINGS_HPP} \
				${JADEBASE_TIMESTAMP_HPP} \
				${JADEBASE_EXCEPTION_HPP} \
				${JADEBASE_VERSION_HPP} \
				\
				${JADEBASE_EVENTS_HPP} \
				${JADEBASE_KEYCODE_HPP} \
				${JADEBASE_WINDOW_HPP} \
				${JADEBASE_WINDOWEVENT_HPP} \
				${JADEBASE_WINDOWMANAGEMENT_HPP}


# Core jadebase objects ########################################################

DYNAMICS_OBJECTS = ${OBJDIR}/jb_gui_dynamics.o

FILETYPES_OBJECTS =	${OBJDIR}/jb_png.o

GUI_OBJECTS =	${OBJDIR}/jb_button.o \
				${OBJDIR}/jb_dial.o \
				${OBJDIR}/jb_element.o \
				${OBJDIR}/jb_group.o \
				${OBJDIR}/jb_image_rsrc.o \
				${OBJDIR}/jb_named_resources.o \
				${OBJDIR}/jb_resource.o \
				${OBJDIR}/jb_scrollset.o \
				${OBJDIR}/jb_tabset.o \
				${OBJDIR}/jb_text_rsrc.o \
				${OBJDIR}/jb_windowview.o

SCRIPTING_OBJECTS =	${OBJDIR}/jb_lua.o \
					${OBJDIR}/jb_lua_initapi.o \
					${OBJDIR}/jb_luaapi.o \
					${OBJDIR}/jb_luaapi_filetypes.o \
					${OBJDIR}/jb_luaapi_gui_imgrsrc.o \
					${OBJDIR}/jb_luaapi_gui_textrsrc.o \
					${OBJDIR}/jb_luaapi_gui_button.o \
					${OBJDIR}/jb_luaapi_gui_dial.o \
					${OBJDIR}/jb_luaapi_gui_group.o \
					${OBJDIR}/jb_luaapi_gui_scrollset.o \
					${OBJDIR}/jb_luaapi_gui_tabset.o \
					${OBJDIR}/jb_luaapi_tasking.o \
					${OBJDIR}/jb_luaapi_utility.o \
					${OBJDIR}/jb_luaapi_windowsys.o

TASKING_OBJECTS =	${OBJDIR}/jb_taskexec.o \
					${OBJDIR}/jb_taskqueue.o

THREADING_OBJECTS =	${OBJDIR}/jb_condition.o \
					${OBJDIR}/jb_mutex.o \
					${OBJDIR}/jb_semaphore.o \
					${OBJDIR}/jb_thread.o \
					${OBJDIR}/jb_threadutil.o

UTILITY_OBJECTS =	${OBJDIR}/jb_exception.o \
					${OBJDIR}/jb_gl.o \
					${OBJDIR}/jb_launchargs.o \
					${OBJDIR}/jb_platform.o \
					${OBJDIR}/jb_settings.o \
					${OBJDIR}/jb_timestamp.o \
					${OBJDIR}/jb_trackable.o \
					${OBJDIR}/jb_version.o

WINDOWSYS_OBJECTS =	${OBJDIR}/jb_events.o \
					${OBJDIR}/jb_keycode.o \
					${OBJDIR}/jb_window.o \
					${OBJDIR}/jb_windowevent.o \
					${OBJDIR}/jb_windowmanagement.o

CORE_OBJECTS =	${DYNAMICS_OBJECTS} \
				${FILETYPES_OBJECTS} \
				${GUI_OBJECTS} \
				${SCRIPTING_OBJECTS} \
				${TASKING_OBJECTS} \
				${THREADING_OBJECTS} \
				${UTILITY_OBJECTS} \
				${WINDOWSYS_OBJECTS} \
				${OBJDIR}/jb_main.o


# Core jadebase recipes ########################################################

${OBJDIR}/jb_gui_dynamics.o: ${SOURCEDIR}/dynamics/jb_gui_dynamics.cpp ${JADEBASE_GUI_DYNAMICS_HPP} ${JADEBASE_EXCEPTION_HPP} ${JADEBASE_LOG_HPP}
	@mkdir -p ${OBJDIR}
	${CPPC} -c ${DEFINES} ${WARNS} -fPIC ${INCLUDE} ${SOURCEDIR}/dynamics/jb_gui_dynamics.cpp -o ${OBJDIR}/jb_gui_dynamics.o

${OBJDIR}/jb_png.o: ${SOURCEDIR}/filetypes/jb_png.cpp ${JADEBASE_PNG_HPP} ${JADEBASE_EXCEPTION_HPP} ${JADEBASE_LOG_HPP}
	@mkdir -p ${OBJDIR}
	${CPPC} -c ${DEFINES} ${WARNS} -fPIC ${INCLUDE} ${SOURCEDIR}/filetypes/jb_png.cpp -o ${OBJDIR}/jb_png.o

${OBJDIR}/jb_button.o: ${SOURCEDIR}/gui/jb_button.cpp ${JADEBASE_NAMED_RESOURCES} ${JADEBASE_RESOURCE_HPP} ${JADEBASE_EXCEPTION_HPP} ${JADEBASE_LOG_HPP} ${JADEBASE_MUTEX_HPP} ${JADEBASE_WINDOW_HPP}
	@mkdir -p ${OBJDIR}
	${CPPC} -c ${DEFINES} ${WARNS} -fPIC ${INCLUDE} ${SOURCEDIR}/gui/jb_button.cpp -o ${OBJDIR}/jb_button.o
${OBJDIR}/jb_dial.o: ${SOURCEDIR}/gui/jb_dial.cpp ${JADEBASE_DIAL_HPP} ${JADEBASE_NAMED_RESOURCES} ${JADEBASE_RESOURCE_HPP} ${JADEBASE_EXCEPTION_HPP} ${JADEBASE_SETTINGS_HPP} ${JADEBASE_WINDOW_HPP}
	@mkdir -p ${OBJDIR}
	${CPPC} -c ${DEFINES} ${WARNS} -fPIC ${INCLUDE} ${SOURCEDIR}/gui/jb_dial.cpp -o ${OBJDIR}/jb_dial.o
${OBJDIR}/jb_element.o: ${SOURCEDIR}/gui/jb_element.cpp ${JADEBASE_ELEMENT_HPP} ${JADEBASE_WINDOW_HPP}
	@mkdir -p ${OBJDIR}
	${CPPC} -c ${DEFINES} ${WARNS} -fPIC ${INCLUDE} ${SOURCEDIR}/gui/jb_element.cpp -o ${OBJDIR}/jb_element.o
${OBJDIR}/jb_group.o: ${SOURCEDIR}/gui/jb_group.cpp ${JADEBASE_GROUP_HPP} ${JADEBASE_EXCEPTION_HPP} ${JADEBASE_GL_HPP} ${JADEBASE_SETTINGS_HPP} ${JADEBASE_WINDOW_HPP}
	@mkdir -p ${OBJDIR}
	${CPPC} -c ${DEFINES} ${WARNS} -fPIC ${INCLUDE} ${SOURCEDIR}/gui/jb_group.cpp -o ${OBJDIR}/jb_group.o
${OBJDIR}/jb_image_rsrc.o: ${SOURCEDIR}/gui/jb_image_rsrc.cpp ${JADEBASE_IMAGE_RSRC_HPP} ${JADEBASE_NAMED_RESOURCES} ${JADEBASE_LAUNCHARGS_HPP}
	@mkdir -p ${OBJDIR}
	${CPPC} -c ${DEFINES} ${WARNS} -fPIC ${INCLUDE} ${SOURCEDIR}/gui/jb_image_rsrc.cpp -o ${OBJDIR}/jb_image_rsrc.o
${OBJDIR}/jb_named_resources.o: ${SOURCEDIR}/gui/jb_named_resources.cpp ${JADEBASE_NAMED_RESOURCES} ${JADEBASE_IMAGE_RSRC_HPP} ${JADEBASE_TEXTURE_HPP} ${JADEBASE_PNG_HPP} ${JADEBASE_TASK_HPP} ${JADEBASE_TASKEXEC_HPP} ${JADEBASE_MUTEX_HPP} ${JADEBASE_EXCEPTION_HPP} ${JADEBASE_GL_HPP} ${JADEBASE_LAUNCHARGS_HPP} ${JADEBASE_LOG_HPP} ${JADEBASE_WINDOWMANAGEMENT_HPP}
	@mkdir -p ${OBJDIR}
	${CPPC} -c ${DEFINES} ${WARNS} -fPIC ${INCLUDE} ${SOURCEDIR}/gui/jb_named_resources.cpp -o ${OBJDIR}/jb_named_resources.o
${OBJDIR}/jb_resource.o: ${SOURCEDIR}/gui/jb_resource.cpp ${JADEBASE_RESOURCE_HPP}
	@mkdir -p ${OBJDIR}
	${CPPC} -c ${DEFINES} ${WARNS} -fPIC ${INCLUDE} ${SOURCEDIR}/gui/jb_resource.cpp -o ${OBJDIR}/jb_resource.o
${OBJDIR}/jb_scrollset.o: ${SOURCEDIR}/gui/jb_scrollset.cpp ${JADEBASE_SCROLLSET_HPP} ${JADEBASE_NAMED_RESOURCES} ${JADEBASE_GROUP_HPP} ${JADEBASE_RESOURCE_HPP} ${JADEBASE_EXCEPTION_HPP} ${JADEBASE_SETTINGS_HPP} ${JADEBASE_WINDOW_HPP} ${JADEBASE_LOG_HPP}
	@mkdir -p ${OBJDIR}
	${CPPC} -c ${DEFINES} ${WARNS} -fPIC ${INCLUDE} ${SOURCEDIR}/gui/jb_scrollset.cpp -o ${OBJDIR}/jb_scrollset.o
${OBJDIR}/jb_tabset.o: ${SOURCEDIR}/gui/jb_tabset.cpp ${JADEBASE_TABSET_HPP} ${JADEBASE_NAMED_RESOURCES} ${JADEBASE_RESOURCE_HPP} ${JADEBASE_EXCEPTION_HPP} ${JADEBASE_GL_HPP} ${JADEBASE_LOG_HPP} ${JADEBASE_SETTINGS_HPP} ${JADEBASE_WINDOW_HPP}
	@mkdir -p ${OBJDIR}
	${CPPC} -c ${DEFINES} ${WARNS} -fPIC ${INCLUDE} ${SOURCEDIR}/gui/jb_tabset.cpp -o ${OBJDIR}/jb_tabset.o
${OBJDIR}/jb_text_rsrc.o: ${SOURCEDIR}/gui/jb_text_rsrc.cpp ${JADEBASE_TEXT_RSRC_HPP} ${JADEBASE_EXCEPTION_HPP} ${JADEBASE_GL_HPP} ${JADEBASE_LOG_HPP}
	@mkdir -p ${OBJDIR}
	${CPPC} -c ${DEFINES} ${WARNS} -fPIC ${INCLUDE} ${SOURCEDIR}/gui/jb_text_rsrc.cpp -o ${OBJDIR}/jb_text_rsrc.o
${OBJDIR}/jb_windowview.o: ${SOURCEDIR}/gui/jb_windowview.cpp ${JADEBASE_WINDOWVIEW_HPP} ${JADEBASE_EXCEPTION_HPP} ${JADEBASE_LAUNCHARGS_HPP} ${JADEBASE_LOG_HPP} ${JADEBASE_WINDOW_HPP}
	@mkdir -p ${OBJDIR}
	${CPPC} -c ${DEFINES} ${WARNS} -fPIC ${INCLUDE} ${SOURCEDIR}/gui/jb_windowview.cpp -o ${OBJDIR}/jb_windowview.o

${OBJDIR}/jb_lua.o: ${SOURCEDIR}/scripting/jb_lua.cpp ${JADEBASE_LUA_HPP} ${JADEBASE_EXCEPTION_HPP} ${JADEBASE_LOG_HPP}
	@mkdir -p ${OBJDIR}
	${CPPC} -c ${DEFINES} ${WARNS} -fPIC ${INCLUDE} ${SOURCEDIR}/scripting/jb_lua.cpp -o ${OBJDIR}/jb_lua.o
${OBJDIR}/jb_lua_initapi.o: ${SOURCEDIR}/scripting/jb_lua_initapi.cpp ${JADEBASE_LUA_HPP} ${JADEBASE_HPP}
	@mkdir -p ${OBJDIR}
	${CPPC} -c ${DEFINES} ${WARNS} -fPIC ${INCLUDE} ${SOURCEDIR}/scripting/jb_lua_initapi.cpp -o ${OBJDIR}/jb_lua_initapi.o
${OBJDIR}/jb_luaapi.o: ${SOURCEDIR}/scripting/jb_luaapi.cpp ${JADEBASE_HPP}
	@mkdir -p ${OBJDIR}
	${CPPC} -c ${DEFINES} ${WARNS} -fPIC ${INCLUDE} ${SOURCEDIR}/scripting/jb_luaapi.cpp -o ${OBJDIR}/jb_luaapi.o
${OBJDIR}/jb_luaapi_filetypes.o: ${SOURCEDIR}/scripting/jb_luaapi_filetypes.cpp ${JADEBASE_LUA_HPP} ${JADEBASE_HPP}
	@mkdir -p ${OBJDIR}
	${CPPC} -c ${DEFINES} ${WARNS} -fPIC ${INCLUDE} ${SOURCEDIR}/scripting/jb_luaapi_filetypes.cpp -o ${OBJDIR}/jb_luaapi_filetypes.o
${OBJDIR}/jb_luaapi_gui_imgrsrc.o: ${SOURCEDIR}/scripting/jb_luaapi_gui_imgrsrc.cpp ${JADEBASE_LUA_HPP} ${JADEBASE_HPP}
	@mkdir -p ${OBJDIR}
	${CPPC} -c ${DEFINES} ${WARNS} -fPIC ${INCLUDE} ${SOURCEDIR}/scripting/jb_luaapi_gui_imgrsrc.cpp -o ${OBJDIR}/jb_luaapi_gui_imgrsrc.o
${OBJDIR}/jb_luaapi_gui_textrsrc.o: ${SOURCEDIR}/scripting/jb_luaapi_gui_textrsrc.cpp ${JADEBASE_LUA_HPP} ${JADEBASE_HPP}
	@mkdir -p ${OBJDIR}
	${CPPC} -c ${DEFINES} ${WARNS} -fPIC ${INCLUDE} ${SOURCEDIR}/scripting/jb_luaapi_gui_textrsrc.cpp -o ${OBJDIR}/jb_luaapi_gui_textrsrc.o
${OBJDIR}/jb_luaapi_gui_button.o: ${SOURCEDIR}/scripting/jb_luaapi_gui_button.cpp ${JADEBASE_LUA_HPP} ${JADEBASE_HPP}
	@mkdir -p ${OBJDIR}
	${CPPC} -c ${DEFINES} ${WARNS} -fPIC ${INCLUDE} ${SOURCEDIR}/scripting/jb_luaapi_gui_button.cpp -o ${OBJDIR}/jb_luaapi_gui_button.o
${OBJDIR}/jb_luaapi_gui_dial.o: ${SOURCEDIR}/scripting/jb_luaapi_gui_dial.cpp ${JADEBASE_LUA_HPP} ${JADEBASE_HPP}
	@mkdir -p ${OBJDIR}
	${CPPC} -c ${DEFINES} ${WARNS} -fPIC ${INCLUDE} ${SOURCEDIR}/scripting/jb_luaapi_gui_dial.cpp -o ${OBJDIR}/jb_luaapi_gui_dial.o
${OBJDIR}/jb_luaapi_gui_group.o: ${SOURCEDIR}/scripting/jb_luaapi_gui_group.cpp ${JADEBASE_LUA_HPP} ${JADEBASE_HPP}
	@mkdir -p ${OBJDIR}
	${CPPC} -c ${DEFINES} ${WARNS} -fPIC ${INCLUDE} ${SOURCEDIR}/scripting/jb_luaapi_gui_group.cpp -o ${OBJDIR}/jb_luaapi_gui_group.o
${OBJDIR}/jb_luaapi_gui_scrollset.o: ${SOURCEDIR}/scripting/jb_luaapi_gui_scrollset.cpp ${JADEBASE_LUA_HPP} ${JADEBASE_HPP}
	@mkdir -p ${OBJDIR}
	${CPPC} -c ${DEFINES} ${WARNS} -fPIC ${INCLUDE} ${SOURCEDIR}/scripting/jb_luaapi_gui_scrollset.cpp -o ${OBJDIR}/jb_luaapi_gui_scrollset.o
${OBJDIR}/jb_luaapi_gui_tabset.o: ${SOURCEDIR}/scripting/jb_luaapi_gui_tabset.cpp ${JADEBASE_LUA_HPP} ${JADEBASE_HPP}
	@mkdir -p ${OBJDIR}
	${CPPC} -c ${DEFINES} ${WARNS} -fPIC ${INCLUDE} ${SOURCEDIR}/scripting/jb_luaapi_gui_tabset.cpp -o ${OBJDIR}/jb_luaapi_gui_tabset.o
${OBJDIR}/jb_luaapi_tasking.o: ${SOURCEDIR}/scripting/jb_luaapi_tasking.cpp ${JADEBASE_LUA_HPP} ${JADEBASE_HPP}
	@mkdir -p ${OBJDIR}
	${CPPC} -c ${DEFINES} ${WARNS} -fPIC ${INCLUDE} ${SOURCEDIR}/scripting/jb_luaapi_tasking.cpp -o ${OBJDIR}/jb_luaapi_tasking.o
${OBJDIR}/jb_luaapi_utility.o: ${SOURCEDIR}/scripting/jb_luaapi_utility.cpp ${JADEBASE_LUA_HPP} ${JADEBASE_HPP}
	@mkdir -p ${OBJDIR}
	${CPPC} -c ${DEFINES} ${WARNS} -fPIC ${INCLUDE} ${SOURCEDIR}/scripting/jb_luaapi_utility.cpp -o ${OBJDIR}/jb_luaapi_utility.o
${OBJDIR}/jb_luaapi_windowsys.o: ${SOURCEDIR}/scripting/jb_luaapi_windowsys.cpp ${JADEBASE_LUA_HPP} ${JADEBASE_HPP}
	@mkdir -p ${OBJDIR}
	${CPPC} -c ${DEFINES} ${WARNS} -fPIC ${INCLUDE} ${SOURCEDIR}/scripting/jb_luaapi_windowsys.cpp -o ${OBJDIR}/jb_luaapi_windowsys.o

${OBJDIR}/jb_taskexec.o: ${SOURCEDIR}/tasking/jb_taskexec.cpp ${JADEBASE_TASKEXEC_HPP} ${JADEBASE_PLATFORM_H} ${JADEBASE_TASKQUEUE_HPP} ${JADEBASE_CONDITION_HPP} ${JADEBASE_MUTEX_HPP} ${JADEBASE_THREAD_HPP} ${JADEBASE_THREADUTIL_HPP} ${JADEBASE_EXCEPTION_HPP} ${JADEBASE_LAUNCHARGS_HPP} ${JADEBASE_LOG_HPP}
	@mkdir -p ${OBJDIR}
	${CPPC} -c ${DEFINES} ${WARNS} -fPIC ${INCLUDE} ${SOURCEDIR}/tasking/jb_taskexec.cpp -o ${OBJDIR}/jb_taskexec.o
${OBJDIR}/jb_taskqueue.o: ${SOURCEDIR}/tasking/jb_taskqueue.cpp ${JADEBASE_TASKQUEUE_HPP} ${JADEBASE_EXCEPTION_HPP} ${JADEBASE_LAUNCHARGS_HPP} ${JADEBASE_LOG_HPP}
	@mkdir -p ${OBJDIR}
	${CPPC} -c ${DEFINES} ${WARNS} -fPIC ${INCLUDE} ${SOURCEDIR}/tasking/jb_taskqueue.cpp -o ${OBJDIR}/jb_taskqueue.o

${OBJDIR}/jb_condition.o: ${SOURCEDIR}/threading/jb_condition.cpp ${JADEBASE_CONDITION_HPP} ${JADEBASE_EXCEPTION_HPP}
	@mkdir -p ${OBJDIR}
	${CPPC} -c ${DEFINES} ${WARNS} -fPIC ${INCLUDE} ${SOURCEDIR}/threading/jb_condition.cpp -o ${OBJDIR}/jb_condition.o
${OBJDIR}/jb_mutex.o: ${SOURCEDIR}/threading/jb_mutex.cpp ${JADEBASE_MUTEX_HPP} ${JADEBASE_EXCEPTION_HPP}
	@mkdir -p ${OBJDIR}
	${CPPC} -c ${DEFINES} ${WARNS} -fPIC ${INCLUDE} ${SOURCEDIR}/threading/jb_mutex.cpp -o ${OBJDIR}/jb_mutex.o
${OBJDIR}/jb_semaphore.o: ${SOURCEDIR}/threading/jb_semaphore.cpp ${JADEBASE_SEMAPHORE_HPP} ${JADEBASE_EXCEPTION_HPP}
	@mkdir -p ${OBJDIR}
	${CPPC} -c ${DEFINES} ${WARNS} -fPIC ${INCLUDE} ${SOURCEDIR}/threading/jb_semaphore.cpp -o ${OBJDIR}/jb_semaphore.o
${OBJDIR}/jb_thread.o: ${SOURCEDIR}/threading/jb_thread.cpp ${JADEBASE_THREAD_HPP} ${JADEBASE_EXCEPTION_HPP}
	@mkdir -p ${OBJDIR}
	${CPPC} -c ${DEFINES} ${WARNS} -fPIC ${INCLUDE} ${SOURCEDIR}/threading/jb_thread.cpp -o ${OBJDIR}/jb_thread.o
${OBJDIR}/jb_threadutil.o: ${SOURCEDIR}/threading/jb_threadutil.cpp ${JADEBASE_THREADUTIL_HPP} ${JADEBASE_EXCEPTION_HPP} ${JADEBASE_LOG_HPP}
	@mkdir -p ${OBJDIR}
	${CPPC} -c ${DEFINES} ${WARNS} -fPIC ${INCLUDE} ${SOURCEDIR}/threading/jb_threadutil.cpp -o ${OBJDIR}/jb_threadutil.o

${OBJDIR}/jb_exception.o: ${SOURCEDIR}/utility/jb_exception.cpp ${JADEBASE_EXCEPTION_HPP}
	@mkdir -p ${OBJDIR}
	${CPPC} -c ${DEFINES} ${WARNS} -fPIC ${INCLUDE} ${SOURCEDIR}/utility/jb_exception.cpp -o ${OBJDIR}/jb_exception.o
${OBJDIR}/jb_gl.o: ${SOURCEDIR}/utility/jb_gl.cpp ${JADEBASE_GL_HPP} ${JADEBASE_EXCEPTION_HPP} ${JADEBASE_LOG_HPP} ${JADEBASE_TASK_HPP} ${JADEBASE_TASKEXEC_HPP}
	@mkdir -p ${OBJDIR}
	${CPPC} -c ${DEFINES} ${WARNS} -fPIC ${INCLUDE} ${SOURCEDIR}/utility/jb_gl.cpp -o ${OBJDIR}/jb_gl.o
${OBJDIR}/jb_launchargs.o: ${SOURCEDIR}/utility/jb_launchargs.cpp ${JADEBASE_LAUNCHARGS_HPP} ${JADEBASE_EXCEPTION_HPP} ${JADEBASE_LOG_HPP} ${JADEBASE_SETTINGS_HPP} ${JADEBASE_VERSION_HPP}
	@mkdir -p ${OBJDIR}
	${CPPC} -c ${DEFINES} ${WARNS} -fPIC ${INCLUDE} ${SOURCEDIR}/utility/jb_launchargs.cpp -o ${OBJDIR}/jb_launchargs.o
# This is an odd one out, as it's C:
${OBJDIR}/jb_platform.o: ${SOURCEDIR}/utility/jb_platform.c ${JADEBASE_PLATFORM_H}
	@mkdir -p ${OBJDIR}
	${CC} -c ${DEFINES} ${WARNS} -fPIC ${INCLUDE} ${SOURCEDIR}/utility/jb_platform.c -o ${OBJDIR}/jb_platform.o
${OBJDIR}/jb_settings.o: ${SOURCEDIR}/utility/jb_settings.cpp ${JADEBASE_SETTINGS_HPP} ${JADEBASE_EXCEPTION_HPP} ${JADEBASE_LAUNCHARGS_HPP} ${JADEBASE_LOG_HPP} ${JADEBASE_MUTEX_HPP}
	@mkdir -p ${OBJDIR}
	${CPPC} -c ${DEFINES} ${WARNS} -fPIC ${INCLUDE} ${SOURCEDIR}/utility/jb_settings.cpp -o ${OBJDIR}/jb_settings.o
${OBJDIR}/jb_timestamp.o: ${SOURCEDIR}/utility/jb_timestamp.cpp ${JADEBASE_TIMESTAMP_HPP}
	@mkdir -p ${OBJDIR}
	${CPPC} -c ${DEFINES} ${WARNS} -fPIC ${INCLUDE} ${SOURCEDIR}/utility/jb_timestamp.cpp -o ${OBJDIR}/jb_timestamp.o
${OBJDIR}/jb_trackable.o: ${SOURCEDIR}/utility/jb_trackable.cpp ${JADEBASE_TRACKABLE_HPP} ${JADEBASE_EXCEPTION_HPP} ${JADEBASE_SETTINGS_HPP} ${JADEBASE_MUTEX_HPP}
	@mkdir -p ${OBJDIR}
	${CPPC} -c ${DEFINES} ${WARNS} -fPIC ${INCLUDE} ${SOURCEDIR}/utility/jb_trackable.cpp -o ${OBJDIR}/jb_trackable.o
${OBJDIR}/jb_version.o: ${SOURCEDIR}/utility/jb_version.cpp ${JADEBASE_VERSION_HPP} ${JADEBASE_MUTEX_HPP} ${LOG}
	@mkdir -p ${OBJDIR}
	${CPPC} -c ${DEFINES} ${WARNS} -fPIC ${INCLUDE} ${SOURCEDIR}/utility/jb_version.cpp -o ${OBJDIR}/jb_version.o

${OBJDIR}/jb_events.o: ${SOURCEDIR}/windowsys/jb_events.cpp ${JADEBASE_EVENTS_HPP} ${JADEBASE_KEYCODE_HPP} ${JADEBASE_WINDOW_HPP} ${JADEBASE_WINDOWEVENT_HPP} ${JADEBASE_WINDOWMANAGEMENT_HPP} ${JADEBASE_NAMED_RESOURCES} ${JADEBASE_TASKEXEC_HPP} ${JADEBASE_MUTEX_HPP} ${JADEBASE_EXCEPTION_HPP} ${JADEBASE_LAUNCHARGS_HPP} ${JADEBASE_LOG_HPP} ${JADEBASE_PLATFORM_H}
	@mkdir -p ${OBJDIR}
	${CPPC} -c ${DEFINES} ${WARNS} -fPIC ${INCLUDE} ${SOURCEDIR}/windowsys/jb_events.cpp -o ${OBJDIR}/jb_events.o
${OBJDIR}/jb_keycode.o: ${SOURCEDIR}/windowsys/jb_keycode.cpp ${JADEBASE_KEYCODE_HPP} ${JADEBASE_WINDOWEVENT_HPP} ${JADEBASE_EXCEPTION_HPP}
	@mkdir -p ${OBJDIR}
	${CPPC} -c ${DEFINES} ${WARNS} -fPIC ${INCLUDE} ${SOURCEDIR}/windowsys/jb_keycode.cpp -o ${OBJDIR}/jb_keycode.o
${OBJDIR}/jb_window.o: ${SOURCEDIR}/windowsys/jb_window.cpp ${JADEBASE_WINDOW_HPP} ${JADEBASE_WINDOWMANAGEMENT_HPP} ${JADEBASE_NAMED_RESOURCES} ${JADEBASE_WINDOWVIEW_HPP} ${JADEBASE_TASKEXEC_HPP} ${JADEBASE_EXCEPTION_HPP} ${JADEBASE_GL_HPP} ${JADEBASE_LAUNCHARGS_HPP} ${JADEBASE_LOG_HPP} ${JADEBASE_SETTINGS_HPP}
	@mkdir -p ${OBJDIR}
	${CPPC} -c ${DEFINES} ${WARNS} -fPIC ${INCLUDE} ${SOURCEDIR}/windowsys/jb_window.cpp -o ${OBJDIR}/jb_window.o
${OBJDIR}/jb_windowevent.o: ${SOURCEDIR}/windowsys/jb_windowevent.cpp ${JADEBASE_WINDOWEVENT_HPP} ${JADEBASE_LOG_HPP}
	@mkdir -p ${OBJDIR}
	${CPPC} -c ${DEFINES} ${WARNS} -fPIC ${INCLUDE} ${SOURCEDIR}/windowsys/jb_windowevent.cpp -o ${OBJDIR}/jb_windowevent.o
${OBJDIR}/jb_windowmanagement.o: ${SOURCEDIR}/windowsys/jb_windowmanagement.cpp ${JADEBASE_WINDOWMANAGEMENT_HPP} ${JADEBASE_TASKEXEC_HPP} ${JADEBASE_MUTEX_HPP} ${JADEBASE_EXCEPTION_HPP} ${JADEBASE_LAUNCHARGS_HPP} ${JADEBASE_LOG_HPP}
	@mkdir -p ${OBJDIR}
	${CPPC} -c ${DEFINES} ${WARNS} -fPIC ${INCLUDE} ${SOURCEDIR}/windowsys/jb_windowmanagement.cpp -o ${OBJDIR}/jb_windowmanagement.o

${OBJDIR}/jb_main.o: ${SOURCEDIR}/main/jb_main.cpp ${JADEBASE_MAIN_H} ${JADEBASE_EXCEPTION_HPP} ${JADEBASE_LAUNCHARGS_HPP} ${JADEBASE_LOG_HPP}
	@mkdir -p ${OBJDIR}
	${CPPC} -c ${DEFINES} ${WARNS} -fPIC ${INCLUDE} ${SOURCEDIR}/main/jb_main.cpp -o ${OBJDIR}/jb_main.o


# Linux ########################################################################

X_INPUTDEVICES_HPP = ${SOURCEDIR}/windowsys/x_inputdevices.hpp ${JADEBASE_PLATFORM_H}

LINUX_OBJECTS = ${OBJDIR}/x_main.o \
				${OBJDIR}/x_inputdevices.o \
				${OBJDIR}/x_window.o

${OBJDIR}/x_main.o: ${SOURCEDIR}/main/x_main.cpp ${JADEBASE_MAIN_H} ${JADEBASE_LAUNCHARGS_HPP} ${JADEBASE_LOG_HPP} ${JADEBASE_PLATFORM_H}
	@mkdir -p ${OBJDIR}
	${CPPC} -c ${DEFINES} ${WARNS} -fPIC ${INCLUDE} ${SOURCEDIR}/main/x_main.cpp -o ${OBJDIR}/x_main.o

${OBJDIR}/x_inputdevices.o: ${SOURCEDIR}/windowsys/x_inputdevices.cpp ${X_INPUTDEVICES_HPP} ${JADEBASE_WINDOWMANAGEMENT_HPP} ${JADEBASE_WINDOWEVENT_HPP} ${JADEBASE_MUTEX_HPP} ${JADEBASE_EXCEPTION_HPP} ${JADEBASE_LAUNCHARGS_HPP} ${JADEBASE_LOG_HPP} ${JADEBASE_SETTINGS_HPP}
	@mkdir -p ${OBJDIR}
	${CPPC} -c ${DEFINES} ${WARNS} -fPIC ${INCLUDE} ${SOURCEDIR}/windowsys/x_inputdevices.cpp -o ${OBJDIR}/x_inputdevices.o

${OBJDIR}/x_window.o: ${SOURCEDIR}/windowsys/x_window.cpp ${JADEBASE_WINDOW_HPP}
	@mkdir -p ${OBJDIR}
	${CPPC} -c ${DEFINES} ${WARNS} -fPIC ${INCLUDE} ${SOURCEDIR}/windowsys/x_window.cpp -o ${OBJDIR}/x_window.o

${BUILDDIR}/lib${PROJNAME}-${CC}.so.0.1: ${CORE_OBJECTS} ${LINUX_OBJECTS} ${FF_OBJECTS}
	@mkdir -p ${BUILDDIR}
	${CPPC} -shared -Wl,-soname,lib${PROJNAME}-${CC}.so.0 -o "${BUILDDIR}/lib${PROJNAME}-${CC}.so.0.1" ${LINUX_LINKS} -lX11 -lXext -lXi $^

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


# OS X #########################################################################

COCOA_APPDELEGATE_H = ${SOURCEDIR}/main/cocoa_appdelegate.h
COCOA_EVENTS_H = ${SOURCEDIR}/windowsys/cocoa_events.h

OSX_OBJECTS =	${OBJDIR}/cocoa_appdelegate.o \
				${OBJDIR}/cocoa_main.o \
				${OBJDIR}/cocoa_window.o

${OBJDIR}/cocoa_appdelegate.o: ${SOURCEDIR}/main/cocoa_appdelegate.mm ${COCOA_APPDELEGATE_H} ${JADEBASE_MAIN_H} ${JADEBASE_SETTINGS_HPP} ${COCOA_EVENTS_H}
	@mkdir -p ${OBJDIR}
	${OBJCC} -c ${DEFINES} ${WARNS} -fPIC ${INCLUDE} ${SOURCEDIR}/main/cocoa_appdelegate.mm -o ${OBJDIR}/cocoa_appdelegate.o

${OBJDIR}/cocoa_main.o: ${SOURCEDIR}/main/cocoa_main.m ${COCOA_APPDELEGATE_H} ${JADEBASE_MAIN_H}
	@mkdir -p ${OBJDIR}
	${OBJCC} -c ${DEFINES} ${WARNS} -fPIC ${INCLUDE} ${SOURCEDIR}/main/cocoa_main.m -o ${OBJDIR}/cocoa_main.o

${OBJDIR}/cocoa_window.o: ${SOURCEDIR}/windowsys/cocoa_window.mm ${JADEBASE_WINDOW_HPP}
	@mkdir -p ${OBJDIR}
	${OBJCC} -c ${DEFINES} ${WARNS} -fPIC ${INCLUDE} ${SOURCEDIR}/windowsys/cocoa_window.mm -o ${OBJDIR}/cocoa_window.o

# Bleh...
osx_build: ${CORE_OBJECTS} ${OSX_OBJECTS} ${FF_OBJECTS}
	mkdir -p ${BUILDDIR}
	${CPPC} -o "${BUILDDIR}/${PROJNAME}" ${COCOA_LINKS} -lobjc $^

osx_install:
	@echo "No working OS X build yet"

osx_uninstall:
	@echo "No working OS X build yet"


# Windows ######################################################################

# TODO:


# Test program(s) ##############################################################

# Linux-only for now
test:
	@mkdir -p ${OBJDIR}
	${CPPC} -c ${DEFINES} ${WARNS} ${INCLUDE} "${SOURCEDIR}/jb_test.cpp" -o "${OBJDIR}/jb_test.o"
	@mkdir -p ${BUILDDIR}
	${CPPC} -o "${BUILDDIR}/jb_test" "${OBJDIR}/jb_test.o" -l${PROJNAME}-${CC} ${LINUX_LINKS}

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


