EXTENSION   = url
MODULES     = url
DATA        = url--1.0.sql url.control # script files to install

LDFLAGS=-lrt

PG_CONFIG ?= pg_config
PGXS = $(shell $(PG_CONFIG) --pgxs)
include $(PGXS)
