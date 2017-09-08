NAME=slu_rmfs/conf

BUILT		?= $(shell date +%Y%m%d%H%M)

RPM_D		:= /home/sts/lanl/sw/fuse/playpen/slurmfs
RPM_SRC_D	:= $(RPM_D)/SOURCES

BASE_D		:= $(RPM_SRC_D)
INC_D		:= ../inc

LN		:= ln
LNFLAGS		:= -f -v

#XXX use Makefile.depend && Makefile.common

sources_headers	:=			\
			rmfs.h		\
			rmfs/conf.h	\
			rmfs/misc.h	\
			rmfs/param.h	\
			rmfs/rnode.h	\
			rmfs/types.h


lib_cp		:=	libcp.a
lib_conf_dir	:=	conf
lib_cp_dir	:=	$(lib_conf_dir)/cp
lib_cp_path	:=	$(lib_cp_dir)/$(lib_cp)

lib_typ		:=	libtyp.a
lib_typ_dir	:=	$(lib_conf_dir)/typ
lib_typ_path	:=	$(lib_typ_dir)/$(lib_typ)

lib_misc	:=	libmisc.a
lib_misc_dir	:=	$(lib_conf_dir)/misc
lib_misc_path	:=	$(lib_misc_dir)/$(lib_misc)

lib_setval	:=	libsetval.a
lib_setval_dir	:=	$(lib_conf_dir)/setval
lib_setval_path	:=	$(lib_setval_dir)/$(lib_setval)

lib_collect	:=	libcollect.a
lib_collect_dir	:=	collect
lib_collect_path :=	$(lib_collect_dir)/$(lib_collect)

lib_rmfs	:=	librmfs.a
lib_rmfs_dir	:=	fuseops
lib_rmfs_path	:=	$(lib_rmfs_dir)/$(lib_rmfs)

lib_rnode	:=	librnode.a
lib_rnode_dir	:=	rnode
lib_rnode_path	:=	$(lib_rnode_dir)/$(lib_rnode)

sources_libs	:=	$(lib_cp_path)		\
			$(lib_typ_path)		\
			$(lib_misc_path)	\
			$(lib_setval_path)	\
			$(lib_collect_path)	\
			$(lib_rmfs_path)	\
			$(lib_rnode_path)

libs		:=	$(lib_cp) $(lib_typ) $(lib_misc) $(lib_setval)	\
			$(lib_collect)					\
			$(lib_rmfs)					\
			$(lib_rnode)

lib_dirs	:=	$(lib_cp_dir) $(lib_typ_dir) $(lib_misc_dir) $(lib_setval_dir)	\
			$(lib_collect_dir)						\
			$(lib_rmfs_dir)							\
			$(lib_rnode_dir)

sources_dirs	:=	$(lib_dirs)
prereq		:=	Makefile $(sources_headers)

vpath %.h $(INC_D)

.PHONY: clean lint todo XXX $(sources_dir) tags TAGS

all:	$(libs)


$(lib_cp): $(prereq)
	@$(MAKE) --no-print-directory --directory=$(dir $(lib_cp_path)) $@
	@$(LN) $(LNFLAGS) $(lib_cp_path) .

$(lib_typ): $(prereq)
	@$(MAKE) --no-print-directory --directory=$(dir $(lib_typ_path)) $@
	@$(LN) $(LNFLAGS) $(lib_typ_path) .

$(lib_misc): $(prereq)
	@$(MAKE) --no-print-directory --directory=$(dir $(lib_misc_path)) $@
	@$(LN) $(LNFLAGS) $(lib_misc_path) .

$(lib_setval): $(prereq)
	@$(MAKE) --no-print-directory --directory=$(dir $(lib_setval_path)) $@
	@$(LN) $(LNFLAGS) $(lib_setval_path) .

$(lib_collect): $(prereq)
	@$(MAKE) --no-print-directory --directory=$(dir $(lib_collect_path)) $@
	@$(LN) $(LNFLAGS) $(lib_collect_path) .

$(lib_rmfs): $(prereq)
	@$(MAKE) --no-print-directory --directory=$(dir $(lib_rmfs_path)) $@
	@$(LN) $(LNFLAGS) $(lib_rmfs_path) .

$(lib_rnode): $(prereq)
	@$(MAKE) --no-print-directory --directory=$(dir $(lib_rnode_path)) $@
	@$(LN) $(LNFLAGS) $(lib_rnode_path) .

tags TAGS: $(lib_dirs)
	@for d in $^							;	\
	do									\
		if [ -d $$d ] ; then						\
			$(MAKE) --no-print-directory --directory=$$d $@	;	\
		fi							;	\
	done

clean lint todo XXX: $(sources_dirs)
	@for d in $^						   ;	\
	do								\
		$(MAKE) --no-print-directory --directory=$$d $@ ;	\
	done
	@$(RM) $(libs)
