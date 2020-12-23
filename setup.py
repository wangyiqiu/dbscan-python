import setuptools
from setuptools.extension import Extension

with open("README.md", "r", encoding="utf-8") as fh:
    long_description = fh.read()

setuptools.setup(
    name="dbscan",
    version="0.0.5",
    author="Yiqiu Wang",
    author_email="yiqiu_wang@icloud.com",
    description="Theoretically efficient and practical parallel DBSCAN",
    long_description=long_description,
    long_description_content_type="text/markdown",
    url="https://github.com/wangyiqiu/dbscan-python",
    packages=setuptools.find_packages(),
    package_dir={'': '.'},
    package_data={'': ['dbscan/DBSCAN.cpython-38-x86_64-linux-gnu.so']},
    classifiers=[
        "Programming Language :: Python :: 3",
        "License :: OSI Approved :: MIT License",
        "Operating System :: POSIX :: Linux",
    ],
    python_requires='>=3.8',
)
