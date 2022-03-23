# VCV Rack modules by Stochastic Telegraph
Modules for use with VCV Rack 2.0, with an emphasis on generative and self-regulating structure. Exploring the region between
random and static.


# Drifter
## Controls
### X DRIFT Input and Button
The distance, in V, that each point can move
along the X-axis (i.e., left-to-right) in one DRIFT event. Setting it to zero locks the points
horizontally in place. Higher values allow larger changes each time.
Hint: start small.
### OFST Button
Sets the range of the expected inputs and outputs from 0-10V or
-5-+5V.
### TOTAL DRIFT Input and Button
The distance, in V, that each point can
move in one DRIFT event. 0 locks the points in place. Higher values allow
larger changes each time. Hint: start small.
### ENDS Button
Selects one of two options: Left and right end points stay locked
at value zero OR end points drift up and down when DRIFT events occur.
### DRIFT Input and Button
A trigger to the input or button press will
cause all of the points defining the output curve to move once, within the
limits set by X DRIFT, TOTAL DRIFT, and ENDS.
### COUNT Knob
The number of segments in the steps/line/curve, from 1 (just
connecting the end points) to 32. **Takes effect at the next RESET.**
### RESET Input and Button
Clicking the button or sending a trigger to
the input resets the line to its starting position, which is all zeroes.
Also applies any change to COUNT.
### STYLE Knob
Selects one of three different line types, Steps/Lines/Curves.
Changes are applied instantly.
### IN Input
Selects the horizontal position of the point on the line to be
selected. Shown on the display as a short line at the bottom of the display.
### OUT output
The vertical position of the line at the position determined by IN.
## Menu Items
### "Save curve in rack"
If set, when the rack is saved, the current position of the line
will be saved with the rack, and that position will be loaded along with the rack.

## Uses
A series of values you wish was gradually (or drastically) changing - melodies, volume levels,
waveforms, CV levels. Note that the use of a Saw wave as the input in the
sample rack is just to better illustrate the idea of it being a
transformation function; you can put whatever you like into IN. Sine wave,
oscillator output, random,... Drifter alters signals, basically.
