About ProjectM
-------------------------------------------------------------------------
projectM is the most advanced open-source music visualizer.
Experience psychedelic and mesmerizing visuals by transforming music into 
equations that render into a limitless array of user-contributed visualizations.

projectM is an open-source project that reimplements the esteemed Winamp Milkdrop 
by Geiss in a more modern, cross-platform reusable library.

https://github.com/projectM-visualizer/projectm


Plugin Implementation
-------------------------------------------------------------------------
🔶 Based on ProjectM v4.1.6.
🔶 The plugin displays the visualization in a separate window (rendering 
   in the app's built-in window is not yet supported).
🔶 All *.milk presets must be placed inside the "aimp_projectM\presets" directory (sub-directories are possible).
   All textures that used in presets must be placed in the "aimp_projectM\textures" directory.
🔶 The plugin will load all listed presets` and will switch between them randomly.
   To forcefully switch to a next preset, click inside the visualization window.