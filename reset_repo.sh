#!/bin/bash
#
# reset_repo.sh
#
# BMC64 - A bare metal Commodore emulator for Raspberry Pi
# Copyright (C) 2025 aminch <https://github.com/aminch>
#
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program.  If not, see <https://www.gnu.org/licenses/>.
#

echo "Resetting repository to clean state..."
echo "This will remove all untracked files and revert all changes!"
echo ""

read -p "Do you want to continue? (y/N): " -n 1 -r
echo ""
if [[ ! $REPLY =~ ^[Yy]$ ]]
then
    echo "Operation cancelled."
    exit 1
fi

echo ""

# Clean and reset main repository
echo "Cleaning main repository..."
git clean -xfd
git checkout .

# Clean and reset all submodules
echo "Cleaning submodules..."
git submodule foreach --recursive 'git clean -xfd'
git submodule foreach --recursive 'git checkout .'

echo "Repository reset complete!"