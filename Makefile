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

################################################################################

clean:
	rm -r ${MAKEDIR}

# TODO: Consider using a perl script for nicer output
#| awk -F: '{ print $$1":"$$2":\n    "; for(i=3;i<NF;i++){printf " %s", $$i} printf "\n" }'
todo:
	grep -nr --include \* --exclude-dir=make "TODO:[ ]\+" .  # Using '[ ]' so the grep line is ignored by grep

linecount:
	wc -l `find ./src -type f`

################################################################################

PROJNAME = jadebase

################################################################################

osx_install: osx
	@echo "No install yet"

linux_install: linux
	@echo "No install yet"

################################################################################

# linux_profile: linux
# 	valgrind --tool=callgrind "${MAKEDIR}/${PROJNAME}/Linux/${PROJNAME}" -d

################################################################################

osx: build_osx
	@echo "No working OS X build yet"

linux: build_linux
	@echo "..."

windows:
	# Not supported yet

################################################################################

# TODO: consider using macro(s)
CORE_OBJECTS =	${OBJDIR}/jb_events.o \
				${OBJDIR}/jb_exception.o \
				${OBJDIR}/jb_gl.o \
				${OBJDIR}/jb_keycode.o \
				${OBJDIR}/jb_launchargs.o \
				${OBJDIR}/jb_platform.c.o \
				${OBJDIR}/jb_png.o \
				${OBJDIR}/jb_settings.o \
				${OBJDIR}/jb_taskexec.o \
				${OBJDIR}/jb_taskqueue.o \
				${OBJDIR}/jb_timestamp.o \
				${OBJDIR}/jb_trackable.o \
				${OBJDIR}/jb_window.o \
				${OBJDIR}/jb_windowevent.o \
				${OBJDIR}/jb_windowmanagement.o \
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
				${OBJDIR}/threading.jb_condition.o \
				${OBJDIR}/threading.jb_mutex.o \
				${OBJDIR}/threading.jb_semaphore.o \
				${OBJDIR}/threading.jb_threadutil.o \
				${OBJDIR}/threading.jb_thread.o

OSX_OBJECTS =	${OBJDIR}/cocoa_appdelegate.o \
				${OBJDIR}/cocoa_main.o

LINUX_OBJECTS = ${OBJDIR}/x_inputdevices.o \
				${OBJDIR}/x_main.o

# FastFormat is statically linked due to the non-standard build methods the
# project uses.  Until there is an official dynamic installation it should
# reamain statically linked for ease of (precompiled binary) distribution.
# Not sure how many of these we need, so include all of them
FF_OBJECTS =	${FFOBJDIR}/core.api.o \
				${FFOBJDIR}/core.fmt_cache.o \
				${FFOBJDIR}/core.fmt_spec_defect_handlers.o \
				${FFOBJDIR}/core.init_code_strings.o \
				${FFOBJDIR}/core.mempool.o \
				${FFOBJDIR}/core.replacements.o \
				${FFOBJDIR}/core.snprintf.o

################################################################################

# ${OBJDIR}/jb_.o

build_osx: ${CORE_OBJECTS} ${OSX_OBJECTS}
	make fastformat
	# mkdir -p ${BUILDDIR}
	# ${CPPC} -o "${BUILDDIR}/${PROJNAME}" ${FRAMEWORKS} ${LINKS} -lobjc $? ${FF_OBJECTS}

build_linux: ${CORE_OBJECTS} ${LINUX_OBJECTS}
	make fastformat
	# mkdir -p ${BUILDDIR}
	# ${CPPC} -o "${BUILDDIR}/${PROJNAME}" ${LINKS} -lpthread -lX11 -lXext -lXi $? ${FF_OBJECTS}

fastformat:
	@echo Trying to automatically build FastFormat\; makefile may need manual editing to compile on some platforms
	cd ${FFOBJDIR}; make build.libs.core

################################################################################

${OBJDIR}/unix_%.o: ${SOURCEDIR}/unix_%.cpp
	mkdir -p ${OBJDIR}
	${CPPC} ${DEFINES} -c ${INCLUDE} $? -o ${OBJDIR}/unix_$*.o

${OBJDIR}/cocoa_%.o: ${SOURCEDIR}/cocoa_%.m
	mkdir -p ${OBJDIR}
	${OBJCC} ${DEFINES} -c ${INCLUDE} $? -o ${OBJDIR}/cocoa_$*.o

${OBJDIR}/jb_%.c.o: ${SOURCEDIR}/jb_%.c
	mkdir -p ${OBJDIR}
	${CC} ${DEFINES} -c ${INCLUDE} $? -o ${OBJDIR}/jb_$*.c.o

${OBJDIR}/jb_%.o: ${SOURCEDIR}/jb_%.cpp
	mkdir -p ${OBJDIR}
	${CPPC} ${DEFINES} -c ${INCLUDE} $? -o ${OBJDIR}/jb_$*.o

${OBJDIR}/gui.jb_%.o: ${SOURCEDIR}/gui/jb_%.cpp
	mkdir -p ${OBJDIR}
	${CPPC} ${DEFINES} -c ${INCLUDE} $? -o ${OBJDIR}/gui.jb_$*.o

${OBJDIR}/threading.jb_%.o: ${SOURCEDIR}/threading/jb_%.cpp
	mkdir -p ${OBJDIR}
	${CPPC} ${DEFINES} -c ${INCLUDE} $? -o ${OBJDIR}/threading.jb_$*.o

${OBJDIR}/x_%.o: ${SOURCEDIR}/x_%.cpp
	mkdir -p ${OBJDIR}
	${CPPC} ${DEFINES} -c ${INCLUDE} $? -o ${OBJDIR}/x_$*.o

################################################################################

.PHONY: fastformat build_linux build_osx osx linux windows clean todo linecount


