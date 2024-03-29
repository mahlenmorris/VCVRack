# Release Notes

### 2.0.13
#### January 3, 2024
* Fixed bug where on startup, WAIT times for some statements would randomly be zero length
instead of the length asked for.

### 2.0.12
#### September 5, 2023
* Introducing the idea of sending text by audio cable via the
[Tipsy protocol](https://github.com/baconpaul/tipsy-encoder).
* Added new [print() and debug() methods](https://github.com/mahlenmorris/VCVRack/blob/main/README.md#text-functions)
to BASICally so it can send text to other modules.
* Added a new module, [TTY](https://library.vcvrack.com/StochasticTelegraph/TTY), for display of text from BASICally.
* Added some quantization Presets to BASICally.

### 2.0.11
#### May 21, 2023
* Added support for PgUp and PgDown keys in BASICally and Fermata.
* Fermata now has six different label-only font sizes, five of them sideways.

### 2.0.10
#### May 11, 2023
* Added a new text and labeling module called Fermata, for annotating and
documenting a patch, or for writing a story while listening to one.
A more capable version of VCV's Notes.
* Also improved the BASICally editor (it's the same as the one Fermata uses).
You can choose from different fonts, and fixed some bugs in scrolling. Added
more presets.

### 2.0.9
#### April 10, 2023
* Additional preset scripts for BASICally.
* Drifter can now RESET to shapes other than a straight line at zero.
* Drifter can drift the left and right end points in sync. This prevents
sudden changes/transients when driving IN with a saw wave.
* Fuse now has an UNTRIGGER input and button, which lowers the accumulated
count of triggers.

### 2.0.8
#### February 12, 2023
* Bug fixes to compilation.
* Added new preset.

### 2.0.7
#### February 6, 2023
* Added notion of "ALSO blocks" that [run in parallel](https://github.com/mahlenmorris/VCVRack#also-blocks).
* Added notion of "WHEN blocks" that are [started when a condition is met](https://github.com/mahlenmorris/VCVRack#when-blocks).
* Added ["CLEAR ALL"](https://github.com/mahlenmorris/VCVRack#clear-all) command, which sets all variables to zero and empties all
arrays.
* Added ["RESET"](https://github.com/mahlenmorris/VCVRack#reset) command, which interrupts all blocks and restarts them from the beginning.
* Added [start() and trigger(INn)](https://github.com/mahlenmorris/VCVRack/edit/main/README.md#functions) functions, to know when the program has
just been compiled or a trigger has been received.
* Added [log2(), loge(), and log10()](https://github.com/mahlenmorris/VCVRack/edit/main/README.md#functions) functions.
* Added [time() and time_millis()](https://github.com/mahlenmorris/VCVRack/edit/main/README.md#functions) functions for tracking the execution time
more easily.
* Added some factory Presets illustrating some interesting features of the
module.
* Added two more OUT ports, for a total of six.
* Added the ability to ["unclamp" OUTn ports](https://github.com/mahlenmorris/VCVRack/edit/main/README.md#clampunclamp-outn-values); they are normally limited to the
range -10 <= x <= 10. Useful for debugging scripts.  
* Fixed a few couple UI bugs, including undo/redo crashing Rack at times.
* Made the screen a couple of lines taller.

### 2.0.5
#### January 9, 2023

* Many UI changes:
  * STYLE knob is smaller, elements moved.
  * Now have nine INx ports instead of six.
  * The minimum width is now one “hole” smaller, and this hides the editing window.
  * Can enter a title for a module (via the menu) that shows up above the IN ports. Handy for stating what role the module plays in the patch.
  * Added menu option to change the status light from green/red to blue/orange.
  * And added new screen color options.
* [Added ‘elseif’](https://github.com/mahlenmorris/VCVRack/blob/main/README.md#if-statements-conditional-behavior). E.g., “IF a == 5 THEN … ELSEIF a == 4 THEN … “
* Added methods about the environment, [sample_rate() and connected()](https://github.com/mahlenmorris/VCVRack/blob/main/README.md#setting-and-using-variables-assignment-and-math).
* Added methods for random number generation, [random() and normal()](https://github.com/mahlenmorris/VCVRack/blob/main/README.md#setting-and-using-variables-assignment-and-math).
* Added [single dimension arrays](https://github.com/mahlenmorris/VCVRack/blob/main/README.md#arrays), and a simple way to set a range of values.

### 2.0.4
#### December 19, 2022
Updates to BASICally:
* Rejiggered UI elements.
* The first line that confuses BASICally will be highlighted in red. This behavior can be turned off in the menu.
* Text window scrolls when you get to the top or bottom. Longer programs!
* Now have six INx ports instead of four.
* Added scientific note notation: “out1 = c#3” and “if IN5 < bb3 then…” are now legal.
* Now can have black text on white.
* Some performance improvement.
* Added all the Math functions to the menu.

### 2.0.3
#### December 12, 2022
Initial release of [BASICally](https://github.com/mahlenmorris/VCVRack/blob/main/README.md#basically).

### 2.0.1
#### April 30, 2022
Initial release of [Fuse](https://github.com/mahlenmorris/VCVRack/blob/main/README.md#fuse).

### 2.0.0
#### April 13, 2022

Initial release of [Drifter](https://github.com/mahlenmorris/VCVRack/blob/main/README.md#drifter).
