.PHONY: all
all:
	@echo "make md5..."
	@mkdir -p build
	@cd build && cmake28 ..
	@make -C build

.PHONY: clean
clean:
	@echo "make clean..."
	@mkdir -p build
	@cd build && cmake28 .. && make clean
