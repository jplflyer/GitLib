#include <iostream>

#include "../src/Git.h"

using std::cout;
using std::endl;
using std::string;

using namespace Git;

/**
 * A little bit of info.
 */
int main(int, char **) {
    git_libgit2_init();

    Repository repo(".");
    Remote::Pointer remote = repo.getRemote();
    string urlString = remote->url();
    URI uri(urlString);

    cout << "We're on branch: " << repo.currentBranch() << endl;
    cout << "Remote URL: " << urlString << endl;

    cout << "Is SSH: " << uri.isSSH()
         << ". Host: " << uri.getHost()
         << ". User: " << uri.getUsername()
         << ". Project: " << uri.getProject()
         << ". Name: " << remote->name()
         << endl;

    cout << "Commits ahead: " << repo.commitsAheadRemote()
         << ". Behind:" << repo.commitsBehindRemote()
         << endl;

    git_libgit2_shutdown();
    return 0;
}
