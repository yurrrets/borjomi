BORJOMI_VERSION_MJ := $(shell git describe --tags | sed -r "s/(\w+)\.(\w+).*/\1/")
BORJOMI_VERSION_MN := $(shell git describe --tags | sed -r "s/(\w+)\.(\w+).*/\2/")
BORJOMI_VERSION_REV := $(shell git rev-list  `git rev-list --tags --no-walk --max-count=1`..HEAD --count)

CPPFLAGS += -DBORJOMI_VERSION_MJ=$(BORJOMI_VERSION_MJ)
CPPFLAGS += -DBORJOMI_VERSION_MN=$(BORJOMI_VERSION_MN)
CPPFLAGS += -DBORJOMI_VERSION_REV=$(BORJOMI_VERSION_REV)

all:
	@echo Git describe: $(shell git describe --tags)
	@echo BORJOMI_VERSION: $(BORJOMI_VERSION_MJ).$(BORJOMI_VERSION_MN).$(BORJOMI_VERSION_REV)
