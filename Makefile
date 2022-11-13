#
# Call ordered depency make commands
#

# Save command line option for child
ARD_HOME_ARGU = ${ARD_HOME}

# Optional include an local override for arduino home
-include lib-build/make/Makefile.inc.local

# Safety check
ifndef ARD_HOME
	$(error $$(ARD_HOME) not defined)
endif

# Only pass to child if defined from command line.
ifndef ARD_HOME_ARGU
	ARD_HOME_ARGU_CHILD = ARD_HOME=${ARD_HOME}
endif

# Locate and check for avr-size
AVR_SIZE ?= $(ARD_HOME)/hardware/tools/avr/bin/avr-size
ifeq ("$(wildcard $(AVR_SIZE))","")
	$(error $$(AVR_SIZE) not found)
endif

# Define all projects to build
PROJECTS = \
xnode-base \
xnode-mega-flash \
xnode-satellite \
xnode-test-blink

# Hook all to projects
all: projects

# Declare subprojects targets
.PHONY: $(PROJECTS)
projects: $(PROJECTS) projects-result
$(PROJECTS):
	$(MAKE) -s -C $@ clean all ${ARD_HOME_ARGU_CHILD}
projects-result:
	@echo
	@echo "Full xnode build is ready to burn."
	@echo
	@echo "Binary program sizes;"
	${AVR_SIZE} xnode*/build/*.hex
	@echo
	@echo "Done."

# Declare subprojects depencies
xnode-mega-flash:       xnode-satellite xnode-base xnode-test-blink
