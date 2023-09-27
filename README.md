# Pure-Waveforms-Generator

A simple command-line tool for generating audio waveforms in WAV format.

## Usage

To use the `synth` command-line tool:

$> ./synth [OPTIONS] FILENAME

## Options

- `-w <form>`: Specify the waveform to generate. Possible values are: 
  - `sine`: Sine waveform
  - `saw`: Sawtooth waveform
  - `triangle`: Triangle waveform
  - `square`: Square waveform
  
- `-f <freq>`: Specify the frequency of the waveform in hertz (Hz). For example, `-f 440` would generate a waveform at 440 Hz.

- `-d <dur>`: Specify the duration of the waveform in seconds. For example, `-d 10` would generate a waveform that lasts for 10 seconds.

- `-p <pulse>`: Specify the number of pulses in the waveform.

## Example

To generate a sine wave with a frequency of 440 Hz, lasting for 5 seconds, with 2 pulses, saved to `output.wav`:

$> ./synth -w sine -f 440 -d 5 -p 2 output.wav
