NAME=slu_rmfs/conf/typ

BUILT		?= $(shell date +%Y%m%d%H%M)

RPM_D		:= /home/sts/rmfs
RPM_SRC_D	:= $(RPM_D)/SOURCES

BASE_D		:= $(RPM_SRC_D)
INC_D		:= ../../../inc


sources_set_val	:=				\
			set_val_bool.c		\
			set_val_charptr.c	\
			set_val_l.c		\
			set_val_pid.c		\
			set_val_ptr.c		\
			set_val_truth.c		\
			set_val_ui.c		\
			set_val_uid.c		\
			set_val_ul.c


sources_headers	:=				\
			rmfs.h			\
			rmfs/conf.h		\
			rmfs/param.h		\
			rmfs/rnode.h		\
			rmfs/types.h


sources		:= $(sources_set_val)
objects		:= $(subst .c,.o,$(sources))
prereq		:= Makefile $(sources_headers)

DEFINES		:= -D_FILE_OFFSET_BITS=64 -D_GNU_SOURCE -D__COMPILE_DATE_TIME__=$(BUILT)
LIBS		:= -lbsd -lslurm
#LIBS		:= -lslurm -lslurmdb -lbsd -lrt -lselinux
INCFLAGS	+= -I. $(addprefix -I,$(INC_D))
CPPFLAGS	+= $(DEFINES) $(INCFLAGS)

WITHSYMS	:= -ggdb3
NOPIE		:= -fno-pie

PEDANTIC	:= -pedantic-errors
WARN		:= -Wall -Wextra
DYNAMIC	 	 = -rdynamic
LDFLAGS		:= $(LDFLAGS) $(WITHSYMS) $(RELOC) $(XREF) $(DYNAMIC)
CFLAGS		:= $(CFLAGS) $(WITHSYMS) $(WARN)

LINT				:= splint
LINTFLAGS			:= -checks -posix-strict-lib -booltype bool_t -boolfalse BFALSE -booltrue BTRUE $(INCFLAGS) $(DEFINES)
#LINTFLAGS_STRICT_STRICT	:= -strict -posix-strict-lib $(INCFLAGS) $(DEFINES)
#LINTFLAGS_STRICT		:= -strict -posix-lib $(INCFLAGS) $(DEFINES)

vpath %.h $(INC_D)

lib_setval	:= libsetval.a

$(lib_setval): $(objects)
	$(AR) $(ARFLAGS) $@ $^
	ranlib $@

all:	$(lib_setval)
 
lint:	$(sources)
	$(LINT) $(LINTFLAGS) $^

TAGS tags:	$(sources) $(sources_headers)
	@for t in etags ctags	; \
	do			  \
		$$t *c 		; \
	done


$(objects): $(sources) $(prereq)

.SECONDARY: $(objects)

clean:
	$(RM) $(lib_setval) $(objects)

.PHONY: clean lint XXX todo

XXX todo:
	grep --with-filename --initial-tab --line-number --after-context=2 --before-context=2 XXX $(sources)
