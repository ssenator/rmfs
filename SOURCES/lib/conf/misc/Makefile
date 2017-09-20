NAME=slu_rmfs/conf/misc

BUILT		?= $(shell date +%Y%m%d%H%M)

RPM_D		:= /home/sts/sw/slurm/slurmfs
RPM_SRC_D	:= $(RPM_D)/SOURCES

BASE_D		:= $(RPM_SRC_D)
INC_D		:= ../../../inc


sources_misc	:=			\
		djb_hash.c		\
		misc_subr.c

sources_headers	:=			\
			rmfs.h		\
			rmfs/conf.h	\
			rmfs/param.h	\
			rmfs/rnode.h	\
			rmfs/misc.h	\
			rmfs/types.h


sources		:= $(sources_misc)
objects		:= $(subst .c,.o,$(sources))
prereq		:= Makefile $(sources_headers)

DEFINES		:= -D_FILE_OFFSET_BITS=64 -D_GNU_SOURCE -D__COMPILE_DATE_TIME__=$(BUILT)
LIBS		:= 
#LIBS		:= -lslurm -lslurmdb -lbsd -lrt -lselinux
INCFLAGS	+= -I. $(addprefix -I,$(INC_D))
CPPFLAGS	+= $(DEFINES) $(INCFLAGS)
WITHSYMS	:= -ggdb
WARN		:= -Wall

CFLAGS		+= $(WARN) $(WITHSYMS)
LDFLAGS		+= $(WITHSYMS)

LINT				:= splint
LINTFLAGS			:= -checks -posix-strict-lib -booltype bool_t -boolfalse BFALSE -booltrue BTRUE $(INCFLAGS) $(DEFINES)
#LINTFLAGS_STRICT_STRICT	:= -strict -posix-strict-lib $(INCFLAGS) $(DEFINES)
#LINTFLAGS_STRICT		:= -strict -posix-lib $(INCFLAGS) $(DEFINES)

vpath %.h $(INC_D)

lib_misc	:= libmisc.a

$(lib_misc): $(objects)
	$(AR) $(ARFLAGS) $@ $^
	ranlib $@

all:	$(lib_misc)
 
lint:	$(sources)
	$(LINT) $(LINTFLAGS) $^


TAGS tags:	$(sources) $(sources_headers)
	@for t in etags ctags	; \
	do			  \
		$$t *.c 	; \
	done


$(objects): $(sources) $(prereq)

.SECONDARY: $(objects)

clean:
	$(RM) $(lib_misc) $(objects)

.PHONY: clean lint XXX todo

XXX todo:
	grep --with-filename --initial-tab --line-number --after-context=2 --before-context=2 XXX $(sources)
