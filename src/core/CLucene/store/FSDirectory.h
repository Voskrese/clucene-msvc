/*------------------------------------------------------------------------------
* Copyright (C) 2003-2006 Ben van Klinken and the CLucene Team

* Updated by https://github.com/farfella/.
*
* Distributable under the terms of either the Apache License (Version 2.0) or
* the GNU Lesser General Public License, as specified in the COPYING file.
------------------------------------------------------------------------------*/
#ifndef _lucene_store_FSDirectory_
#define _lucene_store_FSDirectory_

#include "Directory.h"
#include "IndexInput.h"
#include "IndexOutput.h"
#include <string>
#include <vector>

CL_CLASS_DEF(util, StringBuffer)

CL_NS_DEF(store)

/**
* Straightforward implementation of {@link lucene::store::Directory} as a directory of files.
* <p>If the system property 'disableLuceneLocks' has the String value of
* "true", lock creation will be disabled.
*
* @see Directory
*/
class CLUCENE_EXPORT FSDirectory :public Directory
{
private:
    class FSIndexOutput;
    class FSIndexInput;
    friend class FSDirectory::FSIndexOutput;
    friend class FSDirectory::FSIndexInput;

    int filemode;
protected:
    FSDirectory();
    virtual void init(const wchar_t* path, LockFactory* lockFactory = NULL);
    void priv_getFN(wchar_t* buffer, const wchar_t* name) const;
private:
    std::wstring directory;
    int refCount;
    void create();

    static const wchar_t* LOCK_DIR;
    static const wchar_t* getLockDir();
    wchar_t* getLockPrefix() const;
    static bool disableLocks;

    bool useMMap;

protected:
    /// Removes an existing file in the directory.
    bool doDeleteFile(const wchar_t * name);

public:
    ///Destructor - only call this if you are sure the directory
    ///is not being used anymore. Otherwise use the ref-counting
    ///facilities of _CLDECDELETE
    virtual ~FSDirectory();

    /// Get a list of strings, one for each file in the directory.
    bool list(std::vector<std::wstring>* names) const;

    /// Returns true iff a file with the given name exists.
    bool fileExists(const wchar_t * name) const;

    /// Returns the text name of the directory
    const wchar_t * getDirName() const; ///<returns reference


/**
* Deprecated, see getDirectory(file, lockFactory)
* Use IndexWriter's create flag, instead, to
* create a new index.
*/
    static _CL_DEPRECATED(getDirectory(file, lockFactory))FSDirectory* getDirectory(const wchar_t * file, const bool create, LockFactory* lockFactory = NULL);

    /**
    Returns the directory instance for the named location.

    Do not delete this instance, only use close, otherwise other instances
    will lose this instance.

    <p>Directories are cached, so that, for a given canonical path, the same
    FSDirectory instance will always be returned.  This permits
    synchronization on directories.

    @param file the path to the directory.
    @param create if true, create, or erase any existing contents.
    @return the FSDirectory for the named file.
    */
    static FSDirectory* getDirectory(const wchar_t * file, LockFactory* lockFactory = NULL);

    /// Returns the time the named file was last modified.
    int64_t fileModified(const wchar_t * name) const;

    //static
    /// Returns the time the named file was last modified.
    static int64_t fileModified(const wchar_t * dir, const wchar_t * name);

    //static
    /// Returns the length in bytes of a file in the directory.
    int64_t fileLength(const wchar_t * name) const;

    /// Returns a stream reading an existing file.
    virtual bool openInput(const wchar_t * name, IndexInput*& ret, CLuceneError& err, int32_t bufferSize = -1);

    /// Renames an existing file in the directory.
    void renameFile(const wchar_t * from, const wchar_t * to);

    /** Set the modified time of an existing file to now. */
    void touchFile(const wchar_t * name);

    /// Creates a new, empty file in the directory with the given name.
    ///	Returns a stream writing this file.
    virtual IndexOutput* createOutput(const wchar_t* name);

    ///Decrease the ref-count to the directory by one. If
    ///the object is no longer needed, then the object is
    ///removed from the directory pool.
    void close();

    /**
  * If MMap is available, this can disable use of
    * mmap reading.
    */
    void setUseMMap(bool value);
    /**
  * Gets whether the directory is using MMap for inputstreams.
    */
    bool getUseMMap() const;

    std::wstring toString() const;

    static const std::wstring getClassName();
    const std::wstring getObjectName() const;

    /**
    * Set whether Lucene's use of lock files is disabled. By default,
    * lock files are enabled. They should only be disabled if the index
    * is on a read-only medium like a CD-ROM.
    */
    static void setDisableLocks(bool doDisableLocks);

    /**
    * Returns whether Lucene's use of lock files is disabled.
    * @return true if locks are disabled, false if locks are enabled.
    */
    static bool getDisableLocks();

    /**
    * Sets the file mode for new files. This is passed to new output streams
    * and to the lock factory. The mode should be a valid octal file mode for
    * the 3rd parameter of the file open function (such as 0644)
    *
    * Tip: _tcstoi64(_T("644"), NULL, 8) is also a valid way of
    * creating a file mode
    */
    void setFileMode(int mode);

    /**
    * Gets the file mode for new files
    */
    int getFileMode();
};

CL_NS_END
#endif