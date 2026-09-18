#include <stdint.h>

// --- VRAM ドライバ関数 (vram.c より) ---
void k_vram_init(void);
void k_putchar(char c);
void k_print(const char* str);
void k_set_color(uint8_t fg, uint8_t bg);

// --- キーボード MMIO レジスタ定義 ---
#define KBD_BASE        ((volatile uint8_t*)0x8000)
#define REG_KBD_HEAD    (*(KBD_BASE + 0))
#define REG_KBD_TAIL    (*(KBD_BASE + 1))
#define REG_KBD_MODS    (*(KBD_BASE + 2))
#define KBD_BUFFER      ((volatile uint8_t*)(KBD_BASE + 4))

#define KBD_BUF_SIZE    12

// プロンプトの表示
static void print_prompt(void) {
    k_set_color(0x2, 0x0); // 緑色
    k_print("nuno> ");
    k_set_color(0xF, 0x0); // 白色
}

// キーボードMMIOバッファからの非同期読み取り (ノンブロッキング)
static int k_getchar(void) {
    uint8_t head = REG_KBD_HEAD;
    uint8_t tail = REG_KBD_TAIL;

    // バッファが空の場合は -1 を返す
    if (head == tail) {
        return -1;
    }

    // リングバッファから1バイト取得
    uint8_t c = KBD_BUFFER[head];

    // HEADポインタを1つ進める
    REG_KBD_HEAD = (head + 1) % KBD_BUF_SIZE;

    return (int)c;
}

// カーネル初期化
__attribute__((visibility("default")))
void k_shell_init(void) {
    // 1. キーボードレジスタ初期化
    REG_KBD_HEAD = 0;
    REG_KBD_TAIL = 0;
    REG_KBD_MODS = 0;

    // 2. VRAM初期化 & 起動メッセージ
    k_vram_init();
    k_set_color(0xE, 0x0); // 黄色
    k_print("nuno kernel v0.0\n");
    k_set_color(0x7, 0x0); // グレー
    print_prompt();
}

// エントリポイントから周期的に実行されるメインループ (Tick)
__attribute__((visibility("default")))
void k_tick(void) {
    int c;

    // リングバッファに溜まっている入力文字を全て処理
    while ((c = k_getchar()) != -1) {
        if (c == '\r' || c == '\n') {
            k_putchar('\n');
            print_prompt();
        } else if (c == 0x08) { // Backspace
            k_putchar('\b');
        } else {
            // 受信したアスキー文字をVRAMへ直接エコーバック
            k_putchar((char)c);
        }
    }
}
