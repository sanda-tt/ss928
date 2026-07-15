
target=$(shell ls -d */ | sed "s;/;;g" | grep -v "common")

target_clean=$(addsuffix _clean,$(target))

.PHONY:clean all $(target) $(target_clean)

all:$(target)
	@echo "~~~~~~~~~~~~~~Build All Sample SUCCESS~~~~~~~~~~~~~~"
clean:$(target_clean) 
	@echo "~~~~~~~~~~~~~~Clean All Sample SUCCESS~~~~~~~~~~~~~~"

$(target):
	@echo "~~~~~~~~~~Start build $@~~~~~~~~~~"
	@cd $@ && make
$(target_clean):
	@echo "~~~~~~~~~~Start clean $(subst _clean,,$@)~~~~~~~~~~"
	@cd $(subst _clean,,$@) && make clean

