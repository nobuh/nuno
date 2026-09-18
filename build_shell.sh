clang --target=wasm32 \
      -nostdlib \
      -fuse-ld=lld \
      -Wl,--no-entry \
      -Wl,--export=_start \
      -Wl,--export=k_tick \
      -Wl,--initial-memory=131072 \
      -O2 -o shell.wasm main.c shell.c vram.c
