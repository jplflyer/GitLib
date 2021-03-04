#include <iostream>

#include <showlib/StringUtils.h>

#include "Git.h"

using std::cout;
using std::cerr;
using std::endl;
using std::string;

using namespace Git;
using namespace ShowLib;
using namespace ShowLib::SSH;



//======================================================================
// Repositories.
//======================================================================

/**
 * Constructor. On completion, isValid() will return true if
 * the git library was able to open this directory.
 */
Repository::Repository(const std::string &dir)
    : directory(dir)
{
    git_libgit2_init();

    int error = git_repository_open(&repository, directory.c_str());
    if (!error) {
        valid = true;
    }
}

/**
 * Destructor.
 */
Repository::~Repository() {
    if (repository != nullptr) {
        git_repository_free(repository);
        repository = nullptr;
    }

    git_libgit2_shutdown();
}

/**
 * Get the pointer to the remote server.
 */
Remote::Pointer Repository::getRemote() {
    if (remote == nullptr) {
        remote = std::make_shared<Remote>(*this);
    }
    return remote;
}

/**
 * What is our current repository?
 */
string Repository::currentBranch() {
    Reference ref(*this);
    return ref.currentBranch();

}

typedef  std::pair<Host::Pointer, std::string> SSHCredentialsPayload;

int ssh_creds(git_cred **out, const char *url, const char *username, unsigned int types, void *payloadIn) {
    SSHCredentialsPayload * payload = static_cast<SSHCredentialsPayload *>(payloadIn);

    string privKey = payload->first->getIdentityFile();
    string pubKey = privKey + ".pub";

    int rv = git_credential_ssh_key_new(out, username, pubKey.c_str(), privKey.c_str(), payload->second.c_str());

    if (rv) {
        cout << "Error: " << git_error_last()->message << endl;
    }
    return rv;
}

int basic_creds(git_cred **out, const char *url, const char *username, unsigned int types, void *payload) {
    int rv = git_credential_ssh_key_from_agent(out, "jpl@showpage.org");
    if (rv) {
        cout << "Error: " << git_error_last()->message << endl;
    }
    return rv;
}

void
Repository::fetch(Host::Pointer host, const std::string &password) {
    SSHCredentialsPayload payload;
    payload.first = host;
    payload.second = password;

    git_fetch_options fetchOptions = GIT_FETCH_OPTIONS_INIT;
    fetchOptions.callbacks.credentials = ssh_creds;
    fetchOptions.callbacks.payload = &payload;

    if (git_remote_fetch(getRemote()->gitRemote(), NULL, &fetchOptions, "fetch") < 0) {
        cerr << "Fetch Error: " << git_error_last()->message << endl;
    }
}

void
Repository::fetch(const std::string &, const std::string &) {
    cout << "Do an HTTPS-based fetch." << endl;
}

/**
 * How many commits are on the server but we don't have?
 */
int
Repository::commitsBehindRemote() {
    int rv = 0;
    string branchName = currentBranch();
    string localRefName = string{"refs/remotes/origin/"} + branchName;
    string remoteRefName = string{"refs/heads/"} + branchName;

    git_oid id;
    git_revwalk *	walker = nullptr;
    git_revwalk_new(&walker, repository);

    git_revwalk_push_ref(walker, localRefName.c_str());
    git_revwalk_hide_ref(walker, remoteRefName.c_str());

    while (!git_revwalk_next(&id, walker)) {
        ++rv;
    }

    if (walker != nullptr) {
        git_revwalk_free(walker);
    }

    return rv;
}

/**
 * How many commits are we ahead of the server?
 */
int
Repository::commitsAheadRemote() {
    int rv = 0;
    string branchName = currentBranch();
    string localRefName = string{"refs/remotes/origin/"} + branchName;
    string remoteRefName = string{"refs/heads/"} + branchName;

    git_oid id;
    git_revwalk *	walker = nullptr;
    git_revwalk_new(&walker, repository);

    git_revwalk_push_ref(walker, remoteRefName.c_str());
    git_revwalk_hide_ref(walker, localRefName.c_str());

    while (!git_revwalk_next(&id, walker)) {
        ++rv;
    }

    if (walker != nullptr) {
        git_revwalk_free(walker);
    }

    return rv;
}

//======================================================================
// Remote servers.
//======================================================================

/**
 * Constructor. We need the repo in order to create our origin.
 */
Remote::Remote(Repository &repo) {
    int error = git_remote_lookup(&origin, repo.gitRepository(), "origin");
    valid = error == 0;
    if (!valid) {
        cerr << "Failed to initialize remote." << endl;
    }
}

/**
 * Destructor.
 */
Remote::~Remote() {
    if (origin != nullptr) {
        git_remote_free(origin);
        origin = nullptr;
    }
}

/**
 * Get our URL.
 */
string Remote::url() {
    return git_remote_url(origin);
}

string Remote::name() {
    return git_remote_name(origin);
}

//======================================================================
// References. At least so far, these are fairly transitory.
//======================================================================

/**
 * Constructor.
 */
Reference::Reference(Repository &repo)
    : repository(repo)
{
}

/**
 * Destructor.
 */
Reference::~Reference() {
    free();
}

/**
 * Free our resources.
 */
void Reference::free() {
    if (reference != nullptr) {
        git_reference_free(reference);
        reference = nullptr;
    }
}

/**
 * Get the name of the current branch.
 */
std::string Reference::currentBranch() {
    free();

    string branchName;
    int error = 0;

    error = git_repository_head(&reference, repository.gitRepository());
    if (error == GIT_EUNBORNBRANCH || error == GIT_ENOTFOUND) {
        // I don't think we should hit this in our usage
    }
    else if (error == 0) {
        branchName = git_reference_shorthand(reference);
    }
    else {
        cerr << "Got an error looking up the branch: " << git_error_last() << endl;
    }

    free();
    return branchName;
}

//======================================================================
// URIs
//======================================================================

/**
 * Constructor.
 */
URI::URI(const std::string &value) {
    raw = value;

    auto parts = ShowLib::splitPair(raw, ":");

    //--------------------------------------------------
    // handle https://example.com/gitproject.git
    //--------------------------------------------------
    if (parts.first == "https") {
        http = true;

        auto hostAndProject = ShowLib::splitPair(parts.second.substr(2), "/");
        host = hostAndProject.first;
        project = hostAndProject.second;
    }

    //--------------------------------------------------
    // ssh://[user@]server/project.git
    //--------------------------------------------------
    else if (parts.first == "ssh") {
        ssh = true;

        auto hostAndProject = ShowLib::splitPair(parts.second.substr(2), "/");
        auto hostAndUser = ShowLib::splitPair(hostAndProject.first, "@");

        if (hostAndUser.second.length() == 0) {
            host = hostAndProject.first;
        }
        else {
            username = hostAndUser.first;
            host = hostAndUser.second;
        }

        project = hostAndProject.second;
    }

    //--------------------------------------------------
    // [user@]server:project.git
    //--------------------------------------------------
    else {
        ssh = true;
        auto hostAndUser = ShowLib::splitPair(parts.first, "@");

        if (hostAndUser.second.length() == 0) {
            host = parts.first;
        }
        else {
            username = hostAndUser.first;
            host = hostAndUser.second;
        }

        project = parts.second;
    }
}
