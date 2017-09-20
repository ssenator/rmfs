NAME=slu_rmfs/conf/typ

BUILT		?=$(shell date +%Y%m%d%H%M)

RPM_D		:= /home/sts/sw/slurm/slurmfs
RPM_SRC_D	:= $(RPM_D)/SOURCES

BASE_D		:= $(RPM_SRC_D)
INC_D		:= ../../../inc


sources_typ	:=				\
			typ_opaque.c  		\
			typ_always_fails.c	\
			typ_alpha.c  		\
			typ_alphanum.c      	\
			typ_numericchar.c	\
			typ_xattr.c		\
			typ_dir.c   		\
			typ_file.c		\
			typ_host.c   		\
			typ_inout.c		\
			typ_numeric.c      	\
			typ_truth.c		\
			typ_path.c		\
			typ_context.c		\
			typ_slurm.c  		\
			typ_table.c		\
			typ_check.c

sources_headers	:=				\
			rmfs.h			\
			rmfs/conf.h		\
			rmfs/param.h		\
			rmfs/rnode.h		\
			rmfs/types.h


sources		:= $(sources_typ)
objects		:= $(subst .c,.o,$(sources))
prereq		:= Makefile $(sources_headers)

DEFINES		:= -D_FILE_OFFSET_BITS=64 -D_GNU_SOURCE -D__COMPILE_DATE_TIME__=$(BUILT)
LIBS		:= -lbsd -lslurm -lslurmdb
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

lib_typ	:= libtyp.a


all:	$(lib_typ)


$(lib_typ): $(objects)
	$(AR) $(ARFLAGS) $@ $^
	ranlib $@


lint:	$(sources)
	$(LINT) $(LINTFLAGS) $^

TAGS tags:	$(sources) $(sources_headers)
	@for t in etags ctags	; \
	do			  \
		$$t -R $$d	; \
	done

$(objects): $(sources) $(prereq)


.SECONDARY: $(objects)

clean:
	$(RM) $(lib_typ) $(objects)

.PHONY: clean lint XXX todo

XXX todo:
	grep --with-filename --initial-tab --line-number --after-context=2 --before-context=2 XXX $(sources)
