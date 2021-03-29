
.PHONY: all
all: directories bin lib bins

LDFLAGS += -L./lib
include /usr/local/etc/Makefile-Base

SHOWLIBNAME=show${MACAPPEND}

LIBNAME=showgit${MACAPPEND}
LIB=lib/libshowgit${MACAPPEND}.a
LIB_ARGS= -cvrU
ifeq ($(UNAME), Darwin)
	LIB_ARGS= -cvr
endif

#--------------------------------------------------
# What goes into making the library.
#--------------------------------------------------
LIB_NOSORT := $(wildcard ${SRCDIR}/*.cpp)
LIB_SRC := $(sort ${LIB_NOSORT})
LIB_OBJ := $(patsubst ${SRCDIR}/%.cpp,${OBJDIR}/%.o,${LIB_SRC})

VPATH := ${SRCDIR}:programs
INCLUDES += -I. -I./src

LIBS += -l${LIBNAME}
LIBS += -l${SHOWLIBNAME}
LIBS += -L/usr/local/opt/openssl/lib
LIBS += -lgit2 -lcurl -lssh2 -lssl -lz -liconv -lssh2 -lcrypto
LIBS += -lpthread -lstdc++ -lm -ldl
LIBS += -framework CoreFoundation -framework Security

LDFLAGS += ${LIBS}

# Where we'll install things.
INSTALL_BASE=/usr/local

echo:
	@echo LIB_SRC: ${LIB_SRC}

#======================================================================
# Top-level targets.
#======================================================================
include

Makefile: ;

# Clean the contents of the subdirs.
.PHONY: clean
clean:
	rm -f ${DEPDIR}/* ${OBJDIR}/* ${LIB}

#======================================================================
# Making the library
#======================================================================
lib: ${LIB}

${LIB}: ${LIB_OBJ}
	@mkdir -p lib
	ar ${LIB_ARGS} ${LIB} ${LIB_OBJ}
	ranlib ${LIB}


#======================================================================
# I have a small number of programs.
#======================================================================
bin:
	mkdir -p bin

bins: bin/GitInfo bin/GitFetch

bin/GitInfo: ${OBJDIR}/GitInfo.o ${LIB}
	$(CXX) ${OBJDIR}/GitInfo.o ${LDFLAGS} $(OUTPUT_OPTION)

bin/GitFetch: ${OBJDIR}/GitFetch.o ${LIB}
	$(CXX) ${OBJDIR}/GitFetch.o ${LDFLAGS} $(OUTPUT_OPTION)

#======================================================================
# Installation.
#======================================================================
.PHONY: install
install: ${LIB} install_includes
	cp -p ${LIB} ${INSTALL_BASE}/lib

.PHONY: install_includes
install_includes:
	@mkdir -p ${INSTALL_BASE}/include/git
	cp -p ${SRCDIR}/*.h ${INSTALL_BASE}/include/git
