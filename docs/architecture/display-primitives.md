# Display Primitives

Step 6 introduces a display-controller-independent monochrome drawing layer.

## Memory model

`MonochromeCanvas` is non-owning. The consuming application supplies its buffer,
which allows static allocation on microcontrollers and ordinary memory in native
simulators. Pixels are packed row-major and MSB-first. Each row occupies
`(width + 7) / 8` bytes.

## Clipping

Every drawing operation is clipped by `MonochromeCanvas`. The active clip can be
changed for panels, widgets, and nested UI regions without changing primitive
algorithms.

## Operations

The canvas supports set, clear, and invert operations. Drawing primitives include:

- horizontal and vertical lines;
- arbitrary Bresenham lines;
- custom repeating line patterns;
- dashed and dotted lines;
- unfilled and filled rectangles;
- unfilled and filled circles.

Text, fonts, glyphs, layout, and widgets deliberately remain outside this step.
