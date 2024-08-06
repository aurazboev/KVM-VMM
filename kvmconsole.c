#include <err.h>
#include <fcntl.h>
#include <linux/kvm.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <unistd.h>

#define CONSOLE_IO_PORT 0x42
char console_buffer[1024];
size_t console_buffer_index = 0;

void handle_console_io(uint8_t data) {
    printf("Debug: Received char: %c (0x%X)\n", data, data);
    if (data == '\n') {
        console_buffer[console_buffer_index] = '\0';
        printf("%s\n", console_buffer);
        fflush(stdout);
        console_buffer_index = 0;
    } else if (console_buffer_index < sizeof(console_buffer) - 1) {
        console_buffer[console_buffer_index++] = data;
    }
}

int main(int argc, char *argv[]) {
    int kvm, vmfd, vcpufd, ret;
    uint8_t *mem;
    struct kvm_sregs sregs;
    size_t mmap_size;
    struct kvm_run *run;

    kvm = open("/dev/kvm", O_RDWR | O_CLOEXEC);
    if (kvm == -1) err(1, "/dev/kvm");

    ret = ioctl(kvm, KVM_GET_API_VERSION, NULL);
    if (ret == -1) err(1, "KVM_GET_API_VERSION");
    if (ret != 12) errx(1, "KVM_GET_API_VERSION %d, expected 12", ret);

    vmfd = ioctl(kvm, KVM_CREATE_VM, (unsigned long)0);
    if (vmfd == -1) err(1, "KVM_CREATE_VM");

    mem = mmap(NULL, 0x1000, PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);
    if (!mem) err(1, "allocating guest memory");

    if (argc < 2) errx(1, "Usage: %s <binary file>", argv[0]);
    int codefd = open(argv[1], O_RDONLY);
    if (codefd < 0) err(1, "Could not open code file");
    ssize_t code_size = read(codefd, mem, 0x1000);
    if (code_size <= 0) err(1, "Reading code file");
    close(codefd);

    struct kvm_userspace_memory_region region = {
        .slot = 0,
        .guest_phys_addr = 0x1000,
        .memory_size = 0x1000,
        .userspace_addr = (uint64_t)mem,
    };
    ret = ioctl(vmfd, KVM_SET_USER_MEMORY_REGION, &region);
    if (ret == -1) err(1, "KVM_SET_USER_MEMORY_REGION");

    vcpufd = ioctl(vmfd, KVM_CREATE_VCPU, (unsigned long)0);
    if (vcpufd == -1) err(1, "KVM_CREATE_VCPU");

    ret = ioctl(kvm, KVM_GET_VCPU_MMAP_SIZE, NULL);
    if (ret == -1) err(1, "KVM_GET_VCPU_MMAP_SIZE");
    mmap_size = ret;
    if (mmap_size < sizeof(*run)) errx(1, "KVM_GET_VCPU_MMAP_SIZE unexpectedly small");
    run = mmap(NULL, mmap_size, PROT_READ | PROT_WRITE, MAP_SHARED, vcpufd, 0);
    if (!run) err(1, "mmap vcpu");

    ret = ioctl(vcpufd, KVM_GET_SREGS, &sregs);
    if (ret == -1) err(1, "KVM_GET_SREGS");
    sregs.cs.base = 0;
    sregs.cs.selector = 0;
    ret = ioctl(vcpufd, KVM_SET_SREGS, &sregs);
    if (ret == -1) err(1, "KVM_SET_SREGS");

    struct kvm_regs regs = {
        .rip = 0x1000,
	    .rax = 0,
	    .rbx = 0,
        .rflags = 0x2,
    };
    ret = ioctl(vcpufd, KVM_SET_REGS, &regs);
    if (ret == -1) err(1, "KVM_SET_REGS");

    while (1) {
        ret = ioctl(vcpufd, KVM_RUN, NULL);
        if (ret == -1) err(1, "KVM_RUN");
        if (run->exit_reason == KVM_EXIT_IO) {
            if (run->io.direction == KVM_EXIT_IO_OUT && run->io.port == CONSOLE_IO_PORT && run->io.size == 1 && run->io.count == 1) {
                uint8_t data = *(((uint8_t *)run) + run->io.data_offset);
                handle_console_io(data);
            }
        } else if (run->exit_reason == KVM_EXIT_HLT) {
            puts("KVM_EXIT_HLT");
            return 0;
        } else {
            errx(1, "Unhandled KVM_EXIT reason: %d", run->exit_reason);
        }
    }
}
