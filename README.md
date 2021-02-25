# pixelpie-python

A GPU-accelerated poisson-disk sampling algorithm.  
This is a cython wrapper around https://github.com/salivian/pixelpie.


## Installation

* Make sure opengl, glfw, and cuda things are installed.
* Run `pip install "git+https://github.com/dli7319/pixelpie-python"`

## Usage
```python
import pixelpie

# pixelpie.run(width, height, radius)
pixelpie.run(4096, 4096, 0.1) # Returns a numpy array of shape (77, 2)
```