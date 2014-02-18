MDR: Make Diffs Readable
========================

Overview
--------

Sometimes diff output can be a pain to read. MDR is a simple tool that reads unified diff output from your favorite version control system or straight from the diff program itself and displays a graphical version of the same information complete with colored highlights of the changes and line numbers.

Installation
------------

All you need is the mdr executable (mdr.exe on Windows). Move or copy it to a place where you like to keep executables. I like to use ~/bin. Make sure this location is in your PATH.

Use
---

The easiest way to use MDR is to pipe diff output from another program to it. For example, if you are using git for source control, you can view the current changes you've made in MDR with the following:

    $ git diff | mdr

The same sort of thing works in any situation where you would otherwise be looking at diff output. Some other examples:

    $ hg diff | mdr

    $ hg diff -r30:31 | mdr

    $ diff -u file1.txt file2.txt | mdr

You can also pipe in diff output that has been saved to a file:

    $ diff -u file1.txt file2.txt > changes.diff
    $ mdr < changes.diff

Development
-----------

### Mac and Linux

#### Setup

On Mac, install the Xcode developer tools and you should be ready to go.

On Linux, install WebKitGTK+ and GTK+ 3 development headers. On Ubuntu they are available in the `libwebkitgtk-3.0-dev` package. On ArchLinux you'll find them in the `webkitgtk` package.

#### Build

    rake

You can also run `rake clobber` and `rake clean`

`rake install` will copy the mdr binary to ~/bin/ which can be handy during development.

### Windows

#### Requirements

- gcc
- windres
- xxd
- rake

#### Setup

This is how I set up my dev environment. You might find another set of steps that works for you. In theory, you just need to make sure the requirements are all installed and on your PATH.

1. Install MinGW
    - mingw32-base
    - msys-vim (for xxd)
    - msys-base (for MSYS command line, optional)

3. Install Ruby
    - When done you may need to `gem install rake`

#### Build

You’ll either need to use the MSYS command line, or edit your PATH to include the following paths (or equivalent for your install of MinGW):

- C:\MinGW\bin
- C:\MinGW\msys\1.0\bin

More information on MinGW setup see http://www.mingw.org/wiki/Getting_Started

To actually build mdr:

    rake

You can also run `rake clobber` and `rake clean`

### Ubuntu Linux via Vagrant

These instructions are useful if you are not running Linux but would like to build a Linux binary on your platform.

Prerequisites: [Vagrant](http://vagrantup.com/) and [VirtualBox](https://www.virtualbox.org/).

**Note that this currently fails due to issue #21** but it would look something like this:

    vagrant up
    vagrant ssh
    cd /vagrant
    rake
