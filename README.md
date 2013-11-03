Development
-----------

### Mac

#### Setup

Install the Xcode developer tools and you should be ready to go.

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