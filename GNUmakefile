# non-recursive prologue
sp := $(sp).x
dirstack_$(sp) := $(d)
d := $(abspath $(lastword $(MAKEFILE_LIST))/..)

ifeq ($(origin GUARD_$(d)), undefined)
GUARD_$(d) := 1


$(d)/help: # default target


#
# E N V I R O N M E N T  C O N F I G U R A T I O N
#
-include $(d)/Makeflags

-include $(d)/.config

prefix ?= /usr/local
includedir ?= $(prefix)/include
libdir ?= $(prefix)/lib
datadir ?= $(prefix)/share
bindir ?= $(prefix)/bin
lua51cpath ?= $(libdir)/lua/5.1
lua51path ?= $(datadir)/lua/5.1
lua52cpath ?= $(libdir)/lua/5.2
lua52path ?= $(datadir)/lua/5.2
lua53cpath ?= $(libdir)/lua/5.3
lua53path ?= $(datadir)/lua/5.3

AR ?= ar
RANLIB ?= ranlib
M4 ?= m4
RM ?= rm
CP ?= cp
RMDIR ?= rmdir
MKDIR ?= mkdir
MKDIR_P ?= $(MKDIR) -p
CHMOD ?= chmod
INSTALL ?= install
INSTALL_DATA ?= $(INSTALL) -m 644

.PHONY: $(d)/config

$(d)/config:
	printf 'prefix ?= $(value prefix)'"\n" >| $(@D)/.config
	printf 'includedir ?= $(value includedir)'"\n" >> $(@D)/.config
	printf 'libdir ?= $(value libdir)'"\n" >> $(@D)/.config
	printf 'datadir ?= $(value datadir)'"\n" >> $(@D)/.config
	printf 'bindir ?= $(value bindir)'"\n" >> $(@D)/.config
	printf 'lua51cpath ?= $(value lua51cpath)'"\n" >> $(@D)/.config
	printf 'lua51path ?= $(value lua51path)'"\n" >> $(@D)/.config
	printf 'lua52cpath ?= $(value lua52cpath)'"\n" >> $(@D)/.config
	printf 'lua52path ?= $(value lua52path)'"\n" >> $(@D)/.config
	printf 'lua53cpath ?= $(value lua53cpath)'"\n" >> $(@D)/.config
	printf 'lua53path ?= $(value lua53path)'"\n" >> $(@D)/.config
	printf 'CC ?= $(CC)'"\n" >> $(@D)/.config
	printf 'CPPFLAGS ?= $(value CPPFLAGS)'"\n" >> $(@D)/.config
	printf 'CFLAGS ?= $(value CFLAGS)'"\n" >> $(@D)/.config
	printf 'LDFLAGS ?= $(value LDFLAGS)'"\n" >> $(@D)/.config
	printf 'SOFLAGS ?= $(value SOFLAGS)'"\n" >> $(@D)/.config
	printf 'AR ?= $(value AR)'"\n" >> $(@D)/.config
	printf 'RANLIB ?= $(value RANLIB)'"\n" >> $(@D)/.config
	printf 'M4 ?= $(value M4)'"\n" >> $(@D)/.config
	printf 'RM ?= $(value RM)'"\n" >> $(@D)/.config
	printf 'CP ?= $(value CP)'"\n" >> $(@D)/.config
	printf 'RMDIR ?= $(value RMDIR)'"\n" >> $(@D)/.config
	printf 'MKDIR ?= $(value MKDIR)'"\n" >> $(@D)/.config
	printf 'MKDIR_P ?= $(value MKDIR_P)'"\n" >> $(@D)/.config
	printf 'CHMOD ?= $(value CHMOD)'"\n" >> $(@D)/.config
	printf 'INSTALL ?= $(value INSTALL)'"\n" >> $(@D)/.config
	printf 'INSTALL_DATA ?= $(value INSTALL_DATA)'"\n" >> $(@D)/.config

# add local targets if building from inside project tree
ifneq "$(filter $(abspath $(d)/..)/%, $(abspath $(firstword $(MAKEFILE_LIST))))" ""
.PHONY: config configure

config configure: $(d)/config
endif


#
# S H A R E D  C O M P I L A T I O N  F L A G S
#
cc-option ?= $(shell if $(CC) $(1) -S -o /dev/null -xc /dev/null \
             > /dev/null 2>&1; then echo "$(1)"; else echo "$(2)"; fi;)

VENDOR_OS_$(d) := $(shell uname -s)
VENDOR_CC_$(d) := $(shell env CC="$(CC)" $(d)/mk/luapath ccname)

ifneq ($(VENDOR_OS_$(d)), OpenBSD)
CPPFLAGS_$(d) += -D_REENTRANT -D_THREAD_SAFE -D_GNU_SOURCE
endif

ifeq ($(VENDOR_OS_$(d)), SunOS)
CPPFLAGS_$(d) += -Usun -D_XPG4_2 -D__EXTENSIONS__ -D_POSIX_PTHREAD_SEMANTICS
endif

ifeq ($(VENDOR_CC_$(d)), gcc)
CFLAGS_$(d) += -O2 -std=gnu99 -fPIC
CFLAGS_$(d) += -g -Wall -Wextra $(call cc-option, -Wno-missing-field-initializers) $(call cc-option, -Wno-override-init) -Wno-unused
endif

ifeq ($(VENDOR_CC_$(d)), clang)
CFLAGS_$(d) += -O2 -std=gnu99 -fPIC
CFLAGS_$(d) += -g -Wall -Wextra -Wno-missing-field-initializers -Wno-initializer-overrides -Wno-unused
endif

