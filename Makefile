
######################################################
# Compiler and libraries
CXX := g++
CC  := gcc

SQLITE_LIB=-lsqlite3

LD_EXTRA_OPTIONS += -pthread -lgeos_c
#LD_EXTRA_OPTIONS += -ldl -static-libgcc -static-libstdc++

CXX_EXTRA_OPTIONS += 

######################################################

APPNAME := wkb2twkb-sqlite
LWGEOMNAME := liblwgeom.a

SRCSUBDIR := src
OBJSUBDIR := obj

INCLUDE  = -Ithirdparty/sqlite3pp/headeronly_src -I$(SRCSUBDIR) -Iliblwgeom/src
LIBRARIES += $(SQLITE_LIB) 

OBJS	= $(patsubst $(SRCSUBDIR)/%.cpp,$(OBJSUBDIR)/%.o,$(wildcard $(SRCSUBDIR)/*.cpp))

OBJS_LWGEOM = $(patsubst liblwgeom/src/%.c,$(OBJSUBDIR)/liblwgeom_%.o,$(wildcard liblwgeom/src/*.c))

CXX_EXTRA_OPTIONS += -std=c++11
CXXFLAGS := -O2 -g $(EXTRA_OPTIONS) $(CXX_EXTRA_OPTIONS) $(INCLUDE)
CFLAGS := -O2 -g $(INCLUDE) 

AR       = ar 
LD	 = g++ 

all: $(OBJSUBDIR) $(LWGEOMNAME) $(APPNAME) 

clean:
	rm -rf core* $(APPNAME) $(OBJSUBDIR) $(LWGEOMNAME) 

$(APPNAME): $(OBJS) $(LWGEOMNAME) 
	@echo
	@echo "--------- LINKING --- $@ "
	rm -f $(APPNAME)
	$(LD) -o $@ $^ $(LIBRARIES) $(LD_EXTRA_OPTIONS)
	@echo
	@echo '--------- Make done '
	@echo

$(LWGEOMNAME): $(OBJS_LWGEOM)
	rm -f $(LWGEOMNAME)
	$(AR) crvl $@ $^ 

$(OBJSUBDIR):
	@echo
	@echo "--------- Making dir: $@ "
	mkdir -p $(OBJSUBDIR)
	@echo

$(OBJSUBDIR)/%.o: $(SRCSUBDIR)/%.cpp 
	@echo
	@echo "------------ $< "
	$(CXX) -c $(CPPFLAGS) $(CXXFLAGS) -o $@ $<
	@echo

$(OBJSUBDIR)/liblwgeom_%.o: liblwgeom/src/%.c 
	@echo
	@echo "------------ $< "
	$(CC) -c $(CFLAGS) -o $@ $<
	@echo

