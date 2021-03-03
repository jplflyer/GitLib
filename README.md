# GitLib
This is a C++ Git Library that wraps gitlib2.

# Status
I'm really only doing what I need for my Git-Dashboard app. At this point, everything I need is implemented. Only currently supports SSH-based credentials.

Call it version 0.1.0, if I add a version.h at some point.

# Dependencies
You must have libgit2 installed. This means you also need openssl and ssh2. See the LIBS referenced in the Makefile.

We depend on my ShowLib project at: git@github.com:jplflyer/ShowLib.git. You should have that checked out and have done:

    make all install

You might have to do that separately and sudo the install, depending upon whether you can write to /usr/local.

# Building
I use Qt Creator as my IDE, and you'll see some checked in files related to it. I dislike cmake, so I maintain a Makefile. This builds on Mac and probably under Linux without changes.

    make
    sudo make install

# Usage
See the two little programs in the `programs` directory. I'll walk through some of the code from GitFetch.

You start by instantiating a Repository object, passing in the directory in question. In this case, we just use the current directory (NOT the .git subdirectory):

    Repository repo(".");

You can do a variety of things on that, but if you want to look at anything relating to the remote end, you'll need to do this:

    Remote::Pointer remote = repo.getRemote();

This is GIT URI, which probably isn't really a URI, but whatever.
    URI uri(remote->url());

This isn't currently implemented, but it will be if I ever need it. It just needs the credentials bit handled in Git.cpp.

    if (uri.isHTTP()) {
        repo.fetch(username, password);
    }

Configuration is from ShowLib. It reads your ~/.ssh/config file, keeping track of the stuff I found useful. If you do weird shit in yours, no promises.

    else {
        SSH::Configuration sshConfig;

Load can take an optional path if you don't want to read the normal one.

        sshConfig.load();

Now we need to get the credentials to use. We assume id_rsa and id_rsa.pub, but of course, yours might be named something differently. But we assume that basic naming scheme (add .pub for the public version):

        Host::Pointer key = sshConfig.getHost(uri.getHost());
        repo.fetch(key, password);
    }

That code does a very simple fetch. Error handling barely exists.
