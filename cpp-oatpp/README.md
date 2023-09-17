# stack-server-cpp-oatpp

Stack server implemented using C++ and Oat++.

For practice purpose, I manually implemented a reference counter for the nodes in the stack, making the copying of a stack inexpensive. For concurrent operations on a stack and the map of the stacks, a shared lock is used.

## Development

### Build and Run

#### Using CMake

**Requires** 

- `oatpp` module installed. You may run `utility/install-oatpp-modules.sh` 
script to install required oatpp modules.

```
$ mkdir build && cd build
$ cmake ..
$ make 
$ ./stack-server-exe  # - run application.

```

#### In Docker

```
$ docker build -t stack-server-cpp-oatpp .
$ docker run -p 8000:8000 -t stack-server-cpp-oatpp
```
