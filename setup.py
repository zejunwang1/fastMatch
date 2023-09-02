#
# Copyright (c) 2023-present, Zejun Wang.
# All rights reserved.
#
# This source code is licensed under the MIT license found in the
# LICENSE file in the root directory of this source tree.
#

from setuptools import setup, Extension
from setuptools.command.build_ext import build_ext
import sys
import setuptools
import io

# Based on https://github.com/pybind/python_example
class get_pybind_include(object):
    """Helper class to determine the pybind11 include path
    The purpose of this class is to postpone importing pybind11
    until it is actually installed, so that the ``get_include()``
    method can be invoked. """

    def __init__(self, user=False):
        try:
            import pybind11
        except ImportError:
            if subprocess.call([sys.executable, '-m', 'pip', 'install', 'pybind11']):
                raise RuntimeError('pybind11 install failed.')

        self.user = user

    def __str__(self):
        import pybind11
        return pybind11.get_include(self.user)

ext_modules = [
    Extension(
        'fast_match',
        ['pybind.cpp'],
        include_dirs=[
            # Path to pybind11 headers
            get_pybind_include(),
            get_pybind_include(user=True),
            'include'
        ],
        language='c++',
        extra_compile_args=["-O3"]
    ),
]

# As of Python 3.6, CCompiler has a `has_flag` method.
# cf http://bugs.python.org/issue26689
def has_flag(compiler, flags):
    """Return a boolean indicating whether a flag name is supported on
    the specified compiler.
    """
    import tempfile
    with tempfile.NamedTemporaryFile('w', suffix='.cpp') as f:
        f.write('int main (int argc, char **argv) { return 0; }')
        try:
            compiler.compile([f.name], extra_postargs=flags)
        except setuptools.distutils.errors.CompileError:
            return False
    return True

def cpp_flag(compiler):
    """Return the -std=c++[11/14/17] compiler flag.
    The c++17 is prefered over c++14 (when it is available).
    The c++14 is prefered over c++11 (when it is available).
    """
    if has_flag(compiler, ['-std=c++17']):
        return '-std=c++17'
    elif has_flag(compiler, ['-std=c++14']):
        return '-std=c++14'
    elif has_flag(compiler, ['-std=c++11']):
        return '-std=c++11'
    else:
        raise RuntimeError('Unsupported compiler -- at least C++11 support '
                           'is needed!')

class BuildExt(build_ext):
    """A custom build extension for adding compiler-specific options."""
    c_opts = {
        'msvc': ['/EHsc'],
        'unix': [],
    }

    def build_extensions(self):
        if sys.platform == 'darwin':
            mac_osx_version = float('.'.join(platform.mac_ver()[0].split('.')[:2]))
            os.environ['MACOSX_DEPLOYMENT_TARGET'] = str(mac_osx_version)
            all_flags = ['-stdlib=libc++', '-mmacosx-version-min=10.7']
            if has_flag(self.compiler, [all_flags[0]]):
                self.c_opts['unix'] += [all_flags[0]]
            elif has_flag(self.compiler, all_flags):
                self.c_opts['unix'] += all_flags
            else:
                raise RuntimeError(
                    'libc++ is needed! Failed to compile with {} and {}.'.
                    format(" ".join(all_flags), all_flags[0])
                )
        ct = self.compiler.compiler_type
        opts = self.c_opts.get(ct, [])
        extra_link_args = []
      
        if ct == 'unix':
            opts.append('-DVERSION_INFO="%s"' % self.distribution.get_version())
            opts.append(cpp_flag(self.compiler))
            if has_flag(self.compiler, ['-fvisibility=hidden']):
                opts.append('-fvisibility=hidden')
        elif ct == 'msvc':
            opts.append(
                '/DVERSION_INFO=\\"%s\\"' % self.distribution.get_version()
            )

        for ext in self.extensions:
            ext.extra_compile_args.extend(opts)
            ext.extra_link_args = extra_link_args
        build_ext.build_extensions(self)

def _get_readme():
    """
    Use pandoc to generate rst from md.
    pandoc --from=markdown --to=rst --output=README.rst README.md
    """
    with io.open("README.rst", encoding='utf-8') as fid:
        return fid.read()

setup(
    name='fast_match',
    version='0.1.0',
    author='wangzejun',
    author_email='wangzejunscut@126.com',
    url='https://github.com/zejunwang1/fastMatch',
    description='Efficient exact string matching tool',
    long_description=_get_readme(),
    ext_modules=ext_modules,
    install_requires=['pybind11>=2.2'],
    cmdclass={'build_ext': BuildExt},
    zip_safe=False,
)

