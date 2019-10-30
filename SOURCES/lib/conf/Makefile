NAME=slu_rmfs/conf

BUILT		?= $(shell date +%Y%m%d%H%M)

RPM_D		:= /home/sts/rmfs
RPM_SRC_D	:= $(RPM_D)/SOURCES

BASE_D		:= $(RPM_SRC_D)
INC_D		:= ../../inc

sources_headers	:=			\
			rmfs.h		\
			rmfs/conf.h	\
			rmfs/misc.h	\
			rmfs/param.h	\
			rmfs/rnode.h	\
			rmfs/types.h

lib_cp		:= cp/libcp.a
lib_typ 	:= typ/libtyp.a
lib_misc 	:= misc/libmisc.a
lib_setval 	:= setval/libsetval.a


sources_libs	:=	$(lib_cp) $(lib_typ) $(lib_misc) $(lib_setval)
sources_dirs	:=	$(dir $(sources_libs))
prereq		:=	Makefile $(sources_headers)

vpath %.h $(INC_D)

all:	$(sources_libs)


$(lib_cp): $(prereq)
	@$(MAKE) --no-print-directory --directory=$(dir $@) $(notdir $@)

$(lib_typ): $(prereq)
	@$(MAKE) --no-print-directory --directory=$(dir $@) $(notdir $@)

$(lib_misc): $(prereq)
	@$(MAKE) --no-print-directory --directory=$(dir $@) $(notdir $@)

$(lib_setval): $(prereq)
	@$(MAKE) --no-print-directory --directory=$(dir $@) $(notdir $@)

tags TAGS: $(lib_dirs)
	@$(MAKE) --no-print-directory --directory=$^ $@


lint clean todo XXX:	$(sources_dirs)
	$(foreach d, $(sources_dirs),	\
	 	$(MAKE) --no-print-directory --directory=$d $(notdir $@);)

