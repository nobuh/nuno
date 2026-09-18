#include <stdint.h>

// --- VRAM & MMIO アドレス定義 ---
#define MMIO_BASE        ((volatile uint8_t*)0x8100)

#define REG_CURSOR_X     (*(MMIO_BASE + 0))
#define REG_CURSOR_Y     (*(MMIO_BASE + 1))
#define REG_CURSOR_STYLE (*(MMIO_BASE + 2))
#define REG_RENDER_FLAGS (*(MMIO_BASE + 3))

#define TEXT_BUFFER      ((volatile uint8_t*)(0x8104))
#define ATTR_BUFFER      ((volatile uint8_t*)(0x88D4))

#define COLS 80
#define ROWS 25

// --- 色定義 (ANSI 16色) ---
#define COLOR_BLACK   0x0
#define COLOR_GREEN   0x2
#define COLOR_WHITE   0xF

// 属性値の生成ヘルパー: (背景色 << 4) | 前景色
#define MAKE_ATTR(fg, bg) ((uint8_t)(((bg & 0x07) << 4) | (fg & 0x0F)))

static uint8_t current_attr = MAKE_ATTR(COLOR_WHITE, COLOR_BLACK);

// VRAMの初期化
void k_vram_init(void) {
    REG_CURSOR_X = 0;
    REG_CURSOR_Y = 0;
    REG_CURSOR_STYLE = 1; // 点滅カーソル

    // 画面クリア（全領域を空白とデフォルト属性で埋める）
    for (int i = 0; i < COLS * ROWS; i++) {
        TEXT_BUFFER[i] = ' ';
        ATTR_BUFFER[i] = current_attr;
    }
    REG_RENDER_FLAGS |= 0x01; // DIRTYフラグON
}

// 画面スクロール処理（最終行を超えた場合）
static void k_scroll(void) {
    // 2行目以降を1行分上にコピー
    for (int i = 0; i < (ROWS - 1) * COLS; i++) {
        TEXT_BUFFER[i] = TEXT_BUFFER[i + COLS];
        ATTR_BUFFER[i] = ATTR_BUFFER[i + COLS];
    }
    // 最終行をクリア
    int last_line_offset = (ROWS - 1) * COLS;
    for (int x = 0; x < COLS; x++) {
        TEXT_BUFFER[last_line_offset + x] = ' ';
        ATTR_BUFFER[last_line_offset + x] = current_attr;
    }
    REG_CURSOR_Y = ROWS - 1;
}

// 1文字出力 (putchar)
void k_putchar(char c) {
    if (c == '\n') {
        REG_CURSOR_X = 0;
        REG_CURSOR_Y++;
    } else if (c == '\r') {
        REG_CURSOR_X = 0;
    } else if (c == '\b') {
        if (REG_CURSOR_X > 0) {
            REG_CURSOR_X--;
            int offset = REG_CURSOR_Y * COLS + REG_CURSOR_X;
            TEXT_BUFFER[offset] = ' ';
            ATTR_BUFFER[offset] = current_attr;
        }
    } else {
        int offset = REG_CURSOR_Y * COLS + REG_CURSOR_X;
        TEXT_BUFFER[offset] = (uint8_t)c;
        ATTR_BUFFER[offset] = current_attr;

        REG_CURSOR_X++;
        if (REG_CURSOR_X >= COLS) {
            REG_CURSOR_X = 0;
            REG_CURSOR_Y++;
        }
    }

    // 端に達したらスクロール
    if (REG_CURSOR_Y >= ROWS) {
        k_scroll();
    }

    // レンダラーへ更新通知 (DIRTY Bit)
    REG_RENDER_FLAGS |= 0x01;
}

// 文字列出力 (print)
void k_print(const char* str) {
    while (*str) {
        k_putchar(*str++);
    }
}

// 文字色変更
void k_set_color(uint8_t fg, uint8_t bg) {
    current_attr = MAKE_ATTR(fg, bg);
}
