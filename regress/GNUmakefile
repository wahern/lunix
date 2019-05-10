# non-recursive prologue
sp := $(sp).x
dirstack_$(sp) := $(d)
d := $(abspath $(lastword $(MAKEFILE_LIST))/..)

ifeq ($(origin GUARD_$(d)), undefined)
GUARD_$(d) := 1

include $(d)/../GNUmakefile


.PHONY: $(KNOWN_APIS:%=$(d)/check%) $(d)/check check

$(d)/.local/lib/%/unix.so: $(abspath $(d)/..)/src/%/unix.so
	$(MKDIR) -p $(@D)
	cp -fp $< $@

$(KNOWN_APIS:%=$(d)/check%): $(d)/check%: $(d)/.local/lib/%/unix.so
	@V="$(@:$(@D)/check%=%)"; \
	printf "Testing $$V...\n"; \
	for T in $(@D)/[0123456789]*.lua; do \
		REGRESS_B=1 RUNLUA_R="!$$V" $$T || :; \
	done

check $(d)/check: $(LUA_APIS:%=$(d)/check%)


.PHONY: $(d)/clean $(d)/clean~

$(d)/clean:
	$(RM) -fr $(@D)/.local

$(d)/clean~: $(d)/clean
	$(RM) -f $(@D)/*~

clean: $(d)/clean

clean~: $(d)/clean~


endif # include guard

# non-recursive epilogue
d := $(dirstack_$(sp))
sp := $(basename $(sp))
