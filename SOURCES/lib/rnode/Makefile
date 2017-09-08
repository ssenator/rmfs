NAME=slu_rmfs/lib/rnode

BUILT		?= $(shell date +%Y%m%d%H%M)

RPM_D		:= /home/sts/lanl/sw/fuse/playpen/slurmfs
RPM_SRC_D	:= $(RPM_D)/SOURCES

BASE_D		:= $(RPM_SRC_D)
INC_D		:= ../../inc


sources_rnode	:=				\
			rn_README.c		\
			rmfs_param.c   		\
			rn_alloc.c		\
			rn_provision.c		\
			rn_cast.c		\
			rn_mkfsroot.c		\
			rn_mkcname.c		\
			rn_mkpart.c		\
			rn_mknode.c		\
			rn_mkattr.c		\
			rn_mkctl.c		\
			rn_mkjob.c		\
			rn_mkstep.c		\
			rn_public.c		\
			rn_subr.c		\
			rn_tab.c		\
			rn_ctlroot.c		\
			rn_ctljobid.c

sources_headers	:=				\
			rmfs.h			\
			rmfs/backingstore.h	\
			rmfs/conf.h		\
			rmfs/param.h		\
			rmfs/rnode.h		\
			rmfs/types.h


sources		:= $(sources_rnode)
objects		:= $(subst .c,.o,$(sources))
prereq		:= Makefile $(sources_headers)

DEFINES		:= -D_FILE_OFFSET_BITS=64 -D_GNU_SOURCE -D__COMPILE_DATE_TIME__=$(BUILT)
LIBS		:= -lbsd -lslurm -lslurmdb -lbsd -lrt -lselinux
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

lib_rnode	:= librnode.a

vpath %.h $(INC_D)

all:	$(lib_rnode)

$(lib_rnode): $(objects)
	$(AR) $(ARFLAGS) $@ $^

clean:
	$(RM) $(lib_rnode) $(objects)

lint:	$(sources)
	$(LINT) $(LINTFLAGS) $^

TAGS tags:	$(sources) $(sources_headers)
	@for t in etags ctags	; \
	do			  \
		$$t *c 		; \
	done


$(objects): $(sources) $(prereq)

.SECONDARY: $(objects)

.PHONY: clean lint XXX todo

XXX todo:
	grep --with-filename --initial-tab --line-number --after-context=2 --before-context=2 XXX $(sources)

