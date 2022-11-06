EXTENSION   = url
MODULES     = url
DATA        = url--1.0.sql url.control
PKG_CONFIG = pkg-config
LDFLAGS=-lrt

ifeq (no,$(shell $(PKG_CONFIG) liburiparser || echo no))
$(warning liburiparser not registed with pkg-config, build might fail)
endif

PG_CPPFLAGS += $(shell $(PKG_CONFIG) --cflags-only-I liburiparser)
SHLIB_LINK += $(shell $(PKG_CONFIG) --libs liburiparser)

PG_CONFIG ?= pg_config
PGXS = $(shell $(PG_CONFIG) --pgxs)
include $(PGXS)