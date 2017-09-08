NAME=slu_rmfs/slu_rmfs

BUILT		?= $(shell date +%Y%m%d%H%M)

RPM_D		:= /home/sts/lanl/sw/fuse/playpen/slurmfs
RPM_SRC_D	:= $(RPM_D)/SOURCES

BASE_D		:= $(RPM_SRC_D)
INC_D		:= ../inc
LIB_D		:= ../lib
EXEC_D		:= .


include	        $(BASE_D)/Makefile.depend

sources_slu_rmfs_supt	:= $(notdir $(sources_slu_rmfs_supt))
sources_slu_rmfs_main	:= $(notdir $(sources_slu_rmfs_main))
sources_slu_rmfs	:= $(notdir $(sources_slu_rmfs))
sources			:= $(notdir $(sources_slu_rmfs))

exec		:= $(subst .c,,$(sources_slu_rmfs_main))

our_libs_lowlevel := cp misc typ setval
our_libs	:= collect rmfs rnode $(our_libs_lowlevel)
our_libs_a	:= $(addsuffix .a,$(our_libs))
our_libs_lfa	:= $(addprefix $(LIB_D)/lib,$(our_libs_a))
our_libs_l	:= $(addprefix lib,$(our_libs))
our_libs_la	:= $(addprefix lib,$(our_libs_a))

prereq_libs	:= bsd fuse rt selinux slurm slurmdb
sources_libs	:= $(our_libs)
libs		:= $(prereq_libs) $(our_libs)
sources_dirs	:= $(LIB_D) $(EXEC_D)

prereq		 := Makefile

object_slu_rmfs_main	:= $(subst .c,.o,$(sources_slu_rmfs_main))
objects_slu_rmfs_supt	:= $(subst .c,.o,$(sources_slu_rmfs_supt))
objects			:= $(objects_slu_rmfs_supt)

DEFINES		:= -D_FILE_OFFSET_BITS=64 -D_GNU_SOURCE -D__COMPILE_DATE_TIME__=$(BUILT)
INCFLAGS	+= -I. $(addprefix -I,$(INC_D))

WITHSYMS	:= -ggdb
RELOC		:= --emit-relocs
XREF		:= --cref
WARN		:= -Wall

CPPFLAGS	+= $(DEFINES) $(INCFLAGS)
CFLAGS		+= $(WARN) $(WITHSYMS)

LDFLAGS		:= $(LDFLAGS) $(WITHSYMS) $(RELOC) $(XREF)
LDLIBS		:= $(addprefix -L,$(LIB_D)) $(addprefix -l,$(libs))

LINT				:= splint
LINTFLAGS			:= -checks -posix-strict-lib -booltype bool_t -boolfalse BFALSE -booltrue BTRUE $(INCFLAGS) $(DEFINES)
#LINTFLAGS_STRICT_STRICT	:= -strict -posix-strict-lib $(INCFLAGS) $(DEFINES)
#LINTFLAGS_STRICT		:= -strict -posix-lib $(INCFLAGS) $(DEFINES)

vpath %.a $(LIB_D)
vpath %.h $(INC_D)

.PHONY: clean lint XXX todo

all:	$(exec)

$(exec): $(object_slu_rmfs_main) $(objects_slu_rmfs_supt) $(our_libs_lfa)

$(object_slu_rmfs_supt): $(sources_slu_rmfs_main) $(prereq)

$(objects_slu_rmfs_supt): $(sources_slu_rmfs_supt) $(prereq)

$(our_libs): $(our_libs_lfa)

$(our_libs_lfa): $(LIB_D) 
	@$(MAKE) --no-print-directory --directory=$(LIB_D) $(notdir $@)

TAGS tags: $(sources_dirs) $(sources_headers)
	@for t in etags ctags	; \
	do			  \
		$$t *c 		; \
	done


clean:
	$(RM) $(objects_slu_rmfs_supt) $(object_slu_rmfs_main) $(exec)

lint:
	$(LINT) $(LINTFLAGS) $^

XXX todo:
	grep --with-filename --initial-tab --line-number --after-context=2 --before-context=2 XXX $(sources)