#####REFERENCE#####
#
#  Automatic Variables:
#  target : dependencies
#       commands
#
#  $^ means all of the dependencies
#  $@ means the target
#  $< means the first dependency
#
#  patsubst
#  $(patsubst pattern,replacement,text)
#  Finds whitespace-separated words in text that match pattern and replaces them with replacement
#####END REFERENCE#####

CC = g++

#-ggdb compiles with debug symbols
#-mwindows compiles without terminal
#CFLAGS = -Wall -Wextra -Wshadow -ggdb -O0 -g
CFLAGS = -O3 -std=c++17 -fno-common -g
#LINKERS = -lmingw32 -lglfw3 -lgdi32 -lvulkan-1 
LINKERS = -lglfw3 -lgdi32 -lvulkan-1 


SRCDIR = src
OBJDIR = obj
BINDIR = bin
HDRDIR = include

STB_INCLUDE_PATH =  D:\Github\VulkanTutalong\external	

SRCS = $(wildcard $(SRCDIR)/*.cpp)

HEADERS = $(wildcard $(HDRDIR)/*.hpp)

OBJFILES = $(patsubst $(SRCDIR)/%.cpp,$(OBJDIR)/%.o,$(SRCS))

SRCFILES = $(patsubst $(SRCDIR)/%,%,$(SRCS))
HEADERFILES = $(patsubst $(HDRDIR)/%,%,$(HEADERS))

EXENAME = VulkanTry

INCLUDES = -Iinclude                                                     \
		   -I$(STB_INCLUDE_PATH)										 \
		   -IC:\glfw-3.3.6\include										 \
		   -IC:\VulkanSDK\1.3.211.0\Include								 \
		   -IC:\VulkanSDK\1.3.211.0\Third-Party\Include					 \

LIBS = -LC:\glfw-3.3.6\lib-mingw-w64					\
	   -LC:\VulkanSDK\1.3.211.0\Lib						\

#	   -LC:\VulkanSDK\1.2.198.1\Lib						\

#Need to put the linkers at the end of the call
$(BINDIR)/$(EXENAME): $(OBJFILES)
	@echo cccc
	$(CC) $(CFLAGS) $(LIBS) $^ -o $@ $(LINKERS)

#Note the -c tells the compiler to create obj files
#$(OBJDIR)/%.o: $(SRCS) $(HEADERS)
#	$(CC) $(CFLAGS) -c $^ -o $@ $(INCLUDES)

$(OBJDIR)/%.o: $(SRCDIR)/%.cpp $(HDRDIR)/%.hpp
	$(CC) $(CFLAGS) -c $< -o $@ $(INCLUDES)

#Makes it so that if these files exist, it won't mess up Makefile
.PHONY: clean clearScreen all

clean:
	rm -f $(OBJDIR)/*.o
	rm -f $(BINDIR)/$(EXENAME)

#	For If only using command prompt
#	del $(OBJDIR)\*.o
#	del $(BINDIR)\$(EXENAME)

clearScreen:
	clear

#This target will clean, clearscreen, then make project
all: clean clearScreen $(BINDIR)/$(EXENAME)


#This target prints out variable names, just type:
#make print-VARIABLE
print-%  : ; @echo $* = $($*)