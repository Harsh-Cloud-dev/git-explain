#include "error_types.hpp"

namespace git_explain {

std::string error_type_name(ErrorType t) {
    switch (t) {
        case ErrorType::UnknownCommand:                  return "Unknown Command";
        case ErrorType::AmbiguousCommand:                return "Ambiguous Command";
        case ErrorType::InvalidFlag:                     return "Invalid Flag";
        case ErrorType::NotAGitRepo:                     return "Not A Git Repository";
        case ErrorType::DetachedHead:                    return "Detached HEAD";
        case ErrorType::EmptyRepository:                 return "Empty Repository";
        case ErrorType::BranchNotFound:                  return "Branch Not Found";
        case ErrorType::BranchAlreadyExists:              return "Branch Already Exists";
        case ErrorType::CannotDeleteCurrent:              return "Cannot Delete Current Branch";
        case ErrorType::MergeConflict:                    return "Merge Conflict";
        case ErrorType::MergeInProgress:                  return "Merge In Progress";
        case ErrorType::RebaseInProgress:                 return "Rebase In Progress";
        case ErrorType::CherryPickConflict:               return "Cherry-Pick Conflict";
        case ErrorType::StashEmpty:                       return "Stash Empty";
        case ErrorType::StashConflict:                    return "Stash Conflict";
        case ErrorType::RemoteNotFound:                   return "Remote Not Found";
        case ErrorType::PushRejected:                     return "Push Rejected";
        case ErrorType::FetchFailed:                      return "Fetch Failed";
        case ErrorType::NoTrackingBranch:                 return "No Tracking Branch";
        case ErrorType::UncommittedChanges:               return "Uncommitted Changes";
        case ErrorType::UntrackedFileWouldBeOverwritten:  return "Untracked File Would Be Overwritten";
        case ErrorType::StagingFailed:                    return "Staging Failed";
        case ErrorType::NothingToCommit:                  return "Nothing To Commit";
        case ErrorType::PermissionDenied:                 return "Permission Denied";
        case ErrorType::AuthFailed:                       return "Authentication Failed";
        case ErrorType::CommitNotFound:                   return "Commit Not Found";
        case ErrorType::TagAlreadyExists:                 return "Tag Already Exists";
        case ErrorType::UserNotConfigured:                return "User Not Configured";
        case ErrorType::Unknown:                          return "Unknown Error";
    }
    return "Unknown Error";
}

} 
