DIR := 2022 2023

.PHONY: clean $(DIR)
all: $(DIR)

$(DIR):
	$(MAKE) -C $@

clean: 2022 2023
	$(MAKE) -C 2022 clean
	$(MAKE) -C 2023 clean
