#!/bin/bash
../a -r -p"fill=\"none\" stroke=\"red\" stroke-width=\"2\"" -ez -m"1 0 0 1 100 100" "M 100 0 A 100 50 0 1 1 100 -1"
../a -r -p"fill=\"none\" stroke=\"green\" stroke-width=\"2\"" -ez -m".70710678 .70710678 -0.70710678 .70710678 100 100" "M 100 0 A 100 50 0 1 1 100 -1"
../a -r -p"fill=\"none\" stroke=\"blue\" stroke-width=\"2\"" -ez -m"0 1 -1 0 100 100" "M 100 0 A 100 50 0 1 1 100 -1"
../a -r -p"fill=\"none\" stroke=\"magenta\" stroke-width=\"2\"" -ez -m"-.70710678 .70710678 -0.70710678 -.70710678 100 100" "M 100 0 A 100 50 0 1 1 100 -1"

