import setuptools
from setuptools.extension import Extension

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
    packages=[''],#setuptools.find_packages(),
    package_dir={'': '.'},
    package_data={'': ['dbscan/DBSCAN.cpython-38-x86_64-linux-gnu.so']},
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
)
