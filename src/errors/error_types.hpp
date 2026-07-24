#pragma once
#include <string>
#include <vector>

namespace git_explain {

enum class ErrorType {
    
    UnknownCommand,          
    AmbiguousCommand,        
    InvalidFlag,             

    
    NotAGitRepo,             
    DetachedHead,            
    EmptyRepository,         

    
    BranchNotFound,          
    BranchAlreadyExists,     
    CannotDeleteCurrent,     

    
    MergeConflict,           
    MergeInProgress,         
    RebaseInProgress,        
    CherryPickConflict,      

    
    StashEmpty,              
    StashConflict,           

    
    RemoteNotFound,          
    PushRejected,            
    FetchFailed,             
    NoTrackingBranch,        

    
    UncommittedChanges,      
    UntrackedFileWouldBeOverwritten, 
    StagingFailed,           
    NothingToCommit,         

    
    PermissionDenied,        
    AuthFailed,              

    
    CommitNotFound,          
    TagAlreadyExists,        

    
    UserNotConfigured,       

    
    Unknown,
};

struct GitError {
    ErrorType   type     = ErrorType::Unknown;
    std::string message;                     
    std::string context;                     
    std::vector<std::string> affected_files; 
    int exit_code = 1;
};


std::string error_type_name(ErrorType t);

} 
