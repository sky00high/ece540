TARGET =	project
LIBS =		-lsimple -lcheck -lsuif -luseful
CSRCS = print.c
HEADERS = print.h
OBJS = LICM.o print.o UDChain.o  DFA.o Expression.o ae.o	DFG.o	doproc.o main.o cfg.o BitVector.o RD.o Definition.o
EXTRA_CFLAGS =	$(INCLDIRFLAG)

all:            $(TARGET)
install-bin:    install-prog

include $(SUIFHOME)/Makefile.std

