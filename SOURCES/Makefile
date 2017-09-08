NAME=slu_rmfs

BUILT		:= $(shell date +%Y%m%d%H%M)

RPM_D		:= /home/sts/lanl/sw/fuse/playpen/slurmfs
RPM_SRC_D	:= $(RPM_D)/SOURCES

BASE_D		:= $(RPM_SRC_D)
INC_D		:= inc
LIB_D		:= lib
EXEC_D		:= slu_rmfs

LN		:= ln
LNFLAGS		:= -f -v

include  $(BASE_D)/Makefile.depend

exec		:= $(EXEC_D)/slu_rmfs
target		:= slurmfs
prereq		:= $(sources_depend) $(BASE_D)/Makefile $(BASE_D)/Makefile.depend

#XXX manipulate sources_depend & its components $(dir ...), $(notdir ...)
sources_dirs	:= $(LIB_D) $(EXEC_D)

vpath %.h $(INC_D)

.PHONY: clean lint todo hygiene XXX

all:	$(target) hygiene


$(target):	$(exec) $(prereq)
	$(LN) $(LNFLAGS) $(exec) $(target)

$(exec): $(prereq)
	@$(MAKE) --no-print-directory --directory=$(EXEC_D) $(notdir $@)

$(sources_dirs): $(prereq)
	@$(MAKE) --no-print-directory --directory=$@ $(TARGET)

hygiene: cscope.out TAGS tags


cscope.out: $(sources_depend)
	cscope -I$(inc) -q -R -b $(sources_depend)

TAGS: $(sources_depend)
	etags $^

tags:	$(sources_depend)
	ctags -R $(BASE_D)


clean: $(sources_dirs)
	@$(RM) $(target) tags TAGS cscope.out
	@for d in $^						; \
	do							  \
		$(MAKE) --no-print-directory --directory=$$d $@	; \
	done

lint: $(sources_dirs)
	$(LINT) $(LINTFLAGS) $^


todo XXX: $(sources_dirs)
	grep --with-filename --initial-tab --line-number --after-context=2 --before-context=2 XXX $(sources)