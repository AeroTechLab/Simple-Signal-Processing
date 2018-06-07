# Simple Signal Processing

Basic operations for generic signal processing, that could be applied to a sample-by-sample real-time application:

- Perform offset and max-min range detection before actual measurement
- Set artificial gain and additional offset values
- Set processing to automatically rectifies and/or normalizes signal (based on range detection)
- Automatically apply low-pass filtering (smoothing) to signal (generated from desired [relative cut-off frequency](https://en.wikipedia.org/wiki/Digital_filter))

### Documentation

Descriptions of how the functions and data structures work are available at the [Doxygen](http://www.stack.nl/~dimitri/doxygen/index.html)-generated [documentation pages](https://labdin.github.io/Simple-Signal-Processing/signal__processing_8h.html)