ifeq ($(VENDOR_CC_$(d)), sunpro)
CFLAGS_$(d) += -xcode=pic13
CFLAGS_$(d) += -g
endif

ifeq ($(VENDOR_OS_$(d)), Darwin)
SOFLAGS_$(d) += -bundle -undefined dynamic_lookup
else
SOFLAGS_$(d) += -shared
endif


#
# P R O J E C T  R U L E S
#
include $(d)/src/GNUmakefile


#
# C L E A N  R U L E S
#
.PHONY: $(d)/clean~ clean~ $(d)/distclean distclean

$(d)/clean~:
	$(RM) -f $(@D)/*~

clean~: $(d)/clean~

$(d)/distclean:
	$(RM) -f $(@D)/config.log $(@D)/config.status $(@D)/Makeflags \
	$(@D)/src/config.h

distclean: $(d)/distclean


#
# H E L P  R U L E S
#
.PHONY: $(d)/help help

$(d)/help: # default target
	@echo 'Module targets:'
	@echo '  config     - store all variables for subsequent make invocations'
	@echo '  all        - build Lua 5.1 and 5.2 modules'
	@echo '  all5.1     - build Lua 5.1 module'
	@echo '  all5.2     - build Lua 5.2 module'
	@echo '  all5.3     - build Lua 5.3 module'
	@echo '  install    - install Lua 5.1 and 5.2 modules'
	@echo '  install5.1 - install Lua 5.1 module'
	@echo '  install5.2 - install Lua 5.2 module'
	@echo '  install5.3 - install Lua 5.3 module'
	@echo '  clean      - remove generated files'
	@echo '  clean~     - remove *~ files'
	@echo '  help       - display this help message'
	@echo 'Module variables:'
	@echo '  DESTDIR    - prefix for installation targets'
	@echo '  prefix     - default prefix for includedir, libdir, datadata, and bindir'
	@echo '  includedir - default local include/ directory'
	@echo '  libdir     - default local lib/ directory'
	@echo '  datadir    - default local share/ directory'
	@echo '  bindir     - default local bin/ directory (for finding lua and luac)'
	@echo '  lua51cpath - Lua 5.1 C module installation path'
	@echo '  lua51path  - Lua 5.1 module installation path'
	@echo '  lua52cpath - Lua 5.2 C module installation path'
	@echo '  lua52path  - Lua 5.2 module installation path'
	@echo '  lua53cpath - Lua 5.3 C module installation path'
	@echo '  lua53path  - Lua 5.3 module installation path'
	@echo '  CC         - C compiler path'
	@echo '  CFLAGS     - C compiler flags'
	@echo '  CPPFLAGS   - C preprocessor flags, particularly -I paths for Lua headers'
	@echo '  LDFLAGS    - C compiler linker flags'
	@echo '  SOFLAGS    - C compiler flags necessary for creating loadable module'
	@echo 'Debian package targets:'
	@echo '  debian           - build Debian package containing 5.1 and 5.2 modules'
	@echo '  debian-clean     - make debian/rules clean'
	@echo '  debian-distclean - removed all unversioned files in debian/'

help: $(d)/help


#
# D E B I A N  R U L E S
#
ifneq "$(filter $(abspath $(d))/%, $(abspath $(firstword $(MAKEFILE_LIST))))" ""

DPKG_BUILDPACKAGE ?= dpkg-buildpackage
FAKEROOT ?= fakeroot
DPKG_BUILDPACKAGE_OPTIONS ?= -b -uc -us

.PHONY: $(d)/debian $(d)/debian-clean $(d)/debian-distclean debian deb debian-clean deb-clean debian-distclean

$(d)/debian:
	cd $(@D) && $(DPKG_BUILDPACKAGE) -rfakeroot $(DPKG_BUILDPACKAGE_OPTIONS) \
	|| (RC=$$?; printf "\n>>> try make debian-distclean <<<\n\n" >&2; exit $$RC)

$(d)/debian-clean:
	cd $(@D) && $(FAKEROOT) ./debian/rules clean \
	|| (RC=$$?; printf "\n>>> try make debian-distclean <<<\n\n" >&2; exit $$RC)

# Older versions of dh_lua don't support the new Lua Debian package policy
# very well. They will build once but thereafter choke on intermediate files
# generated by previous runs.
$(d)/debian-distclean:
	@git ls-files --others $(@D)/debian >/dev/null
	git ls-files --others $(@D)/debian | xargs $(RM)

debian deb: $(d)/debian

debian-clean deb-clean: $(d)/debian-clean

debian-distclean: $(d)/debian-distclean

endif # debian guard


#
# R E L E A S E  T A R B A L L  R U L E S
#
ifneq "$(filter $(abspath $(d))/%, $(abspath $(firstword $(MAKEFILE_LIST))))" ""
ifneq "$(findstring release, $(MAKECMDGOALS))" ""

LUNIX_VERSION := $(shell git tag --list | sed -ne 's/^rel-\([[:digit:]]\{8\}\)/\1/p' | sort -n | tail -1)

.PHONY: $(d)/lunux-$(LUNIX_VERSION).tgz release

$(d)/lunix-$(LUNIX_VERSION).tgz:
	cd $(@D) && git archive --format=tar --prefix=$(basename $(@F))/ rel-$(LUNIX_VERSION) | gzip -c > $@

release: $(d)/lunix-$(LUNIX_VERSION).tgz

endif # release in MAKECMDGOALS
endif # release guard


endif # include guard

# non-recursive epilogue
d := $(dirstack_$(sp))
sp := $(basename $(sp))
