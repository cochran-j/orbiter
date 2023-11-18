// ==============================================================
// Parses a tree of files, optionally recursing subdirectories,
// and invoking an abstract callback method for each file and folder.
// 
// Copyright (c) 2018-2021 Douglas Beachy
// Licensed under the MIT License
// ==============================================================

#include <string>
#include <vector>
#include <filesystem>
#include <cassert>

#include "FileList.h"
#include "Orbitersdk.h"   // for oapiRand
#include "ConfigFileParserMacros.h"

// Convenience constructor for when you want to accept all file types
FileList::FileList(const char *pRootPath, const bool bRecurseSubfolders) :
    m_rootPath(pRootPath), m_bRecurseSubfolders(bRecurseSubfolders), m_previousRandomFileIndex(-1)
{
}

// Convenience constructor for when you only need to accept a single file type (e.g., "*.cfg")
FileList::FileList(const char *pRootPath, const bool bRecurseSubfolders, const char *pFileTypeToAccept) :
    FileList(pRootPath, bRecurseSubfolders)
{
    m_fileTypesToAccept.push_back(pFileTypeToAccept);   // copied by value
}

// Normal Constructor
//  pRootPath: base path to scan; may be relative or absolute path
//  bRecurseSubfolders: true to recurse, false to not
//  pFileTypesToAccept: vector of case-insensitive file extensions to accept (e.g., '.cfg', '.flac', etc).  If empty, all files are accepted.
FileList::FileList(const char *pRootPath, const bool bRecurseSubfolders, const vector<std::string> &fileTypesToAccept) :
    FileList(pRootPath, bRecurseSubfolders)
{
    m_fileTypesToAccept.resize(fileTypesToAccept.size());
    for (std::size_t i = 0; i < fileTypesToAccept.size(); ++i) {
        m_fileTypesToAccept[i] = std::filesystem::path{fileTypesToAccept[i]};
    }
}

// Destructor
FileList::~FileList()
{
}

#define STARTSWITH_DOT(fd)  (*(fd.cFileName) == '.')
#define IS_EMPTY(fd)        ((fd.nFileSizeHigh == 0) && (fd.nFileSizeLow == 0))
#define IS_DIRECTORY(fd)    (fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)

// Resursive method to scan a tree of files, invoking clbkFilterNode for each.  
// If clbkFilterNode returns true:
//      For files, *and* the file is not empty, the file node is added to m_allFiles, and
//      For folders, that folder node is recursed into.
// Note: recursionLevel is just here for debugging purposes
void FileList::Scan(const std::filesystem::path& pPath, const int recursionLevel)
{
    assert(!pPath.empty());

    // This code was broken out from XRPayloadClassData::InitializeXRPayloadClassData().
    std::filesystem::path basePath = pPath;

    if (m_bRecurseSubfolders) {
        for (auto& dir_entry : std::filesystem::recursive_directory_iterator{basePath}) {
            const auto& dir_path = dir_entry.path();
            if (!dir_path.empty() &&        // Exclude ., .., and dot files.
                !dir_path.string().front() == '.') {
                if (clbkFilterNode(dir_path)) {
                    if (!dir_entry.is_directory()) {
                        m_allFiles.push_back(dir_path);
                        clbkProcessFile(dir_path);
                    }
                }
            }
        }
    } else {
        for (auto& dir_entry : std::filesystem::directory_iterator{basePath}) {
            const auto& dir_path = dir_entry.path();
            if (!dir_path.empty() &&        // Exclude ., .., and dot files.
                !dir_path.string().front() == '.') {
                if (clbkFilterNode(dir_path)) {
                    if (!dir_entry.is_directory()) {
                        m_allFiles.push_back(dir_path);
                        clbkProcessFile(dir_path);
                    }
                }
            }
        }
    }
}

// Invoked for each file or folder node found.  The default method here looks at bRecurseSubfolders (for folder nodes) and
// pFileTypesToAccept (for file nodes) to decide whether accept a node or not.
// Subclasses should override this method if they want more advanced filtering.
//
// Returns true if file node should be included or folder should be recursed into, or false if the node should be skipped.
bool FileList::clbkFilterNode(const std::filesystem::path& pathOfNode)
{
    /* NOTE(jec):  This check is moved to the root algorithm.  This function now
     * only processes file nodes.
    if (IS_DIRECTORY(fd))
        return m_bRecurseSubfolders; 
    */

    // it's a file node
    bool bAcceptFile = false;
    if (!m_fileTypesToAccept.empty())
    {
        auto fileExtension = pathOfNode.extension();
        if (!fileExtension.empty())     // e.g., ".flac"
        {
            // see if we have a case-insensitive match for this extension in our master list
            for (auto it = m_fileTypesToAccept.cbegin(); it != m_fileTypesToAccept.cend(); it++)
            {
                if (caseInsensitiveEquals(fileExtension.string(), it->string())) {
                    bAcceptFile = true;
                    break;
                }
            }
        }
    } else {
        bAcceptFile = true;     // accept all file types
    }

    return bAcceptFile;
}

// Callback invoked for non-empty file nodes that passed the clbkFilterNode check; this is here for subclasses to hook.
void FileList::clbkProcessFile(const std::filesystem::path& filePath)
{
    static_cast<void>(filePath);
    // no-op; this method is for subclasses to use
}

// Returns a random file entry from the list that is not a repeat of the previous one (provided there are at least two files in the list).
// Returns empty string if the list is empty.
const std::filesystem::path FileList::GetRandomFile()
{
    const int fileCount = GetScannedFileCount();

    // sanity checks
    if (fileCount == 0)
        return "";
    else if (fileCount == 1)
        return m_allFiles[0];   // only one file

    // else we have a normal list of at least two entries, so choose one
    int fileIndex;
    do
    {
        fileIndex = (static_cast<int>(oapiRand() * RAND_MAX) % fileCount); // 0..count-1    
    } while (fileIndex == m_previousRandomFileIndex);

    m_previousRandomFileIndex = fileIndex;
    return m_allFiles[fileIndex];
}

// Returns a random file entry from the list that is not a repeat of the previous one (provided there are at least two files in the list).
//   index: 0..GetScannedFileCount()-1
// Returns empty string if the list is empty.
const std::filesystem::path FileList::GetFile(const int index) const
{
    const int fileCount = GetScannedFileCount();

    // sanity checks
    if ((fileCount == 0) || (index < 0) || index >= fileCount)
        return "";  // index out-of-range

    return m_allFiles[index];
}

// Returns the first file in our file list with the specified basename (case-insensitive search), or nullptr if no file found.
const std::filesystem::path* FileList::FindFileWithBasename(const char *pBasename) const
{
    assert(pBasename);
    assert(*pBasename);

    if (pBasename && *pBasename) /* NOTE(jec):  This is not a no-op with the asserts on NDEBUG builds */
    {
        for (vector<std::filesystem::path>::const_iterator it = m_allFiles.begin();
             it != m_allFiles.end();
             it++)
        {
            const auto& filespec_path = *it;   // e.g., "foo\bar.flac", "C:\foo\bar.flac", etc.

            // see if the basename part matches what we're looking for
            // Note: it would be faster for very large filelists to use a hashmap, but this is faster for lists of a few hundred files or less
            auto filestem = filespec_path.filename().stem();

            if (caseInsensitiveEquals(filestem.string(), pBasename)) {
                return &filespec_path;
            }
        }
    }

    return nullptr;
}

