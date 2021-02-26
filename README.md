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
pixelpie.run(4096, 4096, 10/4096) # Returns a numpy array of shape (112090, 2)
# Note that the values are in [0,1]x[0,1]
```

# Comparison with bridson
```
result = np.asarray(poisson_disc_samples(width=4096, height=4096, r=10))
result = pixelpie.run(width=4096, height=4096, r=10/4096)

#bridson returns (84881, 2) tensor in 1.6675410270690918 seconds
#pixelpie returns (112090, 2) tensor in 0.25662803649902344 seconds
```
