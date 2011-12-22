#-----------------------------------------------------------
# GOGGLES BUILD SYSTEM
#-----------------------------------------------------------
#
# The actual make file
#
#-----------------------------------------------------------

include build/version
include config.make

# Set suffixes
.SUFFIXES:
.SUFFIXES: .cpp .h .gif .png $(OBJEXT) $(BINEXT) $(LIBEXT)

.PHONY : all clean realclean cleanicons install install-desktop

INSTALL=install
INSTALL_PROGRAM=$(INSTALL)
INSTALL_DATA=$(INSTALL) -m 644

# Installation Directory
ifdef DESTDIR
INSTALL_DIR=$(DESTDIR)$(PREFIX)
INSTALL_MANDIR=$(DESTDIR)$(MANDIR)
else
INSTALL_DIR=$(PREFIX)
INSTALL_MANDIR=$(MANDIR)
endif

AC_NAME=audioconvert
AC_BIN=src/$(AC_NAME)
TARNAME=$(AC_NAME)-$(MAJOR).$(MINOR).$(LEVEL)

all: $(AC_BIN)





# Objects to Compile
#----------------------------------------------------------
AC_SRCFILES := src/main.cpp \
src/AudioTools.cpp \
src/AudioConvert.cpp \
src/AudioTags.cpp \
src/AudioFilename.cpp


AC_OBJECTS := $(patsubst %.cpp,%$(OBJEXT),$(AC_SRCFILES))
AC_DEPENDENCIES = $(patsubst %.cpp,%.d,$(AC_SRCFILES))

%$(OBJEXT):	%.cpp
	@echo "    Compiling $< ..."
#	@echo "$(CXX) $(CFLAGS) $(DEFS)  $(CPPFLAGS) -MM -o $*.d -MT $@ $<"
	@$(CXX) $(CFLAGS) $(CPPFLAGS) -MM -o $*.d -MT $@ $<
#	@echo "$(CXX) $(strip $(CFLAGS)) $(strip $(CPPFLAGS)) $(OUTPUTOBJ)$@ -c $<"
	@$(CXX) $(strip $(CFLAGS)) $(strip $(CPPFLAGS)) $(OUTPUTOBJ)$@ -c $<


$(AC_BIN): $(AC_OBJECTS)
	@echo "    Linking $@ ..."
#	@echo "$(LINK) $(LDFLAGS)  $(OUTPUTBIN)$(AC_BIN) $(strip $(AC_OBJECTS)) $(strip $(LIBS))"
	@$(LINK) $(LDFLAGS) $(OUTPUTBIN)$(AC_BIN) $(strip $(AC_OBJECTS)) $(strip $(LIBS))

# Force Dependency on config.make
$(AC_OBJECTS) : config.make


# Install
#----------------------------------------------------------
install: $(AC_BIN)
	@echo "  Installing $(INSTALL_DIR)/bin/$(AC_NAME) ..."
	@$(INSTALL) -m 755 -D $(AC_BIN) $(INSTALL_DIR)/bin/$(AC_NAME)
	@echo "  Installing $(INSTALL_MANDIR)/man1/audioconvert.1"
	@$(INSTALL) -m 644 -D extra/audioconvert.1 $(INSTALL_MANDIR)/man1/audioconvert.1

uninstall:
	@echo "  Removing $(INSTALL_DIR)/bin/$(AC_NAME) ... ..."
	rm -f $(INSTALL_DIR)/bin/$(AC_NAME)
	@echo "  Removing $(INSTALL_MANDIR)/man1/audioconvert.1 ..."
	rm -f $(INSTALL_MANDIR)/man1/audioconvert.1

# Clean
#----------------------------------------------------------
clean :
	@echo "  Remove Executables ..."
	@rm -f $(AC_BIN)

	@echo "  Remove Objects ..."
	@rm -f src/*$(OBJEXT)
	@rm -f src/*.d
#----------------------------------------------------------

realclean :
	@echo "  Remove Configuration ..."
	@rm -f config.make
	@rm -f src/acconfig.h

dist: clean realclean
	@echo " Creating Tarbal .."
	tar --create --xz --file='../../$(TARNAME).tar.xz' --verbose --exclude-vcs --exclude='*.tar.xz' --transform='s/^./$(TARNAME)/' --show-transformed-names .


# How to make everything else
-include $(DEPENDENCIES)
