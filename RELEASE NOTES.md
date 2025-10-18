# Release Notes

### 2.0.23
#### ???
* Fixed bug in text editor for Fermata, TTY, and BASICally that prevented non-7-bit-ASCII characters from being typed in.
* When attached to a MemoryCV, Depict now shows Scope-style lines instead of amplitude. 

### 2.0.22
#### October 5, 2025
* New MemoryCV module! A drop-in replacement for Memory within a Memory Ensemble, but better for storing and playing control voltage.
* Memory (and MemoryCV) can now load and save files with non-ASCII characters in the name or path.
* * Note that the Rack fonts don't support non-ASCII characters, so still can't *display* these names correctly.
* MemoryCV (and Memory) can load and save .CSV (Comma Separated Value) files. Great when you want **extremely** fine editing control over the signals, or if you've longed to use a spreadsheet in your musical process.
* BASICally
* * Improved the resolution of BASICally's time() and time_millis() functions.
* * Fixed flaky issue where time between compile and running new code could be delayed by up to 5 seconds.

### 2.0.21
#### June 24, 2025
* Fermata and TTY:
* * Both of these now support [larger font sizes](https://github.com/mahlenmorris/VCVRack/blob/main/README.md#fermata), all the way up to filling the vertical space with a single line of text.
* * They also can now be stretched out to 300 HP wide. That's wider than 33 VCV VCO's.
* * [A short video](https://www.youtube.com/watch?v=VQs2c6qWk8E) demo.
* * Thanks to maunatanmaya for this suggestion in my GitHub Issues, and for wrapping it in a Ko-fi donation :)
* BASICally:
* * Added [WHILE-END WHILE loops](https://github.com/mahlenmorris/VCVRack/blob/main/README.md#while-loops). Thanks to @gabtiorbi for unintentionally reminding me that it's confusing not having WHILE loops.
* * [Polyphonic INx and OUTx ports](https://github.com/mahlenmorris/VCVRack/blob/main/README.md#polyphonic-inputs-and-outputs)! Thanks again to @gabtiorbi for getting me to think seriously about supporting this. It's made many of my programs much simpler.
* * Adding polyphony convinced me that a faster FOR loop mechanism is (sometimes) needed, so I've added [the NEXTHIGHCPU option](https://github.com/mahlenmorris/VCVRack/blob/main/README.md#faster-for-loops-with-nexthighcpu) to FOR loops.
* * Updated the BASICally presets to use these features when appropriate.

### 2.0.20
#### May 25, 2025
* Added the ternary '?:' operator to BASICally. For example:
```
IF foo >= 2 THEN
  OUT1 = IN1
ELSE
  OUT1 = 0
END IF
```
can now be written as:
```
OUT1 = foo >=2 ? IN1 : 0
```
* In Venn, each Circle can now have a name associated with it, making the display more understandable.
* In Venn, each Circle can now have an arbitrarily complicated formula associated with it, and the computed values become
the polyphonic MATH1 output port. Examples of formula include:
* * "0.125"
* * "c#2" - that is, the V/OCT value for a C sharp in octave 2
* Simple computed values:
* * "bb2 + 0.02" - a slightly sharp Bflat in octave 2.
* * "x / 2" - the value of the X value for this Circle, but divided by two.
* * "(x * y / 10) - 1" - use both the X and Y values for this Circle.
* * "pointx + pointy" - pointx and pointy are the X and Y values of the Point (i.e., the little white circle you move).
* Use built-in functions:
* * "sign(x) * .1" - have the values -0.1 or 0.1, depending on which side of the Circle the Point is on.
* * "min(distance, 5.4)" - be the smaller of the DISTANCE value or 5.4.
* * "limit(distance, 5.4, 8.3)" - be the value of DISTANCE, but never less than 5.4 or more than 8.3.
* * "scale(x, leftx, rightx, -2.3, -1.2)" - instead of X's normal range, scale it so that
 the left edge X is -2.3 and the right edge is -1.2.
* Use simple logic to determine values:
* * "within ? 1.4 : 3.11" - if WITHIN is not zero, return 1.4; if WITHIN is zero, return 3.11.
* * "x < 1 ? 0.2 : sin(x)" - if X is less than 1, return 0.2, otherwise return sin(x).
* Add presets and triangle venn-wave.

### 2.0.19
#### February 1, 2025
* Fixed crashing bug in Brainwash (triggered sometimes by a race condition when PLAY input is high when the patch is loaded.)

### 2.0.18
#### January 18, 2025
* New Memory module, [Brainwash](https://github.com/mahlenmorris/VCVRack/blob/main/Memory.md#brainwash), which can be told to record up to 60 seconds of audio
and then replace the contents of Memory when recording is completed.
* Fixed bug in Fixation when CLOCK and PLAY input both receive triggers at the same moment. (thanks reina!).

I should mention that Brainwash doesn't feel complete to me yet, so I'll openly invite people to make suggestions for how to make it more useful for the kinds of music that benefit from it.

### 2.0.17
#### September 28, 2024
* Added **Venn**, a signal generator with an intuitive graphical control system. A single click can change as many as 64 signals (four polyphonic signals).
* * Videos about Venn:
* * * Introduction and [using as a mixer](https://www.youtube.com/watch?v=yvjIii_FKCs)
* * * Details about [editing and sonic neighborhoods](https://www.youtube.com/watch?v=Csc6DKv9wHI)
* * * [Generating MIDI notes](https://www.youtube.com/watch?v=gOE4iCjMsH8) and moving Point at audio rate.
* TTY can now display a timestamp on each line via a menu option.
* Fixed a bug in Memory's file loading that would clip the end of the file when the file and Rack had different sample rates.
* Added a menu option to Memory to load the last loaded file on module start (thanks, @jue).

### 2.0.16
#### August 10, 2024
* Added **Fixation**, a new module for playing back audio from Memory. It specializes in rhythmic, small interval playing
* * Videos featuring Fixation:
* * * [Initial Techniques](https://www.youtube.com/watch?v=nDcjS6hz9qE)
* * * [Melodies from samples](https://www.youtube.com/watch?v=rDCgencIVIY)
* * * [Highly controllable granular playback](https://www.youtube.com/watch?v=jzkOs-odrig)
* * Documentation [here](https://github.com/mahlenmorris/VCVRack/blob/main/Memory.md#fixation).
* Memory, when loading a file, now smooths the ending and beginning to ensure there is
no sudden "click" when looping from beginning to end (or vice versa).
* Made the text windows on Fermata, BASICally, and TTY more efficient on the UI thread. This won't affect the measured performance of these modules, but you may see slightly higher FPS numbers when using these modules.
* Added string variables and arrays to BASICally:
* * A variable name that ends with a "\$" (e.g., a\$, foo\$, mod_name\$) can only hold a string
(i.e., a series of printable characters) instead of a number.
* * An array whose name ends in a "\$" (e.g., bar\$[], names\$[3]) can only hold an array of strings.
* * These changes are only really useful when using print() to send text messages to other modules. See [docs for details](https://github.com/mahlenmorris/VCVRack/blob/main/README.md#text-functions).

### 2.0.15
#### June 24, 2024
* Memory gained access to the file system:
* * Memory can now load .WAV files into itself, replacing the entire contents of the Memory and resizing it.
* * Memory can save .WAV files as well.
* * Since file operations can take a widely varying amount of time, Memory sends out a trigger when those operations complete.
* * You can load and save files from the menu, or you can use BASICally to send [Tipsy](https://github.com/baconpaul/tipsy-encoder) text messages to Memory to tell it to load or save a file.
* * Since surprising things can happen when loading or saving files, instead of silently failing,
when you have the LOG output of Memory cabled to a TEXTn input on a TTY module, you can read the log of what happened on the TTY.
* * Videos about these features are [here](https://www.youtube.com/watch?v=MvuQLtUkY4w) and [here](https://www.youtube.com/watch?v=fw6dk4pGn1s).
* Ruminate now has two new menu options:
* * Fade on Move - affects the behavior when the slider or SET moves the position of the head.
If checked (the default), the L&R outputs will be silent until the position stops changing. If not checked, then the playback will continue as it's being moved.
See [example video](https://www.youtube.com/watch?v=dOsupn0-Mxw).
* * Use Speed as V/Oct - affects how the SPEED is interpreted. When unchecked (the default),
the sum of the SPEED input and control is how many samples the playhead moves forward per sample emitted, so 1 is normal speed, .5 is half-speed. When checked, this sum will be interpreted the way that V/Oct is interpreted in most modules. See [example video](https://www.youtube.com/watch?v=kGKmS2WjqIs).
* When the sliders on Ruminate and Embellish are released, they now resume normal operation more quickly.
* Put a big "T" behind ports only input or output Tispy text messages. Put a less solid "T"
on the BASICally OUTn ports, to suggest that they can emit both Tipsy and regular values.

### 2.0.14
#### May 6, 2024
* BASICally no longer creates compilation threads on the audio thread, in hopes of eliminating
slower-than-expected responsiveness to changes in the text of the program.
* In BASICally, made '==' and '!=' less prone to the aggravation of floating-point comparison issues by making 'a == b' effectively "abs(a - b) < epsilon", where epsilon scales with larger a and b. I believe this will make equality checking far more intuitive, which in the VCV Rack/amateur programmer context is paramount.
* Added the first release of [the Memory System](https://github.com/mahlenmorris/VCVRack/blob/main/Memory.md), four interrelated modules for audio recording and playback.

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
You can choose from different fonts, and I fixed some bugs in scrolling. Added
more presets.

### 2.0.9
#### April 10, 2023
* Additional preset scripts for BASICally.
* Drifter can now RESET to shapes other than a straight line at zero.
* Drifter can drift the left and right endpoints in sync. This prevents
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
* Now you can have black text on white.
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
