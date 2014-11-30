# non-recursive prologue
sp := $(sp).x
dirstack_$(sp) := $(d)
d := $(abspath $(lastword $(MAKEFILE_LIST))/..)

ifeq ($(origin GUARD_$(d)), undefined)
GUARD_$(d) := 1


#
# E N V I R O N M E N T  C O N F I G U R A T I O N
#
include $(d)/../GNUmakefile


#
# C O M P I L A T I O N  F L A G S
#
LUAPATH_$(d) = $(shell env CC="$(CC)" CPPFLAGS="$(CPPFLAGS)" LDFLAGS="$(LDFLAGS)" $(<D)/../mk/luapath -krxm3 -I$(DESTDIR)$(includedir) -I/usr/include -I/usr/local/include -P$(DESTDIR)$(bindir) -P$(bindir) -L$(DESTDIR)$(libdir) -L$(libdir) -v$(1) $(2))

CPPFLAGS_$(d) = $(CPPFLAGS_$(abspath $(@D)/../..))
CFLAGS_$(d) = $(CFLAGS_$(abspath $(@D)/../..))
LDFLAGS_$(d) = $(LDFLAGS_$(abspath $(@D)/../..))
SOFLAGS_$(d) = $(SOFLAGS_$(abspath $(@D)/../..))

LDFLAGS_$(d) += -L$(DESTDIR)$(libdir) -L$(libdir)

ifeq ($(shell uname -s), Linux)
LDFLAGS_$(d) += -lrt
endif

ifeq ($(shell uname -s), SunOS)
LDFLAGS_$(d) += -lsocket -lnsl
endif

LDFLAGS_$(d) += -lm


#
# C O M P I L A T I O N  R U L E S
#
define BUILD_$(d)

$$(d)/$(1)/%.so: $$(d)/%.c
	@test '$$(notdir $$(@D))' = '$$(call LUAPATH_$$(<D), $$(notdir $$(@D)), version)' \
	|| (printf '%s'"\n" 'Unable to find Lua $(1) API headers (CPPFLAGS = $$(strip $$(call LUAPATH_$$(<D), $$(notdir $$(@D)), cppflags) $(CPPFLAGS)))' >&2; false)
	$$(MKDIR) -p $$(@D)
	$$(CC) $$(CFLAGS_$$(<D)) $$(CFLAGS) $$(call LUAPATH_$$(<D), $$(notdir $$(@D)), cppflags) $$(CPPFLAGS_$$(<D)) $$(CPPFLAGS) -o $$@ $$< $$(SOFLAGS_$$(abspath $$(@D)/..)) $$(SOFLAGS) $$(LDFLAGS_$$(abspath $$(@D)/..)) $$(LDFLAGS)

.SECONDARY: liblua$(1)-unix unix$(1)

liblua$(1)-unix unix$(1): $$(d)/$(1)/unix.so

endef # BUILD_$(d)

$(eval $(call BUILD_$(d),5.1))

$(eval $(call BUILD_$(d),5.2))

$(eval $(call BUILD_$(d),5.3))


#
# I N S T A L L  &  U N I N S T A L L  R U L E S
#
define INSTALL_$(d)

LUAC$(1)_$(d) = $$(or $$(call LUAPATH_$(d), $(1), luac), true)

MODS$(1)_$(d) = $$(DESTDIR)$(2)/unix.so

.SECONDARY: liblua$(1)-unix-install unix$(1)-install unix-install

liblua$(1)-unix-install unix$(1)-install unix-install: $$(MODS$(1)_$$(d))

$$(DESTDIR)$(2)/%.so: $$(d)/$(1)/%.so
	$$(MKDIR) -p $$(@D)
	cp -fp $$< $$@

$$(DESTDIR)$(3)/%.lua: $$(d)/%.lua
	$$(LUAC$(1)_$(d)) -p $$<
	$$(MKDIR) -p $$(@D)
	cp -p $$< $$@

.PHONY: liblua$(1)-unix-uninstall unix$(1)-uninstall unix-uninstall

unix-uninstall: unix$(1)-uninstall

liblua$(1)-unix-uninstall unix$(1)-uninstall:
	$$(RM) -f $$(MODS$(1)_$(d))

endef # INSTALL_$(d)

$(eval $(call INSTALL_$(d),5.1,$$(lua51cpath),$$(lua51path)))

$(eval $(call INSTALL_$(d),5.2,$$(lua52cpath),$$(lua52path)))

$(eval $(call INSTALL_$(d),5.3,$$(lua53cpath),$$(lua53path)))


#
# L O C A L  R U L E S
#
ifneq "$(filter $(abspath $(d)/..)/%, $(abspath $(firstword $(MAKEFILE_LIST))))" ""
.PHONY: all5.1 all5.2 all5.3 all

all5.1: liblua5.1-unix

all5.2: liblua5.2-unix

all5.3: liblua5.3-unix

all: all5.1 all5.2

.SECONDARY: install5.1 install5.2 install5.3 install

install5.1: liblua5.1-unix-install

install5.2: liblua5.2-unix-install

install5.3: liblua5.3-unix-install

install: install5.1 install5.2

.PHONY: uninstall5.1 uninstall5.2 uninstall5.3 uninstall

uninstall5.1: liblua5.1-unix-uninstall

uninstall5.2: liblua5.2-unix-uninstall

uninstall5.3: liblua5.3-unix-uninstall

uninstall: uninstall5.1 uninstall5.2

endif


#
# C L E A N  R U L E S
#
.PHONY: $(d)/clean $(d)/clean~ clean clean~

$(d)/clean:
	rm -rf $(@D)/errno.c $(@D)/*.dSYM $(@D)/5.1 $(@D)/5.2 $(@D)/5.3

$(d)/clean~: $(d)/clean
	rm -f $(@D)/*~

clean: $(d)/clean

clean~: $(d)/clean~


endif # include guard

# non-recursive epilogue
d := $(dirstack_$(sp))
sp := $(basename $(sp))
