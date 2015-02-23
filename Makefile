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
# CC = /usr/local/Cellar/llvm/3.4/bin/clang
CC = clang
CPPC = ${CC}++ -stdlib=libstdc++
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

# Headers & librarires
INCLUDE = -I${FASTFORMAT_ROOT}/include -I${STLSOFT}/include `pkg-config --cflags lua5.2 libpng gl glew pangocairo`
LINKS = -lm `pkg-config --libs lua5.2 libpng gl glew pangocairo`
FRAMEWORKS = -framework Foundation -framework AppKit
DEFINES = -g -DDEBUG -DPLATFORM_XWS_GNUPOSIX

PROJNAME = jadebase

INSTALL_LOC = /usr/local

################################################################################

clean:
	rm -r ${MAKEDIR}

# TODO: Consider using a perl script for nicer output
#| awk -F: '{ print $$1":"$$2":\n    "; for(i=3;i<NF;i++){printf " %s", $$i} printf "\n" }'
todo:
	@grep -nr --include \* --exclude-dir=make "\(TODO\|WARNING\|FIXME\):[ ]\+" .  # Using '[ ]' so the grep line is ignored by grep

linecount:
	wc -l `find ./src -type f`

# linux_profile: linux
# 	valgrind --tool=callgrind "${MAKEDIR}/${PROJNAME}/Linux/${PROJNAME}" -d

################################################################################

# TODO: consider using macro(s)
CORE_OBJECTS =	${OBJDIR}/filetypes.jb_png.o \
				${OBJDIR}/gui.jb_button.o \
				${OBJDIR}/gui.jb_canvasview.o \
				${OBJDIR}/gui.jb_dial.o \
				${OBJDIR}/gui.jb_element.o \
				${OBJDIR}/gui.jb_group.o \
				${OBJDIR}/gui.jb_image_rsrc.o \
				${OBJDIR}/gui.jb_resource.o \
				${OBJDIR}/gui.jb_scrollset.o \
				${OBJDIR}/gui.jb_tabset.o \
				${OBJDIR}/gui.jb_text_rsrc.o \
				${OBJDIR}/gui.jb_named_resources.o \
				${OBJDIR}/main.jb_main.o \
				${OBJDIR}/scripting.jb_lua.o \
				${OBJDIR}/tasking.jb_taskexec.o \
				${OBJDIR}/tasking.jb_taskqueue.o \
				${OBJDIR}/threading.jb_condition.o \
				${OBJDIR}/threading.jb_mutex.o \
				${OBJDIR}/threading.jb_semaphore.o \
				${OBJDIR}/threading.jb_threadutil.o \
				${OBJDIR}/threading.jb_thread.o \
				${OBJDIR}/utility.jb_exception.o \
				${OBJDIR}/utility.jb_gl.o \
				${OBJDIR}/utility.jb_launchargs.o \
				${OBJDIR}/utility.jb_platform.c.o \
				${OBJDIR}/utility.jb_settings.o \
				${OBJDIR}/utility.jb_timestamp.o \
				${OBJDIR}/utility.jb_trackable.o \
				${OBJDIR}/utility.jb_version.o \
				${OBJDIR}/windowsys.jb_events.o \
				${OBJDIR}/windowsys.jb_keycode.o \
				${OBJDIR}/windowsys.jb_window.o \
				${OBJDIR}/windowsys.jb_windowevent.o \
				${OBJDIR}/windowsys.jb_windowmanagement.o

OSX_OBJECTS =	${OBJDIR}/main.cocoa_appdelegate.o \
				${OBJDIR}/main.cocoa_main.o

LINUX_OBJECTS = ${OBJDIR}/main.x_main.o \
				${OBJDIR}/windowsys.x_inputdevices.o

# FastFormat is statically linked due to the non-standard build methods the
# project uses.  Until there is an official dynamic installation it should
# remain statically linked for ease of (precompiled binary) distribution.
# Not sure how many of these we need, so include all of them
FF_OBJECTS =	${FFOBJDIR}/core.api.o \
				${FFOBJDIR}/core.fmt_cache.o \
				${FFOBJDIR}/core.fmt_spec_defect_handlers.o \
				${FFOBJDIR}/core.init_code_strings.o \
				${FFOBJDIR}/core.mempool.o \
				${FFOBJDIR}/core.replacements.o \
				${FFOBJDIR}/core.snprintf.o

################################################################################

test:
	mkdir -p ${OBJDIR}
	${CPPC} ${DEFINES} -Wall -c ${INCLUDE} "${SOURCEDIR}/jb_test.cpp" -o "${OBJDIR}/jb_test.o"
	mkdir -p ${BUILDDIR}
	${CPPC} -o "${BUILDDIR}/jb_test" "${OBJDIR}/jb_test.o" -l${PROJNAME}-${CC} ${LINKS}

################################################################################

osx_install:
	@echo "No working OS X build yet"

osx_uninstall:
	@echo "No working OS X build yet"

linux_install: linux_build
	sudo mkdir -p ${INSTALL_LOC}/lib
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

################################################################################

# ${OBJDIR}/jb_.o

# osx_build: ${CORE_OBJECTS} ${OSX_OBJECTS}
# 	make fastformat
# 	# mkdir -p ${BUILDDIR}
# 	# ${CPPC} -o "${BUILDDIR}/${PROJNAME}" ${FRAMEWORKS} ${LINKS} -lobjc $? ${FF_OBJECTS}

