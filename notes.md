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
- [x] tests/filesys/base/lg-create
- [x] tests/filesys/base/lg-full
- [x] tests/filesys/base/lg-random
- [x] tests/filesys/base/lg-seq-block
- [x] tests/filesys/base/lg-seq-random
- [x] tests/filesys/base/sm-create
- [x] tests/filesys/base/sm-full
- [x] tests/filesys/base/sm-random
- [x] tests/filesys/base/sm-seq-block
- [x] tests/filesys/base/sm-seq-random
- [ ] tests/filesys/base/syn-read
- [x] tests/filesys/base/syn-remove
- [ ] tests/filesys/base/syn-write
- [x] tests/filesys/extended/block-counter
- [x] tests/filesys/extended/dir-empty-name
- [x] tests/filesys/extended/dir-mk-tree
- [x] tests/filesys/extended/dir-mkdir
- [x] tests/filesys/extended/dir-open
- [x] tests/filesys/extended/dir-over-file
- [ ] tests/filesys/extended/dir-rm-cwd
- [ ] tests/filesys/extended/dir-rm-parent
- [x] tests/filesys/extended/dir-rm-root
- [x] tests/filesys/extended/dir-rm-tree
- [ ] tests/filesys/extended/dir-rmdir
- [x] tests/filesys/extended/dir-under-file
- [ ] tests/filesys/extended/dir-vine
- [x] tests/filesys/extended/grow-create
- [x] tests/filesys/extended/grow-dir-lg
- [x] tests/filesys/extended/grow-file-size
- [x] tests/filesys/extended/grow-root-lg
- [x] tests/filesys/extended/grow-root-sm
- [ ] tests/filesys/extended/grow-seq-lg
- [x] tests/filesys/extended/grow-seq-sm
- [x] tests/filesys/extended/grow-sparse
- [x] tests/filesys/extended/grow-tell
- [x] tests/filesys/extended/grow-two-files
- [ ] tests/filesys/extended/syn-rw
- [x] tests/filesys/extended/hit-rate
- [ ] tests/filesys/extended/block-counter-persistence
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
- [ ] tests/filesys/extended/hit-rate-persistence
