# ZMK config for beekeeb Toucan Keyboard

[The beekeeb Toucan Keyboard](https://beekeeb.com/toucan-keyboard/) is a wireless split 42-key column‑stagger keyboard that a display and a trackpad, with an aggressive stagger on the pinky columns.

# Flashing

The left half (`toucan_left`) is the BLE central and carries the nice_view
display; the right half (`toucan_right`) is the peripheral and carries the
cirque trackpad (no display). Flash the matching `.uf2` to each half.

## Right half "dead" after reflashing (won't connect / no trackpad)

This is a split BLE bond desync, not a firmware fault — reflashing the central
can leave the two halves with mismatched bonds, so the peripheral can't
reconnect. The right-half firmware itself is independent of the display widgets
and does not change when those change. To recover:

1. Put **both** halves into the bootloader (double-tap reset) and flash
   `settings_reset-…uf2` to **each**.
2. Flash the real firmware to **both** halves (`toucan_left …` to the display
   half, `toucan_right …` to the trackpad half).
3. Power both on near each other — they re-pair and the right half returns.

Reflashing only one half is the usual cause; always reset/flash both together.

# License

The code in this repo is available under the MIT license.

The included shield nice_view_gem is modified from https://github.com/M165437/nice-view-gem licensed under the MIT License.

ZMK code snippets are taken from the ZMK documentation under the MIT license.

The embedded font QuinqueFive is designed by GGBotNet, licensed under under the SIL Open Font License, Version 1.1.
