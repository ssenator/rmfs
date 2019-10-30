NAME=slu_rmfs/lib/fuseops

BUILT		?= $(shell date +%Y%m%d%H%M)

RPM_D		:= /home/sts/rmfs
RPM_SRC_D	:= $(RPM_D)/SOURCES

BASE_D		:= $(RPM_SRC_D)
INC_D		:= ../../inc


sources_fuseops	:=				\
			namer.c   		\
			rmfs_mayaccess.c   	\
			rmfs_access.c   	\
			rmfs_getattr.c  	\
			rmfs_init.c     	\
			rmfs_open.c  		\
			rmfs_poll.c  		\
			rmfs_read.c     	\
			rmfs_readdir.c  	\
			rmfs_statfs.c  		\
			rmfs_trunc.c   		\
			rmfs_write.c		\
			rmfs_xattr.c		\
			rmfs_fuseops_tab.c

sources_headers	:=				\
			rmfs.h			\
			rmfs/conf.h		\
			rmfs/param.h		\
			rmfs/rnode.h		\
			rmfs/types.h


sources		:= $(sources_fuseops)
objects		:= $(subst .c,.o,$(sources))
prereq		:= Makefile $(sources_headers)

DEFINES		:= -D_FILE_OFFSET_BITS=64 -D_GNU_SOURCE -D__COMPILE_DATE_TIME__=$(BUILT)
OUR_LIBS	:= -lmisc
#PREREQ_LIBS	:= -lbsd -lslurm -lslurmdb -lbsd -lrt -lselinux
PREREQ_LIBS	:= -lbsd -lslurm -lbsd -lrt -lselinux
LIBS		:= $(PREREQ_LIBS) $(OUR_LIBS)
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

lib_fuseops	:= librmfs.a

vpath %.h $(INC_D)

all:	$(lib_fuseops)

$(lib_fuseops): $(objects)
	$(AR) $(ARFLAGS) $@ $^

clean:
	$(RM) $(lib_fuseops) $(objects)

lint:	$(sources)
	$(LINT) $(LINTFLAGS) $^

TAGS tags:	$(sources) $(sources_headers)
	@for t in etags ctags	; \
	do			  \
		$$t *c *h	; \
	done


$(objects): $(sources) $(prereq)

.SECONDARY: $(objects)


.PHONY: clean lint XXX todo

XXX todo:
	grep --with-filename --initial-tab --line-number --after-context=2 --before-context=2 XXX $(sources)
