ifneq ($V,1)
Q ?= @
endif

#DEBUG	= -g -O0
DEBUG	= -O3
CC	= gcc
INCLUDE	= -I/usr/local/include
CFLAGS	= $(DEBUG) -Wall $(INCLUDE) -Winline -pipe -g 

LDFLAGS	= -L/usr/local/lib
LDLIBS    = -lwiringPi -lwiringPiDev -lpthread -lm -lcrypt -lrt

SRC	=	serialTest2.c							\
	
OBJ	=	$(SRC:.c=.o)

BINS	=	$(SRC:.c=)

all:	
	$Q cat README.TXT
	$Q echo "    $(BINS)" | fmt
	$Q echo ""

really-all:	$(BINS) 

serialTest2:	serialTest2.o	VM5F_Engine.o	VM5F_Cmds.o	VM5F_Ctrl.o WriteTag.o
	$Q echo [link]
	$Q $(CC) -o $@ serialTest2.o	VM5F_Engine.o	VM5F_Cmds.o	VM5F_Ctrl.o WriteTag.o $(LDFLAGS) $(LDLIBS)

#Funcions:	Funcions.o
#	$Q echo [link]
#	$Q $(CC) -o $@ Funcions.o $(LDFLAGS) $(LDLIBS)

#VM5F_Cmds:	VM5F_Cmds.o
#	$Q echo [link]
#	$Q $(CC) -o $@ VM5F_Cmds.o $(LDFLAGS) $(LDLIBS)

#VM5F_Ctrl:	VM5F_Ctrl.o
#	$Q echo [link]
#	$Q $(CC) -o $@ VM5F_Ctrl.o $(LDFLAGS) $(LDLIBS)

#VM5F_Engine:	VM5F_Engine.o
#	$Q echo [link]
#	$Q $(CC) -o $@ VM5F_Engine.o $(LDFLAGS) $(LDLIBS)
#VM5F_Cmds:	VM5F_Cmds.o
#	$Q echo [link]
#	$Q $(CC) -o $@ VM5F_Cmds.o $(LDFLAGS) $(LDLIBS)

#VM5F_Ctrl:	VM5F_Ctrl.o
#	$Q echo [link]
#	$Q $(CC) -o $@ VM5F_Ctrl.o $(LDFLAGS) $(LDLIBS)

#VM5F_Engine:	VM5F_Engine.o
#	$Q echo [link]
#	$Q $(CC) -o $@ VM5F_Engine.o $(LDFLAGS) $(LDLIBS)

.c.o:
	$Q echo [CC] $<
	$Q $(CC) -c $(CFLAGS) $< -o $@

clean:
	$Q echo "[Clean]"
	$Q rm -f $(OBJ) *~ core tags $(BINS)

tags:	$(SRC)
	$Q echo [ctags]
	$Q ctags $(SRC)

depend:
	makedepend -Y $(SRC)