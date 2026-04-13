CC        := clang
CFLAGS    := -std=c23 -framework Carbon -Wall -Wextra -Wshadow -Wfloat-equal -Werror
BUILD_DIR := build

.PHONY: all
all: $(BUILD_DIR)/ime

.PHONY: clean
clean:
	rm -rf $(BUILD_DIR)

.PHONY: test
test: $(BUILD_DIR)/ime
	@bash test.sh $(BUILD_DIR)/ime

$(BUILD_DIR)/ime: main.c | $(BUILD_DIR)
	@$(CC) -o $@ $< $(CFLAGS) -O2 -flto -DNDEBUG

$(BUILD_DIR):
	@mkdir -p $@