linux_build: ${CORE_OBJECTS} ${LINUX_OBJECTS}
	make fastformat
	mkdir -p ${BUILDDIR}
	${CPPC} -shared -Wl,-soname,lib${PROJNAME}-${CC}.so.0 -o "${BUILDDIR}/lib${PROJNAME}-${CC}.so.0.1" ${LINKS} -lpthread -lX11 -lXext -lXi $? ${FF_OBJECTS}

fastformat:
	@echo Trying to automatically build FastFormat\; makefile may need manual editing to compile on some platforms
	cd ${FFOBJDIR}; make build.libs.core

################################################################################

# TODO: fix this utter mess

${OBJDIR}/unix_%.o: ${SOURCEDIR}/unix_%.cpp
	mkdir -p ${OBJDIR}
	${CPPC} ${DEFINES} -Wall -fPIC -c ${INCLUDE} $? -o ${OBJDIR}/unix_$*.o

${OBJDIR}/main.cocoa_%.o: ${SOURCEDIR}/main/cocoa_%.m
	mkdir -p ${OBJDIR}
	${OBJCC} ${DEFINES} -Wall -fPIC -c ${INCLUDE} $? -o ${OBJDIR}/main.cocoa_$*.o

${OBJDIR}/windowsys.x_%.o: ${SOURCEDIR}/windowsys/x_%.cpp
	mkdir -p ${OBJDIR}
	${CPPC} ${DEFINES} -Wall -fPIC -c ${INCLUDE} $? -o ${OBJDIR}/windowsys.x_$*.o



${OBJDIR}/main.x_%.o: ${SOURCEDIR}/main/x_%.cpp
	mkdir -p ${OBJDIR}
	${CPPC} ${DEFINES} -Wall -fPIC -c ${INCLUDE} $? -o ${OBJDIR}/main.x_$*.o



${OBJDIR}/jb_%.o: ${SOURCEDIR}/jb_%.cpp
	mkdir -p ${OBJDIR}
	${CPPC} ${DEFINES} -Wall -fPIC -c ${INCLUDE} $? -o ${OBJDIR}/jb_$*.o

${OBJDIR}/filetypes.jb_%.o: ${SOURCEDIR}/filetypes/jb_%.cpp
	mkdir -p ${OBJDIR}
	${CPPC} ${DEFINES} -Wall -fPIC -c ${INCLUDE} $? -o ${OBJDIR}/filetypes.jb_$*.o

${OBJDIR}/gui.jb_%.o: ${SOURCEDIR}/gui/jb_%.cpp
	mkdir -p ${OBJDIR}
	${CPPC} ${DEFINES} -Wall -fPIC -c ${INCLUDE} $? -o ${OBJDIR}/gui.jb_$*.o

${OBJDIR}/main.jb_%.o: ${SOURCEDIR}/main/jb_%.cpp
	mkdir -p ${OBJDIR}
	${CPPC} ${DEFINES} -Wall -fPIC -c ${INCLUDE} $? -o ${OBJDIR}/main.jb_$*.o

${OBJDIR}/scripting.jb_%.o: ${SOURCEDIR}/scripting/jb_%.cpp
	mkdir -p ${OBJDIR}
	${CPPC} ${DEFINES} -Wall -fPIC -c ${INCLUDE} $? -o ${OBJDIR}/scripting.jb_$*.o

${OBJDIR}/tasking.jb_%.o: ${SOURCEDIR}/tasking/jb_%.cpp
	mkdir -p ${OBJDIR}
	${CPPC} ${DEFINES} -Wall -fPIC -c ${INCLUDE} $? -o ${OBJDIR}/tasking.jb_$*.o

${OBJDIR}/threading.jb_%.o: ${SOURCEDIR}/threading/jb_%.cpp
	mkdir -p ${OBJDIR}
	${CPPC} ${DEFINES} -Wall -fPIC -c ${INCLUDE} $? -o ${OBJDIR}/threading.jb_$*.o

${OBJDIR}/utility.jb_%.o: ${SOURCEDIR}/utility/jb_%.cpp
	mkdir -p ${OBJDIR}
	${CPPC} ${DEFINES} -Wall -fPIC -c ${INCLUDE} $? -o ${OBJDIR}/utility.jb_$*.o

${OBJDIR}/utility.jb_%.c.o: ${SOURCEDIR}/utility/jb_%.c
	mkdir -p ${OBJDIR}
	${CC} ${DEFINES} -Wall -fPIC -c ${INCLUDE} $? -o ${OBJDIR}/utility.jb_$*.c.o

${OBJDIR}/windowsys.jb_%.o: ${SOURCEDIR}/windowsys/jb_%.cpp
	mkdir -p ${OBJDIR}
	${CPPC} ${DEFINES} -Wall -fPIC -c ${INCLUDE} $? -o ${OBJDIR}/windowsys.jb_$*.o

################################################################################

.PHONY:	clean \
		fastformat \
		linecount \
		linux \
		linux_build \
		linux_install \
		linux_uninstall \
		osx \
		osx_build \
		osx_install \
		osx_uninstall \
		todo \
		windows


