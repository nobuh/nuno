// main.c
#include <stdint.h>

void k_shell_init(void);

// Wasmのエントリーポイント（これだけを残す）
__attribute__((visibility("default")))
void _start(void) {
    k_shell_init();
}
