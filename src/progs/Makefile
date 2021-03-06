#    Copyright (C) 2014 University of Southern California,
#                       Andrew D. Smith and Ehsan Behnam
#
#    Authors: Andrew D. Smith
#
#    This program is free software: you can redistribute it and/or modify
#    it under the terms of the GNU General Public License as published by
#    the Free Software Foundation, either version 3 of the License, or
#    (at your option) any later version.
#
#    This program is distributed in the hope that it will be useful,
#    but WITHOUT ANY WARRANTY; without even the implied warranty of
#    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#    GNU General Public License for more details.
#
#    You should have received a copy of the GNU General Public License
#    along with this program.  If not, see <http://www.gnu.org/licenses/>.
#

PROGS = build_graph_naively naive_batch_query \
				naive_batch_insert naive_batch_delete\
				normalize_feature_vector normalize_features compute_normalizers \
				generate_hash_function populate_hash_table build_graph \
				generate_euclidean_hash_function \
				amordad_batch_query \
				amordad_batch_insert \
				amordad_batch_delete \
				amordad_batch_refresh \
				simulate_feature_vector test_crow\
				amordad test_disk test_db
#
# PROGS = test
ifndef SMITHLAB_CPP
	SMITHLAB_CPP = ../smithlab_cpp
endif

COMMON = ../common
MYSQL = /usr/include/mysql
CROW = ../crow/include

INCLUDEDIRS = $(SMITHLAB_CPP) $(COMMON)
INCLUDEDIRS += $(BOOST)
INCLUDEDIRS += $(MYSQL)
INCLUDEDIRS += $(CROW)
INCLUDEARGS = $(addprefix -I,$(INCLUDEDIRS))

CXX = g++
CXXFLAGS = -Wall -fmessage-length=50 -std=c++11
OPTFLAGS = -O3
DEBUGFLAGS = -g -pg
LIBS = -lgsl -lgslcblas -lmysqlpp -lboost_system -lboost_thread -lpthread

# Flags for Mavericks
ifeq "$(shell uname)" "Darwin"
CXXFLAGS += -arch x86_64
ifeq "$(shell if [ `sysctl -n kern.osrelease | cut -d . -f 1` -ge 13 ];\
              then echo 'true'; fi)" "true"
CXXFLAGS += -stdlib=libstdc++
else
# Is OMP only implemented for non-Mavericks?
CXXFLAGS += -DHAVE_OMP
LIBS += -fopenmp
endif
endif

ifdef DEBUG
CXXFLAGS += $(DEBUGFLAGS)
endif

ifdef OPT
CXXFLAGS += $(OPTFLAGS)
endif

all: $(PROGS)

generate_hash_function : $(addprefix $(COMMON)/, LSHAngleHashFunction.o)

generate_euclidean_hash_function : $(addprefix $(COMMON)/, LSHEuclideanHashFunction.o)

populate_hash_table : $(addprefix $(COMMON)/, LSHAngleHashTable.o \
	LSHAngleHashFunction.o LSHEuclideanHashFunction.o)

build_graph : $(addprefix $(COMMON)/, RegularNearestNeighborGraph.o \
	LSHAngleHashTable.o)

build_graph_naively : $(addprefix $(COMMON)/, RegularNearestNeighborGraph.o)

amordad_batch_refresh : $(addprefix $(COMMON)/, RegularNearestNeighborGraph.o \
	LSHAngleHashTable.o)

amordad_batch_query : $(addprefix $(COMMON)/, RegularNearestNeighborGraph.o \
	LSHAngleHashTable.o LSHAngleHashFunction.o)

amordad_batch_insert : $(addprefix $(COMMON)/, RegularNearestNeighborGraph.o \
	LSHAngleHashTable.o LSHAngleHashFunction.o)

amordad_batch_delete : $(addprefix $(COMMON)/, RegularNearestNeighborGraph.o \
	LSHAngleHashTable.o LSHAngleHashFunction.o)

amordad : $(addprefix $(COMMON)/, RegularNearestNeighborGraph.o \
	LSHAngleHashTable.o LSHAngleHashFunction.o EngineDB.o)

test_disk : $(addprefix $(COMMON)/, RegularNearestNeighborGraph.o \
	LSHAngleHashTable.o LSHAngleHashFunction.o EngineDB.o)

test_db : $(addprefix $(COMMON)/, RegularNearestNeighborGraph.o \
	LSHAngleHashTable.o LSHAngleHashFunction.o EngineDB.o)


$(PROGS): $(addprefix $(SMITHLAB_CPP)/, smithlab_os.o \
	smithlab_utils.o OptionParser.o) \
	$(addprefix $(COMMON)/, FeatureVector.o)

%.o: %.cpp %.hpp
	$(CXX) $(CXXFLAGS) -c -o $@ $< $(INCLUDEARGS)

%: %.cpp
	$(CXX) $(CXXFLAGS) -o $@ $^ $(INCLUDEARGS) $(LIBS)

install: all
	@mkdir -p ../../bin
	@install -m 755 $(PROGS) ../../bin

clean:
	@-rm -f $(PROGS) *.o *.so *.a

.PHONY: clean
