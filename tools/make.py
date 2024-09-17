# Mahlen, make this run Python 3, not Python 2 (which is now first choice).
#   Note that Python 3 already appears to work.
# Prevent function calls with very little value (e.g., Reverse(Sine()))

from datetime import datetime
import math
import os
import random
import struct
import wave

FRAME_RATE = 1000.0
#FRAME_RATE = 11025.0   # samples per second.
#FRAME_RATE = 44100.0    # samples per second.
#FRAME_RATE = 96000.0    # samples per second.
AMPLITUDE = 16000.0     # multiplier for amplitude. Related to sampwidth in WriteFile.
LENGTH = 5             # Length of generated tones, in seconds
DATA_SIZE = int(LENGTH * FRAME_RATE)

# TODO: effects (reverb, echo) - might add some warmth
#       Some ASDR waveforms

# List of .wav files in the Samples directory.
# ALL_SAMPLES = [file_name.endswith('.wav') for file_name in os.listdir('Samples')]

def WriteFile(samples_left, sample_right, fname):
    # get ready for the wave file to be saved ...
    wav_file = wave.open(fname, "w")
    # give required parameters
    nchannels = 2
    sampwidth = 2
    framerate = int(FRAME_RATE)
    nframes = len(samples_left)
    comptype = "NONE"
    compname = "not compressed"
    # set all the parameters at once
    wav_file.setparams((nchannels, sampwidth, framerate, nframes,
        comptype, compname))
    # now write out the file ...
    for s, t in zip(samples_left, sample_right):
       # write the audio frames to file.
       wav_file.writeframes(struct.pack('h', int(s * AMPLITUDE / 2)))
       wav_file.writeframes(struct.pack('h', int(t * AMPLITUDE / 2)))
    wav_file.close()

def Timestamp():
  return datetime.now().strftime('%b %d %H-%M-%S')

for i in range(1):
  # write the synthetic wave file to ...
  fname = Timestamp() + '.wav'

  print("Generating #%d..." % i)
  samples_left = []
  samples_right = []
  for x in range(DATA_SIZE):
    samples_left.append(math.sin(2 * math.pi * x / DATA_SIZE))
    samples_right.append(math.cos(2 * math.pi * x / DATA_SIZE))

  print('Writing...')
  WriteFile(samples_left, samples_right, 'Waves\\' + fname)
  print('File: %s\n' % fname)
  print('\n\n')
  print("%s written." % fname)

print('Done!')


# Past examples:
# These are the contents of the "for x in range(...)" loop

'''
    # Makes a simple clockwise circle.
    samples_left.append(math.sin(2 * math.pi * x / DATA_SIZE))
    samples_right.append(math.cos(2 * math.pi * x / DATA_SIZE))
'''
