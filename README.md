# Start Here

1. Use this template repository to create a private fork (click the green `Use this template` button and not the `Fork` button).
1. Follow the instructions in `INSTRUCTIONS.md`.
1. Give [@tes-mersive](https://github.com/tes-mersive) (`estephens@mersive.com`) collaborator access when complete.
1. Inform your Mersive contact that the assignment is complete.

# Compilation

```
cd Mersive
mkdir build
cd build
cmake ../
make install
```

## Address sanitizer
Address sanitizer allow you to detect issues with memory. To enable it just run:

```
cmake -asan=on ../
cmake make install
```

## Thread sanitizer
Thread sanitizer allows you to detect issues with thread synchronization. To enable it just run:

```
cmake -tsan=on ../
cmake make install
```

# Installation
The make target "install" will install the binary at ./build/dist/bin/httpServer.

More over this install Gtests at /build/dist/test/ directory.

# Testing
There are two types of tests. The Gtest and product test. Both of them are executed by 'make test' command

Before you execute the tests, you have to run the binary (httpServer). Make sure that you have curl, because the product test uses it.

# User interface
Once you start the binary (httpServer) you can enter several commands:
1. exit - this command will close the software
2. report - this command will report information about currently stored keys

