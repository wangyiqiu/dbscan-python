platformOk = True

import platform

if platform.system() != 'Linux':
    print("Detected an operating system other than Linux, the DBSCAN package only works with 64-bit Linux at the moment.")
    platformOk = False

import sys

if sys.version_info[0] < 3:
    print("Detected Python " + str(sys.version_info[0]) + ", the DBSCAN package requires Python 3.8+.")
    platformOk = False

if sys.version_info[0] == 3 and sys.version_info[1] < 8:
    print("Detected Python " + str(sys.version_info[0]) + "." + str(sys.version_info[1]) + ", the DBSCAN package requires Python 3.8+")
    platformOk = False

if not platformOk:
    print("Please refer to https://github.com/wangyiqiu/dbscan-python for more specific system requirements. Please also make sure the latest version of the package (0.0.9) is installed.")
else:
    from dbscan.DBSCAN import DBSCAN
