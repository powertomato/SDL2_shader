.PHONY: all clean test runtest clean_test flags builddirs

# paths
SRCPATH=src
TESTPATH=test
TESTBIN=test/bin
OBJPATH=obj
BINPATH=bin

# objects to be included in the library
OBJ=SDL_shader.o \
opengl/SDL_GL_shader.o \
d3d/SDL_D3D_shader.o \
opengles2/SDL_GLES2_shader.o
#d3d11/SDL_D3D11_shader.o \


DIRS=opengl/ \
d3d \
d3d11 \
opengles2 \
test

# objects needed for the test cases
TEST_OBJ=test/SDL_test_font.o
TEST_BIN=$(TEST_CASES:%=$(TESTBIN)/test_%$(EXE_EXT))


INCLUDE=
OBJECTS=$(OBJ:%=$(OBJPATH)/%)
TEST_OBJECTS=$(TEST_OBJ:%=$(OBJPATH)/%)
BUILDDIRS=$(DIRS:%=$(OBJPATH)/%)

ifdef NO_COLOR
CLR=
GREEN=
CYAN=
RED=
YELLOW=
else
CLR="\x1b[0m"
GREEN="\x1b[32;01m"
CYAN="\x1b[36;01m"
RED="\x1b[31;01m"
YELLOW="\x1b[33;01m"
endif


all: builddirs flags $(OBJECTS)
	$(AR) rcs libSDL2_shader.a $(OBJECTS)

flags:
	@echo -e $(YELLOW)CFLAGS = $(CFLAGS)$(CLR)
	@echo -e $(YELLOW)CPPFLAGS = $(CPPFLAGS)$(CLR)

builddirs:
	@$(MKDIR_LIST) $(BUILDDIRS)

test: $(OBJECTS) $(TEST_OBJECTS) runtest

runtest: $(TEST_BIN)
	@$(foreach TEST_CASE,$(TEST_CASES),echo -e $(GREEN)"### Testing: $(TEST_CASE) ###"$(CLR) &&  cd $(TESTBIN) && $(RUN) ./test_$(TEST_CASE)$(EXE_EXT) && cd ../.. ;)

$(OBJPATH)/%.o : $(SRCPATH)/%.cpp
	@echo "$(GPP) (CPPFLAGS) -c $< -o $@"
	@$(GPP) $(CPPFLAGS) $(INCLUDE) -c $< -o $@

$(OBJPATH)/%.o : $(SRCPATH)/%.c
	@echo "$(GCC) (CFLAGS) -c $< -o $@"
	@$(GCC) $(CFLAGS) $(INCLUDE) -c $< -o $@
	
$(TESTBIN)/test_%$(EXE_EXT) : $(TESTPATH)/%/main.c
	@echo "$(GCC) (CFLAGS) (OBJECTS) $< -o $@"
	@$(GCC) $(CFLAGS) $(INCLUDE) -I$(SRCPATH) $(OBJECTS) $(TEST_OBJECTS) $< $(LDFLAGS) -o $@
clean_test:
	$(RM) $(TEST_BIN)

clean: clean_test
	$(RM) $(OBJECTS) $(MAIN_OBJECT) $(COMPILE_OBJECT) $(TEST_OBJECTS)

