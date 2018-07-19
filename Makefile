SHELL := /bin/bash
MY_CCFLAGS += -std=gnu99 -fms-extensions
ccflags-y += ${MY_CCFLAGS}
obj-m += procfs.o

.PHONY: rebuild
rebuild: clean test-module disassemble

test-module : ccflags-y +=  -g -DDEBUG -fno-inline
test-module :
	@echo ${ccflags-y}
	$(MAKE) -C /lib/modules/$(shell uname -r)/build M=$(shell pwd) modules

clean:
		$(MAKE) -C /lib/modules/$(shell uname -r)/build M=$(shell pwd) clean
		rm *.o.asm &> /dev/null ||:

module : ccflags-y +=  -02
module :
	@echo ${ccflags-y}
	$(MAKE) -C /lib/modules/$(shell uname -r)/build M=$(shell pwd) modules


.PHONY: lint
lint:
	find . -name "*.c"  -exec cppcheck --force {} \;
	find . -name "*.h"  -exec cppcheck --force {} \;

.PHONY: pretty
pretty:
	find . -name "*.c"  -exec indent -linux {} \;
	find . -name "*.h"  -exec indent -linux {} \;

.PHONY: trim
trim:
	find ./ -name "*.c" | xargs ttws.sh
	find ./ -name "*.h" | xargs ttws.sh
	find ./ -name "*.py" | xargs ttws.sh
	find ./ -name "Makefile" | xargs ttws.sh

.PHONY: disassemble
disassemble:
	find ./ -name "*.o" | xargs dis.sh
