# Autotest your code in Assignment 1 MYFTP

## Prerequisites

- Make sure you autotest your code on a Linux machine and has perl supported. Type
`
/usr/bin/perl --version
`
to check whether it has been correctly installed.

- This tool scrutinizes your code on a coarse-grained level. Passing all the tests only suggests that your implementation are quite likely to be correct.

- For a detailed description of each test case, see below.

## Run the tool

From here onward, suppose your working directory for this assignment is `dr/`. Move the `test/` into your `dr/` folder. Make sure the following demands are met:

- The executable files for MYFTP server and client are `myftpserver` and `myftpclient`, respectively. They should be in folder `dr/`.

- Your makefile lies in `dr/` and has a phnoy target known as `clean`. Moreover, `make clean` removes `dr/myftpserver` and `dr/myftpclient`, while `make` should generate them both.

- Your machine should not run other computation-intensive jobs in parallel. Otherwise, it will affect the result of the multi-thread test.

- Your client and server should support command line arguments as on the Page 11 in your lab handout, though the echoed messages are not required except for the command `ls`, which on invocation should display the files in the current folder (in any order).

- Your client should support downloading large files.

- Your server allow at least 6 threads to run in parallel (including the one that listens for connection request).

- Your server use either `access.ini` or `access.txt` to authenticate the user. Demos of these two files are right below:
```
/* access.txt (This line is not in the file.) */
Alice 123456
Bob ILoveComputerNetwork!
```
```
/* access.ini (This line is not in the file.) */
[user]
Alice = 123456
Bob   = ILoveComputerNetwork!
```

Once you are sure these are met, simply type `cd test && ./test.pl` and you will see the autotest result. Usually each test phases will not last for more than 10 seconds. If it does happen, either your code suffers from a deadlock, or your machine is too slow.

**DO NOT MODIFY ANY FILES IN THIS FOLDER.**

## More on test cases

We test for the following cases. Incidentally, `dr/test/demomyftpclient` and `dr/test/demomyftpserver` are the demos written by TA with which we test your code.

#### make test
Test `make clean` and `make`.
 
### Test your client

#### basic func test
Test for a normal workflow of your client with the demo server. For example,
```
open localhost 43145
auth Alice abcdefghijklmnopqrstuvwxyz
ls
quit
```

#### ascii file test
Transfer short ascii files using MYFTP. (<1k in byte)

#### binary file test
Transfer short binary files using MYFTP. (<2k in byte)

#### large file test
Transfer large files using MYFTP. (about 25MB)
Every single word comes from Jin Yong's brilliant Gong Fu Stories. 

#### put test
Test for put command. Nothing special about this one.

#### bad auth test
On a failed authentication, your client should close the socket and return to the idle state in lieu of a brute shurdown.

#### delayed recv test
Make sure that your client is not receiving network bits based on the assumption of no delay.

### Test your server

#### basic func test
Your server will interact with one single client and carry out neccessary instructions.

#### multi thread test
Your server will interact with about 10 threads in parallel. Jesus! Though we deliberately avoid testing for any races, you will need thread to speed up your server!