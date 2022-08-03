import setuptools
from setuptools.extension import Extension
from Cython.Build import cythonize
import numpy

with open("README.md", "r", encoding="utf-8") as fh:
    long_description = fh.read()

setuptools.setup(
    name="dbscan",
    version="0.0.9",
    author="Yiqiu Wang",
    author_email="yiqiu_wang@icloud.com",
    description="Theoretically efficient and practical parallel DBSCAN",
    long_description=long_description,
    long_description_content_type="text/markdown",
    keywords='cluster clustering density dbscan',
    url="https://github.com/wangyiqiu/dbscan-python",
    license='MIT',
    packages=setuptools.find_packages(where='src'),
    package_dir={'': 'src'},
    ext_modules=cythonize(
        Extension(
            "dbscan",
            ["src/dbscan.pyx"],
            language = 'c++',
            extra_compile_args=["-O3", "-Isrc", "-std=c++17", "-pthread", "-g", "-O3", "-fPIC"],
            language_level = "3",
            include_dirs=[numpy.get_include()],
        )
    ),
    classifiers=[
        'Development Status :: 2 - Pre-Alpha',
        'Intended Audience :: Science/Research',
        'Intended Audience :: Developers',
        "License :: OSI Approved :: MIT License",
        'Programming Language :: C++',
        'Programming Language :: Python :: 3.8',
        'Topic :: Software Development',
        'Topic :: Scientific/Engineering',
        "Operating System :: POSIX :: Linux",
    ],
    python_requires='>=3.8',
    install_requires=[
       'numpy'
    ],
)
