# -*- perl -*-
use strict;
use warnings;
use tests::tests;
check_expected (IGNORE_EXIT_CODES => 1, [<<'EOF']);
(block-counter) begin
(block-counter) create "test.txt"
(block-counter) open "test.txt"
(block-counter) write_cnt is reasonable!
(block-counter) end
EOF
pass;

