# Simple Signal Processing

Basic operations for generic signal processing, that could be applied to a sample-by-sample real-time application:

- Perform offset and max-min range detection before actual measurement
- Set artificial gain and additional offset values
- Set processing to automatically rectify and normalize signal (based on range detection)
- Automatically apply low-pass (smoothing) and high-pass (offset removal) 2nd order Butterworth filtering to signal (generated from desired [relative cut-off frequency](https://en.wikipedia.org/wiki/Digital_filter))

### Documentation

Descriptions of how the functions and data structures work are available at the [Doxygen](http://www.stack.nl/~dimitri/doxygen/index.html)-generated [documentation pages](https://eesc-mkgroup.github.io/Simple-Signal-Processing/signal__processing_8h.html)
