# CMatrix-bitmap

CMatrix-bitmap is based Cmatrix program (see: https://github.com/abishekvashok/cmatrix).
In lieu of fork clone and seperate program while under development (beleived out of scope of cmatrix?). 
This can expand to a feature request/ merge with master branch *if* stable :P

## Goals
Provide a bitmap, pixel-map with user defined characters (assume ascii for starters) that will overlay into the matrix
rain effect and remain stationary. 

Example: take a set size bitmap (say 20x20 square not filled in) and render it into a 100x100 sized terminal window. Ideal output-> stationary square built off user defined character with the rest of the terminal filling in the matrix effect around it.

Further steps:
1. Configure for scaling

Ideas:
treat each character in terminal as a literal character (0-255 val) and hard impose value in matrix slot
32x10 window size (wxh), total matrix size = 320 bytes
0 = normal cmatrix process
1 = enfore specific character at slot\

32x10 window (bitwise control view) running normal
cmatrix: 

00000000000000000000000000000000
00000000000000000000000000000000
00000000000000000000000000000000
00000000000000000000000000000000
00000000000000000000000000000000
00000000000000000000000000000000
00000000000000000000000000000000
00000000000000000000000000000000
00000000000000000000000000000000
00000000000000000000000000000000

32x10 window (bitwise control view) running cmatrix-bitmap
enforced character slotting for drawing a square: 
11111111111111111111111111111111
10000000000000000000000000000001
10000000000000000000000000000001
10000000000000000000000000000001
10000000000000000000000000000001
10000000000000000000000000000001
10000000000000000000000000000001
10000000000000000000000000000001
10000000000000000000000000000001
11111111111111111111111111111111

further explained concept: say user selects $ (cash moni) to be rendered character for bitmap square.
let 0 = cmatrix fillin (0 is not the selected user character).
Stationary dollar signs with matrix rain fill in:

$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$
$000000000000000000000000000000$
$000000000000000000000000000000$
$000000000000000000000000000000$
$000000000000000000000000000000$
$000000000000000000000000000000$
$000000000000000000000000000000$
$000000000000000000000000000000$
$000000000000000000000000000000$
$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$


### License
This software is provided under the GNU GPL v3.

### Disclaimer
We are in no way affiliated in any way with the movie "The Matrix", "Warner Bros" nor
any of its affiliates in any way, just fans.

