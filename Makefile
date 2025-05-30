TARGET_TRIPLET ?= x86_64-elf
CC = $(TARGET_TRIPLET)-gcc
AS = nasm
LD = $(TARGET_TRIPLET)-ld
OBJCOPY = $(TARGET_TRIPLET)-objcopy
QEMU = qemu-system-x86_64

ROOT_DIR = .
ARCH_DIR = $(ROOT_DIR)/arch
NUCLEUS_DIR = $(ROOT_DIR)/nucleus
LIB_DIR = $(ROOT_DIR)/lib
INCLUDE_DIR = $(ROOT_DIR)/include
LIMINE_FILES_DIR = $(ROOT_DIR)/limine_files
BUILD_DIR = $(ROOT_DIR)/build
OBJ_DIR = $(BUILD_DIR)/obj
DIST_DIR = $(BUILD_DIR)/dist
ISO_ROOT_DIR = $(DIST_DIR)/iso_root
FONT_PSF_SRC = $(ROOT_DIR)/resources/font.psf
FONT_PSF_OBJ = $(OBJ_DIR)/font.o

ASM_SOURCES = $(wildcard $(ARCH_DIR)/*/*/*.s) $(wildcard $(ARCH_DIR)/*/*/*/*.s)

C_SOURCES_NUCLEUS = $(wildcard $(NUCLEUS_DIR)/*.c) \
                    $(wildcard $(NUCLEUS_DIR)/*/*.c) \
					$(wildcard $(NUCLEUS_DIR)/*/*/*.c)
