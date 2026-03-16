#define _GNU_SOURCE
#include <errno.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/syscall.h>
#include <unistd.h>

#include <hexdump/hexdump.h>

#if !defined(__linux__)
#  error Unsupported OS.
#endif

#if (defined(__loongarch__) && __loongarch_grlen == 64)
#  define MAX_SYSCALL_ARGS 6
#  define SYSCALL(num_, args_)                                                                     \
          (syscall((num_), (args_)[0], (args_)[1], (args_)[2], (args_)[3], (args_)[4], (args_)[5]))
#elif defined(__x86_64__)
#  define MAX_SYSCALL_ARGS 6
#  define SYSCALL(num_, args_)                                                                     \
          (syscall((num_), (args_)[0], (args_)[1], (args_)[2], (args_)[3], (args_)[4], (args_)[5]))
#else
#  error Unsupported architecture.
#endif

#define MAX_SYSCALL_ARGS 6

#define HXD_FPERROR(fd_, done_, fun_, ...)                                                         \
        do {                                                                                       \
                hxd_error_t err = (fun_)(__VA_ARGS__);                                             \
                if (err) {                                                                         \
                        fprintf((fd_), #fun_ ": %s\n", hxd_strerror(err));                         \
                        goto done_;                                                                \
                }                                                                                  \
        } while (0)

static uint64_t parse_int(const char *str) {
        int base = 10;
        if (strncmp(str, "0x", 2) == 0) {
                base = 16;
                str += 2;
        } else if (strncmp(str, "0o", 2) == 0) {
                base = 8;
                str += 2;
        }
        return strtoull(str, NULL, base);
}

static void *parse_binary(const char *hex_str) {
        const size_t len = strlen(hex_str);
        if (len % 2 != 0) {
                fprintf(stderr, "error: 'b' strings should be multiples of bytes in hex\n");
                exit(121);
        }
        const size_t bin_len = len / 2;
        uint8_t *const buffer = malloc(bin_len);
        if (!buffer) {
                perror("error: malloc");
                exit(122);
        }
        for (size_t i = 0; i < bin_len; i++) {
                sscanf(hex_str + 2 * i, "%2hhx", &buffer[i]);
        }
        return buffer;
}

static void hexdump_hxd(FILE *out, const void *buf, size_t len) {
        hxd_error_t err = 0;
        struct hexdump *x = hxd_open(&err);
        if (!x) {
                fprintf(stderr, "hxd_open: %s\n", hxd_strerror(err));
                return;
        }
        HXD_FPERROR(stderr, done, hxd_compile, x, HEXDUMP_C, 0);
        HXD_FPERROR(stderr, done, hxd_write, x, buf, len);
        HXD_FPERROR(stderr, done, hxd_flush, x);
        char tmp[4096];
        size_t n = 0;
        while ((n = hxd_read(x, tmp, sizeof tmp)) > 0) {
                if (fwrite(tmp, 1, n, out) != n) {
                        perror("error: fwrite");
                        goto done;
                }
        }
done:
        hxd_close(x);
        return;
}

int main(int argc, char *argv[]) {
        if (argc < 2) {
                fprintf(stderr, "usage: %s <syscall ID> [ARGUMENT]...\n", argv[0]);
                return 121;
        }

        if (argc - 2 > MAX_SYSCALL_ARGS) {
                return 120;
        }

        const long sysno = strtol(argv[1], NULL, 10);

        long args[MAX_SYSCALL_ARGS] = {0};
        size_t args_out_size[MAX_SYSCALL_ARGS] = {0};

        for (int i = 2; i < argc; i++) {
                char *const arg = argv[i];
                const int narg = i - 2;

                if (arg[0] == ':') {
                        fprintf(stderr, "error: bad argument %d: '%s'\n", narg, arg);
                        return 121;
                }
                char *const spec = strtok(arg, ":");
                char *const data = strtok(NULL, ":");
                if (spec == NULL || data == NULL) {
                        fprintf(stderr, "error: bad argument %d: format\n", narg);
                        return 121;
                }

                long val = 0;
                size_t val_out_size = 0;
                if (strcmp(spec, "i8") == 0) {
                        val = (long)(int8_t)parse_int(data);
                } else if (strcmp(spec, "i16") == 0) {
                        val = (long)(int16_t)parse_int(data);
                } else if (strcmp(spec, "i32") == 0) {
                        val = (long)(int32_t)parse_int(data);
                } else if (strcmp(spec, "i64") == 0) {
                        val = (long)(int64_t)parse_int(data);
                } else if (strcmp(spec, "u8") == 0) {
                        val = (long)(uint64_t)(uint8_t)parse_int(data);
                } else if (strcmp(spec, "u16") == 0) {
                        val = (long)(uint64_t)(uint16_t)parse_int(data);
                } else if (strcmp(spec, "u32") == 0) {
                        val = (long)(uint64_t)(uint32_t)parse_int(data);
                } else if (strcmp(spec, "u64") == 0) {
                        val = (long)(uint64_t)parse_int(data);
                } else if (strcmp(spec, "s") == 0) {
                        val = (long)data;
                } else if (strcmp(spec, "b") == 0) {
                        val = (long)parse_binary(data);
                } else if (strcmp(spec, "o") == 0) {
                        const size_t size = (size_t)parse_int(data);
                        void *const p = malloc(size);
                        if (p == NULL) {
                                perror("error: malloc");
                                return 121;
                        }
                        val = (long)p;
                        val_out_size = size;
                } else {
                        fprintf(stderr, "error: bad argument %d: unknown spec '%s'\n", narg, spec);
                        return 121;
                }
                args[narg] = val;
                args_out_size[narg] = val_out_size;
        }

        const long ret = SYSCALL(sysno, args);
        const int errno_ = errno;
        for (size_t i = 0; i < MAX_SYSCALL_ARGS; i++) {
                if (args_out_size[i] != 0) {
                        void *const p = (void *)args[i];
                        printf("%zu:\n", i);
                        hexdump_hxd(stdout, p, args_out_size[i]);
                        putchar('\n');
                        free(p);
                }
        }

        if (ret == -1) {
                printf("syscall: %s\n", strerror(errno_));
                return errno_;
        }

        return 0;
}
