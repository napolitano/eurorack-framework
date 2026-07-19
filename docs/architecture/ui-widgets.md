# UI Widgets

Step 8 adds a stateless widget layer above the canvas, drawing primitives, text,
and glyph APIs.

Widgets consume explicit view models and do not own application state.

Included widgets:

- parameter label and value
- potentiometer with normalized position indicator
- encoder with bounded position indicator
- progress bar
- top bar
- footer bar
- scrollable list menu
- confirmation overlay
- error overlay

Menu scrolling is deterministic. `ensureMenuSelectionVisible()` calculates the
first visible row required to keep the selection on screen.

Overlays clear and frame only their assigned rectangle. Applications decide
whether the underlying scene is redrawn before or after dismissal.
