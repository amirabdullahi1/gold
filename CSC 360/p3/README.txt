CSc 360: Operating Systems (Fall 2025)
Programming Assignment 3 (P3)
A Simple File System (SFS)

Implementation Status

All parts of the assignment (Parts I–IV) have been implemented and tested on linux.csc.uvic.ca. The following summarizes the features implemented and their correctness.

Part I – diskinfo
-----------------
Status: Implemented correctly
Description: Displays information about the file system superblock and FAT.
Testing: Verified using test.img and non-empty.img. Outputs match the expected format exactly.

Part II – disklist
------------------
Status: Implemented correctly
Description: Lists files and directories in the root or a specified sub-directory.
Testing: Verified against test.img, non-empty.img, and large.img. Output matches the specified format exactly, including file type, file size, file name, and creation time.

Part III – diskget
------------------
Status: Implemented correctly
Description: Copies a file from the file system image to the host OS.
Testing: Successfully retrieves files from test.img and large.img. Outputs correct error messages when a file is not found.
Validation: SHA1 checksums confirm that copied files are identical to the originals.

Example:
aabdul@Mac p3 % ./diskget large.img /dir1/dir2/dir3/dir4/dir5/video.mp4 vm.mp4 
aabdul@Mac p3 % sha1sum vm.mp4 
13ab263bf13fe4d836cce90d077897a08e394ed2  vm.mp4

Part IV – diskput
-----------------
Status: Implemented correctly
Description: Copies a file from the host OS into the file system image. Can create subdirectories if they do not exist.
Testing: Successfully adds files to test.img and large.img. Verified via disklist that files are placed correctly and subdirectories are created if needed.

Example:
aabdul@Mac p3 % ./diskput test.img README.txt /readme.txt
aabdul@Mac p3 % ./disklist test.img /                    
D          0                              . 2025/10/31 17:35:05
F       2348                     readme.txt 2025/11/28 14:09:32

Compilation
-----------
Status: Works correctly
Command: make in the root directory produces the following executables:
diskinfo
disklist
diskget
diskput

All programs return 0 on success and non-zero on error as specified.

Conclusion
----------
All features from Parts I–IV are fully implemented, tested, and functioning correctly according to the assignment specification.
