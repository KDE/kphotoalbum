#!/bin/sh -e
# SPDX-FileCopyrightText: none
# SPDX-License-Identifier: CC0-1.0
# Sync the master branch with 5.x (the tracking branch for neon/dev-stable)

echo "=== Updating Neon/dev-stable branch (5.x) ==="
git checkout 5.x
git pull
git merge master
git push
git checkout master