C_SOURCES_ARCH = $(wildcard $(ARCH_DIR)/*/*/*.c) \
                 $(wildcard $(ARCH_DIR)/*/*/*/*.c)
C_SOURCES_LIB = $(wildcard $(LIB_DIR)/*.c) \
				$(wildcard $(LIB_DIR)/*/*/*.c) \
                $(wildcard $(LIB_DIR)/*/*/*/*.c)

C_SOURCES = $(C_SOURCES_NUCLEUS) $(C_SOURCES_ARCH) $(C_SOURCES_LIB)

ASM_OBJECTS = $(patsubst $(ROOT_DIR)/%.s,$(OBJ_DIR)/%.o,$(ASM_SOURCES))
C_OBJECTS   = $(patsubst $(ROOT_DIR)/%.c,$(OBJ_DIR)/%.o,$(C_SOURCES))
OBJECTS     = $(ASM_OBJECTS) $(C_OBJECTS) $(FONT_PSF_OBJ)
DEPS        = $(C_OBJECTS:.o=.d)

KERNEL_ELF = $(BUILD_DIR)/nucleus.elf
OS_ISO = $(DIST_DIR)/seren.iso

INCLUDES = -I$(INCLUDE_DIR) \
           -I$(INCLUDE_DIR)/nucleus \
           -I$(LIBC_DIR)/include \
           -I$(ARCH_DIR)/x86_64/include

CFLAGS = -std=c11 $(INCLUDES) -Wall -Wextra -Werror -pedantic -O2 -g \
	-ffreestanding -fno-stack-protector -fno-pie \
	-mno-red-zone -mcmodel=kernel -mgeneral-regs-only \
	-MMD -MP

ASFLAGS = -f elf64 -g

LDFLAGS =  -T $(ROOT_DIR)/linker.ld -nostdlib -static -no-pie --no-dynamic-linker \
	-z max-page-size=0x1000

LIMINE_BIOS_SYS_SRC ?= $(LIMINE_FILES_DIR)/limine-bios.sys
LIMINE_BIOS_CD_SRC ?= $(LIMINE_FILES_DIR)/limine-bios-cd.bin
ESSENTIAL_LIMINE_FILES = \
	$(LIMINE_BIOS_SYS_SRC) \
	$(LIMINE_BIOS_CD_SRC)

.PHONY: all clean iso run $(FONT_PSF_OBJ)

all: $(KERNEL_ELF)

directories:
	@mkdir -p $(OBJ_DIR) $(BUILD_DIR) $(DIST_DIR) $(ISO_ROOT_DIR)
	@mkdir -p $(patsubst %/,%,$(sort $(dir $(OBJECTS))))

$(KERNEL_ELF): $(OBJECTS) $(ROOT_DIR)/linker.ld | directories
	$(LD) $(LDFLAGS) -o $@ $(OBJECTS)

$(OBJ_DIR)/%.o: $(ROOT_DIR)/%.c Makefile $(wildcard $(dir $<)*.h) $(wildcard $(INCLUDE_DIR)/*.h) $(wildcard $(INCLUDE_DIR)/nucleus/*.h) $(wildcard $(LIBC_DIR)/include/*.h) | directories
	$(CC) $(CFLAGS) -c $< -o $@

$(OBJ_DIR)/%.o: $(ROOT_DIR)/%.s Makefile | directories
	$(AS) $(ASFLAGS) $< -o $@

$(FONT_PSF_OBJ): $(FONT_PSF_SRC) | directories
	$(OBJCOPY) -I binary -O elf64-x86-64 $< $@

clean:
	rm -rf $(BUILD_DIR) $(DIST_DIR)

iso: $(KERNEL_ELF) $(ROOT_DIR)/limine.conf check-limine-files | directories
	@echo "--> Preparing files for ISO image..."
	cp $(KERNEL_ELF) $(ISO_ROOT_DIR)/nucleus.elf
	cp $(ROOT_DIR)/limine.conf $(ISO_ROOT_DIR)/limine.conf

	@echo "--> Copying Limine BIOS files..."
	cp $(LIMINE_BIOS_SYS_SRC) $(ISO_ROOT_DIR)
	cp $(LIMINE_BIOS_CD_SRC) $(ISO_ROOT_DIR)

	@echo "--> Creating ISO image: $(OS_ISO)..."

	xorriso -as mkisofs \
		-b limine-bios-cd.bin \
		--no-emul-boot -boot-load-size 4 -boot-info-table \
		$(ISO_ROOT_DIR) -o $(OS_ISO)
	
	@echo "--> ISO image created successfully: $(OS_ISO)"

QEMU_FLAGS = -m 256M

.PHONY: fetch-limine
fetch-limine:
	@if [ ! -d "$(LIMINE_FILES_DIR)" ]; then mkdir -p $(LIMINE_FILES_DIR); fi
	@sh tools/fetch_limine.sh

check-limine-files:
	@missing_files=0; \
	for file in $(ESSENTIAL_LIMINE_FILES); do \
		if [ ! -f "$$file" ]; then \
			echo "ERROR: Limine file '$$file' is missing."; \
			missing_files=1; \
		fi; \
	done; \
	if [ "$$missing_files" -ne 0 ]; then \
	    echo "--> Some Limine files are missing. Attempting to fetch them..."; \
	    $(MAKE) fetch-limine; \
	    missing_files_after_fetch=0; \
	    for file_after_fetch in $(ESSENTIAL_LIMINE_FILES); do \
	        if [ ! -f "$$file_after_fetch" ]; then \
	            echo "ERROR: Limine file '$$file_after_fetch' is still missing after fetch attempt."; \
	            missing_files_after_fetch=1; \
	        fi; \
	    done; \
	    if [ "$$missing_files_after_fetch" -ne 0 ]; then \
	        echo "Please ensure 'tools/fetch_limine.sh' ran successfully or place the files manually in '$(LIMINE_FILES_DIR)'."; \
	        exit 1; \
	    fi; \
	    echo "--> Limine files successfully fetched."; \
	else \
	    echo "--> All essential Limine files found."; \
	fi

run: iso
	$(QEMU) $(QEMU_FLAGS) -cdrom $(OS_ISO)

qemu_debug: iso
	$(QEMU) $(QEMU_FLAGS) -cdrom $(OS_ISO) -s -S

-include $(DEPS)