# Enums - Remote Storage

## RemoteStoragePlatform

Enumerator | Value
---------- | -----
REMOTE_STORAGE_PLATFORM_NONE | 0
REMOTE_STORAGE_PLATFORM_WINDOWS | (1<<0)
REMOTE_STORAGE_PLATFORM_OSX | (1<<1)
REMOTE_STORAGE_PLATFORM_PS3 | (1<<2)
REMOTE_STORAGE_PLATFORM_LINUX | (1<<3)
REMOTE_STORAGE_PLATFORM_RESERVED2 | (1<<4)
REMOTE_STORAGE_PLATFORM_ALL | 0XFFFFFFFF

## RemoteStoragePublishedFileVisibility

Enumerator | Value
---------- | -----
REMOTE_STORAGE_PUBLISHED_VISIBLITY_PUBLIC | 0
REMOTE_STORAGE_PUBLISHED_VISIBLITY_FRIENDS_ONLY | 1
REMOTE_STORAGE_PUBLISHED_VISIBLITY_PRIVATE | 2

## UGCReadAction

Enumerator | Value
---------- | -----
UGC_READ_CONTINUE_READING_UNTIL_FINISHED | 0
UGC_READ_CONTINUE_READING | 1
UGC_READ_CLOSE | 2

## WorkshopEnumerationType

Enumerator | Value
---------- | -----
WORKSHOP_ENUMERATION_TYPE_RANKED_BY_VOTE | 0
WORKSHOP_ENUMERATION_TYPE_RECENT | 1
WORKSHOP_ENUMERATION_TYPE_TRENDING | 2
WORKSHOP_ENUMERATION_TYPE_FAVORITES_OF_FRIENDS | 3
WORKSHOP_ENUMERATION_TYPE_VOTED_BY_FRIENDS | 4
WORKSHOP_ENUMERATION_TYPE_CONTENT_BY_FRIENDS | 5
WORKSHOP_ENUMERATION_TYPE_RECENT_FROM_FOLLOWED_USERS | 6

## WorkshopFileAction

Enumerator | Value
---------- | -----
WORKSHOP_FILE_ACTION_PLAYED | 0
WORKSHOP_FILE_ACTION_COMPLETED | 1

## WorkshopFileType

Enumerator | Value
---------- | -----
WORKSHOP_FILE_TYPE_FIRST | 0
WORKSHOP_FILE_TYPE_COMMUNITY | 0
WORKSHOP_FILE_TYPE_MICROTRANSACTION | 1
WORKSHOP_FILE_TYPE_COLLECTION | 2
WORKSHOP_FILE_TYPE_ART | 3
WORKSHOP_FILE_TYPE_VIDEO | 4
WORKSHOP_FILE_TYPE_SCREENSHOT | 5
WORKSHOP_FILE_TYPE_GAME | 6
WORKSHOP_FILE_TYPE_SOFTWARE | 7
WORKSHOP_FILE_TYPE_CONCEPT | 8
WORKSHOP_FILE_TYPE_WEB_GUIDE | 9
WORKSHOP_FILE_TYPE_INTEGRATED_GUIDE | 10
WORKSHOP_FILE_TYPE_MERCH | 11
WORKSHOP_FILE_TYPE_CONTROLLER_BINDING | 12
WORKSHOP_FILE_TYPE_STEAMWORKS_ACCESS_INVITE | 13
WORKSHOP_FILE_TYPE_STEAM_VIDEO | 14
WORKSHOP_FILE_TYPE_GAME_MANAGED_ITEM | 15
WORKSHOP_FILE_TYPE_MAX | 16

## WorkshopVideoProvider

Enumerator | Value
---------- | -----
WORKSHOP_VIDEO_PROVIDER_NONE | 0
WORKSHOP_VIDEO_PROVIDER_YOUTUBE | 1

## WorkshopVote

Enumerator | Value
---------- | -----
WORKSHOP_VOTE_UNVOTED | 0
WORKSHOP_VOTE_FOR | 1
WORKSHOP_VOTE_AGAINST | 2
WORKSHOP_VOTE_LATER | 3

## LocalFileChange

Enumerator | Value
---------- | -----
LOCAL_FILE_CHANGE_INVALID | 0
LOCAL_FILE_CHANGE_FILE_UPDATED | 1
LOCAL_FILE_CHANGE_FILE_DELETED | 2

## FilePathType

Enumerator | Value
---------- | -----
FILE_PATH_TYPE_INVALID | 0
FILE_PATH_TYPE_ABSOLUTE | 1
FILE_PATH_TYPE_API_FILENAME | 2