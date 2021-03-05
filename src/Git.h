#pragma once

#include <memory>
#include <mutex>
#include <string>

#include <git2.h>

#include <showlib/SSHConfiguration.h>

//======================================================================
// Wrappers around libgit2.
//======================================================================

namespace Git {
    class Reference;
    class Remote;
    class Repository;
    class URI;
}

/**
 * Git URLs may or may not resemble other URLs, so I have a special class for it.
 *
 * ssh://[user@]server/project.git
 * [user@]server:project.git
 * https://example.com[:port]/gitproject.git
 */
class Git::URI {
public:
    URI() = default;
    URI(const std::string &);

    void setURL(const std::string &);

    bool isSSH() const { return ssh; }
    bool isHTTP() const { return http; }

    std::string getRaw() const { return raw; }
    std::string getHost() const { return host; }
    std::string getUsername() const { return username; }
    std::string getProject() const { return project; }

private:
    std::string		raw;
    std::string		host;
    std::string		username;
    std::string		project;
    bool			ssh = false;
    bool			http = false;
};

/**
 * The remote portion of a local repository.
 */
class Git::Remote {
public:
    typedef std::shared_ptr<Remote> Pointer;

    Remote(Git::Repository &);
    virtual ~Remote();

    bool				isValid() const { return valid; }

    git_remote *		gitRemote() { return origin; }

    std::string url();
    std::string remoteName();
    std::string name();

private:
    git_remote *		origin = nullptr;
    URI					uri;
    bool				valid = false;
};

class Git::Reference {
public:
    typedef std::shared_ptr<Reference> Pointer;

    Reference(Git::Repository &);
    virtual ~Reference();

    std::string currentBranch();

private:
    Git::Repository &	repository;
    git_reference *		reference = nullptr;

    void free();
};


/**
 * A local repository.
 */
class Git::Repository {
public:
    typedef std::shared_ptr<Repository> Pointer;

    Repository(const std::string &dir);
    virtual ~Repository();

    /** Be very careful about this. */
    git_repository *	gitRepository() { return repository; }

    bool				isValid() const { return valid; }

    Remote::Pointer		getRemote();
    std::string			currentBranch();
    int					commitsBehindRemote();
    int					commitsAheadRemote();

    void				fetch(ShowLib::SSH::Host::Pointer host, const std::string &pw);
    void				fetch(const std::string &username, const std::string &pw);

private:
    std::mutex			myMutex;
    Remote::Pointer		remote = nullptr;
    git_repository *	repository = nullptr;
    std::string			directory;
    bool				valid = false;
};

