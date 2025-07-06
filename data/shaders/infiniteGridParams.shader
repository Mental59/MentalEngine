// extents of grid in world coordinates
const float gGridSize = 100.0;

// size of one cell
const float gGridCellSize = 0.025;

// color of thin lines
const vec4 gGridColorThin = vec4(0.4, 0.4, 0.4, 1.0);

// color of thick lines (every tenth line)
const vec4 gGridColorThick = vec4(0.05, 0.05, 0.05, 1.0);

// minimum number of pixels between cell lines before LOD switch should occur.
const float gGridMinPixelsBetweenCells = 2.0;
