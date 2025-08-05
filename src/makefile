# Generated automatically by Visual-MinGW.
# http://visual-mingw.sourceforge.net/

CC = gcc
WRES = windres
CPPFLAGS = -D_WIN32_IE=0x0400
LDBASEFLAGS = -mwindows  -lcomctl32 -lole32
INCDIRS = -I ..\include
OPTIMIZ = -O2
STRIP = -s

ifeq ($(MAKECMDGOALS),debug)
CXXFLAGS = -W -Wall -pedantic $(INCDIRS) -g -fvtable-thunks -fno-rtti
LDFLAGS = $(LDBASEFLAGS)
else
CXXFLAGS = -W -Wall -pedantic $(INCDIRS) $(OPTIMIZ) -fvtable-thunks -fno-rtti
LDFLAGS = $(STRIP) $(LDBASEFLAGS)
endif

SRCDIR = .\src
BINDIR = ..\bin
LIBDIRS = 

%.o : %.rc
	$(WRES) $< $@

OBJS	=\
	CList.o\
	editor.o\
	main.o\
	process.o\
	project.o\
	rsrc.o\
	winui.o

TARGET =	$(BINDIR)\visual-mingw.exe

# Targets
all:	$(TARGET)

debug:	$(TARGET)

cleanobjs:
	rm -f $(OBJS)

cleanbin:
	rm -f $(TARGET)

clean:	cleanobjs cleanbin

# Dependency rules
$(BINDIR)\visual-mingw.exe: $(OBJS)
	$(CXX) -o $(BINDIR)\visual-mingw.exe $(OBJS) $(INCDIRS) $(LIBDIRS) $(LDFLAGS)

rsrc.o: rsrc.rc

CList.o: CList.cpp CList.h
editor.o: editor.cpp editor.h ../include/Scintilla.h \
 ../include/SciLexer.h winui.h CList.h rsrc.h
main.o: main.cpp process.h CList.h project.h winui.h main.h editor.h \
 ../include/Scintilla.h ../include/SciLexer.h rsrc.h
process.o: process.cpp process.h CList.h project.h winui.h main.h \
 editor.h ../include/Scintilla.h ../include/SciLexer.h rsrc.h
project.o: project.cpp project.h winui.h CList.h main.h editor.h \
 ../include/Scintilla.h ../include/SciLexer.h process.h rsrc.h
winui.o: winui.cpp winui.h CList.h
