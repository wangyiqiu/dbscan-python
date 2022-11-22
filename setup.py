import setuptools
from setuptools.extension import Extension
import numpy
import ast
import sys
import os

# Override wheel's default ABI tag.
try:
    if sys.implementation.name != 'cpython': raise
    import wheel.bdist_wheel

    current_tag = f"cp{sys.version_info.major}{sys.version_info.minor}"
    old_initialize_options = wheel.bdist_wheel.bdist_wheel.initialize_options
    def initialize_options(self):
        old_initialize_options(self)
        self.py_limited_api = current_tag
    wheel.bdist_wheel.bdist_wheel.initialize_options = initialize_options
except:
    # Give up if it doesn't work. Not a big deal.
    pass

# Read README.md and set it as the description
with open("README.md", "r", encoding="utf-8") as fh:
    long_description = fh.read()

# Get the module version from src/dbscanmodule.cpp
with open("src/dbscanmodule.cpp", "r") as fh:
    version = fh.readline().strip()
    assert version.startswith('#define DBSCAN_VERSION ')
    version = ast.literal_eval(version.replace('#define DBSCAN_VERSION ', ''))
    assert isinstance(version, str)


if os.name == 'nt':
    # Windows compile time arguments
    extra_compile_args = ["/std:c++17", "/Ot"]
else:
    # Mac/Linux GCC compile time arguments
    extra_compile_args = ["-std=c++17", "-pthread", "-g", "-O3", "-fPIC", "-Wno-unused"]

setuptools.setup(
    name="dbscan",
    version=version,
    author="Yiqiu Wang",
    author_email="yiqiu_wang@icloud.com",
    description="Theoretically efficient and practical parallel DBSCAN",
    long_description=long_description,
    long_description_content_type="text/markdown",
    keywords='cluster clustering density dbscan',
    url="https://github.com/wangyiqiu/dbscan-python",
    license='MIT',
    packages=('dbscan',),
    package_dir={'dbscan': 'src'},
    ext_modules=[Extension(
        "dbscan._dbscan",
        ["src/dbscanmodule.cpp", "src/capi.cpp"],
        language = 'c++',
        extra_compile_args=extra_compile_args,
        include_dirs=[numpy.get_include(), 'include'],
        py_limited_api=True,
        define_macros=[('Py_LIMITED_API', '0x03020000'), ('NPY_NO_DEPRECATED_API', 'NPY_1_7_API_VERSION')]
    )],
    classifiers=[
        'Development Status :: 2 - Pre-Alpha',
        'Intended Audience :: Science/Research',
        'Intended Audience :: Developers',
        "License :: OSI Approved :: MIT License",
        'Programming Language :: C++',
        'Programming Language :: Python :: 3',
        'Topic :: Software Development',
        'Topic :: Scientific/Engineering',
    ],
    python_requires=f'>={sys.version_info.major}.{sys.version_info.minor},<4',
    install_requires=[
        f'numpy>={numpy.__version__},<2'
    ],
    extras_require={
        'scikit-learn': ['scikit-learn'],
        'example': ['scikit-learn', 'matplotlib'],
    },
    zip_safe=False,
)
