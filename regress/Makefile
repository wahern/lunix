V = 5.1 5.2 5.3

check:
	@for V in $(V); do \
		printf "Testing $${V}...\n"; \
		REGRESS_R="$${V}" ./regress.sh || continue; \
		export RUNLUA_R="!$$V"; \
		for T in ./[0123456789]*.lua; do \
			REGRESS_B=1 ./$$T || :; \
		done; \
	done
