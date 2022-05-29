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
pintos --gdb --filesys-size=2 -p ./build/tests/userprog/args-single -a args-single -- -q -f run "args-single arg-one"
```
```
cd pintos/src/userprog/build
pintos-gdb ./kernel.o
debugpintos or deb or target remote localhost:1234
```

## Phase 3 Tests
- [ ] tests/filesys/base/lg-create
- [ ] tests/filesys/base/lg-full
- [ ] tests/filesys/base/lg-random
- [ ] tests/filesys/base/lg-seq-block
- [ ] tests/filesys/base/lg-seq-random
- [ ] tests/filesys/base/sm-create
- [ ] tests/filesys/base/sm-full
- [ ] tests/filesys/base/sm-random
- [ ] tests/filesys/base/sm-seq-block
- [ ] tests/filesys/base/sm-seq-random
- [ ] tests/filesys/base/syn-read
- [ ] tests/filesys/base/syn-remove
- [ ] tests/filesys/base/syn-write
- [ ] tests/filesys/extended/dir-empty-name
- [ ] tests/filesys/extended/dir-mk-tree
- [ ] tests/filesys/extended/dir-mkdir
- [ ] tests/filesys/extended/dir-open
- [ ] tests/filesys/extended/dir-over-file
- [ ] tests/filesys/extended/dir-rm-cwd
- [ ] tests/filesys/extended/dir-rm-parent
- [ ] tests/filesys/extended/dir-rm-root
- [ ] tests/filesys/extended/dir-rm-tree
- [ ] tests/filesys/extended/dir-rmdir
- [ ] tests/filesys/extended/dir-under-file
- [ ] tests/filesys/extended/dir-vine
- [ ] tests/filesys/extended/grow-create
- [ ] tests/filesys/extended/grow-dir-lg
- [ ] tests/filesys/extended/grow-file-size
- [ ] tests/filesys/extended/grow-root-lg
- [ ] tests/filesys/extended/grow-root-sm
- [ ] tests/filesys/extended/grow-seq-lg
- [ ] tests/filesys/extended/grow-seq-sm
- [ ] tests/filesys/extended/grow-sparse
- [ ] tests/filesys/extended/grow-tell
- [ ] tests/filesys/extended/grow-two-files
- [ ] tests/filesys/extended/syn-rw
- [ ] tests/filesys/extended/dir-empty-name-persistence
- [ ] tests/filesys/extended/dir-mk-tree-persistence
- [ ] tests/filesys/extended/dir-mkdir-persistence
- [ ] tests/filesys/extended/dir-open-persistence
- [ ] tests/filesys/extended/dir-over-file-persistence
- [ ] tests/filesys/extended/dir-rm-cwd-persistence
- [ ] tests/filesys/extended/dir-rm-parent-persistence
- [ ] tests/filesys/extended/dir-rm-root-persistence
- [ ] tests/filesys/extended/dir-rm-tree-persistence
- [ ] tests/filesys/extended/dir-rmdir-persistence
- [ ] tests/filesys/extended/dir-under-file-persistence
- [ ] tests/filesys/extended/dir-vine-persistence
- [ ] tests/filesys/extended/grow-create-persistence
- [ ] tests/filesys/extended/grow-dir-lg-persistence
- [ ] tests/filesys/extended/grow-file-size-persistence
- [ ] tests/filesys/extended/grow-root-lg-persistence
- [ ] tests/filesys/extended/grow-root-sm-persistence
- [ ] tests/filesys/extended/grow-seq-lg-persistence
- [ ] tests/filesys/extended/grow-seq-sm-persistence
- [ ] tests/filesys/extended/grow-sparse-persistence
- [ ] tests/filesys/extended/grow-tell-persistence
- [ ] tests/filesys/extended/grow-two-files-persistence
- [ ] tests/filesys/extended/syn-rw-persistence