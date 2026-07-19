# Text and Glyph Rendering

Step 7 adds a display-independent text and glyph layer above the monochrome
canvas.

## Built-in font

The framework includes a fixed-width 5x7 ASCII bitmap font covering characters
32 through 126. Unknown characters are rendered as `?`.

The font representation is public and reusable. Additional fonts may be
provided by consuming projects without changing the renderer.

## Text layout

Text may be drawn from an explicit origin or aligned inside a rectangle.

Supported horizontal alignment:

- left
- center
- right

Supported vertical alignment:

- top
- middle
- bottom

Text can be clipped to its layout rectangle.

## Inverted text

Inverted text fills the target rectangle using the foreground operation and
draws glyph pixels with the configured background operation. This supports
selected menu entries, focused parameters, and compact status labels.

## Glyphs

The built-in scalable glyphs are:

- arrow left
- arrow right
- arrow up
- arrow down
- check
- cross
- plus
- minus

Glyphs use the existing line primitives rather than a separate bitmap format.
