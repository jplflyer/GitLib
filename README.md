# GitLib
This is a C++ Git Library that wraps gitlib2.

# Dependencies
You must have libgit2 installed.

We depend on my ShowLib project at: git@github.com:jplflyer/ShowLib.git. You should have that checked out and have done:

    make all install

You might have to do that separately and sudo the install, depending upon whether you can write to /usr/local.

# Building
I use Qt Creator as my IDE, and you'll see some checked in files related to it. I dislike cmake, so I maintain a Makefile. This builds on Mac and probably under Linux without changes.
