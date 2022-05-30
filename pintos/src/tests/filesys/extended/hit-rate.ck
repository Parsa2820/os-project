# -*- perl -*-
use strict;
use warnings;
use tests::tests;
use tests::random;
check_expected (IGNORE_USER_FAULTS => 1, [<<'EOF']);
(hit-rate) begin
(hit-rate) create "test.txt"
(hit-rate) open "test.txt"
(hit-rate) new hit rate is better than old one
(hit-rate) end
hit-rate: exit(0)
EOF
pass;