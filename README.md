## Build
```
cd pintos/src/userprog
make
```

## Test
```
cd pintos/src/userprog
make check
```

## Debug
```
cd pintos/src/userprog
pintos --gdb --filesys-size=2 -p ./build/tests/userprog/do-nothing -a do-nothing -- -q -f run do-nothing
```
```
cd pintos/src/userprog/build
pintos-gdb ./kernel.o
debugpintos or deb or target remote localhost:1234
```

