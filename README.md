# rc1_dbg
Debug menu enabler for RC1 PAL on Vita.

# Usage
* `L + R + O`   => open/close debug menu (if current game mode is 0)
* `L + R + DPad-Left`  => emulate L3 press (does nothing)
* `L + R + DPad-Right`  => emulate R3 press (does nothing)
All those combos are editable, see main.c

# Building
`mkdir build && cd build && cmake .. && make`

Requires DolceSDK