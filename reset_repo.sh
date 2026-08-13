#!/bin/bash

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