# About ProjectM

projectM is the most advanced open-source music visualizer.
Experience psychedelic and mesmerizing visuals by transforming music into 
equations that render into a limitless array of user-contributed visualizations.

projectM is an open-source project that reimplements the esteemed Winamp Milkdrop 
by Geiss in a more modern, cross-platform reusable library.

https://github.com/projectM-visualizer/projectm


# aimp_projectM

aimp_projectM is ProjectM-based visualization plugin for the [AIMP](https://www.aimp.ru) app.

**Important**: plugin is currently undergoing heavy development.
Please, refer to our forum section for nigtly-builds, suggestions and dicussions: https://aimp.ru/forum/index.php?topic=77704.0

**Implementation details**

* The plugin displays the visualization in a separate window (rendering in the app's built-in window is not yet supported).
   
* All *.milk presets must be placed inside the "aimp_projectM\presets" directory (sub-directories are possible). All textures that used in presets must be placed in the "aimp_projectM\textures" directory.
   
* The plugin will load all listed presets` and will switch between them randomly. To forcefully switch to a next preset, click inside the visualization window.
   
   
**Requirements to build**

You'll need a few things before you can compile the source code:
* [AIMP C++ SDK](https://www.aimp.ru/?do=download&os=windows&cat=sdk), extract content of "Source/Cpp" folder to "./aimp_sdk/"
* [ProjectM-4](https://github.com/projectM-visualizer/projectm) source code must be extracted to ./libprojectM/
* Microsoft VisualStudio 2026


**To-do**
* Linux support
* Rendering to AIMP's embedded visualization window
