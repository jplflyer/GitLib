#include <iostream>

#include <showlib/OptionHandler.h>

#include "../src/Git.h"

using std::cout;
using std::endl;
using std::string;

using namespace Git;
using namespace ShowLib;
using namespace ShowLib::SSH;

/**
 * A little bit of info.
 */
int main(int argc, char **argv) {
    OptionHandler::ArgumentVector args;
    string username;
    string password;

    args.addArg  ("username", [&](const char *val) { username = val; }, "uname", "Your https username.");
    args.addArg  ("password", [&](const char *val) { password = val; }, "pw", "Your password (basic or SSH).");

    if (!OptionHandler::handleOptions(argc, argv, args)) {
        return 0;
    }

    Repository repo(".");
    Remote::Pointer remote = repo.getRemote();
    URI uri(remote->url());

    if (uri.isHTTP()) {
        repo.fetch(username, password);
    }
    else {
        Configuration sshConfig;

        sshConfig.load();
        Host::Pointer key = sshConfig.getHost(uri.getHost());
        repo.fetch(key, password);
    }

    return 0;
}
