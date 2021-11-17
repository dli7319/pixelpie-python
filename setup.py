import os
from os.path import join as pjoin
import pathlib
from setuptools import find_packages, Extension, setup
from Cython.Build import cythonize
from Cython.Distutils import build_ext


def find_in_path(name, path):
    """Find a file in a search path"""

    # Adapted fom http://code.activestate.com/recipes/52224
    for dir in path.split(os.pathsep):
        binpath = pjoin(dir, name)
        if os.path.exists(binpath):
            return os.path.abspath(binpath)
    return None


def locate_cuda():
    """Locate the CUDA environment on the system
    Returns a dict with keys 'home', 'nvcc', 'include', and 'lib64'
    and values giving the absolute path to each directory.
    Starts by looking for the CUDAHOME env variable. If not found,
    everything is based on finding 'nvcc' in the PATH.
    """

    # First check if the CUDAHOME env variable is in use
    if 'CUDAHOME' in os.environ:
        home = os.environ['CUDAHOME']
        nvcc = pjoin(home, 'bin', 'nvcc')
    else:
        # Otherwise, search the PATH for NVCC
        nvcc = find_in_path('nvcc', os.environ['PATH'])
        if nvcc is None:
            raise EnvironmentError('The nvcc binary could not be '
                                   'located in your $PATH. Either add it to your path, '
                                   'or set $CUDAHOME')
        home = os.path.dirname(os.path.dirname(nvcc))

    cudaconfig = {'home': home, 'nvcc': nvcc,
                  'include': pjoin(home, 'include'),
                  'lib64': pjoin(home, 'lib64')}
    for k, v in iter(cudaconfig.items()):
        if not os.path.exists(v):
            raise EnvironmentError('The CUDA %s path could not be '
                                   'located in %s' % (k, v))

    return cudaconfig


def customize_compiler_for_nvcc(self):
    """Inject deep into distutils to customize how the dispatch
    to gcc/nvcc works.
    If you subclass UnixCCompiler, it's not trivial to get your subclass
    injected in, and still have the right customizations (i.e.
    distutils.sysconfig.customize_compiler) run on it. So instead of going
    the OO route, I have this. Note, it's kindof like a wierd functional
    subclassing going on.
    """

    # Tell the compiler it can processes .cu
    self.src_extensions.append('.cu')

    # Save references to the default compiler_so and _comple methods
    default_compiler_so = self.compiler_so
    super = self._compile

    # Now redefine the _compile method. This gets executed for each
    # object but distutils doesn't have the ability to change compilers
    # based on source extension: we add it.
    def _compile(obj, src, ext, cc_args, extra_postargs, pp_opts):
        if os.path.splitext(src)[1] == '.cu':
            # use the cuda for .cu files
            self.set_executable('compiler_so', CUDA['nvcc'])
            # use only a subset of the extra_postargs, which are 1-1
            # translated from the extra_compile_args in the Extension class
            postargs = extra_postargs['nvcc']
        else:
            postargs = extra_postargs['gcc']

        super(obj, src, ext, cc_args, postargs, pp_opts)
        # Reset the default compiler_so, which we might have changed for cuda
        self.compiler_so = default_compiler_so

    # Inject our redefined _compile method into the class
    self._compile = _compile


# Run the customize_compiler
class custom_build_ext(build_ext):
    def build_extensions(self):
        customize_compiler_for_nvcc(self.compiler)
        build_ext.build_extensions(self)


CUDA = locate_cuda()

ext = Extension(
    "pixelpie",
    sources=["src/pixelpie.pyx", "src/run_exp.cpp", "src/lodepng.cpp",
             "src/PoissonDiskSampler.cpp", "src/cudaThrustOGL.cu", "src/glew.c"],
    libraries=['EGL', 'GL', 'cudart'],
    language="c++",
    extra_compile_args={
        'gcc': ['-g', '-O2'],
        'nvcc': ['-g', '-O2', '--compiler-options', "'-fPIC'"]
    },
    include_dirs=['.', './include', CUDA['include']]
)

HERE = pathlib.Path(__file__).parent
README = (HERE / "README.md").read_text()

setup(
    name="pixelpie",
    version="1.0.0",
    description="poisson disk sampling with cuda and opengl",
    long_description=README,
    long_description_content_type="text/markdown",
    url="https://github.com/dli7319/pixelpie-python",
    author="David Li",
    author_email="david@davidl.me",
    classifiers=[
        "Programming Language :: Python :: 3",
        "Programming Language :: Python :: 3.7",
    ],
    include_package_data=True,
    install_requires=["numpy"],
    ext_modules=cythonize(
        ext,
        compiler_directives={'language_level': "3"}),
    cmdclass={'build_ext': custom_build_ext},
)
