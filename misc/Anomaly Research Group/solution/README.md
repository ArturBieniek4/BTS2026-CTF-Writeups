### Note: this is probably an extremely overkill solution

The challenge provided a file with integer seeds and a [link to a video](https://www.youtube.com/watch?v=4FxPbUSFlMY). The video shows someone exploring a Minecraft world. Its description contains:

```
uggcf://lbhgh.or/k5XnAKLOK_b
```

This is ROT13-encoded and decodes to [the second video link](https://youtu.be/x5KaNXYBX_o). At the end of the second video, the contents of the desert temple chests are visible. There is also `1.12.2` in the description, which is Minecraft version.

My approach was to sweep over the provided seeds and find the one where two desert temple chests matched the observed loot.

The relevant chest contents were:

First chest:

- 1x Fire Aspect I book
- 13x bone
- 9x rotten flesh
- 2x spider eye

Second chest:

- 16x bone
- 4x sand
- 1x saddle
- 1x golden apple
- 4x rotten flesh

The C++ solver finds the matching seed by simulating the desert temple loot generation for the provided candidates. On a single Ryzen 9 7950X, it found the solution in under four minutes.
