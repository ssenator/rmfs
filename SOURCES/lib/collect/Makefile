NAME=slu_rmfs/lib/collect

BUILT		?= $(shell date +%Y%m%d%H%M)

RPM_D		:= /home/sts/rmfs
RPM_SRC_D	:= $(RPM_D)/SOURCES

BASE_D		:= $(RPM_SRC_D)
INC_D		:= ../../inc

sources_collect	:=				\
			calcul.c		\
			collect_backingstore.c  \
			collect_cp.c		\
			collect_default.c	\
			collect_env.c		\
			collect_fuse.c		\
			collect_macconf.c	\
			collect_os.c		\
			collect_predecessor.c	\
			collect_slurm.c		\
			collect_slurm_job.c	\
			collect_slurm_node.c	\
			collect_slurm_part.c	\
			collect_slurm_step.c	\
			typ_slurm.c

sources_headers	:=				\
			rmfs.h			\
			rmfs/backingstore.h	\
			rmfs/conf.h		\
			rmfs/param.h		\
			rmfs/rnode.h		\
			rmfs/types.h


sources		:= $(sources_collect)
objects		:= $(subst .c,.o,$(sources))
prereq		:= Makefile $(sources_headers)

DEFINES		:= -D_FILE_OFFSET_BITS=64 -D_GNU_SOURCE -D__COMPILE_DATE_TIME__=$(BUILT)

INCFLAGS	+= -I. $(addprefix -I,$(INC_D))
CPPFLAGS	+= $(DEFINES) $(INCFLAGS)
WITHSYMS	:= -ggdb3
NOPIE		:= -fno-pie
PEDANTIC	:= -pedantic-errors
WARN		:= -Wall -Wextra
DYNAMIC	 	 = -rdynamic
LDFLAGS		:= $(LDFLAGS) $(WITHSYMS) $(RELOC) $(XREF) $(DYNAMIC)

CFLAGS		+= $(WARN) $(WITHSYMS)

LINT				:= splint
LINTFLAGS			:= -checks -posix-strict-lib -booltype bool_t -boolfalse BFALSE -booltrue BTRUE $(INCFLAGS) $(DEFINES)
#LINTFLAGS_STRICT_STRICT	:= -strict -posix-strict-lib $(INCFLAGS) $(DEFINES)
#LINTFLAGS_STRICT		:= -strict -posix-lib $(INCFLAGS) $(DEFINES)

lib_collect	:= libcollect.a

vpath %.h $(INC_D)

all:	$(lib_collect)

$(lib_collect): $(objects)
	$(AR) $(ARFLAGS) $@ $^
	ranlib $@

clean:
	$(RM) $(lib_collect) $(objects)

lint:	$(sources)
	$(LINT) $(LINTFLAGS) $^

tags TAGS:	$(sources) $(sources_headers)
	@for t in etags ctags	; \
	do			  \
		$$t *c 		; \
	done


$(objects): $(sources) $(prereq)

.SECONDARY: $(objects)


.PHONY: clean lint XXX todo

XXX todo:
	grep --with-filename --initial-tab --line-number --after-context=2 --before-context=2 XXX $(sources)

