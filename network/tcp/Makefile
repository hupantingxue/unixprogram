.PHONY: all
all:
	@echo "make tcp..."
	@mkdir -p build
	@cd build && cmake28 ..
	@make -C build

.PHONY: clean
clean:
	@echo "make clean..."
	@mkdir -p build
	@cd build && cmake28 .. && make clean
