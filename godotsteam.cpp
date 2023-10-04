/////////////////////////////////////////////////
///// SILENCE STEAMWORKS WARNINGS
/////////////////////////////////////////////////
//
// Turn off MSVC-only warning about strcpy
#ifdef _MSC_VER
#define _CRT_SECURE_NO_WARNINGS 1
#pragma warning(disable : 4996)
#pragma warning(disable : 4828)
#endif

/////////////////////////////////////////////////
///// HEADER INCLUDES
/////////////////////////////////////////////////
//
// Include GodotSteam header
#include "godotsteam.h"

// Include some Godot headers
#include "core/io/ip.h"
#include "core/io/ip_address.h"

// Include some system headers
#include "fstream"
#include "vector"

#include "./godotsteam_constants.h"

/////////////////////////////////////////////////
///// STEAM SINGLETON? STEAM SINGLETON
/////////////////////////////////////////////////
//
SteamServer *SteamServer::singleton = NULL;

/////////////////////////////////////////////////
///// STEAM OBJECT WITH CALLBACKS
/////////////////////////////////////////////////
//
SteamServer::SteamServer() :
		// Apps callbacks ////////////////////////////
		callbackDLCInstalled(this, &SteamServer::dlc_installed),
		callbackFileDetailsResult(this, &SteamServer::file_details_result),
		callbackNewLaunchURLParameters(this, &SteamServer::new_launch_url_parameters),
		callbackTimedTrialStatus(this, &SteamServer::timed_trial_status),

		// Apps List callbacks ///////////////////////
		callbackAppInstalled(this, &SteamServer::app_installed),
		callbackAppUninstalled(this, &SteamServer::app_uninstalled),

		// Friends callbacks ////////////////////////
		callbackAvatarLoaded(this, &SteamServer::avatar_loaded),
		callbackAvatarImageLoaded(this, &SteamServer::avatar_image_loaded),
		callbackClanActivityDownloaded(this, &SteamServer::clan_activity_downloaded),
		callbackFriendRichPresenceUpdate(this, &SteamServer::friend_rich_presence_update),
		callbackConnectedChatJoin(this, &SteamServer::connected_chat_join),
		callbackConnectedChatLeave(this, &SteamServer::connected_chat_leave),
		callbackConnectedClanChatMessage(this, &SteamServer::connected_clan_chat_message),
		callbackConnectedFriendChatMessage(this, &SteamServer::connected_friend_chat_message),
		callbackJoinRequested(this, &SteamServer::join_requested),
		callbackOverlayToggled(this, &SteamServer::overlay_toggled),
		callbackJoinGameRequested(this, &SteamServer::join_game_requested),
		callbackChangeServerRequested(this, &SteamServer::change_server_requested),
		callbackJoinClanChatComplete(this, &SteamServer::join_clan_chat_complete),
		callbackPersonaStateChange(this, &SteamServer::persona_state_change),
		callbackNameChanged(this, &SteamServer::name_changed),
		callbackOverlayBrowserProtocol(this, &SteamServer::overlay_browser_protocol),
		callbackUnreadChatMessagesChanged(this, &SteamServer::unread_chat_messages_changed),
		callbackEquippedProfileItemsChanged(this, &SteamServer::equipped_profile_items_changed),

		// Game Search callbacks ////////////////////
		callbackSearchForGameProgress(this, &SteamServer::search_for_game_progress),
		callbackSearchForGameResult(this, &SteamServer::search_for_game_result),
		callbackRequestPlayersForGameProgress(this, &SteamServer::request_players_for_game_progress),
		callbackRequestPlayersForGameResult(this, &SteamServer::request_players_for_game_result),
		callbackRequestPlayersForGameFinalResult(this, &SteamServer::request_players_for_game_final_result),
		callbackSubmitPlayerResult(this, &SteamServer::submit_player_result),
		callbackEndGameResult(this, &SteamServer::end_game_result),

		// HTTP callbacks ///////////////////////////
		callbackHTTPRequestCompleted(this, &SteamServer::http_request_completed),
		callbackHTTPRequestDataReceived(this, &SteamServer::http_request_data_received),
		callbackHTTPRequestHeadersReceived(this, &SteamServer::http_request_headers_received),

		// Inventory callbacks //////////////////////
		callbackInventoryDefinitionUpdate(this, &SteamServer::inventory_definition_update),
		callbackInventoryFullUpdate(this, &SteamServer::inventory_full_update),
		callbackInventoryResultReady(this, &SteamServer::inventory_result_ready),

		// Game Server callbacks ////////////////////
		callbackClientApproved(this, &SteamServer::client_approved),
		callbackClientDenied(this, &SteamServer::client_denied),
		callbackClientKick(this, &SteamServer::client_kick),
		callbackPlayerCompat(this, &SteamServer::player_compat),
		callbackPolicyResponse(this, &SteamServer::policy_response),
		callbackConnectFailure(this, &SteamServer::server_connect_failure),
		callbackServerConnected(this, &SteamServer::server_connected),
		callbackServerDisconnected(this, &SteamServer::server_disconnected),

		// Matchmaking callbacks ////////////////////
		callbackFavoritesListAccountsUpdated(this, &SteamServer::favorites_list_accounts_updated),
		callbackFavoritesListChanged(this, &SteamServer::favorites_list_changed),
		callbackLobbyMessage(this, &SteamServer::lobby_message),
		callbackLobbyChatUpdate(this, &SteamServer::lobby_chat_update),
		callbackLobbyDataUpdate(this, &SteamServer::lobby_data_update),
		callbackLobbyJoined(this, &SteamServer::lobby_joined),
		callbackLobbyGameCreated(this, &SteamServer::lobby_game_created),
		callbackLobbyInvite(this, &SteamServer::lobby_invite),
		callbackLobbyKicked(this, &SteamServer::lobby_kicked),

		// Networking callbacks /////////////////////
		callbackP2PSessionConnectFail(this, &SteamServer::p2p_session_connect_fail),
		callbackP2PSessionRequest(this, &SteamServer::p2p_session_request),

		// Networking Messages callbacks ////////////
		callbackNetworkMessagesSessionRequest(this, &SteamServer::network_messages_session_request),
		callbackNetworkMessagesSessionFailed(this, &SteamServer::network_messages_session_failed),

		// Networking Sockets callbacks /////////////
		callbackNetworkConnectionStatusChanged(this, &SteamServer::network_connection_status_changed),
		callbackNetworkAuthenticationStatus(this, &SteamServer::network_authentication_status),
		callbackNetworkingFakeIPResult(this, &SteamServer::fake_ip_result),

		// Networking Utils callbacks ///////////////
		callbackRelayNetworkStatus(this, &SteamServer::relay_network_status),

		// Parties callbacks ////////////////////////
		callbackReserveNotification(this, &SteamServer::reservation_notification),
		callbackAvailableBeaconLocationsUpdated(this, &SteamServer::available_beacon_locations_updated),
		callbackActiveBeaconsUpdated(this, &SteamServer::active_beacons_updated),

		// Remote Storage callbacks /////////////////
		callbackLocalFileChanged(this, &SteamServer::local_file_changed),

		// UGC callbacks ////////////////////////////
		callbackItemDownloaded(this, &SteamServer::item_downloaded),
		callbackItemInstalled(this, &SteamServer::item_installed),
		callbackUserSubscribedItemsListChanged(this, &SteamServer::user_subscribed_items_list_changed),

		// User callbacks ///////////////////////////
		callbackClientGameServerDeny(this, &SteamServer::client_game_server_deny),
		callbackGameWebCallback(this, &SteamServer::game_web_callback),
		callbackGetUserAuthSessionTicketResponse(this, &SteamServer::get_auth_session_ticket_response),
		callbackGetTicketForWebApiResponse(this, &SteamServer::get_ticket_for_web_api),
		callbackIPCFailure(this, &SteamServer::ipc_failure),
		callbackLicensesUpdated(this, &SteamServer::licenses_updated),
		callbackMicrotransactionAuthResponse(this, &SteamServer::microtransaction_auth_response),
		callbackSteamServerConnected(this, &SteamServer::steam_server_connected),
		callbackSteamServerDisconnected(this, &SteamServer::steam_server_disconnected),
		callbackValidateAuthTicketResponse(this, &SteamServer::validate_auth_ticket_response),

		// User stat callbacks //////////////////////
		callbackUserAchievementStored(this, &SteamServer::user_achievement_stored),
		callbackCurrentStatsReceived(this, &SteamServer::current_stats_received),
		callbackUserStatsStored(this, &SteamServer::user_stats_stored),
		callbackUserStatsUnloaded(this, &SteamServer::user_stats_unloaded),

		// Utility callbacks ////////////////////////
		callbackGamepadTextInputDismissed(this, &SteamServer::gamepad_text_input_dismissed),
		callbackIPCountry(this, &SteamServer::ip_country),
		callbackLowPower(this, &SteamServer::low_power),
		callbackSteamAPICallCompleted(this, &SteamServer::steam_api_call_completed),
		callbackSteamShutdown(this, &SteamServer::steam_shutdown),
		callbackAppResumingFromSuspend(this, &SteamServer::app_resuming_from_suspend),
		callbackFloatingGamepadTextInputDismissed(this, &SteamServer::floating_gamepad_text_input_dismissed),
		callbackFilterTextDictionaryChanged(this, &SteamServer::filter_text_dictionary_changed),

		// Video callbacks //////////////////////////
		callbackGetOPFSettingsResult(this, &SteamServer::get_opf_settings_result),
		callbackGetVideoResult(this, &SteamServer::get_video_result) {
	is_init_success = false;
	singleton = this;
}

/////////////////////////////////////////////////
///// INTERNAL FUNCTIONS
/////////////////////////////////////////////////
//
// Get the Steam singleton, obviously
SteamServer *SteamServer::get_singleton() {
	return singleton;
}

// Creating a Steam ID for internal use
CSteamID SteamServer::createSteamID(uint64_t steam_id, AccountType account_type) {
	CSteamID converted_steam_id;
	if (account_type < 0 || account_type >= AccountType(k_EAccountTypeMax)) {
		account_type = ACCOUNT_TYPE_INDIVIDUAL;
	}
	converted_steam_id.Set(steam_id, k_EUniversePublic, EAccountType(account_type));
	return converted_steam_id;
}

/////////////////////////////////////////////////
///// MAIN FUNCTIONS
/////////////////////////////////////////////////
//
// Returns true/false if Steam is running.
bool SteamServer::checkMasterRestartRequest(uint32 app_id) {
	return SteamGameServer()->WasRestartRequested();
}

//! Initialize Steamworks for dedicated servers
bool SteamServer::serverInit(const String &ip, int game_port, int query_port, ServerMode server_mode, const String &version_string) {
	// Convert string ip to uint32. Sending an invalid ip adress can cause a crash for forbidden memory access
	uint32 unIP = 0;
	const char *start;
	start = (const char *)ip.to_utf8_buffer().ptr();
	for (int i = 0; i < 4; i++) {
		char c;
		int n = 0;
		while (1) {
			c = *start;
			start++;
			if (c >= '0' && c <= '9') {
				n *= 10;
				n += c - '0';
			} else if ((i < 3 && c == '.') || i == 3) {
				break;
			}
		}
		unIP *= 256; // Send the previous value 1 byte back
		unIP += n; // Add the current byte
	}
	is_init_success = SteamGameServer_InitEx(unIP, (uint16)game_port, (uint16)query_port, (EServerMode)server_mode, version_string.utf8().get_data(), &init_error_message);
	return is_init_success;
}

bool SteamServer::isLoggedOn() {
	return SteamGameServer()->BLoggedOn();
}

// Shuts down the Steamworks API, releases pointers and frees memory.
void SteamServer::steamShutdown() {
	SteamGameServer_Shutdown();
}

/////////////////////////////////////////////////
///// APPS
/////////////////////////////////////////////////
//
// Returns metadata for a DLC by index.
Array SteamServer::getDLCDataByIndex() {
	if (SteamApps() == NULL) {
		return Array();
	}
	int32 count = SteamApps()->GetDLCCount();
	Array dlcData;
	for (int i = 0; i < count; i++) {
		AppId_t app_id = 0;
		bool available = false;
		char name[STEAM_BUFFER_SIZE];
		bool success = SteamApps()->BGetDLCDataByIndex(i, &app_id, &available, name, STEAM_BUFFER_SIZE);
		if (success) {
			Dictionary dlc;
			dlc["id"] = app_id;
			dlc["available"] = available;
			dlc["name"] = name;
			dlcData.append(dlc);
		}
	}
	return dlcData;
}

// Check if given application/game is installed, not necessarily owned.
bool SteamServer::isAppInstalled(uint32_t app_id) {
	if (SteamApps() == NULL) {
		return false;
	}
	return SteamApps()->BIsAppInstalled((AppId_t)app_id);
}

// Checks whether the current App ID is for Cyber Cafes.
bool SteamServer::isCybercafe() {
	if (SteamApps() == NULL) {
		return false;
	}
	return SteamApps()->BIsCybercafe();
}

// Checks if the user owns a specific DLC and if the DLC is installed
bool SteamServer::isDLCInstalled(uint32_t dlc_id) {
	if (SteamApps() == NULL) {
		return false;
	}
	return SteamApps()->BIsDlcInstalled((AppId_t)dlc_id);
}

// Checks if the license owned by the user provides low violence depots.
bool SteamServer::isLowViolence() {
	if (SteamApps() == NULL) {
		return false;
	}
	return SteamApps()->BIsLowViolence();
}

// Checks if the active user is subscribed to the current App ID.
bool SteamServer::isSubscribed() {
	if (SteamApps() == NULL) {
		return false;
	}
	return SteamApps()->BIsSubscribed();
}

// Checks if the active user is subscribed to a specified AppId.
bool SteamServer::isSubscribedApp(uint32_t app_id) {
	if (SteamApps() == NULL) {
		return false;
	}
	return SteamApps()->BIsSubscribedApp((AppId_t)app_id);
}

// Checks if the active user is accessing the current app_id via a temporary Family Shared license owned by another user.
// If you need to determine the steam_id of the permanent owner of the license, use getAppOwner.
bool SteamServer::isSubscribedFromFamilySharing() {
	if (SteamApps() == NULL) {
		return false;
	}
	return SteamApps()->BIsSubscribedFromFamilySharing();
}

// Checks if the user is subscribed to the current app through a free weekend.
// This function will return false for users who have a retail or other type of license.
// Suggested you contact Valve on how to package and secure your free weekend properly.
bool SteamServer::isSubscribedFromFreeWeekend() {
	if (SteamApps() == NULL) {
		return false;
	}
	return SteamApps()->BIsSubscribedFromFreeWeekend();
}

// Check if game is a timed trial with limited playtime.
Dictionary SteamServer::isTimedTrial() {
	Dictionary trial;
	if (SteamApps() != NULL) {
		uint32 allowed = 0;
		uint32 played = 0;
		if (SteamApps()->BIsTimedTrial(&allowed, &played)) {
			trial["seconds_allowed"] = allowed;
			trial["seconds_played"] = played;
		}
	}
	return trial;
}

// Checks if the user has a VAC ban on their account.
bool SteamServer::isVACBanned() {
	if (SteamApps() == NULL) {
		return false;
	}
	return SteamApps()->BIsVACBanned();
}

// Return the build ID for this app; will change based on backend updates.
int SteamServer::getAppBuildId() {
	if (SteamApps() == NULL) {
		return 0;
	}
	return SteamApps()->GetAppBuildId();
}

// Gets the install folder for a specific AppID.
Dictionary SteamServer::getAppInstallDir(uint32_t app_id) {
	Dictionary app_install;
	if (SteamApps() != NULL) {
		char buffer[STEAM_BUFFER_SIZE];
		uint32 install_size = SteamApps()->GetAppInstallDir((AppId_t)app_id, buffer, STEAM_BUFFER_SIZE);
		String install_directory = buffer;
		// If we get no install directory, mention a possible cause
		if (install_directory.is_empty()) {
			install_directory = "Possible wrong app ID or missing depot";
		}
		app_install["directory"] = install_directory;
		app_install["install_size"] = install_size;
	}
	return app_install;
}

// Gets the Steam ID of the original owner of the current app. If it's different from the current user then it is borrowed.
uint64_t SteamServer::getAppOwner() {
	if (SteamApps() == NULL) {
		return 0;
	}
	CSteamID converted_steam_id = SteamApps()->GetAppOwner();
	return converted_steam_id.ConvertToUint64();
}

// Gets a comma separated list of the languages the current app supports.
String SteamServer::getAvailableGameLanguages() {
	if (SteamApps() == NULL) {
		return "None";
	}
	return SteamApps()->GetAvailableGameLanguages();
}

// Checks if the user is running from a beta branch, and gets the name of the branch if they are.
String SteamServer::getCurrentBetaName() {
	String beta_name = "";
	if (SteamApps() != NULL) {
		char name_string[STEAM_LARGE_BUFFER_SIZE];
		if (SteamApps()->GetCurrentBetaName(name_string, STEAM_LARGE_BUFFER_SIZE)) {
			beta_name = String(name_string);
		}
	}
	return beta_name;
}

// Gets the current language that the user has set.
String SteamServer::getCurrentGameLanguage() {
	if (SteamApps() == NULL) {
		return "None";
	}
	return SteamApps()->GetCurrentGameLanguage();
}

// Get the number of DLC the user owns for a parent application/game.
int SteamServer::getDLCCount() {
	if (SteamApps() == NULL) {
		return false;
	}
	return SteamApps()->GetDLCCount();
}

// Gets the download progress for optional DLC.
Dictionary SteamServer::getDLCDownloadProgress(uint32_t dlc_id) {
	Dictionary progress;
	if (SteamApps() == NULL) {
		progress["ret"] = false;
	} else {
		uint64 downloaded = 0;
		uint64 total = 0;
		// Get the progress
		progress["ret"] = SteamApps()->GetDlcDownloadProgress((AppId_t)dlc_id, &downloaded, &total);
		if (progress["ret"]) {
			progress["downloaded"] = uint64_t(downloaded);
			progress["total"] = uint64_t(total);
		}
	}
	return progress;
}

// Gets the time of purchase of the specified app in Unix epoch format (time since Jan 1st, 1970).
uint32_t SteamServer::getEarliestPurchaseUnixTime(uint32_t app_id) {
	if (SteamApps() == NULL) {
		return 0;
	}
	return SteamApps()->GetEarliestPurchaseUnixTime((AppId_t)app_id);
}

// Asynchronously retrieves metadata details about a specific file in the depot manifest.
void SteamServer::getFileDetails(const String &filename) {
	if (SteamApps() != NULL) {
		SteamApps()->GetFileDetails(filename.utf8().get_data());
	}
}

// Gets a list of all installed depots for a given App ID.
// @param app_id App ID to check.
// @return Array of the installed depots, returned in mount order.
Array SteamServer::getInstalledDepots(uint32_t app_id) {
	if (SteamApps() == NULL) {
		return Array();
	}
	Array installed_depots;
	DepotId_t *depots = new DepotId_t[32];
	uint32 installed = SteamApps()->GetInstalledDepots((AppId_t)app_id, depots, 32);
	for (uint32 i = 0; i < installed; i++) {
		installed_depots.append(depots[i]);
	}
	delete[] depots;
	return installed_depots;
}

// Gets the command line if the game was launched via Steam URL, e.g. steam://run/<appid>//<command line>/. This method is preferable to launching with a command line via the operating system, which can be a security risk. In order for rich presence joins to go through this and not be placed on the OS command line, you must enable "Use launch command line" from the Installation > General page on your app.
String SteamServer::getLaunchCommandLine() {
	if (SteamApps() == NULL) {
		return "";
	}
	char commands[STEAM_BUFFER_SIZE];
	SteamApps()->GetLaunchCommandLine(commands, STEAM_BUFFER_SIZE);
	String command_line;
	command_line += commands;
	return command_line;
}

// Gets the associated launch parameter if the game is run via steam://run/<appid>/?param1=value1;param2=value2;param3=value3 etc.
String SteamServer::getLaunchQueryParam(const String &key) {
	if (SteamApps() == NULL) {
		return "";
	}
	return SteamApps()->GetLaunchQueryParam(key.utf8().get_data());
}

// Allows you to install an optional DLC.
void SteamServer::installDLC(uint32_t dlc_id) {
	if (SteamApps() != NULL) {
		SteamApps()->InstallDLC((AppId_t)dlc_id);
	}
}

// Allows you to force verify game content on next launch.
bool SteamServer::markContentCorrupt(bool missing_files_only) {
	if (SteamApps() == NULL) {
		return false;
	}
	return SteamApps()->MarkContentCorrupt(missing_files_only);
}

// Set current DLC AppID being played (or 0 if none). Allows Steam to track usage of major DLC extensions.
bool SteamServer::setDLCContext(uint32_t app_id) {
	if (SteamApps() == NULL) {
		return false;
	}
	return SteamApps()->SetDlcContext((AppId_t)app_id);
}

// Allows you to uninstall an optional DLC.
void SteamServer::uninstallDLC(uint32_t dlc_id) {
	if (SteamApps() != NULL) {
		SteamApps()->UninstallDLC((AppId_t)dlc_id);
	}
}

/////////////////////////////////////////////////
///// APP LISTS
/////////////////////////////////////////////////
//
// This is a restricted interface that can only be used by previously approved apps, contact your Steam Account Manager if you believe you need access to this API.
//
// Get the number of installed apps for this player.
uint32 SteamServer::getNumInstalledApps() {
	if (SteamAppList() == NULL) {
		return 0;
	}
	return SteamAppList()->GetNumInstalledApps();
}

// Get a list of app IDs for installed apps for this player.
Array SteamServer::getInstalledApps(uint32 max_app_ids) {
	Array installed_apps;
	if (SteamAppList() != NULL) {
		uint32 *app_ids = nullptr;
		uint32 these_apps = SteamAppList()->GetInstalledApps(app_ids, max_app_ids);
		// Which is greater?
		if (these_apps < max_app_ids) {
			max_app_ids = these_apps;
		}
		// Parse it
		for (uint32 i = 0; i < max_app_ids; i++) {
			installed_apps.append(app_ids[i]);
		}
	}
	return installed_apps;
}

// Get a given app ID's name.
String SteamServer::getAppName(uint32_t app_id, int name_max) {
	String app_name;
	if (SteamAppList() != NULL) {
		char *app_buffer = new char[name_max];
		int buffer_size = SteamAppList()->GetAppName((AppId_t)app_id, app_buffer, name_max);
		if (buffer_size != 0) {
			app_name += app_buffer;
		}
		delete[] app_buffer;
	}
	return app_name;
}

// Get a given app ID's install directory.
String SteamServer::getAppListInstallDir(uint32_t app_id, int name_max) {
	String directory_name;
	if (SteamAppList() != NULL) {
		char *directory_buffer = new char[name_max];
		int buffer_size = SteamAppList()->GetAppInstallDir((AppId_t)app_id, directory_buffer, name_max);
		if (buffer_size != 0) {
			directory_name += directory_buffer;
		}
		delete[] directory_buffer;
	}
	return directory_name;
}

// Get a given app ID's build.
int SteamServer::getAppListBuildId(uint32_t app_id) {
	if (SteamAppList() == NULL) {
		return 0;
	}
	return SteamAppList()->GetAppBuildId((AppId_t)app_id);
}

/////////////////////////////////////////////////
///// FRIENDS
/////////////////////////////////////////////////
//
// Activates the overlay with optional dialog to open the following: "Friends", "Community", "Players", "Settings", "OfficialGameGroup", "Stats", "Achievements", "LobbyInvite".
void SteamServer::activateGameOverlay(const String &url) {
	if (SteamFriends() != NULL) {
		SteamFriends()->ActivateGameOverlay(url.utf8().get_data());
	}
}

// Activates game overlay to open the invite dialog. Invitations will be sent for the provided lobby.
void SteamServer::activateGameOverlayInviteDialog(uint64_t steam_id) {
	if (SteamFriends() != NULL) {
		CSteamID user_id = (uint64)steam_id;
		SteamFriends()->ActivateGameOverlayInviteDialog(user_id);
	}
}

// Activates the game overlay to open an invite dialog that will send the provided Rich Presence connect string to selected friends.
void SteamServer::activateGameOverlayInviteDialogConnectString(const String &connect_string) {
	if (SteamFriends() != NULL) {
		SteamFriends()->ActivateGameOverlayInviteDialogConnectString(connect_string.utf8().get_data());
	}
}

// Activates the overlay with the application/game Steam store page.
void SteamServer::activateGameOverlayToStore(uint32_t app_id) {
	if (SteamFriends() != NULL) {
		SteamFriends()->ActivateGameOverlayToStore(AppId_t(app_id), EOverlayToStoreFlag(0));
	}
}

// Activates the overlay to the following: "steamid", "chat", "jointrade", "stats", "achievements", "friendadd", "friendremove", "friendrequestaccept", "friendrequestignore".
void SteamServer::activateGameOverlayToUser(const String &url, uint64_t steam_id) {
	if (SteamFriends() != NULL) {
		CSteamID user_id = (uint64)steam_id;
		SteamFriends()->ActivateGameOverlayToUser(url.utf8().get_data(), user_id);
	}
}

// Activates the overlay with specified web address.
void SteamServer::activateGameOverlayToWebPage(const String &url) {
	if (SteamFriends() != NULL) {
		SteamFriends()->ActivateGameOverlayToWebPage(url.utf8().get_data());
	}
}

// Clear the game information in Steam; used in 'View Game Info'.
void SteamServer::clearRichPresence() {
	if (SteamFriends() != NULL) {
		SteamFriends()->ClearRichPresence();
	}
}

// Closes the specified Steam group chat room in the Steam UI.
bool SteamServer::closeClanChatWindowInSteam(uint64_t chat_id) {
	if (SteamFriends() == NULL) {
		return false;
	}
	CSteamID chat = (uint64)chat_id;
	return SteamFriends()->CloseClanChatWindowInSteam(chat);
}

// For clans a user is a member of, they will have reasonably up-to-date information, but for others you'll have to download the info to have the latest.
void SteamServer::downloadClanActivityCounts(uint64_t clan_id, int clans_to_request) {
	if (SteamFriends() != NULL) {
		clan_activity = (uint64)clan_id;
		SteamFriends()->DownloadClanActivityCounts(&clan_activity, clans_to_request);
	}
}

// Gets the list of users that the current user is following.
void SteamServer::enumerateFollowingList(uint32 start_index) {
	if (SteamFriends() != NULL) {
		SteamAPICall_t api_call = SteamFriends()->EnumerateFollowingList(start_index);
		callResultEnumerateFollowingList.Set(api_call, this, &SteamServer::enumerate_following_list);
	}
}

// Gets the Steam ID at the given index in a Steam group chat.
uint64_t SteamServer::getChatMemberByIndex(uint64_t clan_id, int user) {
	if (SteamFriends() == NULL) {
		return 0;
	}
	clan_activity = (uint64)clan_id;
	CSteamID chat_id = SteamFriends()->GetChatMemberByIndex(clan_activity, user);
	return chat_id.ConvertToUint64();
}

// Gets the most recent information we have about what the users in a Steam Group are doing.
Dictionary SteamServer::getClanActivityCounts(uint64_t clan_id) {
	Dictionary activity;
	if (SteamFriends() == NULL) {
		return activity;
	}
	clan_activity = (uint64)clan_id;
	int online = 0;
	int ingame = 0;
	int chatting = 0;
	bool success = SteamFriends()->GetClanActivityCounts(clan_activity, &online, &ingame, &chatting);
	// Add these to the dictionary if successful
	if (success) {
		activity["clan"] = clan_id;
		activity["online"] = online;
		activity["ingame"] = ingame;
		activity["chatting"] = chatting;
	}
	return activity;
}

// Gets the Steam group's Steam ID at the given index.
uint64_t SteamServer::getClanByIndex(int clan) {
	if (SteamFriends() == NULL) {
		return 0;
	}
	return SteamFriends()->GetClanByIndex(clan).ConvertToUint64();
}

// Get the number of users in a Steam group chat.
int SteamServer::getClanChatMemberCount(uint64_t clan_id) {
	if (SteamFriends() == NULL) {
		return 0;
	}
	clan_activity = (uint64)clan_id;
	return SteamFriends()->GetClanChatMemberCount(clan_activity);
}

//  Gets the data from a Steam group chat room message.  This should only ever be called in response to a GameConnectedClanChatMsg_t callback.
Dictionary SteamServer::getClanChatMessage(uint64_t chat_id, int message) {
	Dictionary chat_message;
	if (SteamFriends() == NULL) {
		return chat_message;
	}
	CSteamID chat = (uint64)chat_id;
	char text[STEAM_LARGE_BUFFER_SIZE];
	EChatEntryType type = k_EChatEntryTypeInvalid;
	CSteamID user_id;
	chat_message["ret"] = SteamFriends()->GetClanChatMessage(chat, message, text, STEAM_LARGE_BUFFER_SIZE, &type, &user_id);
	chat_message["text"] = String(text);
	chat_message["type"] = type;
	uint64_t user = user_id.ConvertToUint64();
	chat_message["chatter"] = user;
	return chat_message;
}

// Gets the number of Steam groups that the current user is a member of.  This is used for iteration, after calling this then GetClanByIndex can be used to get the Steam ID of each Steam group.
int SteamServer::getClanCount() {
	if (SteamFriends() == NULL) {
		return 0;
	}
	return SteamFriends()->GetClanCount();
}

// Gets the display name for the specified Steam group; if the local client knows about it.
String SteamServer::getClanName(uint64_t clan_id) {
	if (SteamFriends() == NULL) {
		return "";
	}
	clan_activity = (uint64)clan_id;
	return String::utf8(SteamFriends()->GetClanName(clan_activity));
}

// Returns the steam_id of a clan officer, by index, of range [0,GetClanOfficerCount).
uint64_t SteamServer::getClanOfficerByIndex(uint64_t clan_id, int officer) {
	if (SteamFriends() == NULL) {
		return 0;
	}
	clan_activity = (uint64)clan_id;
	CSteamID officer_id = SteamFriends()->GetClanOfficerByIndex(clan_activity, officer);
	return officer_id.ConvertToUint64();
}

// Returns the number of officers in a clan (including the owner).
int SteamServer::getClanOfficerCount(uint64_t clan_id) {
	if (SteamFriends() == NULL) {
		return 0;
	}
	clan_activity = (uint64)clan_id;
	return SteamFriends()->GetClanOfficerCount(clan_activity);
}

// Returns the steam_id of the clan owner.
uint64_t SteamServer::getClanOwner(uint64_t clan_id) {
	if (SteamFriends() == NULL) {
		return 0;
	}
	clan_activity = (uint64)clan_id;
	CSteamID owner_id = SteamFriends()->GetClanOwner(clan_activity);
	return owner_id.ConvertToUint64();
}

// Gets the unique tag (abbreviation) for the specified Steam group; If the local client knows about it.  The Steam group abbreviation is a unique way for people to identify the group and is limited to 12 characters. In some games this will appear next to the name of group members.
String SteamServer::getClanTag(uint64_t clan_id) {
	if (SteamFriends() == NULL) {
		return "";
	}
	clan_activity = (uint64)clan_id;
	return String::utf8(SteamFriends()->GetClanTag(clan_activity));
}

// Gets the Steam ID of the recently played with user at the given index.
uint64_t SteamServer::getCoplayFriend(int friend_number) {
	if (SteamFriends() == NULL) {
		return 0;
	}
	CSteamID friend_id = SteamFriends()->GetCoplayFriend(friend_number);
	return friend_id.ConvertToUint64();
}

// Gets the number of players that the current users has recently played with, across all games.  This is used for iteration, after calling this then GetCoplayFriend can be used to get the Steam ID of each player.  These players are have been set with previous calls to SetPlayedWith.
int SteamServer::getCoplayFriendCount() {
	if (SteamFriends() == NULL) {
		return 0;
	}
	return SteamFriends()->GetCoplayFriendCount();
}

// Gets the number of users following the specified user.
void SteamServer::getFollowerCount(uint64_t steam_id) {
	if (SteamFriends() != NULL) {
		CSteamID user_id = (uint64)steam_id;
		SteamAPICall_t api_call = SteamFriends()->GetFollowerCount(user_id);
		callResultFollowerCount.Set(api_call, this, &SteamServer::get_follower_count);
	}
}

// Returns the Steam ID of a user.
uint64_t SteamServer::getFriendByIndex(int friend_number, BitField<FriendFlags> friend_flags) {
	if (SteamFriends() == NULL) {
		return 0;
	}
	CSteamID friend_id = SteamFriends()->GetFriendByIndex(friend_number, (int)friend_flags);
	return friend_id.ConvertToUint64();
}

// Gets the app ID of the game that user played with someone on their recently-played-with list.
uint32 SteamServer::getFriendCoplayGame(uint64_t friend_id) {
	if (SteamFriends() == NULL) {
		return 0;
	}
	CSteamID steam_id = (uint64)friend_id;
	return SteamFriends()->GetFriendCoplayGame(steam_id);
}

// Gets the timestamp of when the user played with someone on their recently-played-with list.  The time is provided in Unix epoch format (seconds since Jan 1st 1970).
int SteamServer::getFriendCoplayTime(uint64_t friend_id) {
	if (SteamFriends() == NULL) {
		return 0;
	}
	CSteamID steam_id = (uint64)friend_id;
	return SteamFriends()->GetFriendCoplayTime(steam_id);
}

// Get number of friends user has.
int SteamServer::getFriendCount(int friend_flags) {
	if (SteamFriends() == NULL) {
		return 0;
	}
	return SteamFriends()->GetFriendCount(friend_flags);
}

// Iterators for getting users in a chat room, lobby, game server or clan.
int SteamServer::getFriendCountFromSource(uint64_t source_id) {
	if (SteamFriends() == NULL) {
		return 0;
	}
	CSteamID source = (uint64)source_id;
	return SteamFriends()->GetFriendCountFromSource(source);
}

// Gets the Steam ID at the given index from a source (Steam group, chat room, lobby, or game server).
uint64_t SteamServer::getFriendFromSourceByIndex(uint64_t source_id, int friend_number) {
	if (SteamFriends() == NULL) {
		return 0;
	}
	CSteamID source = (uint64)source_id;
	CSteamID friend_id = SteamFriends()->GetFriendFromSourceByIndex(source, friend_number);
	return friend_id.ConvertToUint64();
}

// Returns dictionary of friend game played if valid
Dictionary SteamServer::getFriendGamePlayed(uint64_t steam_id) {
	Dictionary friend_game;
	if (SteamFriends() == NULL) {
		return friend_game;
	}
	FriendGameInfo_t game_info;
	CSteamID user_id = (uint64)steam_id;
	bool success = SteamFriends()->GetFriendGamePlayed(user_id, &game_info);
	// If successful
	if (success) {
		// Is there a valid lobby?
		if (game_info.m_steamIDLobby.IsValid()) {
			friend_game["id"] = game_info.m_gameID.AppID();
			// Convert the IP address back to a string
			const int NBYTES = 4;
			uint8 octet[NBYTES];
			char gameIP[16];
			for (int j = 0; j < NBYTES; j++) {
				octet[j] = game_info.m_unGameIP >> (j * 8);
			}
			sprintf(gameIP, "%d.%d.%d.%d", octet[0], octet[1], octet[2], octet[3]);
			friend_game["ip"] = gameIP;
			friend_game["game_port"] = game_info.m_usGamePort;
			friend_game["query_port"] = (char)game_info.m_usQueryPort;
			friend_game["lobby"] = uint64_t(game_info.m_steamIDLobby.ConvertToUint64());
		} else {
			friend_game["id"] = game_info.m_gameID.AppID();
			friend_game["ip"] = "0.0.0.0";
			friend_game["game_port"] = "0";
			friend_game["query_port"] = "0";
			friend_game["lobby"] = "No valid lobby";
		}
	}
	return friend_game;
}

// Gets the data from a Steam friends message. This should only ever be called in response to a GameConnectedFriendChatMsg_t callback.
Dictionary SteamServer::getFriendMessage(uint64_t friend_id, int message) {
	Dictionary chat;
	if (SteamFriends() == NULL) {
		return chat;
	}
	char text[STEAM_LARGE_BUFFER_SIZE];
	EChatEntryType type = k_EChatEntryTypeInvalid;
	chat["ret"] = SteamFriends()->GetFriendMessage(createSteamID(friend_id), message, text, STEAM_LARGE_BUFFER_SIZE, &type);
	chat["text"] = String(text);
	return chat;
}

// Get given friend's Steam username.
String SteamServer::getFriendPersonaName(uint64_t steam_id) {
	if (SteamFriends() != NULL && steam_id > 0) {
		CSteamID user_id = (uint64)steam_id;
		bool is_data_loading = SteamFriends()->RequestUserInformation(user_id, true);
		if (!is_data_loading) {
			return String::utf8(SteamFriends()->GetFriendPersonaName(user_id));
		}
	}
	return "";
}

// Accesses old friends names; returns an empty string when there are no more items in the history.
String SteamServer::getFriendPersonaNameHistory(uint64_t steam_id, int name_history) {
	if (SteamFriends() == NULL) {
		return "";
	}
	CSteamID user_id = (uint64)steam_id;
	return String::utf8(SteamFriends()->GetFriendPersonaNameHistory(user_id, name_history));
}

// Returns the current status of the specified user.
SteamServer::PersonaState SteamServer::getFriendPersonaState(uint64_t steam_id) {
	if (SteamFriends() == NULL) {
		return PERSONA_STATE_OFFLINE;
	}
	CSteamID user_id = (uint64)steam_id;
	return PersonaState(SteamFriends()->GetFriendPersonaState(user_id));
}

// Returns a relationship to a user.
SteamServer::FriendRelationship SteamServer::getFriendRelationship(uint64_t steam_id) {
	if (SteamFriends() == NULL) {
		return FRIEND_RELATION_NONE;
	}
	CSteamID user_id = (uint64)steam_id;
	return FriendRelationship(SteamFriends()->GetFriendRelationship(user_id));
}

// Get a Rich Presence value from a specified friend (typically only used for debugging).
String SteamServer::getFriendRichPresence(uint64_t friend_id, const String &key) {
	if (SteamFriends() == NULL) {
		return "";
	}
	CSteamID user = (uint64)friend_id;
	return SteamFriends()->GetFriendRichPresence(user, key.utf8().get_data());
}

// Gets the number of Rich Presence keys that are set on the specified user.
int SteamServer::getFriendRichPresenceKeyCount(uint64_t friend_id) {
	if (SteamFriends() == NULL) {
		return 0;
	}
	CSteamID user = (uint64)friend_id;
	return SteamFriends()->GetFriendRichPresenceKeyCount(user);
}

// Returns an empty string ("") if the index is invalid or the specified user has no Rich Presence data available.
String SteamServer::getFriendRichPresenceKeyByIndex(uint64_t friend_id, int key) {
	if (SteamFriends() == NULL) {
		return "";
	}
	CSteamID user = (uint64)friend_id;
	return SteamFriends()->GetFriendRichPresenceKeyByIndex(user, key);
}

// Gets the number of friends groups (tags) the user has created.  This is used for iteration, after calling this then GetFriendsGroupIDByIndex can be used to get the ID of each friend group.  This is not to be confused with Steam groups. Those can be obtained with GetClanCount.
int SteamServer::getFriendsGroupCount() {
	if (SteamFriends() == NULL) {
		return 0;
	}
	return SteamFriends()->GetFriendsGroupCount();
}

// Gets the friends group ID for the given index.
int16 SteamServer::getFriendsGroupIDByIndex(int16 friend_group) {
	if (SteamFriends() == NULL) {
		return 0;
	}
	return SteamFriends()->GetFriendsGroupIDByIndex(friend_group);
}

// Gets the number of friends in a given friends group.  This should be called before getting the list of friends with GetFriendsGroupMembersList.
int SteamServer::getFriendsGroupMembersCount(int16 friend_group) {
	if (SteamFriends() == NULL) {
		return 0;
	}
	return SteamFriends()->GetFriendsGroupMembersCount(friend_group);
}

// Gets the number of friends in the given friends group.  If fewer friends exist than requested those positions' Steam IDs will be invalid.  You must call GetFriendsGroupMembersCount before calling this to set up the pOutSteamIDMembers array with an appropriate size!
Array SteamServer::getFriendsGroupMembersList(int16 friend_group, int member_count) {
	Array member_list;
	if (SteamFriends() == NULL) {
		return member_list;
	}
	CSteamID friend_ids;
	SteamFriends()->GetFriendsGroupMembersList((FriendsGroupID_t)friend_group, &friend_ids, member_count);
	uint64_t friends = friend_ids.ConvertToUint64();
	member_list.append(friends);
	return member_list;
}

// Gets the name for the given friends group.
String SteamServer::getFriendsGroupName(int16 friend_group) {
	if (SteamFriends() == NULL) {
		return "";
	}
	return String::utf8(SteamFriends()->GetFriendsGroupName(friend_group));
}

// Get friend's steam level, obviously.
int SteamServer::getFriendSteamLevel(uint64_t steam_id) {
	if (SteamFriends() == NULL) {
		return 0;
	}
	CSteamID user_id = (uint64)steam_id;
	return SteamFriends()->GetFriendSteamLevel(user_id);
}

// Gets the large (184x184) avatar of the current user, which is a handle to be used in GetImageRGBA(), or 0 if none set.
int SteamServer::getLargeFriendAvatar(uint64_t steam_id) {
	if (SteamFriends() == NULL) {
		return 0;
	}
	CSteamID user_id = (uint64)steam_id;
	return SteamFriends()->GetLargeFriendAvatar(user_id);
}

// Gets the medium (64x64) avatar of the current user, which is a handle to be used in GetImageRGBA(), or 0 if none set.
int SteamServer::getMediumFriendAvatar(uint64_t steam_id) {
	if (SteamFriends() == NULL) {
		return 0;
	}
	CSteamID user_id = (uint64)steam_id;
	return SteamFriends()->GetMediumFriendAvatar(user_id);
}

// Get the user's Steam username.
String SteamServer::getPersonaName() {
	if (SteamFriends() == NULL) {
		return "";
	}
	return String::utf8(SteamFriends()->GetPersonaName());
}

// Gets the status of the current user.
SteamServer::PersonaState SteamServer::getPersonaState() {
	if (SteamFriends() == NULL) {
		return PERSONA_STATE_OFFLINE;
	}
	return PersonaState(SteamFriends()->GetPersonaState());
}

// Get player's avatar.
void SteamServer::getPlayerAvatar(int size, uint64_t steam_id) {
	// If no Steam ID is given, use the current user's
	if (steam_id == 0) {
		steam_id = getSteamID();
	}
	if (SteamFriends() != NULL && size > 0 && size < 4) {
		int handle = -2;
		switch (size) {
			case 1: {
				handle = getSmallFriendAvatar(steam_id);
				size = 32;
				break;
			}
			case 2: {
				handle = getMediumFriendAvatar(steam_id);
				size = 64;
				break;
			}
			case 3: {
				handle = getLargeFriendAvatar(steam_id);
				size = 184;
				break;
			}
			default:
				return;
		}
		if (handle <= 0) {
			printf("[Steam] Error retrieving avatar handle.");
		}
		AvatarImageLoaded_t *avatar_data = new AvatarImageLoaded_t;
		CSteamID avatar_id = (uint64)steam_id;
		avatar_data->m_steamID = avatar_id;
		avatar_data->m_iImage = handle;
		avatar_data->m_iWide = size;
		avatar_data->m_iTall = size;
		avatar_loaded(avatar_data);
		delete avatar_data;
	}
}

// Returns nickname the current user has set for the specified player. Returns NULL if the no nickname has been set for that player.
String SteamServer::getPlayerNickname(uint64_t steam_id) {
	if (SteamFriends() == NULL) {
		return "";
	}
	CSteamID user_id = (uint64)steam_id;
	return String::utf8(SteamFriends()->GetPlayerNickname(user_id));
}

// Returns a string property for a user's equipped profile item.
String SteamServer::getProfileItemPropertyString(uint64_t steam_id, CommunityProfileItemType item_type, CommunityProfileItemProperty item_property) {
	if (SteamFriends() == NULL) {
		return "";
	}
	CSteamID user_id = (uint64)steam_id;
	return String::utf8(SteamFriends()->GetProfileItemPropertyString(user_id, (ECommunityProfileItemType)item_type, (ECommunityProfileItemProperty)item_property));
}

// Returns an unsigned integer property for a user's equipped profile item.
uint32 SteamServer::getProfileItemPropertyInt(uint64_t steam_id, CommunityProfileItemType item_type, CommunityProfileItemProperty item_property) {
	if (SteamFriends() == NULL) {
		return 0;
	}
	CSteamID user_id = (uint64)steam_id;
	return SteamFriends()->GetProfileItemPropertyUint(user_id, (ECommunityProfileItemType)item_type, (ECommunityProfileItemProperty)item_property);
}

// Get list of players user has recently played game with.
Array SteamServer::getRecentPlayers() {
	if (SteamFriends() == NULL) {
		return Array();
	}
	int count = SteamFriends()->GetCoplayFriendCount();
	Array recents;
	for (int i = 0; i < count; i++) {
		CSteamID player_id = SteamFriends()->GetCoplayFriend(i);
		if (SteamFriends()->GetFriendCoplayGame(player_id) == SteamUtils()->GetAppID()) {
			Dictionary player;
			int time = SteamFriends()->GetFriendCoplayTime(player_id);
			int status = SteamFriends()->GetFriendPersonaState(player_id);
			player["id"] = (uint64_t)player_id.ConvertToUint64();
			player["name"] = String::utf8(SteamFriends()->GetFriendPersonaName(player_id));
			player["time"] = time;
			player["status"] = status;
			recents.append(player);
		}
	}
	return recents;
}

// Gets the small (32x32) avatar of the current user, which is a handle to be used in GetImageRGBA(), or 0 if none set.
int SteamServer::getSmallFriendAvatar(uint64_t steam_id) {
	if (SteamFriends() == NULL) {
		return 0;
	}
	CSteamID user_id = (uint64)steam_id;
	return SteamFriends()->GetSmallFriendAvatar(user_id);
}

// Get list of friends groups (tags) the user has created. This is not to be confused with Steam groups.
Array SteamServer::getUserFriendsGroups() {
	if (SteamFriends() == NULL) {
		return Array();
	}
	int tag_count = SteamFriends()->GetFriendsGroupCount();
	Array friends_groups;
	for (int i = 0; i < tag_count; i++) {
		Dictionary tags;
		int16 friends_group_id = SteamFriends()->GetFriendsGroupIDByIndex(i);
		String group_name = SteamFriends()->GetFriendsGroupName(friends_group_id);
		int group_members = SteamFriends()->GetFriendsGroupMembersCount(friends_group_id);
		tags["id"] = friends_group_id;
		tags["name"] = group_name;
		tags["members"] = group_members;
		friends_groups.append(tags);
	}
	return friends_groups;
}

// If current user is chat restricted, he can't send or receive any text/voice chat messages. The user can't see custom avatars. But the user can be online and send/recv game invites.
uint32 SteamServer::getUserRestrictions() {
	if (SteamFriends() == NULL) {
		return 0;
	}
	return SteamFriends()->GetUserRestrictions();
}

// Get a list of user's Steam friends; a mix of different Steamworks API friend functions.
Array SteamServer::getUserSteamFriends() {
	if (SteamFriends() == NULL) {
		return Array();
	}
	int count = SteamFriends()->GetFriendCount(0x04);
	Array steam_friends;
	for (int i = 0; i < count; i++) {
		Dictionary friends;
		CSteamID friend_id = SteamFriends()->GetFriendByIndex(i, 0x04);
		int status = SteamFriends()->GetFriendPersonaState(friend_id);
		friends["id"] = (uint64_t)friend_id.ConvertToUint64();
		friends["name"] = String::utf8(SteamFriends()->GetFriendPersonaName(friend_id));
		friends["status"] = status;
		steam_friends.append(friends);
	}
	return steam_friends;
}

// Get list of user's Steam groups; a mix of different Steamworks API group functions.
Array SteamServer::getUserSteamGroups() {
	if (SteamFriends() == NULL) {
		return Array();
	}
	int group_count = SteamFriends()->GetClanCount();
	Array steam_groups;
	for (int i = 0; i < group_count; i++) {
		Dictionary groups;
		CSteamID group_id = SteamFriends()->GetClanByIndex(i);
		String name = SteamFriends()->GetClanName(group_id);
		String tag = SteamFriends()->GetClanTag(group_id);
		groups["id"] = (uint64_t)group_id.ConvertToUint64();
		groups["name"] = name;
		groups["tag"] = tag;
		steam_groups.append(groups);
	}
	return steam_groups;
}

// After calling RequestEquippedProfileItems, you can use this function to check if the user has a type of profile item equipped or not.
bool SteamServer::hasEquippedProfileItem(uint64_t steam_id, CommunityProfileItemType item_type) {
	if (SteamFriends() == NULL) {
		return false;
	}
	CSteamID user_id = (uint64)steam_id;
	return SteamFriends()->BHasEquippedProfileItem(user_id, (ECommunityProfileItemType)item_type);
}

// Returns true if the specified user meets any of the criteria specified in iFriendFlags.
bool SteamServer::hasFriend(uint64_t steam_id, BitField<FriendFlags> friend_flags) {
	if (SteamFriends() == NULL) {
		return false;
	}
	CSteamID user_id = (uint64)steam_id;
	return SteamFriends()->HasFriend(user_id, (int)friend_flags);
}

// Invite friend to current game/lobby.
bool SteamServer::inviteUserToGame(uint64_t steam_id, const String &connect_string) {
	if (SteamFriends() == NULL) {
		return false;
	}
	CSteamID user_id = (uint64)steam_id;
	return SteamFriends()->InviteUserToGame(user_id, connect_string.utf8().get_data());
}

// Checks if a user in the Steam group chat room is an admin.
bool SteamServer::isClanChatAdmin(uint64_t chat_id, uint64_t steam_id) {
	if (SteamFriends() == NULL) {
		return false;
	}
	CSteamID chat = (uint64)chat_id;
	CSteamID user_id = (uint64)steam_id;
	return SteamFriends()->IsClanChatAdmin(chat, user_id);
}

// Checks if the Steam group is public.
bool SteamServer::isClanPublic(uint64_t clan_id) {
	if (SteamFriends() == NULL) {
		return false;
	}
	clan_activity = (uint64)clan_id;
	return SteamFriends()->IsClanPublic(clan_activity);
}

// Checks if the Steam group is an official game group/community hub.
bool SteamServer::isClanOfficialGameGroup(uint64_t clan_id) {
	if (SteamFriends() == NULL) {
		return false;
	}
	clan_activity = (uint64)clan_id;
	return SteamFriends()->IsClanOfficialGameGroup(clan_activity);
}

// Checks if the Steam Group chat room is open in the Steam UI.
bool SteamServer::isClanChatWindowOpenInSteam(uint64_t chat_id) {
	if (SteamFriends() == NULL) {
		return false;
	}
	CSteamID chat = (uint64)chat_id;
	return SteamFriends()->IsClanChatWindowOpenInSteam(chat);
}

// Checks if the current user is following the specified user.
void SteamServer::isFollowing(uint64_t steam_id) {
	if (SteamFriends() != NULL) {
		CSteamID user_id = (uint64)steam_id;
		SteamAPICall_t api_call = SteamFriends()->IsFollowing(user_id);
		callResultIsFollowing.Set(api_call, this, &SteamServer::is_following);
	}
}

// Returns true if the local user can see that steam_id_user is a member or in source_id.
bool SteamServer::isUserInSource(uint64_t steam_id, uint64_t source_id) {
	if (SteamFriends() == NULL) {
		return false;
	}
	CSteamID user_id = (uint64)steam_id;
	CSteamID source = (uint64)source_id;
	return SteamFriends()->IsUserInSource(user_id, source);
}

// Allows the user to join Steam group (clan) chats right within the game.
void SteamServer::joinClanChatRoom(uint64_t clan_id) {
	if (SteamFriends() != NULL) {
		clan_activity = (uint64)clan_id;
		SteamFriends()->JoinClanChatRoom(clan_activity);
	}
}

// Leaves a Steam group chat that the user has previously entered with JoinClanChatRoom.
bool SteamServer::leaveClanChatRoom(uint64_t clan_id) {
	if (SteamFriends() == NULL) {
		return false;
	}
	clan_activity = (uint64)clan_id;
	return SteamFriends()->LeaveClanChatRoom(clan_activity);
}

// Opens the specified Steam group chat room in the Steam UI.
bool SteamServer::openClanChatWindowInSteam(uint64_t chat_id) {
	if (SteamFriends() == NULL) {
		return false;
	}
	CSteamID chat = (uint64)chat_id;
	return SteamFriends()->OpenClanChatWindowInSteam(chat);
}

// Call this before calling ActivateGameOverlayToWebPage() to have the Steam Overlay Browser block navigations to your specified protocol (scheme) uris and instead dispatch a OverlayBrowserProtocolNavigation_t callback to your game.
bool SteamServer::registerProtocolInOverlayBrowser(const String &protocol) {
	if (SteamFriends() == NULL) {
		return false;
	}
	return SteamFriends()->RegisterProtocolInOverlayBrowser(protocol.utf8().get_data());
}

// Sends a message to a Steam friend.
bool SteamServer::replyToFriendMessage(uint64_t steam_id, const String &message) {
	if (SteamFriends() == NULL) {
		return false;
	}
	CSteamID user_id = (uint64)steam_id;
	return SteamFriends()->ReplyToFriendMessage(user_id, message.utf8().get_data());
}

// Requests information about a clan officer list; when complete, data is returned in ClanOfficerListResponse_t call result.
void SteamServer::requestClanOfficerList(uint64_t clan_id) {
	if (SteamFriends() != NULL) {
		clan_activity = (uint64)clan_id;
		SteamAPICall_t api_call = SteamFriends()->GetFollowerCount(clan_activity);
		callResultClanOfficerList.Set(api_call, this, &SteamServer::request_clan_officer_list);
	}
}

// Requests the list of equipped Steam Community profile items for the given user from Steam.
void SteamServer::requestEquippedProfileItems(uint64_t steam_id) {
	if (SteamFriends() != NULL) {
		CSteamID user_id = (uint64)steam_id;
		SteamAPICall_t api_call = SteamFriends()->RequestEquippedProfileItems(user_id);
		callResultEquippedProfileItems.Set(api_call, this, &SteamServer::equipped_profile_items);
	}
}

// Requests rich presence for a specific user.
void SteamServer::requestFriendRichPresence(uint64_t friend_id) {
	if (SteamFriends() != NULL) {
		CSteamID user = (uint64)friend_id;
		return SteamFriends()->RequestFriendRichPresence(user);
	}
}

// Requests information about a user - persona name & avatar; if bRequireNameOnly is set, then the avatar of a user isn't downloaded.
bool SteamServer::requestUserInformation(uint64_t steam_id, bool require_name_only) {
	if (SteamFriends() == NULL) {
		return false;
	}
	CSteamID user_id = (uint64)steam_id;
	return SteamFriends()->RequestUserInformation(user_id, require_name_only);
}

// Sends a message to a Steam group chat room.
bool SteamServer::sendClanChatMessage(uint64_t chat_id, const String &text) {
	if (SteamFriends() == NULL) {
		return false;
	}
	CSteamID chat = (uint64)chat_id;
	return SteamFriends()->SendClanChatMessage(chat, text.utf8().get_data());
}

// User is in a game pressing the talk button (will suppress the microphone for all voice comms from the Steam friends UI).
void SteamServer::setInGameVoiceSpeaking(uint64_t steam_id, bool speaking) {
	if (SteamFriends() != NULL) {
		CSteamID user_id = (uint64)steam_id;
		SteamFriends()->SetInGameVoiceSpeaking(user_id, speaking);
	}
}

// Listens for Steam friends chat messages.
bool SteamServer::setListenForFriendsMessages(bool intercept) {
	if (SteamFriends() == NULL) {
		return false;
	}
	return SteamFriends()->SetListenForFriendsMessages(intercept);
}

// Sets the player name, stores it on the server and publishes the changes to all friends who are online.
void SteamServer::setPersonaName(const String &name) {
	if (SteamFriends() != NULL) {
		SteamFriends()->SetPersonaName(name.utf8().get_data());
	}
}

// Set player as 'Played With' for game.
void SteamServer::setPlayedWith(uint64_t steam_id) {
	if (SteamFriends() != NULL) {
		CSteamID user_id = (uint64)steam_id;
		SteamFriends()->SetPlayedWith(user_id);
	}
}

// Set the game information in Steam; used in 'View Game Info'
bool SteamServer::setRichPresence(const String &key, const String &value) {
	// Rich presence data is automatically shared between friends in the same game.
	// Each user has a set of key/value pairs, up to 20 can be set.
	// Two magic keys (status, connect).
	// setGameInfo() to an empty string deletes the key.
	if (SteamFriends() == NULL) {
		return false;
	}
	return SteamFriends()->SetRichPresence(key.utf8().get_data(), value.utf8().get_data());
}

/////////////////////////////////////////////////
///// GAME SEARCH
/////////////////////////////////////////////////
//
// A keyname and a list of comma separated values: one of which is must be found in order for the match to qualify; fails if a search is currently in progress.
int SteamServer::addGameSearchParams(const String &key, const String &values) {
	if (SteamGameSearch() == NULL) {
		return 9;
	}
	return SteamGameSearch()->AddGameSearchParams(key.utf8().get_data(), values.utf8().get_data());
}

// All players in lobby enter the queue and await a SearchForGameNotificationCallback_t callback. Fails if another search is currently in progress. If not the owner of the lobby or search already in progress this call fails. Periodic callbacks will be sent as queue time estimates change.
int SteamServer::searchForGameWithLobby(uint64_t lobby_id, int player_min, int player_max) {
	if (SteamGameSearch() == NULL) {
		return 9;
	}
	CSteamID lobby = (uint64)lobby_id;
	return SteamGameSearch()->SearchForGameWithLobby(lobby, player_min, player_max);
}

// User enter the queue and await a SearchForGameNotificationCallback_t callback. fails if another search is currently in progress. Periodic callbacks will be sent as queue time estimates change.
int SteamServer::searchForGameSolo(int player_min, int player_max) {
	if (SteamGameSearch() == NULL) {
		return 9;
	}
	return SteamGameSearch()->SearchForGameSolo(player_min, player_max);
}

// After receiving SearchForGameResultCallback_t, accept or decline the game. Multiple SearchForGameResultCallback_t will follow as players accept game until the host starts or cancels the game.
int SteamServer::acceptGame() {
	if (SteamGameSearch() == NULL) {
		return 9;
	}
	return SteamGameSearch()->AcceptGame();
}

// After receiving SearchForGameResultCallback_t, accept or decline the game. Multiple SearchForGameResultCallback_t will follow as players accept game until the host starts or cancels the game.
int SteamServer::declineGame() {
	if (SteamGameSearch() == NULL) {
		return 9;
	}
	return SteamGameSearch()->DeclineGame();
}

// After receiving GameStartedByHostCallback_t get connection details to server.
String SteamServer::retrieveConnectionDetails(uint64_t host_id) {
	if (SteamGameSearch() == NULL) {
		return "";
	}
	CSteamID host = (uint64)host_id;
	char connection_details;
	SteamGameSearch()->RetrieveConnectionDetails(host, &connection_details, 256);
	// Messy conversion from char to String
	String details;
	details += connection_details;
	return details;
}

// Leaves queue if still waiting.
int SteamServer::endGameSearch() {
	if (SteamGameSearch() == NULL) {
		return 9;
	}
	return SteamGameSearch()->EndGameSearch();
}

// A keyname and a list of comma separated values: all the values you allow.
int SteamServer::setGameHostParams(const String &key, const String &value) {
	if (SteamGameSearch() == NULL) {
		return 9;
	}
	return SteamGameSearch()->SetGameHostParams(key.utf8().get_data(), value.utf8().get_data());
}

// Set connection details for players once game is found so they can connect to this server.
int SteamServer::setConnectionDetails(const String &details, int connection_details) {
	if (SteamGameSearch() == NULL) {
		return 9;
	}
	return SteamGameSearch()->SetConnectionDetails(details.utf8().get_data(), connection_details);
}

// Mark server as available for more players with nPlayerMin,nPlayerMax desired. Accept no lobbies with playercount greater than nMaxTeamSize.
int SteamServer::requestPlayersForGame(int player_min, int player_max, int max_team_size) {
	if (SteamGameSearch() == NULL) {
		return 9;
	}
	return SteamGameSearch()->RequestPlayersForGame(player_min, player_max, max_team_size);
}

// Accept the player list and release connection details to players.
int SteamServer::hostConfirmGameStart(uint64_t game_id) {
	if (SteamGameSearch() == NULL) {
		return 9;
	}
	return SteamGameSearch()->HostConfirmGameStart(game_id);
}

// Cancel request and leave the pool of game hosts looking for players.
int SteamServer::cancelRequestPlayersForGame() {
	if (SteamGameSearch() == NULL) {
		return 9;
	}
	return SteamGameSearch()->CancelRequestPlayersForGame();
}

// Submit a result for one player. does not end the game. ullUniqueGameID continues to describe this game.
int SteamServer::submitPlayerResult(uint64_t game_id, uint64_t player_id, PlayerResult player_result) {
	if (SteamGameSearch() == NULL) {
		return 9;
	}
	CSteamID player = (uint64)player_id;
	return SteamGameSearch()->SubmitPlayerResult(game_id, player, (EPlayerResult_t)player_result);
}

// Ends the game. no further SubmitPlayerResults for ullUniqueGameID will be accepted.
int SteamServer::endGame(uint64_t game_id) {
	if (SteamGameSearch() == NULL) {
		return 9;
	}
	return SteamGameSearch()->EndGame(game_id);
}

/////////////////////////////////////////////////
///// HTTP
/////////////////////////////////////////////////
//
// Creates a cookie container to store cookies during the lifetime of the process. This API is just for during process lifetime, after steam restarts no cookies are persisted and you have no way to access the cookie container across repeat executions of your process.
uint32_t SteamServer::createCookieContainer(bool allow_responses_to_modify) {
	if (SteamHTTP() == NULL) {
		return 0;
	}
	return SteamHTTP()->CreateCookieContainer(allow_responses_to_modify);
}

// Initializes a new HTTP request.
uint32_t SteamServer::createHTTPRequest(HTTPMethod request_method, const String &absolute_url) {
	if (SteamHTTP() != NULL) {
		return SteamHTTP()->CreateHTTPRequest((EHTTPMethod)request_method, absolute_url.utf8().get_data());
	}
	return HTTPREQUEST_INVALID_HANDLE;
}

// Defers a request which has already been sent by moving it at the back of the queue.
bool SteamServer::deferHTTPRequest(uint32 request_handle) {
	if (SteamHTTP() == NULL) {
		return false;
	}
	return SteamHTTP()->DeferHTTPRequest(request_handle);
}

// Gets progress on downloading the body for the request.
float SteamServer::getHTTPDownloadProgressPct(uint32 request_handle) {
	float percent_one = 0.0;
	if (SteamHTTP() != NULL) {
		SteamHTTP()->GetHTTPDownloadProgressPct(request_handle, &percent_one);
	}
	return percent_one;
}

// Check if the reason the request failed was because we timed it out (rather than some harder failure).
bool SteamServer::getHTTPRequestWasTimedOut(uint32 request_handle) {
	bool was_timed_out = false;
	if (SteamHTTP() != NULL) {
		SteamHTTP()->GetHTTPRequestWasTimedOut(request_handle, &was_timed_out);
	}
	return was_timed_out;
}

// Gets the body data from an HTTP response.
PackedByteArray SteamServer::getHTTPResponseBodyData(uint32 request_handle, uint32 buffer_size) {
	PackedByteArray body_data;
	body_data.resize(buffer_size);
	if (SteamHTTP() != NULL) {
		SteamHTTP()->GetHTTPResponseBodyData(request_handle, body_data.ptrw(), buffer_size);
	}
	return body_data;
}

// Gets the size of the body data from an HTTP response.
uint32 SteamServer::getHTTPResponseBodySize(uint32 request_handle) {
	uint32 body_size = 0;
	if (SteamHTTP() != NULL) {
		SteamHTTP()->GetHTTPResponseBodySize(request_handle, &body_size);
	}
	return body_size;
}

// Checks if a header is present in an HTTP response and returns its size.
uint32 SteamServer::getHTTPResponseHeaderSize(uint32 request_handle, const String &header_name) {
	uint32 response_header_size = 0;
	if (SteamHTTP() != NULL) {
		SteamHTTP()->GetHTTPResponseHeaderSize(request_handle, header_name.utf8().get_data(), &response_header_size);
	}
	return response_header_size;
}

// Gets a header value from an HTTP response.
uint8 SteamServer::getHTTPResponseHeaderValue(uint32 request_handle, const String &header_name, uint32 buffer_size) {
	uint8 value_buffer = 0;
	if (SteamHTTP() != NULL) {
		SteamHTTP()->GetHTTPResponseHeaderValue(request_handle, header_name.utf8().get_data(), &value_buffer, buffer_size);
	}
	return value_buffer;
}

// Gets the body data from a streaming HTTP response.
uint8 SteamServer::getHTTPStreamingResponseBodyData(uint32 request_handle, uint32 offset, uint32 buffer_size) {
	uint8 body_data_buffer = 0;
	if (SteamHTTP() != NULL) {
		SteamHTTP()->GetHTTPStreamingResponseBodyData(request_handle, offset, &body_data_buffer, buffer_size);
	}
	return body_data_buffer;
}

// Prioritizes a request which has already been sent by moving it at the front of the queue.
bool SteamServer::prioritizeHTTPRequest(uint32 request_handle) {
	if (SteamHTTP() == NULL) {
		return false;
	}
	return SteamHTTP()->PrioritizeHTTPRequest(request_handle);
}

// Releases a cookie container, freeing the memory allocated within Steam.
bool SteamServer::releaseCookieContainer(uint32 cookie_handle) {
	if (SteamHTTP() == NULL) {
		return false;
	}
	return SteamHTTP()->ReleaseCookieContainer(cookie_handle);
}

// Releases an HTTP request handle, freeing the memory allocated within Steam.
bool SteamServer::releaseHTTPRequest(uint32 request_handle) {
	if (SteamHTTP() == NULL) {
		return false;
	}
	return SteamHTTP()->ReleaseHTTPRequest(request_handle);
}

// Sends an HTTP request.
bool SteamServer::sendHTTPRequest(uint32 request_handle) {
	if (SteamHTTP() == NULL) {
		return false;
	}
	SteamAPICall_t call_handle;
	return SteamHTTP()->SendHTTPRequest(request_handle, &call_handle);
}

// Sends an HTTP request and streams the response back in chunks.
bool SteamServer::sendHTTPRequestAndStreamResponse(uint32 request_handle) {
	if (SteamHTTP() == NULL) {
		return false;
	}
	SteamAPICall_t call_handle;
	return SteamHTTP()->SendHTTPRequestAndStreamResponse(request_handle, &call_handle);
}

// Adds a cookie to the specified cookie container that will be used with future requests.
bool SteamServer::setHTTPCookie(uint32 cookie_handle, const String &host, const String &url, const String &cookie) {
	if (SteamHTTP() == NULL) {
		return false;
	}
	return SteamHTTP()->SetCookie(cookie_handle, host.utf8().get_data(), url.utf8().get_data(), cookie.utf8().get_data());
}

// Set an absolute timeout in milliseconds for the HTTP request. This is the total time timeout which is different than the network activity timeout which is set with SetHTTPRequestNetworkActivityTimeout which can bump everytime we get more data.
bool SteamServer::setHTTPRequestAbsoluteTimeoutMS(uint32 request_handle, uint32 milliseconds) {
	if (SteamHTTP() == NULL) {
		return false;
	}
	return SteamHTTP()->SetHTTPRequestAbsoluteTimeoutMS(request_handle, milliseconds);
}

// Set a context value for the request, which will be returned in the HTTPRequestCompleted_t callback after sending the request. This is just so the caller can easily keep track of which callbacks go with which request data. Must be called before sending the request.
bool SteamServer::setHTTPRequestContextValue(uint32 request_handle, uint64_t context_value) {
	if (SteamHTTP() == NULL) {
		return false;
	}
	return SteamHTTP()->SetHTTPRequestContextValue(request_handle, context_value);
}

// Associates a cookie container to use for an HTTP request.
bool SteamServer::setHTTPRequestCookieContainer(uint32 request_handle, uint32 cookie_handle) {
	if (SteamHTTP() == NULL) {
		return false;
	}
	return SteamHTTP()->SetHTTPRequestCookieContainer(request_handle, cookie_handle);
}

// Set a GET or POST parameter value on the HTTP request. Must be called prior to sending the request.
bool SteamServer::setHTTPRequestGetOrPostParameter(uint32 request_handle, const String &name, const String &value) {
	if (SteamHTTP() == NULL) {
		return false;
	}
	return SteamHTTP()->SetHTTPRequestGetOrPostParameter(request_handle, name.utf8().get_data(), value.utf8().get_data());
}

// Set a request header value for the HTTP request. Must be called before sending the request.
bool SteamServer::setHTTPRequestHeaderValue(uint32 request_handle, const String &header_name, const String &header_value) {
	if (SteamHTTP() == NULL) {
		return false;
	}
	return SteamHTTP()->SetHTTPRequestHeaderValue(request_handle, header_name.utf8().get_data(), header_value.utf8().get_data());
}

// Set the timeout in seconds for the HTTP request.
bool SteamServer::setHTTPRequestNetworkActivityTimeout(uint32 request_handle, uint32 timeout_seconds) {
	if (SteamHTTP() == NULL) {
		return false;
	}
	return SteamHTTP()->SetHTTPRequestNetworkActivityTimeout(request_handle, timeout_seconds);
}

// Sets the body for an HTTP Post request.
uint8 SteamServer::setHTTPRequestRawPostBody(uint32 request_handle, const String &content_type, uint32 body_length) {
	uint8 body = 0;
	if (SteamHTTP()) {
		SteamHTTP()->SetHTTPRequestRawPostBody(request_handle, content_type.utf8().get_data(), &body, body_length);
	}
	return body;
}

// Sets that the HTTPS request should require verified SSL certificate via machines certificate trust store. This currently only works Windows and macOS.
bool SteamServer::setHTTPRequestRequiresVerifiedCertificate(uint32 request_handle, bool require_verified_certificate) {
	if (SteamHTTP() == NULL) {
		return false;
	}
	return SteamHTTP()->SetHTTPRequestRequiresVerifiedCertificate(request_handle, require_verified_certificate);
}

// Set additional user agent info for a request.
bool SteamServer::setHTTPRequestUserAgentInfo(uint32 request_handle, const String &user_agent_info) {
	if (SteamHTTP() == NULL) {
		return false;
	}
	return SteamHTTP()->SetHTTPRequestUserAgentInfo(request_handle, user_agent_info.utf8().get_data());
}

/////////////////////////////////////////////////
///// INVENTORY
/////////////////////////////////////////////////
//
// When dealing with any inventory handles, you should call CheckResultSteamID on the result handle when it completes to verify that a remote player is not pretending to have a different user's inventory.
// Also, you must call DestroyResult on the provided inventory result when you are done with it.
//
// Grant a specific one-time promotional item to the current user.
int32 SteamServer::addPromoItem(uint32 item) {
	int32 new_inventory_handle = 0;
	if (SteamInventory() != NULL) {
		if (SteamInventory()->AddPromoItem(&new_inventory_handle, item)) {
			// Update the internally stored handle
			inventory_handle = new_inventory_handle;
		}
	}
	return new_inventory_handle;
}

// Grant a specific one-time promotional items to the current user.
int32 SteamServer::addPromoItems(PackedInt64Array items) {
	int32 new_inventory_handle = 0;
	if (SteamInventory() != NULL) {
		int count = items.size();
		SteamItemDef_t *new_items = new SteamItemDef_t[items.size()];
		for (int i = 0; i < count; i++) {
			new_items[i] = items[i];
		}
		if (SteamInventory()->AddPromoItems(&new_inventory_handle, new_items, count)) {
			// Update the internally stored handle
			inventory_handle = new_inventory_handle;
		}
		delete[] new_items;
	}
	return new_inventory_handle;
}

// Checks whether an inventory result handle belongs to the specified Steam ID.
bool SteamServer::checkResultSteamID(uint64_t steam_id_expected, int32 this_inventory_handle) {
	if (SteamInventory() == NULL) {
		return false;
	}
	CSteamID steam_id = (uint64)steam_id_expected;
	// If no inventory handle is passed, use internal one
	if (this_inventory_handle == 0) {
		this_inventory_handle = inventory_handle;
	}
	return SteamInventory()->CheckResultSteamID((SteamInventoryResult_t)this_inventory_handle, steam_id);
}

// Consumes items from a user's inventory. If the quantity of the given item goes to zero, it is permanently removed.
int32 SteamServer::consumeItem(uint64_t item_consume, uint32 quantity) {
	int32 new_inventory_handle = 0;
	if (SteamInventory() != NULL) {
		if (SteamInventory()->ConsumeItem(&new_inventory_handle, (SteamItemInstanceID_t)item_consume, quantity)) {
			// Update the internally stored handle
			inventory_handle = new_inventory_handle;
		}
	}
	return new_inventory_handle;
}

// Deserializes a result set and verifies the signature bytes.
int32 SteamServer::deserializeResult(PackedByteArray buffer) {
	int32 new_inventory_handle = 0;
	if (SteamInventory() != NULL) {
		if (SteamInventory()->DeserializeResult(&new_inventory_handle, &buffer, buffer.size(), false)) {
			// Update the internally stored handle
			inventory_handle = new_inventory_handle;
		}
	}
	return new_inventory_handle;
}

// Destroys a result handle and frees all associated memory.
void SteamServer::destroyResult(int this_inventory_handle) {
	if (SteamInventory() != NULL) {
		// If no inventory handle is passed, use internal one
		if (this_inventory_handle == 0) {
			this_inventory_handle = inventory_handle;
		}
		SteamInventory()->DestroyResult((SteamInventoryResult_t)this_inventory_handle);
	}
}

// Grant one item in exchange for a set of other items.
int32 SteamServer::exchangeItems(const PackedInt64Array output_items, const uint32 output_quantity, const uint64_t input_items, const uint32 input_quantity) {
	int32 new_inventory_handle = 0;
	if (SteamInventory() != NULL) {
		SteamItemDef_t *generated = new SteamItemDef_t[output_quantity];
		for (uint32 i = 0; i < output_quantity; i++) {
			generated[i] = output_items[i];
		}
		if (SteamInventory()->ExchangeItems(&new_inventory_handle, generated, &output_quantity, 1, (const uint64 *)input_items, &input_quantity, 1)) {
			// Update the internally stored handle
			inventory_handle = new_inventory_handle;
		}
		delete[] generated;
	}
	return new_inventory_handle;
}

// Grants specific items to the current user, for developers only.
int32 SteamServer::generateItems(const PackedInt64Array items, const uint32 quantity) {
	int32 new_inventory_handle = 0;
	if (SteamInventory() != NULL) {
		SteamItemDef_t *generated = new SteamItemDef_t[quantity];
		for (uint32 i = 0; i < quantity; i++) {
			generated[i] = items[i];
		}
		if (SteamInventory()->GenerateItems(&new_inventory_handle, generated, &quantity, items.size())) {
			// Update the internally stored handle
			inventory_handle = new_inventory_handle;
		}
		delete[] generated;
	}
	return new_inventory_handle;
}

// Start retrieving all items in the current users inventory.
int32 SteamServer::getAllItems() {
	int32 new_inventory_handle = 0;
	if (SteamInventory() != NULL) {
		if (SteamInventory()->GetAllItems(&new_inventory_handle)) {
			// Update the internally stored handle
			inventory_handle = new_inventory_handle;
		}
	}
	return new_inventory_handle;
}

// Gets a string property from the specified item definition.  Gets a property value for a specific item definition.
String SteamServer::getItemDefinitionProperty(uint32 definition, const String &name) {
	if (SteamInventory() == NULL) {
		return "";
	}
	uint32 buffer_size = STEAM_BUFFER_SIZE;
	char *buffer = new char[buffer_size];
	SteamInventory()->GetItemDefinitionProperty(definition, name.utf8().get_data(), buffer, &buffer_size);
	String property = String::utf8(buffer, buffer_size);
	delete[] buffer;
	return property;
}

// Gets the state of a subset of the current user's inventory.
int32 SteamServer::getItemsByID(const uint64_t id_array, uint32 count) {
	int32 new_inventory_handle = 0;
	if (SteamInventory() != NULL) {
		if (SteamInventory()->GetItemsByID(&new_inventory_handle, (const uint64 *)id_array, count)) {
			// Update the internally stored handle
			inventory_handle = new_inventory_handle;
		}
	}
	return new_inventory_handle;
}

// After a successful call to RequestPrices, you can call this method to get the pricing for a specific item definition.
uint64_t SteamServer::getItemPrice(uint32 definition) {
	if (SteamInventory() == NULL) {
		return 0;
	}
	uint64 price = 0;
	uint64 basePrice = 0;
	SteamInventory()->GetItemPrice(definition, &price, &basePrice);
	return price;
}

// After a successful call to RequestPrices, you can call this method to get all the pricing for applicable item definitions. Use the result of GetNumItemsWithPrices as the the size of the arrays that you pass in.
Array SteamServer::getItemsWithPrices(uint32 length) {
	if (SteamInventory() == NULL) {
		return Array();
	}
	// Create the return array
	Array price_array;
	// Create a temporary array
	SteamItemDef_t *ids = new SteamItemDef_t[length];
	uint64 *prices = new uint64[length];
	uint64 *base_prices = new uint64[length];
	if (SteamInventory()->GetItemsWithPrices(ids, prices, base_prices, length)) {
		for (uint32 i = 0; i < length; i++) {
			Dictionary price_group;
			price_group["item"] = ids[i];
			price_group["price"] = (uint64_t)prices[i];
			price_group["base_prices"] = (uint64_t)base_prices[i];
			price_array.append(price_group);
		}
	}
	delete[] ids;
	delete[] prices;
	delete[] base_prices;
	return price_array;
}

// After a successful call to RequestPrices, this will return the number of item definitions with valid pricing.
uint32 SteamServer::getNumItemsWithPrices() {
	if (SteamInventory() == NULL) {
		return 0;
	}
	return SteamInventory()->GetNumItemsWithPrices();
}

// Gets the dynamic properties from an item in an inventory result set.
String SteamServer::getResultItemProperty(uint32 index, const String &name, int32 this_inventory_handle) {
	if (SteamInventory() != NULL) {
		// Set up variables to fill
		uint32 buffer_size = 256;
		char *value = new char[buffer_size];
		// If no inventory handle is passed, use internal one
		if (this_inventory_handle == 0) {
			this_inventory_handle = inventory_handle;
		}
		SteamInventory()->GetResultItemProperty((SteamInventoryResult_t)this_inventory_handle, index, name.utf8().get_data(), (char *)value, &buffer_size);
		String property = value;
		delete[] value;
		return property;
	}
	return "";
}

// Get the items associated with an inventory result handle.
Array SteamServer::getResultItems(int32 this_inventory_handle) {
	if (SteamInventory() == NULL) {
		return Array();
	}
	// Set up return array
	Array items;
	uint32 size = 0;
	if (SteamInventory()->GetResultItems((SteamInventoryResult_t)this_inventory_handle, NULL, &size)) {
		items.resize(size);
		SteamItemDetails_t *item_array = new SteamItemDetails_t[size];
		// If no inventory handle is passed, use internal one
		if (this_inventory_handle == 0) {
			this_inventory_handle = inventory_handle;
		}
		if (SteamInventory()->GetResultItems((SteamInventoryResult_t)this_inventory_handle, item_array, &size)) {
			for (uint32 i = 0; i < size; i++) {
				items.push_back((uint64_t)item_array[i].m_itemId);
			}
		}
		delete[] item_array;
	}
	return items;
}

// Find out the status of an asynchronous inventory result handle.
String SteamServer::getResultStatus(int32 this_inventory_handle) {
	if (SteamInventory() == NULL) {
		return "";
	}
	// If no inventory handle is passed, use internal one
	if (this_inventory_handle == 0) {
		this_inventory_handle = inventory_handle;
	}
	int result = SteamInventory()->GetResultStatus((SteamInventoryResult_t)this_inventory_handle);
	// Parse result
	if (result == k_EResultPending) {
		return "Still in progress.";
	} else if (result == k_EResultOK) {
		return "Finished successfully.";
	} else if (result == k_EResultExpired) {
		return "Finished but may be out-of-date.";
	} else if (result == k_EResultInvalidParam) {
		return "ERROR: invalid API call parameters.";
	} else if (result == k_EResultServiceUnavailable) {
		return "ERROR: server temporarily down; retry later.";
	} else if (result == k_EResultLimitExceeded) {
		return "ERROR: operation would exceed per-user inventory limits.";
	} else {
		return "ERROR: generic / unknown.";
	}
}

// Gets the server time at which the result was generated.
uint32 SteamServer::getResultTimestamp(int32 this_inventory_handle) {
	if (SteamInventory() == NULL) {
		return 0;
	}
	// If no inventory handle is passed, use internal one
	if (this_inventory_handle == 0) {
		this_inventory_handle = inventory_handle;
	}
	return SteamInventory()->GetResultTimestamp((SteamInventoryResult_t)this_inventory_handle);
}

// Grant all potential one-time promotional items to the current user.
int32 SteamServer::grantPromoItems() {
	int32 new_inventory_handle = 0;
	if (SteamInventory() != NULL) {
		if (SteamInventory()->GrantPromoItems(&new_inventory_handle)) {
			// Update the internally stored handle
			inventory_handle = new_inventory_handle;
		}
	}
	return new_inventory_handle;
}

// Triggers an asynchronous load and refresh of item definitions.
bool SteamServer::loadItemDefinitions() {
	if (SteamInventory() == NULL) {
		return false;
	}
	return SteamInventory()->LoadItemDefinitions();
}

// Request the list of "eligible" promo items that can be manually granted to the given user.
void SteamServer::requestEligiblePromoItemDefinitionsIDs(uint64_t steam_id) {
	if (SteamInventory() != NULL) {
		CSteamID user_id = (uint64)steam_id;
		SteamAPICall_t api_call = SteamInventory()->RequestEligiblePromoItemDefinitionsIDs(user_id);
		callResultEligiblePromoItemDefIDs.Set(api_call, this, &SteamServer::inventory_eligible_promo_item);
	}
}

// Request prices for all item definitions that can be purchased in the user's local currency. A SteamInventoryRequestPricesResult_t call result will be returned with the user's local currency code. After that, you can call GetNumItemsWithPrices and GetItemsWithPrices to get prices for all the known item definitions, or GetItemPrice for a specific item definition.
void SteamServer::requestPrices() {
	if (SteamInventory() != NULL) {
		SteamAPICall_t api_call = SteamInventory()->RequestPrices();
		callResultRequestPrices.Set(api_call, this, &SteamServer::inventory_request_prices_result);
	}
}

// Serialized result sets contain a short signature which can't be forged or replayed across different game sessions.
String SteamServer::serializeResult(int32 this_inventory_handle) {
	String result_serialized;
	if (SteamInventory() != NULL) {
		// If no inventory handle is passed, use internal one
		if (this_inventory_handle == 0) {
			this_inventory_handle = inventory_handle;
		}
		// Set up return array
		uint32 buffer_size = STEAM_BUFFER_SIZE;
		char *buffer = new char[buffer_size];
		if (SteamInventory()->SerializeResult((SteamInventoryResult_t)this_inventory_handle, buffer, &buffer_size)) {
			result_serialized = String::utf8(buffer, buffer_size);
		}
		delete[] buffer;
	}
	return result_serialized;
}

// Starts the purchase process for the user, given a "shopping cart" of item definitions that the user would like to buy. The user will be prompted in the Steam Overlay to complete the purchase in their local currency, funding their Steam Wallet if necessary, etc.
void SteamServer::startPurchase(const PackedInt64Array items, const uint32 quantity) {
	if (SteamInventory() != NULL) {
		SteamItemDef_t *purchases = new SteamItemDef_t[quantity];
		for (uint32 i = 0; i < quantity; i++) {
			purchases[i] = items[i];
		}
		SteamAPICall_t api_call = SteamInventory()->StartPurchase(purchases, &quantity, items.size());
		callResultStartPurchase.Set(api_call, this, &SteamServer::inventory_start_purchase_result);
		delete[] purchases;
	}
}

// Transfer items between stacks within a user's inventory.
int32 SteamServer::transferItemQuantity(uint64_t item_id, uint32 quantity, uint64_t item_destination, bool split) {
	int32 new_inventory_handle = 0;
	if (SteamInventory() != NULL) {
		if (split) {
			if (SteamInventory()->TransferItemQuantity(&new_inventory_handle, (SteamItemInstanceID_t)item_id, quantity, k_SteamItemInstanceIDInvalid)) {
				// Update the internally stored handle
				inventory_handle = new_inventory_handle;
			}
		} else {
			if (SteamInventory()->TransferItemQuantity(&new_inventory_handle, (SteamItemInstanceID_t)item_id, quantity, (SteamItemInstanceID_t)item_destination)) {
				// Update the internally stored handle
				inventory_handle = new_inventory_handle;
			}
		}
	}
	return new_inventory_handle;
}

// Trigger an item drop if the user has played a long enough period of time.
int32 SteamServer::triggerItemDrop(uint32 definition) {
	int32 new_inventory_handle = 0;
	if (SteamInventory() != NULL) {
		if (SteamInventory()->TriggerItemDrop(&new_inventory_handle, (SteamItemDef_t)definition)) {
			// Update the internally stored handle
			inventory_handle = new_inventory_handle;
		}
	}
	return new_inventory_handle;
}

// Starts a transaction request to update dynamic properties on items for the current user. This call is rate-limited by user, so property modifications should be batched as much as possible (e.g. at the end of a map or game session). After calling SetProperty or RemoveProperty for all the items that you want to modify, you will need to call SubmitUpdateProperties to send the request to the Steam servers. A SteamInventoryResultReady_t callback will be fired with the results of the operation.
void SteamServer::startUpdateProperties() {
	if (SteamInventory() != NULL) {
		inventory_update_handle = SteamInventory()->StartUpdateProperties();
	}
}

// Submits the transaction request to modify dynamic properties on items for the current user. See StartUpdateProperties.
int32 SteamServer::submitUpdateProperties(uint64_t this_inventory_update_handle) {
	int32 new_inventory_handle = 0;
	if (SteamInventory() != NULL) {
		// If no inventory update handle is passed, use internal one
		if (this_inventory_update_handle == 0) {
			this_inventory_update_handle = inventory_update_handle;
		}
		if (SteamInventory()->SubmitUpdateProperties((SteamInventoryUpdateHandle_t)this_inventory_update_handle, &new_inventory_handle)) {
			// Update the internally stored handle
			inventory_handle = new_inventory_handle;
		}
	}
	return new_inventory_handle;
}

// Removes a dynamic property for the given item.
bool SteamServer::removeProperty(uint64_t item_id, const String &name, uint64_t this_inventory_update_handle) {
	if (SteamInventory() == NULL) {
		return false;
	}
	// If no inventory update handle is passed, use internal one
	if (this_inventory_update_handle == 0) {
		this_inventory_update_handle = inventory_update_handle;
	}
	return SteamInventory()->RemoveProperty((SteamInventoryUpdateHandle_t)this_inventory_update_handle, (SteamItemInstanceID_t)item_id, name.utf8().get_data());
}

// Sets a dynamic property for the given item. Supported value types are strings.
bool SteamServer::setPropertyString(uint64_t item_id, const String &name, const String &value, uint64_t this_inventory_update_handle) {
	if (SteamInventory() == NULL) {
		return false;
	}
	// If no inventory update handle is passed, use internal one
	if (this_inventory_update_handle == 0) {
		this_inventory_update_handle = inventory_update_handle;
	}
	return SteamInventory()->SetProperty((SteamInventoryUpdateHandle_t)this_inventory_update_handle, (SteamItemInstanceID_t)item_id, name.utf8().get_data(), value.utf8().get_data());
}

// Sets a dynamic property for the given item. Supported value types are boolean.
bool SteamServer::setPropertyBool(uint64_t item_id, const String &name, bool value, uint64_t this_inventory_update_handle) {
	if (SteamInventory() == NULL) {
		return false;
	}
	// If no inventory update handle is passed, use internal one
	if (this_inventory_update_handle == 0) {
		this_inventory_update_handle = inventory_update_handle;
	}
	return SteamInventory()->SetProperty((SteamInventoryUpdateHandle_t)this_inventory_update_handle, (SteamItemInstanceID_t)item_id, name.utf8().get_data(), value);
}

// Sets a dynamic property for the given item. Supported value types are 64 bit integers.
bool SteamServer::setPropertyInt(uint64_t item_id, const String &name, uint64_t value, uint64_t this_inventory_update_handle) {
	if (SteamInventory() == NULL) {
		return false;
	}
	// If no inventory update handle is passed, use internal one
	if (this_inventory_update_handle == 0) {
		this_inventory_update_handle = inventory_update_handle;
	}
	return SteamInventory()->SetProperty((SteamInventoryUpdateHandle_t)this_inventory_update_handle, (SteamItemInstanceID_t)item_id, name.utf8().get_data(), (int64)value);
}

// Sets a dynamic property for the given item. Supported value types are 32 bit floats.
bool SteamServer::setPropertyFloat(uint64_t item_id, const String &name, float value, uint64_t this_inventory_update_handle) {
	if (SteamInventory() == NULL) {
		return false;
	}
	// If no inventory update handle is passed, use internal one
	if (this_inventory_update_handle == 0) {
		this_inventory_update_handle = inventory_update_handle;
	}
	return SteamInventory()->SetProperty((SteamInventoryUpdateHandle_t)this_inventory_update_handle, (SteamItemInstanceID_t)item_id, name.utf8().get_data(), value);
}

/////////////////////////////////////////////////
///// MATCHMAKING
/////////////////////////////////////////////////
//
// Gets an array of the details of the favorite game servers.
Array SteamServer::getFavoriteGames() {
	if (SteamMatchmaking() == NULL) {
		return Array();
	}
	int count = SteamMatchmaking()->GetFavoriteGameCount();
	Array favorites;
	for (int i = 0; i < count; i++) {
		Dictionary favorite;
		AppId_t app_id = 0;
		uint32 ip = 0;
		uint16 port = 0;
		uint16 query_port = 0;
		uint32 flags = 0;
		uint32 last_played = 0;
		favorite["ret"] = SteamMatchmaking()->GetFavoriteGame(i, &app_id, &ip, &port, &query_port, &flags, &last_played);
		if (favorite["ret"]) {
			favorite["app"] = app_id;
			// Convert the IP address back to a string
			const int NBYTES = 4;
			uint8 octet[NBYTES];
			char favoriteIP[16];
			for (int j = 0; j < NBYTES; j++) {
				octet[j] = ip >> (j * 8);
			}
			sprintf(favoriteIP, "%d.%d.%d.%d", octet[0], octet[1], octet[2], octet[3]);
			favorite["ip"] = favoriteIP;
			favorite["game_port"] = port;
			favorite["query_port"] = query_port;
			favorite["flags"] = flags;
			favorite["played"] = last_played;
			favorites.append(favorite);
		}
	}
	return favorites;
}

// Adds the game server to the local list; updates the time played of the server if it already exists in the list.
int SteamServer::addFavoriteGame(uint32 ip, uint16 port, uint16 query_port, uint32 flags, uint32 last_played) {
	if (SteamMatchmaking() == NULL) {
		return 0;
	}
	return SteamMatchmaking()->AddFavoriteGame((AppId_t)current_app_id, ip, port, query_port, flags, last_played);
}

// Removes the game server from the local storage; returns true if one was removed.
bool SteamServer::removeFavoriteGame(uint32 app_id, uint32 ip, uint16 port, uint16 query_port, uint32 flags) {
	if (SteamMatchmaking() == NULL) {
		return false;
	}
	return SteamMatchmaking()->RemoveFavoriteGame((AppId_t)app_id, ip, port, query_port, flags);
}

// Get a list of relevant lobbies.
void SteamServer::requestLobbyList() {
	if (SteamMatchmaking() != NULL) {
		SteamAPICall_t api_call = SteamMatchmaking()->RequestLobbyList();
		callResultLobbyList.Set(api_call, this, &SteamServer::lobby_match_list);
	}
}

// Adds a string comparison filter to the next RequestLobbyList call.
void SteamServer::addRequestLobbyListStringFilter(const String &key_to_match, const String &value_to_match, LobbyComparison comparison_type) {
	if (SteamMatchmaking() != NULL) {
		SteamMatchmaking()->AddRequestLobbyListStringFilter(key_to_match.utf8().get_data(), value_to_match.utf8().get_data(), (ELobbyComparison)comparison_type);
	}
}

// Adds a numerical comparison filter to the next RequestLobbyList call.
void SteamServer::addRequestLobbyListNumericalFilter(const String &key_to_match, int value_to_match, LobbyComparison comparison_type) {
	if (SteamMatchmaking() != NULL) {
		SteamMatchmaking()->AddRequestLobbyListNumericalFilter(key_to_match.utf8().get_data(), value_to_match, (ELobbyComparison)comparison_type);
	}
}

// Returns results closest to the specified value. Multiple near filters can be added, with early filters taking precedence.
void SteamServer::addRequestLobbyListNearValueFilter(const String &key_to_match, int value_to_be_close_to) {
	if (SteamMatchmaking() != NULL) {
		SteamMatchmaking()->AddRequestLobbyListNearValueFilter(key_to_match.utf8().get_data(), value_to_be_close_to);
	}
}

// Returns only lobbies with the specified number of slots available.
void SteamServer::addRequestLobbyListFilterSlotsAvailable(int slots_available) {
	if (SteamMatchmaking() != NULL) {
		SteamMatchmaking()->AddRequestLobbyListFilterSlotsAvailable(slots_available);
	}
}

// Sets the distance for which we should search for lobbies (based on users IP address to location map on the Steam backed).
void SteamServer::addRequestLobbyListDistanceFilter(LobbyDistanceFilter distance_filter) {
	if (SteamMatchmaking() != NULL) {
		SteamMatchmaking()->AddRequestLobbyListDistanceFilter((ELobbyDistanceFilter)distance_filter);
	}
}

// Sets how many results to return, the lower the count the faster it is to download the lobby results & details to the client.
void SteamServer::addRequestLobbyListResultCountFilter(int max_results) {
	if (SteamMatchmaking() != NULL) {
		SteamMatchmaking()->AddRequestLobbyListResultCountFilter(max_results);
	}
}

// Create a lobby on the Steam servers, if private the lobby will not be returned by any RequestLobbyList() call.
void SteamServer::createLobby(LobbyType lobby_type, int max_members) {
	if (SteamMatchmaking() != NULL) {
		SteamAPICall_t api_call = SteamMatchmaking()->CreateLobby((ELobbyType)lobby_type, max_members);
		callResultCreateLobby.Set(api_call, this, &SteamServer::lobby_created);
	}
}

// Join an existing lobby.
void SteamServer::joinLobby(uint64_t steam_lobby_id) {
	if (SteamMatchmaking() != NULL) {
		CSteamID lobby_id = (uint64)steam_lobby_id;
		SteamMatchmaking()->JoinLobby(lobby_id);
	}
}

// Leave a lobby, this will take effect immediately on the client side, other users will be notified by LobbyChatUpdate_t callback.
void SteamServer::leaveLobby(uint64_t steam_lobby_id) {
	if (SteamMatchmaking() != NULL) {
		CSteamID lobby_id = (uint64)steam_lobby_id;
		SteamMatchmaking()->LeaveLobby(lobby_id);
	}
}

// Invite another user to the lobby, the target user will receive a LobbyInvite_t callback, will return true if the invite is successfully sent, whether or not the target responds.
bool SteamServer::inviteUserToLobby(uint64_t steam_lobby_id, uint64_t steam_id_invitee) {
	if (SteamMatchmaking() == NULL) {
		return false;
	}
	CSteamID lobby_id = (uint64)steam_lobby_id;
	CSteamID invitee_id = (uint64)steam_id_invitee;
	return SteamMatchmaking()->InviteUserToLobby(lobby_id, invitee_id);
}

// Lobby iteration, for viewing details of users in a lobby.
int SteamServer::getNumLobbyMembers(uint64_t steam_lobby_id) {
	if (SteamMatchmaking() == NULL) {
		return 0;
	}
	CSteamID lobby_id = (uint64)steam_lobby_id;
	return SteamMatchmaking()->GetNumLobbyMembers(lobby_id);
}

// Returns the CSteamID of a user in the lobby.
uint64_t SteamServer::getLobbyMemberByIndex(uint64_t steam_lobby_id, int member) {
	if (SteamMatchmaking() == NULL) {
		return 0;
	}
	CSteamID lobby_id = (uint64)steam_lobby_id;
	CSteamID lobbyMember = SteamMatchmaking()->GetLobbyMemberByIndex(lobby_id, member);
	return lobbyMember.ConvertToUint64();
}

// Get data associated with this lobby.
String SteamServer::getLobbyData(uint64_t steam_lobby_id, const String &key) {
	if (SteamMatchmaking() == NULL) {
		return "";
	}
	CSteamID lobby_id = (uint64)steam_lobby_id;
	return String::utf8(SteamMatchmaking()->GetLobbyData(lobby_id, key.utf8().get_data()));
}

// Sets a key/value pair in the lobby metadata.
bool SteamServer::setLobbyData(uint64_t steam_lobby_id, const String &key, const String &value) {
	if (SteamMatchmaking() == NULL) {
		return false;
	}
	CSteamID lobby_id = (uint64)steam_lobby_id;
	return SteamMatchmaking()->SetLobbyData(lobby_id, key.utf8().get_data(), value.utf8().get_data());
}

// Get lobby data by the lobby's ID
Dictionary SteamServer::getAllLobbyData(uint64_t steam_lobby_id) {
	Dictionary data;
	if (SteamMatchmaking() == NULL) {
		return data;
	}
	CSteamID lobby_id = (uint64)steam_lobby_id;
	int data_count = SteamMatchmaking()->GetLobbyDataCount(lobby_id);
	char key[MAX_LOBBY_KEY_LENGTH];
	char value[CHAT_METADATA_MAX];
	for (int i = 0; i < data_count; i++) {
		bool success = SteamMatchmaking()->GetLobbyDataByIndex(lobby_id, i, key, MAX_LOBBY_KEY_LENGTH, value, CHAT_METADATA_MAX);
		if (success) {
			data["index"] = i;
			data["key"] = key;
			data["value"] = value;
		}
	}
	return data;
}

// Removes a metadata key from the lobby.
bool SteamServer::deleteLobbyData(uint64_t steam_lobby_id, const String &key) {
	if (SteamMatchmaking() == NULL) {
		return false;
	}
	CSteamID lobby_id = (uint64)steam_lobby_id;
	return SteamMatchmaking()->DeleteLobbyData(lobby_id, key.utf8().get_data());
}

// Gets per-user metadata for someone in this lobby.
String SteamServer::getLobbyMemberData(uint64_t steam_lobby_id, uint64_t steam_id_user, const String &key) {
	if (SteamMatchmaking() == NULL) {
		return "";
	}
	CSteamID lobby_id = (uint64)steam_lobby_id;
	CSteamID user_id = (uint64)steam_id_user;
	return SteamMatchmaking()->GetLobbyMemberData(lobby_id, user_id, key.utf8().get_data());
}

// Sets per-user metadata (for the local user implicitly).
void SteamServer::setLobbyMemberData(uint64_t steam_lobby_id, const String &key, const String &value) {
	if (SteamMatchmaking() != NULL) {
		CSteamID lobby_id = (uint64)steam_lobby_id;
		SteamMatchmaking()->SetLobbyMemberData(lobby_id, key.utf8().get_data(), value.utf8().get_data());
	}
}

// Broadcasts a chat message to the all the users in the lobby.
bool SteamServer::sendLobbyChatMsg(uint64_t steam_lobby_id, const String &message_body) {
	if (SteamMatchmaking() == NULL) {
		return false;
	}
	CSteamID lobby_id = (uint64)steam_lobby_id;
	return SteamMatchmaking()->SendLobbyChatMsg(lobby_id, message_body.utf8().get_data(), strlen(message_body.utf8().get_data()) + 1);
}

// Refreshes metadata for a lobby you're not necessarily in right now.
bool SteamServer::requestLobbyData(uint64_t steam_lobby_id) {
	if (SteamMatchmaking() == NULL) {
		return false;
	}
	CSteamID lobby_id = (uint64)steam_lobby_id;
	return SteamMatchmaking()->RequestLobbyData(lobby_id);
}

// Sets the game server associated with the lobby.
void SteamServer::setLobbyGameServer(uint64_t steam_lobby_id, const String &server_ip, uint16 server_port, uint64_t steam_id_game_server) {
	if (SteamMatchmaking() != NULL) {
		// Resolve address and convert it from IPAddress string to uint32_t
		IPAddress address;
		if (server_ip.is_valid_ip_address()) {
			address = server_ip;
		} else {
			address = IP::get_singleton()->resolve_hostname(server_ip, IP::TYPE_IPV4);
		}
		// Resolution succeeded
		if (address.is_valid()) {
			uint32_t ip4 = *((uint32_t *)address.get_ipv4());
			CSteamID lobby_id = (uint64)steam_lobby_id;
			// If setting a game server with no server (fake) Steam ID
			if (steam_id_game_server == 0) {
				SteamMatchmaking()->SetLobbyGameServer(lobby_id, ip4, server_port, k_steamIDNil);
			} else {
				CSteamID game_id = (uint64)steam_id_game_server;
				SteamMatchmaking()->SetLobbyGameServer(lobby_id, ip4, server_port, game_id);
			}
		}
	}
}

// Returns the details of a game server set in a lobby - returns false if there is no game server set, or that lobby doesn't exist.
Dictionary SteamServer::getLobbyGameServer(uint64_t steam_lobby_id) {
	Dictionary game_server;
	if (SteamMatchmaking() != NULL) {
		CSteamID lobby_id = (uint64)steam_lobby_id;
		uint32 server_ip = 0;
		uint16 server_port = 0;
		CSteamID server_id;
		game_server["ret"] = SteamMatchmaking()->GetLobbyGameServer(lobby_id, &server_ip, &server_port, &server_id);
		if (game_server["ret"]) {
			// Convert the IP address back to a string
			const int NBYTES = 4;
			uint8 octet[NBYTES];
			char ip[16];
			for (int i = 0; i < NBYTES; i++) {
				octet[i] = server_ip >> (i * 8);
			}
			sprintf(ip, "%d.%d.%d.%d", octet[0], octet[1], octet[2], octet[3]);
			game_server["ip"] = ip;
			game_server["port"] = server_port;
			// Convert the server ID
			uint64_t server = server_id.ConvertToUint64();
			game_server["id"] = server;
		}
	}
	return game_server;
}

// Set the limit on the # of users who can join the lobby.
bool SteamServer::setLobbyMemberLimit(uint64_t steam_lobby_id, int max_members) {
	if (SteamMatchmaking() == NULL) {
		return false;
	}
	CSteamID lobby_id = (uint64)steam_lobby_id;
	return SteamMatchmaking()->SetLobbyMemberLimit(lobby_id, max_members);
}

// Returns the current limit on the # of users who can join the lobby; returns 0 if no limit is defined.
int SteamServer::getLobbyMemberLimit(uint64_t steam_lobby_id) {
	if (SteamMatchmaking() == NULL) {
		return 0;
	}
	CSteamID lobby_id = (uint64)steam_lobby_id;
	return SteamMatchmaking()->GetLobbyMemberLimit(lobby_id);
}

// Updates which type of lobby it is.
bool SteamServer::setLobbyType(uint64_t steam_lobby_id, LobbyType lobby_type) {
	if (SteamMatchmaking() == NULL) {
		return false;
	}
	CSteamID lobby_id = (uint64)steam_lobby_id;
	return SteamMatchmaking()->SetLobbyType(lobby_id, (ELobbyType)lobby_type);
}

// Sets whether or not a lobby is joinable - defaults to true for a new lobby.
bool SteamServer::setLobbyJoinable(uint64_t steam_lobby_id, bool joinable) {
	if (SteamMatchmaking() == NULL) {
		return false;
	}
	CSteamID lobby_id = (uint64)steam_lobby_id;
	return SteamMatchmaking()->SetLobbyJoinable(lobby_id, joinable);
}

// Returns the current lobby owner.
uint64_t SteamServer::getLobbyOwner(uint64_t steam_lobby_id) {
	if (SteamMatchmaking() == NULL) {
		return 0;
	}
	CSteamID lobby_id = (uint64)steam_lobby_id;
	CSteamID owner_id = SteamMatchmaking()->GetLobbyOwner(lobby_id);
	return owner_id.ConvertToUint64();
}

// Changes who the lobby owner is.
bool SteamServer::setLobbyOwner(uint64_t steam_lobby_id, uint64_t steam_id_new_owner) {
	if (SteamMatchmaking() == NULL) {
		return false;
	}
	CSteamID lobby_id = (uint64)steam_lobby_id;
	CSteamID owner_id = (uint64)steam_id_new_owner;
	return SteamMatchmaking()->SetLobbyOwner(lobby_id, owner_id);
}

/////////////////////////////////////////////////
///// GAME SERVERS
/////////////////////////////////////////////////
//
//! Associate this game server with this clan for the purposes of computing player compatibility
void SteamServer::associateWithClan(uint64_t clan_id) {
	SteamGameServer()->AssociateWithClan(clan_id);
}

//! With a ticket generated, begin server authentication
int SteamServer::beginServerAuthSession(PackedByteArray auth_ticket, int ticket_size, uint64_t steam_id) {
	return (int)SteamGameServer()->BeginAuthSession(auth_ticket.ptr(), ticket_size, steam_id);
}

void SteamServer::cancelServerAuthTicket(uint32 auth_ticket) {
	SteamGameServer()->CancelAuthTicket(auth_ticket);
}

void SteamServer::clearAllKeyValues() {
	SteamGameServer()->ClearAllKeyValues();
}

void SteamServer::computeNewPlayerCompatibility(uint64_t steam_id) {
	SteamGameServer()->ComputeNewPlayerCompatibility(steam_id);
}

void SteamServer::endServerAuthSession(uint64_t steam_id) {
	SteamGameServer()->EndAuthSession(steam_id);
}

//! Requests the ticket for authentication, works by altering the variables in place, using pointers to them
uint32 SteamServer::getServerAuthSessionTicket(PackedByteArray auth_ticket, int max_ticket_size, uint32_t ticket_size, uint32 ip_addr, uint16 port) {
	// Ensure correct size of buffer
	auth_ticket.clear();
	auth_ticket.resize(max_ticket_size);
	networking_identity.Clear();
	networking_identity.SetIPv4Addr(ip_addr, port);
	//Request the ticket and return a handle to it
	return SteamGameServer()->GetAuthSessionTicket((void *)auth_ticket.ptr(), max_ticket_size, &ticket_size, &networking_identity);
}

void SteamServer::getNextOutgoingPacket() {
	// TODO: Im in a rush right now, i cant finish all functions
}

//! Asks Steam for this servers public IP. It gets returned as an int, but godot uses string for ips
uint32 SteamServer::getPublicIP() {
	return SteamGameServer()->GetPublicIP().m_unIPv4;
}

uint64_t SteamServer::getSteamServerID() {
	return (uint64_t)SteamGameServer_GetSteamID();
}

/*
PackedByteArray Steam::handleIncomingPacket(int packet, const String &ip, int port){
	// TODO: Im in a rush right now, i cant finish all functions
	return PackedByteArray();
}
*/

/*
PackedByteArray handleIncomingPacket(int packet, const String &ip, int port){
	// TODO: This function
}
*/

void SteamServer::logOff() {
	SteamGameServer()->LogOff();
}

//! Logs on using a token from https://steamcommunity.com/dev/managegameservers
void SteamServer::logOn(const String &token) {
	SteamGameServer()->LogOn(token.utf8().get_data());
}

//! Logs in without a token
void SteamServer::logOnAnonymous() {
	SteamGameServer()->LogOnAnonymous();
}

//! Returns if the server is marked as secured
bool SteamServer::secure() {
	return SteamGameServer()->BSecure();
}

//! Enables sending heartbeats to steam to appear in the server browser
void SteamServer::setAdvertiseServerActive(bool active) {
	SteamGameServer()->SetAdvertiseServerActive(active);
}

//! Most of these functions down here set variables that show up on the server browser
void SteamServer::setBotPlayerCount(int bots) {
	SteamGameServer()->SetBotPlayerCount(bots);
}

void SteamServer::setDedicatedServer(bool dedicated) {
	SteamGameServer()->SetDedicatedServer(dedicated);
}

void SteamServer::setGameData(const String &data) {
	SteamGameServer()->SetGameData(data.utf8().get_data());
}

void SteamServer::setGameDescription(const String &description) {
	SteamGameServer()->SetGameDescription(description.utf8().get_data());
}

void SteamServer::setGameTags(const String &tags) {
	SteamGameServer()->SetGameTags(tags.utf8().get_data());
}

void SteamServer::setKeyValue(const String &key, const String &value) {
	SteamGameServer()->SetKeyValue(key.utf8().get_data(), value.utf8().get_data());
}

void SteamServer::setMapName(const String &map) {
	SteamGameServer()->SetMapName(map.utf8().get_data());
}

void SteamServer::setMaxPlayerCount(int players_max) {
	SteamGameServer()->SetMaxPlayerCount(players_max);
}

void SteamServer::setModDir(const String &mod_dir) {
	SteamGameServer()->SetModDir(mod_dir.utf8().get_data());
}

void SteamServer::setPasswordProtected(bool password_protected) {
	SteamGameServer()->SetPasswordProtected(password_protected);
}

void SteamServer::setProduct(const String &product) {
	SteamGameServer()->SetProduct(product.utf8().get_data());
}

void SteamServer::setRegion(const String &region) {
	SteamGameServer()->SetRegion(region.utf8().get_data());
}

void SteamServer::setServerName(const String &name) {
	SteamGameServer()->SetServerName(name.utf8().get_data());
}

void SteamServer::setSpectatorPort(uint32 port) {
	SteamGameServer()->SetSpectatorPort((uint16)port);
}

void SteamServer::setSpectatorServerName(const String &name) {
	SteamGameServer()->SetSpectatorServerName(name.utf8().get_data());
}

//! After receiving a user's authentication data, and passing it to SendUserConnectAndAuthenticate, use this function to determine if the user owns downloadable content specified by the provided AppID.
int SteamServer::serverUserHasLicenseForApp(uint64_t steam_id, uint32 app_id) {
	return (int)SteamGameServer()->UserHasLicenseForApp(steam_id, app_id);
}

String SteamServer::get_init_error_message() {
	return String(init_error_message);
}

/////////////////////////////////////////////////
///// MATCHMAKING SERVERS
/////////////////////////////////////////////////
//
// Cancel an outstanding server list request.
void SteamServer::cancelQuery(uint64_t this_server_list_request) {
	if (SteamMatchmakingServers() != NULL) {
		// If no request list handle was passed, use the internal one
		if (this_server_list_request == 0) {
			this_server_list_request = (uint64)server_list_request;
		}
		SteamMatchmakingServers()->CancelQuery((HServerListRequest)this_server_list_request);
	}
}

// Cancel an outstanding individual server query.
void SteamServer::cancelServerQuery(int server_query) {
	if (SteamMatchmakingServers() != NULL) {
		SteamMatchmakingServers()->CancelServerQuery((HServerQuery)server_query);
	}
}

// Gets the number of servers in the given list.
int SteamServer::getServerCount(uint64_t this_server_list_request) {
	if (SteamMatchmakingServers() == NULL) {
		return 0;
	}
	// If no request list handle was passed, use the internal one
	if (this_server_list_request == 0) {
		this_server_list_request = (uint64)server_list_request;
	}
	return SteamMatchmakingServers()->GetServerCount((HServerListRequest)this_server_list_request);
}

// Get the details of a given server in the list.
Dictionary SteamServer::getServerDetails(int server, uint64_t this_server_list_request) {
	// Create a dictionary to populate
	Dictionary game_server;
	if (SteamMatchmakingServers() != NULL) {
		// If no request list handle was passed, use the internal one
		if (this_server_list_request == 0) {
			this_server_list_request = (uint64)server_list_request;
		}
		gameserveritem_t *server_item = new gameserveritem_t;
		SteamMatchmakingServers()->GetServerDetails((HServerListRequest)this_server_list_request, server);
		// Populate the dictionary
		game_server["ping"] = server_item->m_nPing;
		game_server["success_response"] = server_item->m_bHadSuccessfulResponse;
		game_server["no_refresh"] = server_item->m_bDoNotRefresh;
		game_server["game_dir"] = server_item->m_szGameDir;
		game_server["map"] = server_item->m_szMap;
		game_server["description"] = server_item->m_szGameDescription;
		game_server["app_id"] = server_item->m_nAppID;
		game_server["players"] = server_item->m_nPlayers;
		game_server["max_players"] = server_item->m_nMaxPlayers;
		game_server["bot_players"] = server_item->m_nBotPlayers;
		game_server["password"] = server_item->m_bPassword;
		game_server["secure"] = server_item->m_bSecure;
		game_server["last_played"] = server_item->m_ulTimeLastPlayed;
		game_server["server_version"] = server_item->m_nServerVersion;
		// Clean up
		delete server_item;
	}
	// Return the dictionary
	return game_server;
}

// Returns true if the list is currently refreshing its server list.
bool SteamServer::isRefreshing(uint64_t this_server_list_request) {
	if (SteamMatchmakingServers() == NULL) {
		return false;
	}
	// If no request list handle was passed, use the internal one
	if (this_server_list_request == 0) {
		this_server_list_request = (uint64)server_list_request;
	}
	return SteamMatchmakingServers()->IsRefreshing((HServerListRequest)this_server_list_request);
}

// Queries an individual game servers directly via IP/Port to request an updated ping time and other details from the server.
int SteamServer::pingServer(const String &ip, uint16 port) {
	if (SteamMatchmakingServers() == NULL) {
		return 0;
	}
	// Resolve address and convert it from IPAddress string to uint32_t
	IPAddress address;
	if (ip.is_valid_ip_address()) {
		address = ip;
	} else {
		address = IP::get_singleton()->resolve_hostname(ip, IP::TYPE_IPV4);
	}
	// Resolution failed
	if (!address.is_valid()) {
		return 0;
	}
	uint32_t ip4 = *((uint32_t *)address.get_ipv4());
	return SteamMatchmakingServers()->PingServer(ip4, port, ping_response);
}

// Request the list of players currently playing on a server.
int SteamServer::playerDetails(const String &ip, uint16 port) {
	if (SteamMatchmakingServers() == NULL) {
		return 0;
	}
	// Resolve address and convert it from IPAddress string to uint32_t
	IPAddress address;
	if (ip.is_valid_ip_address()) {
		address = ip;
	} else {
		address = IP::get_singleton()->resolve_hostname(ip, IP::TYPE_IPV4);
	}
	// Resolution failed
	if (!address.is_valid()) {
		return 0;
	}
	uint32_t ip4 = *((uint32_t *)address.get_ipv4());
	return SteamMatchmakingServers()->PlayerDetails(ip4, port, player_response);
}

// Ping every server in your list again but don't update the list of servers. Query callback installed when the server list was requested will be used again to post notifications and RefreshComplete, so the callback must remain valid until another RefreshComplete is called on it or the request is released with ReleaseRequest( hRequest ).
void SteamServer::refreshQuery(uint64_t this_server_list_request) {
	if (SteamMatchmakingServers() != NULL) {
		// If no request list handle was passed, use the internal one
		if (this_server_list_request == 0) {
			this_server_list_request = (uint64)server_list_request;
		}
		SteamMatchmakingServers()->RefreshQuery((HServerListRequest)this_server_list_request);
	}
}

// Refresh a single server inside of a query (rather than all the servers).
void SteamServer::refreshServer(int server, uint64_t this_server_list_request) {
	if (SteamMatchmakingServers() != NULL) {
		// If no request list handle was passed, use the internal one
		if (this_server_list_request == 0) {
			this_server_list_request = (uint64)server_list_request;
		}
		SteamMatchmakingServers()->RefreshServer((HServerListRequest)this_server_list_request, server);
	}
}

// Releases the asynchronous request object and cancels any pending query on it if there's a pending query in progress.
void SteamServer::releaseRequest(uint64_t this_server_list_request) {
	if (SteamMatchmakingServers() != NULL) {
		// If no request list handle was passed, use the internal one
		if (this_server_list_request == 0) {
			this_server_list_request = (uint64)server_list_request;
		}
		SteamMatchmakingServers()->ReleaseRequest((HServerListRequest)this_server_list_request);
	}
}

// Request a new list of servers of a particular type.  These calls each correspond to one of the EMatchMakingType values.
uint64_t SteamServer::requestFavoritesServerList(uint32 app_id, Array filters) {
	server_list_request = 0;
	if (SteamMatchmakingServers() != NULL) {
		uint32 filter_size = filters.size();
		MatchMakingKeyValuePair_t *filters_array = new MatchMakingKeyValuePair_t[filter_size];
		for (uint32 i = 0; i < filter_size; i++) {
			// Create a new filter pair to populate
			MatchMakingKeyValuePair_t *filter_array = new MatchMakingKeyValuePair_t();
			// Get the key/value pair
			Array pair = filters[i];
			// Get the key from the filter pair
			String key = (String)pair[0];
			char *this_key = new char[256];
			strcpy(this_key, key.utf8().get_data());
			filter_array->m_szKey[i] = *this_key;
			delete[] this_key;
			// Get the value from the filter pair
			String value = pair[1];
			char *this_value = new char[256];
			strcpy(this_value, value.utf8().get_data());
			filter_array->m_szValue[i] = *this_value;
			delete[] this_value;
			// Append this to the array
			filters_array[i] = *filter_array;
			// Free up the memory
			delete filter_array;
		}
		server_list_request = SteamMatchmakingServers()->RequestFavoritesServerList((AppId_t)app_id, &filters_array, filter_size, server_list_response);
		delete[] filters_array;
	}
	return (uint64)server_list_request;
}

// Request a new list of servers of a particular type.  These calls each correspond to one of the EMatchMakingType values.
uint64_t SteamServer::requestFriendsServerList(uint32 app_id, Array filters) {
	server_list_request = 0;
	if (SteamMatchmakingServers() != NULL) {
		uint32 filter_size = filters.size();
		MatchMakingKeyValuePair_t *filters_array = new MatchMakingKeyValuePair_t[filter_size];
		for (uint32 i = 0; i < filter_size; i++) {
			// Create a new filter pair to populate
			MatchMakingKeyValuePair_t *filter_array = new MatchMakingKeyValuePair_t();
			// Get the key/value pair
			Array pair = filters[i];
			// Get the key from the filter pair
			String key = (String)pair[0];
			char *this_key = new char[256];
			strcpy(this_key, key.utf8().get_data());
			filter_array->m_szKey[i] = *this_key;
			delete[] this_key;
			// Get the value from the filter pair
			String value = pair[1];
			char *this_value = new char[256];
			strcpy(this_value, value.utf8().get_data());
			filter_array->m_szValue[i] = *this_value;
			delete[] this_value;
			// Append this to the array
			filters_array[i] = *filter_array;
			// Free up the memory
			delete filter_array;
		}
		server_list_request = SteamMatchmakingServers()->RequestFriendsServerList((AppId_t)app_id, &filters_array, filter_size, server_list_response);
		delete[] filters_array;
	}
	return (uint64)server_list_request;
}

// Request a new list of servers of a particular type.  These calls each correspond to one of the EMatchMakingType values.
uint64_t SteamServer::requestHistoryServerList(uint32 app_id, Array filters) {
	server_list_request = 0;
	if (SteamMatchmakingServers() != NULL) {
		uint32 filter_size = filters.size();
		MatchMakingKeyValuePair_t *filters_array = new MatchMakingKeyValuePair_t[filter_size];
		for (uint32 i = 0; i < filter_size; i++) {
			// Create a new filter pair to populate
			MatchMakingKeyValuePair_t *filter_array = new MatchMakingKeyValuePair_t();
			// Get the key/value pair
			Array pair = filters[i];
			// Get the key from the filter pair
			String key = (String)pair[0];
			char *this_key = new char[256];
			strcpy(this_key, key.utf8().get_data());
			filter_array->m_szKey[i] = *this_key;
			delete[] this_key;
			// Get the value from the filter pair
			String value = pair[1];
			char *this_value = new char[256];
			strcpy(this_value, value.utf8().get_data());
			filter_array->m_szValue[i] = *this_value;
			delete[] this_value;
			// Append this to the array
			filters_array[i] = *filter_array;
			// Free up the memory
			delete filter_array;
		}
		server_list_request = SteamMatchmakingServers()->RequestHistoryServerList((AppId_t)app_id, &filters_array, filter_size, server_list_response);
		delete[] filters_array;
	}
	return (uint64)server_list_request;
}

// Request a new list of servers of a particular type.  These calls each correspond to one of the EMatchMakingType values.
uint64_t SteamServer::requestInternetServerList(uint32 app_id, Array filters) {
	server_list_request = 0;
	if (SteamMatchmakingServers() != NULL) {
		uint32 filter_size = filters.size();
		MatchMakingKeyValuePair_t *filters_array = new MatchMakingKeyValuePair_t[filter_size];
		for (uint32 i = 0; i < filter_size; i++) {
			// Create a new filter pair to populate
			MatchMakingKeyValuePair_t *filter_array = new MatchMakingKeyValuePair_t();
			// Get the key/value pair
			Array pair = filters[i];
			// Get the key from the filter pair
			String key = (String)pair[0];
			char *this_key = new char[256];
			strcpy(this_key, key.utf8().get_data());
			filter_array->m_szKey[i] = *this_key;
			delete[] this_key;
			// Get the value from the filter pair
			String value = pair[1];
			char *this_value = new char[256];
			strcpy(this_value, value.utf8().get_data());
			filter_array->m_szValue[i] = *this_value;
			delete[] this_value;
			// Append this to the array
			filters_array[i] = *filter_array;
			// Free up the memory
			delete filter_array;
		}
		server_list_request = SteamMatchmakingServers()->RequestInternetServerList((AppId_t)app_id, &filters_array, filter_size, server_list_response);
		delete[] filters_array;
	}
	return (uint64)server_list_request;
}

// Request a new list of servers of a particular type.  These calls each correspond to one of the EMatchMakingType values.
uint64_t SteamServer::requestLANServerList(uint32 app_id) {
	server_list_request = 0;
	if (SteamMatchmakingServers() != NULL) {
		server_list_request = SteamMatchmakingServers()->RequestLANServerList((AppId_t)app_id, server_list_response);
	}
	return (uint64)server_list_request;
}

// Request a new list of servers of a particular type.  These calls each correspond to one of the EMatchMakingType values.
uint64_t SteamServer::requestSpectatorServerList(uint32 app_id, Array filters) {
	server_list_request = 0;
	if (SteamMatchmakingServers() != NULL) {
		uint32 filter_size = filters.size();
		MatchMakingKeyValuePair_t *filters_array = new MatchMakingKeyValuePair_t[filter_size];
		for (uint32 i = 0; i < filter_size; i++) {
			// Create a new filter pair to populate
			MatchMakingKeyValuePair_t *filter_array = new MatchMakingKeyValuePair_t();
			// Get the key/value pair
			Array pair = filters[i];
			// Get the key from the filter pair
			String key = (String)pair[0];
			char *this_key = new char[256];
			strcpy(this_key, key.utf8().get_data());
			filter_array->m_szKey[i] = *this_key;
			delete[] this_key;
			// Get the value from the filter pair
			String value = pair[1];
			char *this_value = new char[256];
			strcpy(this_value, value.utf8().get_data());
			filter_array->m_szValue[i] = *this_value;
			delete[] this_value;
			// Append this to the array
			filters_array[i] = *filter_array;
			// Free up the memory
			delete filter_array;
		}
		server_list_request = SteamMatchmakingServers()->RequestSpectatorServerList((AppId_t)app_id, &filters_array, filter_size, server_list_response);
		delete[] filters_array;
	}
	return (uint64)server_list_request;
}

// Request the list of rules that the server is running (See ISteamGameServer::SetKeyValue() to set the rules server side)
int SteamServer::serverRules(const String &ip, uint16 port) {
	if (SteamMatchmakingServers() == NULL) {
		return 0;
	}
	// Resolve address and convert it from IPAddress string to uint32_t
	IPAddress address;
	if (ip.is_valid_ip_address()) {
		address = ip;
	} else {
		address = IP::get_singleton()->resolve_hostname(ip, IP::TYPE_IPV4);
	}
	// Resolution failed
	if (!address.is_valid()) {
		return 0;
	}
	uint32_t ip4 = *((uint32_t *)address.get_ipv4());
	return SteamMatchmakingServers()->ServerRules(ip4, port, rules_response);
}

/////////////////////////////////////////////////
///// NETWORKING
/////////////////////////////////////////////////
//
// This allows the game to specify accept an incoming packet.
bool SteamServer::acceptP2PSessionWithUser(uint64_t steam_id_remote) {
	if (SteamNetworking() == NULL) {
		return false;
	}
	CSteamID steam_id = createSteamID(steam_id_remote);
	return SteamNetworking()->AcceptP2PSessionWithUser(steam_id);
}

// Allow or disallow P2P connections to fall back to being relayed through the Steam servers if a direct connection or NAT-traversal cannot be established.
bool SteamServer::allowP2PPacketRelay(bool allow) {
	if (SteamNetworking() == NULL) {
		return false;
	}
	return SteamNetworking()->AllowP2PPacketRelay(allow);
}

// Closes a P2P channel when you're done talking to a user on the specific channel.
bool SteamServer::closeP2PChannelWithUser(uint64_t steam_id_remote, int channel) {
	if (SteamNetworking() == NULL) {
		return false;
	}
	CSteamID steam_id = createSteamID(steam_id_remote);
	return SteamNetworking()->CloseP2PChannelWithUser(steam_id, channel);
}

// This should be called when you're done communicating with a user, as this will free up all of the resources allocated for the connection under-the-hood.
bool SteamServer::closeP2PSessionWithUser(uint64_t steam_id_remote) {
	if (SteamNetworking() == NULL) {
		return false;
	}
	CSteamID steam_id = createSteamID(steam_id_remote);
	return SteamNetworking()->CloseP2PSessionWithUser(steam_id);
}

// Fills out a P2PSessionState_t structure with details about the connection like whether or not there is an active connection.
Dictionary SteamServer::getP2PSessionState(uint64_t steam_id_remote) {
	Dictionary result;
	if (SteamNetworking() == NULL) {
		return result;
	}
	CSteamID steam_id = createSteamID(steam_id_remote);
	P2PSessionState_t p2pSessionState;
	bool success = SteamNetworking()->GetP2PSessionState(steam_id, &p2pSessionState);
	if (!success) {
		return result;
	}
	result["connection_active"] = p2pSessionState.m_bConnectionActive; // true if we've got an active open connection
	result["connecting"] = p2pSessionState.m_bConnecting; // true if we're currently trying to establish a connection
	result["session_error"] = p2pSessionState.m_eP2PSessionError; // last error recorded (see enum in isteamnetworking.h)
	result["using_relay"] = p2pSessionState.m_bUsingRelay; // true if it's going through a relay server (TURN)
	result["bytes_queued_for_send"] = p2pSessionState.m_nBytesQueuedForSend;
	result["packets_queued_for_send"] = p2pSessionState.m_nPacketsQueuedForSend;
	result["remote_ip"] = p2pSessionState.m_nRemoteIP; // potential IP:Port of remote host. Could be TURN server.
	result["remote_port"] = p2pSessionState.m_nRemotePort; // Only exists for compatibility with older authentication api's
	return result;
}

// Calls IsP2PPacketAvailable() under the hood, returns the size of the available packet or zero if there is no such packet.
uint32_t SteamServer::getAvailableP2PPacketSize(int channel) {
	if (SteamNetworking() == NULL) {
		return 0;
	}
	uint32_t messageSize = 0;
	return (SteamNetworking()->IsP2PPacketAvailable(&messageSize, channel)) ? messageSize : 0;
}

// Reads in a packet that has been sent from another user via SendP2PPacket.
Dictionary SteamServer::readP2PPacket(uint32_t packet, int channel) {
	Dictionary result;
	if (SteamNetworking() == NULL) {
		return result;
	}
	PackedByteArray data;
	data.resize(packet);
	CSteamID steam_id;
	uint32_t bytesRead = 0;
	if (SteamNetworking()->ReadP2PPacket(data.ptrw(), packet, &bytesRead, &steam_id, channel)) {
		data.resize(bytesRead);
		uint64_t steam_id_remote = steam_id.ConvertToUint64();
		result["data"] = data;
		result["steam_id_remote"] = steam_id_remote;
	} else {
		data.resize(0);
	}
	return result;
}

// Sends a P2P packet to the specified user.
bool SteamServer::sendP2PPacket(uint64_t steam_id_remote, PackedByteArray data, P2PSend send_type, int channel) {
	if (SteamNetworking() == NULL) {
		return false;
	}
	CSteamID steam_id = createSteamID(steam_id_remote);
	return SteamNetworking()->SendP2PPacket(steam_id, data.ptr(), data.size(), EP2PSend(send_type), channel);
}

/////////////////////////////////////////////////
///// NETWORKING MESSAGES
/////////////////////////////////////////////////
//
// AcceptSessionWithUser() should only be called in response to a SteamP2PSessionRequest_t callback SteamP2PSessionRequest_t will be posted if another user tries to send you a message, and you haven't tried to talk to them.
bool SteamServer::acceptSessionWithUser(const String &identity_reference) {
	if (SteamNetworkingMessages() == NULL) {
		return false;
	}
	return SteamNetworkingMessages()->AcceptSessionWithUser(networking_identities[identity_reference.utf8().get_data()]);
}

// Call this  when you're done talking to a user on a specific channel. Once all open channels to a user have been closed, the open session to the user will be closed, and any new data from this user will trigger a SteamP2PSessionRequest_t callback.
bool SteamServer::closeChannelWithUser(const String &identity_reference, int channel) {
	if (SteamNetworkingMessages() == NULL) {
		return false;
	}
	return SteamNetworkingMessages()->CloseChannelWithUser(networking_identities[identity_reference.utf8().get_data()], channel);
}

// Call this when you're done talking to a user to immediately free up resources under-the-hood.
bool SteamServer::closeSessionWithUser(const String &identity_reference) {
	if (SteamNetworkingMessages() == NULL) {
		return false;
	}
	return SteamNetworkingMessages()->CloseSessionWithUser(networking_identities[identity_reference.utf8().get_data()]);
}

// Returns information about the latest state of a connection, if any, with the given peer.
Dictionary SteamServer::getSessionConnectionInfo(const String &identity_reference, bool get_connection, bool get_status) {
	Dictionary connection_info;
	if (SteamNetworkingMessages() != NULL) {
		SteamNetConnectionInfo_t this_info;
		SteamNetConnectionRealTimeStatus_t this_status;
		int connection_state = SteamNetworkingMessages()->GetSessionConnectionInfo(networking_identities[identity_reference.utf8().get_data()], &this_info, &this_status);
		// Parse the data to a dictionary
		connection_info["connection_state"] = connection_state;
		// If getting the connection information
		if (get_connection) {
			char identity[STEAM_BUFFER_SIZE];
			this_info.m_identityRemote.ToString(identity, STEAM_BUFFER_SIZE);
			connection_info["identity"] = identity;
			connection_info["user_data"] = (uint64_t)this_info.m_nUserData;
			connection_info["listen_socket"] = this_info.m_hListenSocket;
			char ip_address[STEAM_BUFFER_SIZE];
			this_info.m_addrRemote.ToString(ip_address, STEAM_BUFFER_SIZE, true);
			connection_info["remote_address"] = ip_address;
			connection_info["remote_pop"] = this_info.m_idPOPRemote;
			connection_info["pop_relay"] = this_info.m_idPOPRelay;
			connection_info["connection_state"] = this_info.m_eState;
			connection_info["end_reason"] = this_info.m_eEndReason;
			connection_info["end_debug"] = this_info.m_szEndDebug;
			connection_info["debug_description"] = this_info.m_szConnectionDescription;
		}
		// If getting the quick status
		if (get_status) {
			connection_info["state"] = this_status.m_eState;
			connection_info["ping"] = this_status.m_nPing;
			connection_info["local_quality"] = this_status.m_flConnectionQualityLocal;
			connection_info["remote_quality"] = this_status.m_flConnectionQualityRemote;
			connection_info["packets_out_per_second"] = this_status.m_flOutPacketsPerSec;
			connection_info["bytes_out_per_second"] = this_status.m_flOutBytesPerSec;
			connection_info["packets_in_per_second"] = this_status.m_flInPacketsPerSec;
			connection_info["bytes_in_per_second"] = this_status.m_flInBytesPerSec;
			connection_info["send_rate"] = this_status.m_nSendRateBytesPerSecond;
			connection_info["pending_unreliable"] = this_status.m_cbPendingUnreliable;
			connection_info["pending_reliable"] = this_status.m_cbPendingReliable;
			connection_info["sent_unacknowledged_reliable"] = this_status.m_cbSentUnackedReliable;
			connection_info["queue_time"] = (uint64_t)this_status.m_usecQueueTime;
		}
	}
	return connection_info;
}

// Reads the next message that has been sent from another user via SendMessageToUser() on the given channel. Returns number of messages returned into your list.  (0 if no message are available on that channel.)
Array SteamServer::receiveMessagesOnChannel(int channel, int max_messages) {
	Array messages;
	if (SteamNetworkingMessages() != NULL) {
		// Allocate the space for the messages
		SteamNetworkingMessage_t **channel_messages = new SteamNetworkingMessage_t *[max_messages];
		// Get the messages
		int available_messages = SteamNetworkingMessages()->ReceiveMessagesOnChannel(channel, channel_messages, max_messages);
		// Loop through and create the messages as dictionaries then add to the messages array
		for (int i = 0; i < available_messages; i++) {
			// Set up the mesage dictionary
			Dictionary message;
			// Get the data / message
			int message_size = channel_messages[i]->m_cbSize;
			PackedByteArray data;
			data.resize(message_size);
			uint8_t *source_data = (uint8_t *)channel_messages[i]->m_pData;
			uint8_t *output_data = data.ptrw();
			for (int j = 0; j < message_size; j++) {
				output_data[j] = source_data[j];
			}
			message["payload"] = data;
			message["size"] = message_size;
			message["connection"] = channel_messages[i]->m_conn;
			char identity[STEAM_BUFFER_SIZE];
			channel_messages[i]->m_identityPeer.ToString(identity, STEAM_BUFFER_SIZE);
			message["identity"] = identity;
			message["user_data"] = (uint64_t)channel_messages[i]->m_nConnUserData;
			message["time_received"] = (uint64_t)channel_messages[i]->m_usecTimeReceived;
			message["message_number"] = (uint64_t)channel_messages[i]->m_nMessageNumber;
			message["channel"] = channel_messages[i]->m_nChannel;
			message["flags"] = channel_messages[i]->m_nFlags;
			message["user_data"] = (uint64_t)channel_messages[i]->m_nUserData;
			messages.append(message);
			// Release the message
			channel_messages[i]->Release();
		}
		delete[] channel_messages;
	}
	return messages;
}

// Sends a message to the specified host. If we don't already have a session with that user, a session is implicitly created. There might be some handshaking that needs to happen before we can actually begin sending message data.
int SteamServer::sendMessageToUser(const String &identity_reference, const PackedByteArray data, int flags, int channel) {
	if (SteamNetworkingMessages() == NULL) {
		return 0;
	}
	return SteamNetworkingMessages()->SendMessageToUser(networking_identities[identity_reference.utf8().get_data()], data.ptr(), data.size(), flags, channel);
}

/////////////////////////////////////////////////
///// NETWORKING SOCKETS
/////////////////////////////////////////////////
//
// Creates a "server" socket that listens for clients to connect to by calling ConnectByIPAddress, over ordinary UDP (IPv4 or IPv6)
uint32 SteamServer::createListenSocketIP(const String &ip_reference, Array options) {
	if (SteamNetworkingSockets() == NULL) {
		return 0;
	}
	const SteamNetworkingConfigValue_t *these_options = convertOptionsArray(options);
	uint32 listen_socket = SteamNetworkingSockets()->CreateListenSocketIP(ip_addresses[ip_reference.utf8().get_data()], options.size(), these_options);
	delete[] these_options;
	return listen_socket;
}

// Like CreateListenSocketIP, but clients will connect using ConnectP2P. The connection will be relayed through the Valve network.
uint32 SteamServer::createListenSocketP2P(int virtual_port, Array options) {
	if (SteamNetworkingSockets() == NULL) {
		return 0;
	}
	const SteamNetworkingConfigValue_t *these_options = convertOptionsArray(options);
	uint32 listen_socket = SteamNetworkingSockets()->CreateListenSocketP2P(virtual_port, options.size(), these_options);
	delete[] these_options;
	return listen_socket;
}

// Begin connecting to a server that is identified using a platform-specific identifier. This uses the default rendezvous service, which depends on the platform and library configuration. (E.g. on Steam, it goes through the steam backend.) The traffic is relayed over the Steam Datagram Relay network.
uint32 SteamServer::connectP2P(const String &identity_reference, int virtual_port, Array options) {
	if (SteamNetworkingSockets() == NULL) {
		return 0;
	}
	const SteamNetworkingConfigValue_t *these_options = convertOptionsArray(options);
	uint32 listen_socket = SteamNetworkingSockets()->ConnectP2P(networking_identities[identity_reference.utf8().get_data()], virtual_port, options.size(), these_options);
	delete[] these_options;
	return listen_socket;
}

// Begin connecting to a server listen socket that is identified using an [ip-address]:[port], i.e. 127.0.0.1:27015. Used with createListenSocketIP
uint32 SteamServer::connectByIPAddress(const String &ip_address_with_port, Array options) {
	if (SteamNetworkingSockets() == NULL) {
		return 0;
	}

	SteamNetworkingIPAddr steamAddr;
	steamAddr.Clear();
	steamAddr.ParseString(ip_address_with_port.utf8().get_data());

	return SteamNetworkingSockets()->ConnectByIPAddress(steamAddr, options.size(), convertOptionsArray(options));
}

// Client call to connect to a server hosted in a Valve data center, on the specified virtual port. You must have placed a ticket for this server into the cache, or else this connect attempt will fail!
uint32 SteamServer::connectToHostedDedicatedServer(const String &identity_reference, int virtual_port, Array options) {
	if (SteamNetworkingSockets() == NULL) {
		return 0;
	}
	const SteamNetworkingConfigValue_t *these_options = convertOptionsArray(options);
	uint32 listen_socket = SteamNetworkingSockets()->ConnectToHostedDedicatedServer(networking_identities[identity_reference.utf8().get_data()], virtual_port, options.size(), these_options);
	delete[] these_options;
	return listen_socket;
}

// Accept an incoming connection that has been received on a listen socket.
int SteamServer::acceptConnection(uint32 connection_handle) {
	if (SteamNetworkingSockets() == NULL) {
		return 0;
	}
	return SteamNetworkingSockets()->AcceptConnection((HSteamNetConnection)connection_handle);
}

// Disconnects from the remote host and invalidates the connection handle. Any unread data on the connection is discarded.
bool SteamServer::closeConnection(uint32 peer, int reason, const String &debug_message, bool linger) {
	if (SteamNetworkingSockets() == NULL) {
		return false;
	}
	return SteamNetworkingSockets()->CloseConnection((HSteamNetConnection)peer, reason, debug_message.utf8().get_data(), linger);
}

// Destroy a listen socket. All the connections that were accepted on the listen socket are closed ungracefully.
bool SteamServer::closeListenSocket(uint32 socket) {
	if (SteamNetworkingSockets() == NULL) {
		return false;
	}
	return SteamNetworkingSockets()->CloseListenSocket((HSteamListenSocket)socket);
}

// Create a pair of connections that are talking to each other, e.g. a loopback connection. This is very useful for testing, or so that your client/server code can work the same even when you are running a local "server".
Dictionary SteamServer::createSocketPair(bool loopback, const String &identity_reference1, const String &identity_reference2) {
	// Create a dictionary to populate
	Dictionary connection_pair;
	if (SteamNetworkingSockets() != NULL) {
		// Turn the strings back to structs - Should be a check for failure to parse from string
		const SteamNetworkingIdentity identity_struct1 = networking_identities[identity_reference1.utf8().get_data()];
		const SteamNetworkingIdentity identity_struct2 = networking_identities[identity_reference2.utf8().get_data()];
		// Get connections
		uint32 connection1 = 0;
		uint32 connection2 = 0;
		bool success = SteamNetworkingSockets()->CreateSocketPair(&connection1, &connection2, loopback, &identity_struct1, &identity_struct2);
		// Populate the dictionary
		connection_pair["success"] = success;
		connection_pair["connection1"] = connection1;
		connection_pair["connection2"] = connection2;
	}
	return connection_pair;
}

// Send a message to the remote host on the specified connection.
Dictionary SteamServer::sendMessageToConnection(uint32 connection_handle, const PackedByteArray data, int flags) {
	Dictionary message_response;
	if (SteamNetworkingSockets() != NULL) {
		int64 number;
		int result = SteamNetworkingSockets()->SendMessageToConnection((HSteamNetConnection)connection_handle, data.ptr(), data.size(), flags, &number);
		// Populate the dictionary
		message_response["result"] = result;
		message_response["message_number"] = (uint64_t)number;
	}
	return message_response;
}

// Send one or more messages without copying the message payload. This is the most efficient way to send messages. To use this function, you must first allocate a message object using ISteamNetworkingUtils::AllocateMessage. (Do not declare one on the stack or allocate your own.)
void SteamServer::sendMessages(int messages, const PackedByteArray data, uint32 connection_handle, int flags) {
	if (SteamNetworkingSockets() != NULL) {
		SteamNetworkingMessage_t *networkMessage;
		networkMessage = SteamNetworkingUtils()->AllocateMessage(0);
		networkMessage->m_pData = (void *)data.ptr();
		networkMessage->m_cbSize = data.size();
		networkMessage->m_conn = (HSteamNetConnection)connection_handle;
		networkMessage->m_nFlags = flags;
		int64 result;
		SteamNetworkingSockets()->SendMessages(messages, &networkMessage, &result);
		// Release the message
		networkMessage->Release();
	}
}

// Flush any messages waiting on the Nagle timer and send them at the next transmission opportunity (often that means right now).
int SteamServer::flushMessagesOnConnection(uint32 connection_handle) {
	if (SteamNetworkingSockets() == NULL) {
		return 0;
	}
	return SteamNetworkingSockets()->FlushMessagesOnConnection((HSteamNetConnection)connection_handle);
}

// Fetch the next available message(s) from the connection, if any. Returns the number of messages returned into your array, up to nMaxMessages. If the connection handle is invalid, -1 is returned. If no data is available, 0, is returned.
Array SteamServer::receiveMessagesOnConnection(uint32 connection_handle, int max_messages) {
	Array messages;
	if (SteamNetworkingSockets() != NULL) {
		// Allocate the space for the messages
		SteamNetworkingMessage_t **connection_messages = new SteamNetworkingMessage_t *[max_messages];
		// Get the messages
		int available_messages = SteamNetworkingSockets()->ReceiveMessagesOnConnection((HSteamNetConnection)connection_handle, connection_messages, max_messages);
		// Loop through and create the messages as dictionaries then add to the messages array
		for (int i = 0; i < available_messages; i++) {
			// Create the message dictionary to send back
			Dictionary message;
			// Get the message data
			int message_size = connection_messages[i]->m_cbSize;
			PackedByteArray data;
			data.resize(message_size);
			uint8_t *source_data = (uint8_t *)connection_messages[i]->m_pData;
			uint8_t *output_data = data.ptrw();
			for (int j = 0; j < message_size; j++) {
				output_data[j] = source_data[j];
			}
			message["payload"] = data;
			message["size"] = message_size;
			message["connection"] = connection_messages[i]->m_conn;
			char identity[STEAM_BUFFER_SIZE];
			connection_messages[i]->m_identityPeer.ToString(identity, STEAM_BUFFER_SIZE);
			message["identity"] = identity;
			message["user_data"] = (uint64_t)connection_messages[i]->m_nConnUserData;
			message["time_received"] = (uint64_t)connection_messages[i]->m_usecTimeReceived;
			message["message_number"] = (uint64_t)connection_messages[i]->m_nMessageNumber;
			message["channel"] = connection_messages[i]->m_nChannel;
			message["flags"] = connection_messages[i]->m_nFlags;
			message["user_data"] = (uint64_t)connection_messages[i]->m_nUserData;
			messages.append(message);
			// Release the message
			connection_messages[i]->Release();
		}
		delete[] connection_messages;
	}
	return messages;
}

// Create a new poll group.
uint32 SteamServer::createPollGroup() {
	if (SteamNetworkingSockets() == NULL) {
		return 0;
	}
	return SteamNetworkingSockets()->CreatePollGroup();
}

// Destroy a poll group created with CreatePollGroup.
bool SteamServer::destroyPollGroup(uint32 poll_group) {
	if (SteamNetworkingSockets() == NULL) {
		return false;
	}
	return SteamNetworkingSockets()->DestroyPollGroup((HSteamNetPollGroup)poll_group);
}

// Assign a connection to a poll group. Note that a connection may only belong to a single poll group. Adding a connection to a poll group implicitly removes it from any other poll group it is in.
bool SteamServer::setConnectionPollGroup(uint32 connection_handle, uint32 poll_group) {
	if (SteamNetworkingSockets() == NULL) {
		return false;
	}
	return SteamNetworkingSockets()->SetConnectionPollGroup((HSteamNetConnection)connection_handle, (HSteamNetPollGroup)poll_group);
}

// Same as ReceiveMessagesOnConnection, but will return the next messages available on any connection in the poll group. Examine SteamNetworkingMessage_t::m_conn to know which connection. (SteamNetworkingMessage_t::m_nConnUserData might also be useful.)
Array SteamServer::receiveMessagesOnPollGroup(uint32 poll_group, int max_messages) {
	Array messages;
	if (SteamNetworkingSockets() != NULL) {
		// Allocate the space for the messages
		SteamNetworkingMessage_t **poll_messages = new SteamNetworkingMessage_t *[max_messages];
		// Get the messages
		int available_messages = SteamNetworkingSockets()->ReceiveMessagesOnPollGroup((HSteamNetPollGroup)poll_group, poll_messages, max_messages);
		// Loop through and create the messages as dictionaries then add to the messages array
		for (int i = 0; i < available_messages; i++) {
			// Create the message dictionary to send back
			Dictionary message;
			// Get the message data
			int message_size = poll_messages[i]->m_cbSize;
			PackedByteArray data;
			data.resize(message_size);
			uint8_t *source_data = (uint8_t *)poll_messages[i]->m_pData;
			uint8_t *output_data = data.ptrw();
			for (int j = 0; j < message_size; j++) {
				output_data[j] = source_data[j];
			}
			message["payload"] = data;
			message["size"] = message_size;
			message["connection"] = poll_messages[i]->m_conn;
			char identity[STEAM_BUFFER_SIZE];
			poll_messages[i]->m_identityPeer.ToString(identity, STEAM_BUFFER_SIZE);
			message["identity"] = identity;
			message["user_data"] = (uint64_t)poll_messages[i]->m_nConnUserData;
			message["time_received"] = (uint64_t)poll_messages[i]->m_usecTimeReceived;
			message["message_number"] = (uint64_t)poll_messages[i]->m_nMessageNumber;
			message["channel"] = poll_messages[i]->m_nChannel;
			message["flags"] = poll_messages[i]->m_nFlags;
			message["user_data"] = (uint64_t)poll_messages[i]->m_nUserData;
			messages.append(message);
			// Release the message
			poll_messages[i]->Release();
		}
		delete[] poll_messages;
	}
	return messages;
}

// Returns basic information about the high-level state of the connection. Returns false if the connection handle is invalid.
Dictionary SteamServer::getConnectionInfo(uint32 connection_handle) {
	Dictionary connection_info;
	if (SteamNetworkingSockets() != NULL) {
		SteamNetConnectionInfo_t info;
		if (SteamNetworkingSockets()->GetConnectionInfo((HSteamNetConnection)connection_handle, &info)) {
			char identity[STEAM_BUFFER_SIZE];
			info.m_identityRemote.ToString(identity, STEAM_BUFFER_SIZE);
			connection_info["identity"] = identity;
			connection_info["user_data"] = (uint64_t)info.m_nUserData;
			connection_info["listen_socket"] = info.m_hListenSocket;
			char ip_address[STEAM_BUFFER_SIZE];
			info.m_addrRemote.ToString(ip_address, STEAM_BUFFER_SIZE, true);
			connection_info["remote_address"] = ip_address;
			connection_info["remote_pop"] = info.m_idPOPRemote;
			connection_info["pop_relay"] = info.m_idPOPRelay;
			connection_info["connection_state"] = info.m_eState;
			connection_info["end_reason"] = info.m_eEndReason;
			connection_info["end_debug"] = info.m_szEndDebug;
			connection_info["debug_description"] = info.m_szConnectionDescription;
		}
	}
	return connection_info;
}

// Returns very detailed connection stats in diagnostic text format. Useful for dumping to a log, etc. The format of this information is subject to change.
Dictionary SteamServer::getDetailedConnectionStatus(uint32 connection) {
	Dictionary connectionStatus;
	if (SteamNetworkingSockets() != NULL) {
		char buffer[STEAM_LARGE_BUFFER_SIZE];
		int success = SteamNetworkingSockets()->GetDetailedConnectionStatus((HSteamNetConnection)connection, buffer, STEAM_LARGE_BUFFER_SIZE);
		// Add data to dictionary
		connectionStatus["success"] = success;
		connectionStatus["status"] = buffer;
	}
	// Send the data back to the user
	return connectionStatus;
}

// Fetch connection user data. Returns -1 if handle is invalid or if you haven't set any userdata on the connection.
uint64_t SteamServer::getConnectionUserData(uint32 peer) {
	if (SteamNetworkingSockets() == NULL) {
		return 0;
	}
	return SteamNetworkingSockets()->GetConnectionUserData((HSteamNetConnection)peer);
}

// Set a name for the connection, used mostly for debugging
void SteamServer::setConnectionName(uint32 peer, const String &name) {
	if (SteamNetworkingSockets() != NULL) {
		SteamNetworkingSockets()->SetConnectionName((HSteamNetConnection)peer, name.utf8().get_data());
	}
}

// Fetch connection name into your buffer, which is at least nMaxLen bytes. Returns false if handle is invalid.
String SteamServer::getConnectionName(uint32 peer) {
	// Set empty string variable for use
	String connection_name = "";
	if (SteamNetworkingSockets() != NULL) {
		char name[STEAM_BUFFER_SIZE];
		if (SteamNetworkingSockets()->GetConnectionName((HSteamNetConnection)peer, name, STEAM_BUFFER_SIZE)) {
			connection_name += name;
		}
	}
	return connection_name;
}

// Returns local IP and port that a listen socket created using CreateListenSocketIP is bound to.
bool SteamServer::getListenSocketAddress(uint32 socket) {
	if (SteamNetworkingSockets() == NULL) {
		return false;
	}
	SteamNetworkingIPAddr address;
	return SteamNetworkingSockets()->GetListenSocketAddress((HSteamListenSocket)socket, &address);
}

// Get the identity assigned to this interface.
String SteamServer::getIdentity() {
	String identity_string = "";
	if (SteamNetworkingSockets() != NULL) {
		SteamNetworkingIdentity this_identity;
		if (SteamNetworkingSockets()->GetIdentity(&this_identity)) {
			char *this_buffer = new char[128];
			this_identity.ToString(this_buffer, 128);
			identity_string = String(this_buffer);
			delete[] this_buffer;
		}
	}
	return identity_string;
}

// Indicate our desire to be ready participate in authenticated communications. If we are currently not ready, then steps will be taken to obtain the necessary certificates. (This includes a certificate for us, as well as any CA certificates needed to authenticate peers.)
SteamServer::NetworkingAvailability SteamServer::initAuthentication() {
	if (SteamNetworkingSockets() == NULL) {
		return NETWORKING_AVAILABILITY_UNKNOWN;
	}
	return NetworkingAvailability(SteamNetworkingSockets()->InitAuthentication());
}

// Query our readiness to participate in authenticated communications. A SteamNetAuthenticationStatus_t callback is posted any time this status changes, but you can use this function to query it at any time.
SteamServer::NetworkingAvailability SteamServer::getAuthenticationStatus() {
	if (SteamNetworkingSockets() == NULL) {
		return NETWORKING_AVAILABILITY_UNKNOWN;
	}
	return NetworkingAvailability(SteamNetworkingSockets()->GetAuthenticationStatus(NULL));
}

// Call this when you receive a ticket from your backend / matchmaking system. Puts the ticket into a persistent cache, and optionally returns the parsed ticket.
//Dictionary Steam::receivedRelayAuthTicket(){
//	Dictionary ticket;
//	if(SteamNetworkingSockets() != NULL){
//		SteamDatagramRelayAuthTicket parsed_ticket;
//		PackedByteArray incoming_ticket;
//		incoming_ticket.resize(512);
//		if(SteamNetworkingSockets()->ReceivedRelayAuthTicket(incoming_ticket.ptrw(), 512, &parsed_ticket)){
//			char game_server;
//			parsed_ticket.m_identityGameserver.ToString(&game_server, 128);
//			ticket["game_server"] = game_server;
//			char authorized_client;
//			parsed_ticket.m_identityAuthorizedClient.ToString(&authorized_client, 128);
//			ticket["authorized_client"] = authorized_client;
//			ticket["public_ip"] = parsed_ticket.m_unPublicIP;		// uint32
//			ticket["expiry"] = parsed_ticket.m_rtimeTicketExpiry;	// RTime32
//			ticket["routing"] = parsed_ticket.m_routing.GetPopID();			// SteamDatagramHostAddress
//			ticket["app_id"] = parsed_ticket.m_nAppID;				// uint32
//			ticket["restrict_to_v_port"] = parsed_ticket.m_nRestrictToVirtualPort;	// int
//			ticket["number_of_extras"] = parsed_ticket.m_nExtraFields;		// int
//			ticket["extra_fields"] = parsed_ticket.m_vecExtraFields;		// ExtraField
//		}
//	}
//	return ticket;
//}

// Search cache for a ticket to talk to the server on the specified virtual port. If found, returns the number of seconds until the ticket expires, and optionally the complete cracked ticket. Returns 0 if we don't have a ticket.
//int Steam::findRelayAuthTicketForServer(int port){
//	int expires_in_seconds = 0;
//	if(SteamNetworkingSockets() != NULL){
//		expires_in_seconds = SteamNetworkingSockets()->FindRelayAuthTicketForServer(game_server, port, &relay_auth_ticket);
//	}
//	return expires_in_seconds;
//}

// Returns the value of the SDR_LISTEN_PORT environment variable. This is the UDP server your server will be listening on. This will configured automatically for you in production environments.
uint16 SteamServer::getHostedDedicatedServerPort() {
	if (SteamNetworkingSockets() == NULL) {
		return 0;
	}
	return SteamNetworkingSockets()->GetHostedDedicatedServerPort();
}

// Returns 0 if SDR_LISTEN_PORT is not set. Otherwise, returns the data center the server is running in. This will be k_SteamDatagramPOPID_dev in non-production envirionment.
uint32 SteamServer::getHostedDedicatedServerPOPId() {
	if (SteamNetworkingSockets() == NULL) {
		return 0;
	}
	return SteamNetworkingSockets()->GetHostedDedicatedServerPOPID();
}

// Return info about the hosted server. This contains the PoPID of the server, and opaque routing information that can be used by the relays to send traffic to your server.
//int Steam::getHostedDedicatedServerAddress(){
//	int result = 2;
//	if(SteamNetworkingSockets() != NULL){
//		result = SteamNetworkingSockets()->GetHostedDedicatedServerAddress(&hosted_address);
//	}
//	return result;
//}

// Create a listen socket on the specified virtual port. The physical UDP port to use will be determined by the SDR_LISTEN_PORT environment variable. If a UDP port is not configured, this call will fail.
uint32 SteamServer::createHostedDedicatedServerListenSocket(int port, Array options) {
	if (SteamNetworkingSockets() == NULL) {
		return 0;
	}
	const SteamNetworkingConfigValue_t *these_options = convertOptionsArray(options);
	uint32 listen_socket = SteamGameServerNetworkingSockets()->CreateHostedDedicatedServerListenSocket(port, options.size(), these_options);
	delete[] these_options;
	return listen_socket;
}

// Generate an authentication blob that can be used to securely login with your backend, using SteamDatagram_ParseHostedServerLogin. (See steamdatagram_gamecoordinator.h)
//int Steam::getGameCoordinatorServerLogin(const String& app_data){
//	int result = 2;
//	if(SteamNetworkingSockets() != NULL){
//		SteamDatagramGameCoordinatorServerLogin *server_login = new SteamDatagramGameCoordinatorServerLogin;
//		server_login->m_cbAppData = app_data.size();
//		strcpy(server_login->m_appData, app_data.utf8().get_data());
//		int signed_blob = k_cbMaxSteamDatagramGameCoordinatorServerLoginSerialized;
//		routing_blob.resize(signed_blob);
//		result = SteamNetworkingSockets()->GetGameCoordinatorServerLogin(server_login, &signed_blob, routing_blob.ptrw());
//		delete server_login;
//	}
//	return result;
//}

// Returns a small set of information about the real-time state of the connection and the queue status of each lane.
Dictionary SteamServer::getConnectionRealTimeStatus(uint32 connection, int lanes, bool get_status) {
	// Create the dictionary for returning
	Dictionary real_time_status;
	if (SteamNetworkingSockets() != NULL) {
		SteamNetConnectionRealTimeStatus_t this_status;
		SteamNetConnectionRealTimeLaneStatus_t *lanes_array = new SteamNetConnectionRealTimeLaneStatus_t[lanes];
		int result = SteamNetworkingSockets()->GetConnectionRealTimeStatus((HSteamNetConnection)connection, &this_status, lanes, lanes_array);
		// Append the status
		real_time_status["response"] = result;
		// If the result is good, get more data
		if (result == 0) {
			// Get the connection status if requested
			Dictionary connection_status;
			if (get_status) {
				connection_status["state"] = this_status.m_eState;
				connection_status["ping"] = this_status.m_nPing;
				connection_status["local_quality"] = this_status.m_flConnectionQualityLocal;
				connection_status["remote_quality"] = this_status.m_flConnectionQualityRemote;
				connection_status["packets_out_per_second"] = this_status.m_flOutPacketsPerSec;
				connection_status["bytes_out_per_second"] = this_status.m_flOutBytesPerSec;
				connection_status["packets_in_per_second"] = this_status.m_flInPacketsPerSec;
				connection_status["bytes_in_per_second"] = this_status.m_flInBytesPerSec;
				connection_status["send_rate"] = this_status.m_nSendRateBytesPerSecond;
				connection_status["pending_unreliable"] = this_status.m_cbPendingUnreliable;
				connection_status["pending_reliable"] = this_status.m_cbPendingReliable;
				connection_status["sent_unacknowledged_reliable"] = this_status.m_cbSentUnackedReliable;
				connection_status["queue_time"] = (uint64_t)this_status.m_usecQueueTime;
			}
			real_time_status["connection_status"] = connection_status;
			// Get the lane information
			Array lanes_status;
			for (int i = 0; i < lanes; i++) {
				Dictionary lane_status;
				lane_status["pending_unreliable"] = lanes_array[i].m_cbPendingUnreliable;
				lane_status["pending_reliable"] = lanes_array[i].m_cbPendingReliable;
				lane_status["sent_unacknowledged_reliable"] = lanes_array[i].m_cbSentUnackedReliable;
				lane_status["queue_time"] = (uint64_t)lanes_array[i].m_usecQueueTime;
				lanes_status.append(lane_status);
			}
			delete[] lanes_array;
			real_time_status["lanes_status"] = lanes_status;
		}
	}
	return real_time_status;
}

// Configure multiple outbound messages streams ("lanes") on a connection, and control head-of-line blocking between them.
// Messages within a given lane are always sent in the order they are queued, but messages from different lanes may be sent out of order.
// Each lane has its own message number sequence.  The first message sent on each lane will be assigned the number 1.
int SteamServer::configureConnectionLanes(uint32 connection, int lanes, Array priorities, Array weights) {
	if (SteamNetworkingSockets() == NULL) {
		return 0;
	}
	// Convert the priorities array to an int
	int *lane_priorities = new int[lanes];
	for (int i = 0; i < lanes; i++) {
		lane_priorities[i] = priorities[i];
	}
	// Convert the weights array to an int
	uint16 *lane_weights = new uint16[lanes];
	for (int i = 0; i < lanes; i++) {
		lane_weights[i] = weights[i];
	}
	int result = SteamNetworkingSockets()->ConfigureConnectionLanes((HSteamNetConnection)connection, lanes, lane_priorities, lane_weights);
	delete[] lane_priorities;
	delete[] lane_weights;
	return result;
}

// Certificate provision by the application. On Steam, we normally handle all this automatically and you will not need to use these advanced functions.
Dictionary SteamServer::getCertificateRequest() {
	Dictionary cert_information;
	if (SteamNetworkingSockets() != NULL) {
		int *certificate = new int[512];
		int cert_size = 0;
		SteamNetworkingErrMsg error_message;
		if (SteamNetworkingSockets()->GetCertificateRequest(&cert_size, &certificate, error_message)) {
			cert_information["certificate"] = certificate;
			cert_information["cert_size"] = cert_size;
			cert_information["error_message"] = error_message;
		}
		delete[] certificate;
	}
	return cert_information;
}

// Set the certificate. The certificate blob should be the output of SteamDatagram_CreateCert.
Dictionary SteamServer::setCertificate(const PackedByteArray &certificate) {
	Dictionary certificate_data;
	if (SteamNetworkingSockets() != NULL) {
		bool success = false;
		SteamNetworkingErrMsg error_message;
		success = SteamNetworkingSockets()->SetCertificate((void *)certificate.ptr(), certificate.size(), error_message);
		if (success) {
			certificate_data["response"] = success;
			certificate_data["error"] = error_message;
		}
	}
	return certificate_data;
}

// Reset the identity associated with this instance. Any open connections are closed.  Any previous certificates, etc are discarded.
// You can pass a specific identity that you want to use, or you can pass NULL, in which case the identity will be invalid until you set it using SetCertificate.
// NOTE: This function is not actually supported on Steam!  It is included for use on other platforms where the active user can sign out and a new user can sign in.
void SteamServer::resetIdentity(const String &identity_reference) {
	if (SteamNetworkingSockets() != NULL) {
		SteamNetworkingIdentity resetting_identity = networking_identities[identity_reference.utf8().get_data()];
		SteamNetworkingSockets()->ResetIdentity(&resetting_identity);
	}
}

// Invoke all callback functions queued for this interface. See k_ESteamNetworkingConfig_Callback_ConnectionStatusChanged, etc.
// You don't need to call this if you are using Steam's callback dispatch mechanism (SteamAPI_RunCallbacks and SteamGameserver_RunCallbacks).
void SteamServer::runNetworkingCallbacks() {
	if (SteamNetworkingSockets() != NULL) {
		SteamNetworkingSockets()->RunCallbacks();
	}
}

// Begin asynchronous process of allocating a fake IPv4 address that other peers can use to contact us via P2P.
// IP addresses returned by this function are globally unique for a given appid.
// Returns false if a request was already in progress, true if a new request was started.
// A SteamNetworkingFakeIPResult_t will be posted when the request completes.
bool SteamServer::beginAsyncRequestFakeIP(int num_ports) {
	if (SteamNetworkingSockets() == NULL) {
		return false;
	}
	return SteamNetworkingSockets()->BeginAsyncRequestFakeIP(num_ports);
}

// Return info about the FakeIP and port(s) that we have been assigned, if any.
// idxFirstPort is currently reserved and must be zero. Make sure and check SteamNetworkingFakeIPResult_t::m_eResult
Dictionary SteamServer::getFakeIP(int first_port) {
	// Create the return dictionary
	Dictionary fake_ip;
	if (SteamNetworkingSockets() != NULL) {
		SteamNetworkingFakeIPResult_t fake_ip_result;
		SteamNetworkingSockets()->GetFakeIP(first_port, &fake_ip_result);
		// Populate the dictionary
		fake_ip["result"] = fake_ip_result.m_eResult;
		fake_ip["identity_type"] = fake_ip_result.m_identity.m_eType;
		fake_ip["ip"] = fake_ip_result.m_unIP;
		char ports[8];
		for (size_t i = 0; i < sizeof(fake_ip_result.m_unPorts) / sizeof(fake_ip_result.m_unPorts[0]); i++) {
			ports[i] = fake_ip_result.m_unPorts[i];
		}
		fake_ip["ports"] = ports;
	}
	return fake_ip;
}

// Create a listen socket that will listen for P2P connections sent to our FakeIP.
// A peer can initiate connections to this listen socket by calling ConnectByIPAddress.
uint32 SteamServer::createListenSocketP2PFakeIP(int fake_port, Array options) {
	if (SteamNetworkingSockets() == NULL) {
		return 0;
	}

	const SteamNetworkingConfigValue_t *these_options = convertOptionsArray(options);
	uint32 listen_socket = SteamNetworkingSockets()->CreateListenSocketP2PFakeIP(fake_port, options.size(), these_options);
	delete[] these_options;
	return listen_socket;
}

// If the connection was initiated using the "FakeIP" system, then we we can get an IP address for the remote host.  If the remote host had a global FakeIP at the time the connection was established, this function will return that global IP.
// Otherwise, a FakeIP that is unique locally will be allocated from the local FakeIP address space, and that will be returned.
Dictionary SteamServer::getRemoteFakeIPForConnection(uint32 connection) {
	// Create the return dictionary
	Dictionary this_fake_address;
	if (SteamNetworkingSockets() != NULL) {
		SteamNetworkingIPAddr fake_address;
		int result = SteamNetworkingSockets()->GetRemoteFakeIPForConnection((HSteamNetConnection)connection, &fake_address);
		// Send back the data
		this_fake_address["result"] = result;
		this_fake_address["port"] = fake_address.m_port;
		this_fake_address["ip_type"] = fake_address.GetFakeIPType();
		ip_addresses["fake_ip_address"] = fake_address;
	}

	return this_fake_address;
}

// Get an interface that can be used like a UDP port to send/receive datagrams to a FakeIP address.
// This is intended to make it easy to port existing UDP-based code to take advantage of SDR.
// To create a "client" port (e.g. the equivalent of an ephemeral UDP port) pass -1.
void SteamServer::createFakeUDPPort(int fake_server_port_index) {
	if (SteamNetworkingSockets() != NULL) {
		SteamNetworkingSockets()->CreateFakeUDPPort(fake_server_port_index);
	}
}

/////////////////////////////////////////////////
///// NETWORKING TYPES
/////////////////////////////////////////////////
//
// Create a new network identity and store it for use
bool SteamServer::addIdentity(const String &reference_name) {
	networking_identities[reference_name.utf8().get_data()] = SteamNetworkingIdentity();
	if (networking_identities.count(reference_name.utf8().get_data()) > 0) {
		return true;
	}
	return false;
}

// Clear a network identity's data
void SteamServer::clearIdentity(const String &reference_name) {
	networking_identities[reference_name.utf8().get_data()].Clear();
}

// Get a list of all known network identities
Array SteamServer::getIdentities() {
	Array these_identities;
	// Loop through the map
	for (auto &identity : networking_identities) {
		Dictionary this_identity;
		this_identity["reference_name"] = identity.first;
		this_identity["steam_id"] = (uint64_t)getIdentitySteamID64(identity.first);
		this_identity["type"] = networking_identities[identity.first].m_eType;
		these_identities.append(this_identity);
	}
	return these_identities;
}

// Return true if we are the invalid type.  Does not make any other validity checks (e.g. is SteamID actually valid)
bool SteamServer::isIdentityInvalid(const String &reference_name) {
	return networking_identities[reference_name.utf8().get_data()].IsInvalid();
}

// Set a 32-bit Steam ID
void SteamServer::setIdentitySteamID(const String &reference_name, uint32 steam_id) {
	networking_identities[reference_name.utf8().get_data()].SetSteamID(createSteamID(steam_id));
}

// Return CSteamID (!IsValid()) if identity is not a SteamID
uint32 SteamServer::getIdentitySteamID(const String &reference_name) {
	CSteamID steam_id = networking_identities[reference_name.utf8().get_data()].GetSteamID();
	return steam_id.ConvertToUint64();
}

// Takes SteamID as raw 64-bit number
void SteamServer::setIdentitySteamID64(const String &reference_name, uint64_t steam_id) {
	networking_identities[reference_name.utf8().get_data()].SetSteamID64(steam_id);
}

// Returns 0 if identity is not SteamID
uint64_t SteamServer::getIdentitySteamID64(const String &reference_name) {
	return networking_identities[reference_name.utf8().get_data()].GetSteamID64();
}

// Set to specified IP:port.
bool SteamServer::setIdentityIPAddr(const String &reference_name, const String &ip_address_name) {
	if (ip_addresses.count(ip_address_name.utf8().get_data()) > 0) {
		const SteamNetworkingIPAddr this_address = ip_addresses[ip_address_name.utf8().get_data()];
		networking_identities[reference_name.utf8().get_data()].SetIPAddr(this_address);
		return true;
	}
	return false;
}

// Returns null if we are not an IP address.
uint32 SteamServer::getIdentityIPAddr(const String &reference_name) {
	const SteamNetworkingIPAddr *this_address = networking_identities[reference_name.utf8().get_data()].GetIPAddr();
	if (this_address == NULL) {
		return 0;
	}
	return this_address->GetIPv4();
}

// Retrieve this identity's Playstation Network ID.
uint64_t SteamServer::getPSNID(const String &reference_name) {
	return networking_identities[reference_name.utf8().get_data()].GetPSNID();
}

// Retrieve this identity's Google Stadia ID.
uint64_t SteamServer::getStadiaID(const String &reference_name) {
	return networking_identities[reference_name.utf8().get_data()].GetStadiaID();
}

// Retrieve this identity's XBox pair ID.
String SteamServer::getXboxPairwiseID(const String &reference_name) {
	return networking_identities[reference_name.utf8().get_data()].GetXboxPairwiseID();
}

// Set to localhost. (We always use IPv6 ::1 for this, not 127.0.0.1).
void SteamServer::setIdentityLocalHost(const String &reference_name) {
	networking_identities[reference_name.utf8().get_data()].SetLocalHost();
}

// Return true if this identity is localhost.
bool SteamServer::isIdentityLocalHost(const String &reference_name) {
	return networking_identities[reference_name.utf8().get_data()].IsLocalHost();
}

// Returns false if invalid length.
bool SteamServer::setGenericString(const String &reference_name, const String &this_string) {
	return networking_identities[reference_name.utf8().get_data()].SetGenericString(this_string.utf8().get_data());
}

// Returns nullptr if not generic string type
String SteamServer::getGenericString(const String &reference_name) {
	return networking_identities[reference_name.utf8().get_data()].GetGenericString();
}

// Returns false if invalid size.
bool SteamServer::setGenericBytes(const String &reference_name, uint8 data) {
	const void *this_data = &data;
	return networking_identities[reference_name.utf8().get_data()].SetGenericBytes(this_data, sizeof(data));
}

// Returns null if not generic bytes type.
uint8 SteamServer::getGenericBytes(const String &reference_name) {
	uint8 these_bytes = 0;
	if (!reference_name.is_empty()) {
		int length = 0;
		const uint8 *generic_bytes = networking_identities[reference_name.utf8().get_data()].GetGenericBytes(length);
		these_bytes = *generic_bytes;
	}
	return these_bytes;
}

// Add a new IP address struct
bool SteamServer::addIPAddress(const String &reference_name) {
	ip_addresses[reference_name.utf8().get_data()] = SteamNetworkingIPAddr();
	if (ip_addresses.count(reference_name.utf8().get_data()) > 0) {
		return true;
	}
	return false;
}

// Get a list of all IP address structs and their names
Array SteamServer::getIPAddresses() {
	Array these_addresses;
	// Loop through the map
	for (auto &address : ip_addresses) {
		Dictionary this_address;
		this_address["reference_name"] = address.first;
		this_address["localhost"] = isAddressLocalHost(address.first);
		this_address["ip_address"] = getIPv4(address.first);
		these_addresses.append(this_address);
	}
	return these_addresses;
}

// IP Address - Set everything to zero. E.g. [::]:0
void SteamServer::clearIPAddress(const String &reference_name) {
	ip_addresses[reference_name.utf8().get_data()].Clear();
}

// Return true if the IP is ::0. (Doesn't check port.)
bool SteamServer::isIPv6AllZeros(const String &reference_name) {
	return ip_addresses[reference_name.utf8().get_data()].IsIPv6AllZeros();
}

// Set IPv6 address. IP is interpreted as bytes, so there are no endian issues. (Same as inaddr_in6.) The IP can be a mapped IPv4 address.
void SteamServer::setIPv6(const String &reference_name, uint8 ipv6, uint16 port) {
	const uint8 *this_ipv6 = &ipv6;
	ip_addresses[reference_name.utf8().get_data()].SetIPv6(this_ipv6, port);
}

// Sets to IPv4 mapped address. IP and port are in host byte order.
void SteamServer::setIPv4(const String &reference_name, uint32 ip, uint16 port) {
	ip_addresses[reference_name.utf8().get_data()].SetIPv4(ip, port);
}

// Return true if IP is mapped IPv4.
bool SteamServer::isIPv4(const String &reference_name) {
	return ip_addresses[reference_name.utf8().get_data()].IsIPv4();
}

// Returns IP in host byte order (e.g. aa.bb.cc.dd as 0xaabbccdd). Returns 0 if IP is not mapped IPv4.
uint32 SteamServer::getIPv4(const String &reference_name) {
	return ip_addresses[reference_name.utf8().get_data()].GetIPv4();
}

// Set to the IPv6 localhost address ::1, and the specified port.
void SteamServer::setIPv6LocalHost(const String &reference_name, uint16 port) {
	ip_addresses[reference_name.utf8().get_data()].SetIPv6LocalHost(port);
}

// Set the Playstation Network ID for this identity.
void SteamServer::setPSNID(const String &reference_name, uint64_t psn_id) {
	networking_identities[reference_name.utf8().get_data()].SetPSNID(psn_id);
}

// Set the Google Stadia ID for this identity.
void SteamServer::setStadiaID(const String &reference_name, uint64_t stadia_id) {
	networking_identities[reference_name.utf8().get_data()].SetStadiaID(stadia_id);
}

// Set the Xbox Pairwise ID for this identity.
bool SteamServer::setXboxPairwiseID(const String &reference_name, const String &xbox_id) {
	return networking_identities[reference_name.utf8().get_data()].SetXboxPairwiseID(xbox_id.utf8().get_data());
}

// Return true if this identity is localhost. (Either IPv6 ::1, or IPv4 127.0.0.1).
bool SteamServer::isAddressLocalHost(const String &reference_name) {
	return ip_addresses[reference_name.utf8().get_data()].IsLocalHost();
}

// Parse back a string that was generated using ToString. If we don't understand the string, but it looks "reasonable" (it matches the pattern type:<type-data> and doesn't have any funky characters, etc), then we will return true, and the type is set to k_ESteamNetworkingIdentityType_UnknownType.
// false will only be returned if the string looks invalid.
bool SteamServer::parseIdentityString(const String &reference_name, const String &string_to_parse) {
	if (!reference_name.is_empty() && !string_to_parse.is_empty()) {
		if (networking_identities[reference_name.utf8().get_data()].ParseString(string_to_parse.utf8().get_data())) {
			return true;
		}
		return false;
	}
	return false;
}

// Parse an IP address and optional port.  If a port is not present, it is set to 0. (This means that you cannot tell if a zero port was explicitly specified.).
bool SteamServer::parseIPAddressString(const String &reference_name, const String &string_to_parse) {
	if (!reference_name.is_empty() && !string_to_parse.is_empty()) {
		if (ip_addresses[reference_name.utf8().get_data()].ParseString(string_to_parse.utf8().get_data())) {
			return true;
		}
		return false;
	}
	return false;
}

// Print to a string, with or without the port. Mapped IPv4 addresses are printed as dotted decimal (12.34.56.78), otherwise this will print the canonical form according to RFC5952.
// If you include the port, IPv6 will be surrounded by brackets, e.g. [::1:2]:80. Your buffer should be at least k_cchMaxString bytes to avoid truncation.
String SteamServer::toIPAddressString(const String &reference_name, bool with_port) {
	String ip_address_string = "";
	char *this_buffer = new char[128];
	ip_addresses[reference_name.utf8().get_data()].ToString(this_buffer, 128, with_port);
	ip_address_string = String(this_buffer);
	delete[] this_buffer;
	return ip_address_string;
}

// Print to a human-readable string.  This is suitable for debug messages or any other time you need to encode the identity as a string.
// It has a URL-like format (type:<type-data>). Your buffer should be at least k_cchMaxString bytes big to avoid truncation.
String SteamServer::toIdentityString(const String &reference_name) {
	String identity_string = "";
	char *this_buffer = new char[128];
	networking_identities[reference_name.utf8().get_data()].ToString(this_buffer, 128);
	identity_string = String(this_buffer);
	delete[] this_buffer;
	return identity_string;
}

// Helper function to turn an array of options into an array of SteamNetworkingConfigValue_t structs
const SteamNetworkingConfigValue_t *SteamServer::convertOptionsArray(Array options) {
	// Get the number of option arrays in the array.
	int options_size = options.size();
	// Create the array for options.
	SteamNetworkingConfigValue_t *option_array = new SteamNetworkingConfigValue_t[options_size];
	// If there are options
	if (options_size > 0) {
		// Populate the options
		for (int i = 0; i < options_size; i++) {
			SteamNetworkingConfigValue_t this_option;
			Array sent_option = options[i];
			// Get the configuration value.
			// This is a convoluted way of doing it but can't seem to cast the value as an enum so here we are.
			ESteamNetworkingConfigValue this_value = ESteamNetworkingConfigValue((int)sent_option[0]);
			if ((int)sent_option[1] == 1) {
				this_option.SetInt32(this_value, sent_option[2]);
			} else if ((int)sent_option[1] == 2) {
				this_option.SetInt64(this_value, sent_option[2]);
			} else if ((int)sent_option[1] == 3) {
				this_option.SetFloat(this_value, sent_option[2]);
			} else if ((int)sent_option[1] == 4) {
				char *this_string = { 0 };
				String passed_string = sent_option[2];
				strcpy(this_string, passed_string.utf8().get_data());
				this_option.SetString(this_value, this_string);
			} else {
				Object *this_pointer;
				this_pointer = sent_option[2];
				this_option.SetPtr(this_value, this_pointer);
			}
			option_array[i] = this_option;
		}
	}
	return option_array;
}

/////////////////////////////////////////////////
///// NETWORKING UTILS
/////////////////////////////////////////////////
//
// If you know that you are going to be using the relay network (for example, because you anticipate making P2P connections), call this to initialize the relay network. If you do not call this, the initialization will be delayed until the first time you use a feature that requires access to the relay network, which will delay that first access.
void SteamServer::initRelayNetworkAccess() {
	if (SteamNetworkingUtils() != NULL) {
		SteamNetworkingUtils()->InitRelayNetworkAccess();
	}
}

// Fetch current status of the relay network.  If you want more details, you can pass a non-NULL value.
SteamServer::NetworkingAvailability SteamServer::getRelayNetworkStatus() {
	if (SteamNetworkingUtils() == NULL) {
		return NETWORKING_AVAILABILITY_UNKNOWN;
	}
	return NetworkingAvailability(SteamNetworkingUtils()->GetRelayNetworkStatus(NULL));
}

// Return location info for the current host. Returns the approximate age of the data, in seconds, or -1 if no data is available.
Dictionary SteamServer::getLocalPingLocation() {
	Dictionary ping_location;
	if (SteamNetworkingUtils() != NULL) {
		SteamNetworkPingLocation_t location;
		float age = SteamNetworkingUtils()->GetLocalPingLocation(location);
		// Populate the dictionary
		PackedByteArray data;
		data.resize(512);
		uint8_t *output_data = data.ptrw();
		for (int j = 0; j < 512; j++) {
			output_data[j] = location.m_data[j];
		}
		ping_location["age"] = age;
		ping_location["location"] = data;
	}
	return ping_location;
}

// Estimate the round-trip latency between two arbitrary locations, in milliseconds. This is a conservative estimate, based on routing through the relay network. For most basic relayed connections, this ping time will be pretty accurate, since it will be based on the route likely to be actually used.
int SteamServer::estimatePingTimeBetweenTwoLocations(PackedByteArray location1, PackedByteArray location2) {
	if (SteamNetworkingUtils() == NULL) {
		return 0;
	}
	// Add these locations to ping structs
	SteamNetworkPingLocation_t ping_location1;
	SteamNetworkPingLocation_t ping_location2;
	uint8_t *input_location_1 = (uint8 *)location1.ptr();
	for (int j = 0; j < 512; j++) {
		ping_location1.m_data[j] = input_location_1[j];
	}
	uint8_t *input_location_2 = (uint8 *)location2.ptr();
	for (int j = 0; j < 512; j++) {
		ping_location2.m_data[j] = (uint8)input_location_2[j];
	}
	return SteamNetworkingUtils()->EstimatePingTimeBetweenTwoLocations(ping_location1, ping_location2);
}

// Same as EstimatePingTime, but assumes that one location is the local host. This is a bit faster, especially if you need to calculate a bunch of these in a loop to find the fastest one.
int SteamServer::estimatePingTimeFromLocalHost(PackedByteArray location) {
	if (SteamNetworkingUtils() == NULL) {
		return 0;
	}
	// Add this location to ping struct
	SteamNetworkPingLocation_t ping_location;
	uint8_t *input_location = (uint8 *)location.ptr();
	for (int j = 0; j < 512; j++) {
		ping_location.m_data[j] = input_location[j];
	}
	return SteamNetworkingUtils()->EstimatePingTimeFromLocalHost(ping_location);
}

// Convert a ping location into a text format suitable for sending over the wire. The format is a compact and human readable. However, it is subject to change so please do not parse it yourself. Your buffer must be at least k_cchMaxSteamNetworkingPingLocationString bytes.
String SteamServer::convertPingLocationToString(PackedByteArray location) {
	String location_string = "";
	if (SteamNetworkingUtils() != NULL) {
		char *buffer = new char[512];
		// Add this location to ping struct
		SteamNetworkPingLocation_t ping_location;
		uint8_t *input_location = (uint8 *)location.ptr();
		for (int j = 0; j < 512; j++) {
			ping_location.m_data[j] = input_location[j];
		}
		SteamNetworkingUtils()->ConvertPingLocationToString(ping_location, buffer, k_cchMaxSteamNetworkingPingLocationString);
		location_string += buffer;
		delete[] buffer;
	}
	return location_string;
}

// Parse back SteamNetworkPingLocation_t string. Returns false if we couldn't understand the string.
Dictionary SteamServer::parsePingLocationString(const String &location_string) {
	Dictionary parse_string;
	if (SteamNetworkingUtils() != NULL) {
		SteamNetworkPingLocation_t result;
		bool success = SteamNetworkingUtils()->ParsePingLocationString(location_string.utf8().get_data(), result);
		// Populate the dictionary
		PackedByteArray data;
		data.resize(512);
		uint8_t *output_data = data.ptrw();
		for (int j = 0; j < 512; j++) {
			output_data[j] = result.m_data[j];
		}
		parse_string["success"] = success;
		parse_string["ping_location"] = data;
	}
	return parse_string;
}

// Check if the ping data of sufficient recency is available, and if it's too old, start refreshing it.
bool SteamServer::checkPingDataUpToDate(float max_age_in_seconds) {
	if (SteamNetworkingUtils() == NULL) {
		return false;
	}
	return SteamNetworkingUtils()->CheckPingDataUpToDate(max_age_in_seconds);
}

// Fetch ping time of best available relayed route from this host to the specified data center.
Dictionary SteamServer::getPingToDataCenter(uint32 pop_id) {
	Dictionary data_center_ping;
	if (SteamNetworkingUtils() != NULL) {
		SteamNetworkingPOPID via_relay_pop;
		int ping = SteamNetworkingUtils()->GetPingToDataCenter((SteamNetworkingPOPID)pop_id, &via_relay_pop);
		// Populate the dictionary
		data_center_ping["pop_relay"] = via_relay_pop;
		data_center_ping["ping"] = ping;
	}
	return data_center_ping;
}

// Get *direct* ping time to the relays at the point of presence.
int SteamServer::getDirectPingToPOP(uint32 pop_id) {
	if (SteamNetworkingUtils() == NULL) {
		return 0;
	}
	return SteamNetworkingUtils()->GetDirectPingToPOP((SteamNetworkingPOPID)pop_id);
}

// Get number of network points of presence in the config
int SteamServer::getPOPCount() {
	if (SteamNetworkingUtils() == NULL) {
		return 0;
	}
	return SteamNetworkingUtils()->GetPOPCount();
}

// Get list of all POP IDs. Returns the number of entries that were filled into your list.
Array SteamServer::getPOPList() {
	Array pop_list;
	if (SteamNetworkingUtils() != NULL) {
		SteamNetworkingPOPID *list = new SteamNetworkingPOPID[256];
		int pops = SteamNetworkingUtils()->GetPOPList(list, 256);
		// Iterate and add
		for (int i = 0; i < pops; i++) {
			int pop_id = list[i];
			pop_list.append(pop_id);
		}
		delete[] list;
	}
	return pop_list;
}

// Set a configuration value.
//bool Steam::setConfigValue(NetworkingConfigValue setting, NetworkingConfigScope scope_type, uint32_t connection_handle, NetworkingConfigDataType data_type, auto value){
//	if(SteamNetworkingUtils() == NULL){
//		return false;
//	}
//	return SteamNetworkingUtils()->SetConfigValue((ESteamNetworkingConfigValue)setting, (ESteamNetworkingConfigScope)scope_type, connection_handle, (ESteamNetworkingConfigDataType)data_type, value);
//}

// Get a configuration value.
Dictionary SteamServer::getConfigValue(NetworkingConfigValue config_value, NetworkingConfigScope scope_type, uint32_t connection_handle) {
	Dictionary config_info;
	if (SteamNetworkingUtils() != NULL) {
		ESteamNetworkingConfigDataType data_type;
		size_t buffer_size;
		PackedByteArray config_result;
		int result = SteamNetworkingUtils()->GetConfigValue((ESteamNetworkingConfigValue)config_value, (ESteamNetworkingConfigScope)scope_type, connection_handle, &data_type, &config_result, &buffer_size);
		// Populate the dictionary
		config_info["result"] = result;
		config_info["type"] = data_type;
		config_info["value"] = config_result;
		config_info["buffer"] = (uint64_t)buffer_size;
	}
	return config_info;
}

// Returns info about a configuration value.
Dictionary SteamServer::getConfigValueInfo(NetworkingConfigValue config_value) {
	Dictionary config_info;
	if (SteamNetworkingUtils() != NULL) {
		ESteamNetworkingConfigDataType data_type;
		ESteamNetworkingConfigScope scope;
		if (SteamNetworkingUtils()->GetConfigValueInfo((ESteamNetworkingConfigValue)config_value, &data_type, &scope)) {
			// Populate the dictionary
			config_info["type"] = data_type;
			config_info["scope"] = scope;
		}
	}
	return config_info;
}

// The following functions are handy shortcuts for common use cases.
bool SteamServer::setGlobalConfigValueInt32(NetworkingConfigValue config, int32 value) {
	if (SteamNetworkingUtils() == NULL) {
		return false;
	}
	return SteamNetworkingUtils()->SetGlobalConfigValueInt32((ESteamNetworkingConfigValue)config, value);
}
bool SteamServer::setGlobalConfigValueFloat(NetworkingConfigValue config, float value) {
	if (SteamNetworkingUtils() == NULL) {
		return false;
	}
	return SteamNetworkingUtils()->SetGlobalConfigValueFloat((ESteamNetworkingConfigValue)config, value);
}
bool SteamServer::setGlobalConfigValueString(NetworkingConfigValue config, const String &value) {
	if (SteamNetworkingUtils() == NULL) {
		return false;
	}
	return SteamNetworkingUtils()->SetGlobalConfigValueString((ESteamNetworkingConfigValue)config, value.utf8().get_data());
}
bool SteamServer::setConnectionConfigValueInt32(uint32 connection, NetworkingConfigValue config, int32 value) {
	if (SteamNetworkingUtils() == NULL) {
		return false;
	}
	return SteamNetworkingUtils()->SetConnectionConfigValueInt32(connection, (ESteamNetworkingConfigValue)config, value);
}
bool SteamServer::setConnectionConfigValueFloat(uint32 connection, NetworkingConfigValue config, float value) {
	if (SteamNetworkingUtils() == NULL) {
		return false;
	}
	return SteamNetworkingUtils()->SetConnectionConfigValueFloat(connection, (ESteamNetworkingConfigValue)config, value);
}

bool SteamServer::setConnectionConfigValueString(uint32 connection, NetworkingConfigValue config, const String &value) {
	if (SteamNetworkingUtils() == NULL) {
		return false;
	}
	return SteamNetworkingUtils()->SetConnectionConfigValueString(connection, (ESteamNetworkingConfigValue)config, value.utf8().get_data());
}

// A general purpose high resolution local timer with the following properties: Monotonicity is guaranteed. The initial value will be at least 24*3600*30*1e6, i.e. about 30 days worth of microseconds. In this way, the timestamp value of 0 will always be at least "30 days ago". Also, negative numbers will never be returned. Wraparound / overflow is not a practical concern.
uint64_t SteamServer::getLocalTimestamp() {
	if (SteamNetworkingUtils() == NULL) {
		return 0;
	}
	return SteamNetworkingUtils()->GetLocalTimestamp();
}

/////////////////////////////////////////////////
///// PARTIES
/////////////////////////////////////////////////
//
// Get the list of locations in which you can post a party beacon.
Array SteamServer::getAvailableBeaconLocations(uint32 max) {
	if (SteamParties() == NULL) {
		return Array();
	}
	Array beaconLocations;
	uint32 locations = 0;
	SteamPartyBeaconLocation_t *beacons = new SteamPartyBeaconLocation_t[256];
	if (SteamParties()->GetNumAvailableBeaconLocations(&locations)) {
		// If max is lower than locations, set it to locations
		if (max < locations) {
			max = locations;
		}
		// Now get the beacon location list
		if (SteamParties()->GetAvailableBeaconLocations(beacons, max)) {
			for (uint32 i = 0; i < max; i++) {
				Dictionary beacon_data;
				beacon_data["type"] = beacons[i].m_eType;
				beacon_data["location_id"] = (uint64_t)beacons[i].m_ulLocationID;
				beaconLocations.append(beacon_data);
			}
		}
	}
	delete[] beacons;
	return beaconLocations;
}

// Create a beacon. You can only create one beacon at a time. Steam will display the beacon in the specified location, and let up to unOpenSlots users "follow" the beacon to your party.
void SteamServer::createBeacon(uint32 open_slots, uint64_t location, PartyBeaconLocationType type, const String &connect_string, const String &beacon_metadata) {
	if (SteamParties() != NULL) {
		// Add data to the beacon location struct
		SteamPartyBeaconLocation_t *beacon_data = new SteamPartyBeaconLocation_t;
		beacon_data->m_eType = (ESteamPartyBeaconLocationType)type;
		beacon_data->m_ulLocationID = location;
		SteamAPICall_t api_call = SteamParties()->CreateBeacon(open_slots, beacon_data, connect_string.utf8().get_data(), beacon_metadata.utf8().get_data());
		callResultCreateBeacon.Set(api_call, this, &SteamServer::create_beacon);
		delete beacon_data;
	}
}

// When a user follows your beacon, Steam will reserve one of the open party slots for them, and send your game a ReservationNotificationCallback_t callback. When that user joins your party, call OnReservationCompleted to notify Steam that the user has joined successfully.
void SteamServer::onReservationCompleted(uint64_t beacon_id, uint64_t steam_id) {
	if (SteamParties() != NULL) {
		CSteamID user_id = (uint64)steam_id;
		SteamParties()->OnReservationCompleted(beacon_id, user_id);
	}
}

// To cancel a reservation (due to timeout or user input), call this. Steam will open a new reservation slot. Note: The user may already be in-flight to your game, so it's possible they will still connect and try to join your party.
void SteamServer::cancelReservation(uint64_t beacon_id, uint64_t steam_id) {
	if (SteamParties() != NULL) {
		CSteamID user_id = (uint64)steam_id;
		SteamParties()->CancelReservation(beacon_id, user_id);
	}
}

// If a user joins your party through other matchmaking (perhaps a direct Steam friend, or your own matchmaking system), your game should reduce the number of open slots that Steam is managing through the party beacon. For example, if you created a beacon with five slots, and Steam sent you two ReservationNotificationCallback_t callbacks, and then a third user joined directly, you would want to call ChangeNumOpenSlots with a value of 2 for unOpenSlots. That value represents the total number of new users that you would like Steam to send to your party.
void SteamServer::changeNumOpenSlots(uint64_t beacon_id, uint32 open_slots) {
	if (SteamParties() != NULL) {
		SteamAPICall_t api_call = SteamParties()->ChangeNumOpenSlots(beacon_id, open_slots);
		callResultChangeNumOpenSlots.Set(api_call, this, &SteamServer::change_num_open_slots);
	}
}

// Call this method to destroy the Steam party beacon. This will immediately cause Steam to stop showing the beacon in the target location. Note that any users currently in-flight may still arrive at your party expecting to join.
bool SteamServer::destroyBeacon(uint64_t beacon_id) {
	if (SteamParties() == NULL) {
		return false;
	}
	return SteamParties()->DestroyBeacon(beacon_id);
}

// Get the number of active party beacons created by other users for your game, that are visible to the current user.
uint32 SteamServer::getNumActiveBeacons() {
	if (SteamParties() == NULL) {
		return 0;
	}
	return SteamParties()->GetNumActiveBeacons();
}

// Use with ISteamParties::GetNumActiveBeacons to iterate the active beacons visible to the current user. unIndex is a zero-based index, so iterate over the range [0, GetNumActiveBeacons() - 1]. The return is a PartyBeaconID_t that can be used with ISteamParties::GetBeaconDetails to get information about the beacons suitable for display to the user.
uint64_t SteamServer::getBeaconByIndex(uint32 index) {
	if (SteamParties() == NULL) {
		return 0;
	}
	return SteamParties()->GetBeaconByIndex(index);
}

// Get details about the specified beacon. You can use the ISteamFriends API to get further details about pSteamIDBeaconOwner, and ISteamParties::GetBeaconLocationData to get further details about pLocation. The pchMetadata contents are specific to your game, and will be whatever was set (if anything) by the game process that created the beacon.
Dictionary SteamServer::getBeaconDetails(uint64_t beacon_id) {
	Dictionary details;
	if (SteamParties() != NULL) {
		CSteamID owner;
		SteamPartyBeaconLocation_t location;
		char beacon_metadata[STEAM_LARGE_BUFFER_SIZE];
		if (SteamParties()->GetBeaconDetails(beacon_id, &owner, &location, beacon_metadata, STEAM_LARGE_BUFFER_SIZE)) {
			details["beacon_id"] = beacon_id;
			details["owner_id"] = (uint64_t)owner.ConvertToUint64();
			details["type"] = location.m_eType;
			details["location_id"] = (uint64_t)location.m_ulLocationID;
			details["metadata"] = beacon_metadata;
		}
	}
	return details;
}

// When the user indicates they wish to join the party advertised by a given beacon, call this method. On success, Steam will reserve a slot for this user in the party and return the necessary "join game" string to use to complete the connection.
void SteamServer::joinParty(uint64_t beacon_id) {
	if (SteamParties() != NULL) {
		SteamAPICall_t api_call = SteamParties()->JoinParty(beacon_id);
		callResultJoinParty.Set(api_call, this, &SteamServer::join_party);
	}
}

// Query general metadata for the given beacon location. For instance the Name, or the URL for an icon if the location type supports icons (for example, the icon for a Steam Chat Room Group).
String SteamServer::getBeaconLocationData(uint64_t location_id, PartyBeaconLocationType location_type, PartyBeaconLocationData location_data) {
	String beacon_location_data = "";
	if (SteamParties() != NULL) {
		char *beacon_data = new char[2048];
		// Add data to SteamPartyBeaconLocation struct
		SteamPartyBeaconLocation_t *beacon = new SteamPartyBeaconLocation_t;
		beacon->m_eType = (ESteamPartyBeaconLocationType)location_type;
		beacon->m_ulLocationID = location_id;
		if (SteamParties()->GetBeaconLocationData(*beacon, (ESteamPartyBeaconLocationData)location_data, (char *)beacon_data, 2048)) {
			beacon_location_data = beacon_data;
		}
		delete[] beacon_data;
		delete beacon;
	}
	return beacon_location_data;
}

/////////////////////////////////////////////////
///// REMOTE STORAGE
/////////////////////////////////////////////////
//
// Delete a given file in Steam Cloud.
bool SteamServer::fileDelete(const String &file) {
	if (SteamRemoteStorage() == NULL) {
		return false;
	}
	return SteamRemoteStorage()->FileDelete(file.utf8().get_data());
}

// Check if a given file exists in Steam Cloud.
bool SteamServer::fileExists(const String &file) {
	if (SteamRemoteStorage() == NULL) {
		return false;
	}
	return SteamRemoteStorage()->FileExists(file.utf8().get_data());
}

// Delete file from remote storage but leave it on local disk to remain accessible.
bool SteamServer::fileForget(const String &file) {
	if (SteamRemoteStorage() == NULL) {
		return false;
	}
	return SteamRemoteStorage()->FileForget(file.utf8().get_data());
}

// Check if a given file is persisted in Steam Cloud.
bool SteamServer::filePersisted(const String &file) {
	if (SteamRemoteStorage() == NULL) {
		return false;
	}
	return SteamRemoteStorage()->FilePersisted(file.utf8().get_data());
}

// Read given file from Steam Cloud.
Dictionary SteamServer::fileRead(const String &file, int32_t data_to_read) {
	Dictionary d;
	if (SteamRemoteStorage() == NULL) {
		d["ret"] = false;
		return d;
	}
	PackedByteArray data;
	data.resize(data_to_read);
	d["ret"] = SteamRemoteStorage()->FileRead(file.utf8().get_data(), data.ptrw(), data_to_read);
	d["buf"] = data;
	return d;
}

// Starts an asynchronous read from a file. The offset and amount to read should be valid for the size of the file, as indicated by GetFileSize or GetFileTimestamp.
void SteamServer::fileReadAsync(const String &file, uint32 offset, uint32_t data_to_read) {
	if (SteamRemoteStorage() != NULL) {
		SteamAPICall_t api_call = SteamRemoteStorage()->FileReadAsync(file.utf8().get_data(), offset, data_to_read);
		callResultFileReadAsyncComplete.Set(api_call, this, &SteamServer::file_read_async_complete);
	}
}

// Share a file.
void SteamServer::fileShare(const String &file) {
	if (SteamRemoteStorage() != NULL) {
		SteamAPICall_t api_call = SteamRemoteStorage()->FileShare(file.utf8().get_data());
		callResultFileShareResult.Set(api_call, this, &SteamServer::file_share_result);
	}
}

// Write to given file from Steam Cloud.
bool SteamServer::fileWrite(const String &file, PackedByteArray data, int32 size) {
	if (SteamRemoteStorage() != NULL) {
		// Get the size from the poolbytearray, just in case
		int32 data_size = data.size();
		// If a size was passed, use that instead
		if (size > 0) {
			data_size = size;
		}
		return SteamRemoteStorage()->FileWrite(file.utf8().get_data(), data.ptr(), data_size);
	}
	return false;
}

// Creates a new file and asynchronously writes the raw byte data to the Steam Cloud, and then closes the file. If the target file already exists, it is overwritten.
void SteamServer::fileWriteAsync(const String &file, PackedByteArray data, int32 size) {
	if (SteamRemoteStorage() != NULL) {
		// Get the size from the PackedByteArray, just in case
		int32 data_size = data.size();
		// If a size was passed, use that instead
		if (size > 0) {
			data_size = size;
		}
		SteamAPICall_t api_call = SteamRemoteStorage()->FileWriteAsync(file.utf8().get_data(), data.ptr(), data_size);
		callResultFileWriteAsyncComplete.Set(api_call, this, &SteamServer::file_write_async_complete);
	}
}

// Cancels a file write stream that was started by FileWriteStreamOpen.  This trashes all of the data written and closes the write stream, but if there was an existing file with this name, it remains untouched.
bool SteamServer::fileWriteStreamCancel(uint64_t write_handle) {
	if (SteamRemoteStorage() == NULL) {
		return false;
	}
	return SteamRemoteStorage()->FileWriteStreamCancel((UGCFileWriteStreamHandle_t)write_handle);
}

// Closes a file write stream that was started by FileWriteStreamOpen. This flushes the stream to the disk, overwriting the existing file if there was one.
bool SteamServer::fileWriteStreamClose(uint64_t write_handle) {
	if (SteamRemoteStorage() == NULL) {
		return false;
	}
	return SteamRemoteStorage()->FileWriteStreamClose((UGCFileWriteStreamHandle_t)write_handle);
}

// Creates a new file output stream allowing you to stream out data to the Steam Cloud file in chunks. If the target file already exists, it is not overwritten until FileWriteStreamClose has been called. To write data out to this stream you can use FileWriteStreamWriteChunk, and then to close or cancel you use FileWriteStreamClose and FileWriteStreamCancel respectively.
uint64_t SteamServer::fileWriteStreamOpen(const String &file) {
	if (SteamRemoteStorage() == NULL) {
		return 0;
	}
	return SteamRemoteStorage()->FileWriteStreamOpen(file.utf8().get_data());
}

// Writes a blob of data to the file write stream.
bool SteamServer::fileWriteStreamWriteChunk(uint64_t write_handle, PackedByteArray data) {
	if (SteamRemoteStorage() == NULL) {
		return false;
	}
	return SteamRemoteStorage()->FileWriteStreamWriteChunk((UGCFileWriteStreamHandle_t)write_handle, data.ptr(), data.size());
}

// Gets the number of cached UGC.
int32 SteamServer::getCachedUGCCount() {
	if (SteamRemoteStorage() == NULL) {
		return 0;
	}
	return SteamRemoteStorage()->GetCachedUGCCount();
}

// Gets the cached UGC's handle.
uint64_t SteamServer::getCachedUGCHandle(int32 content) {
	if (SteamRemoteStorage() == NULL) {
		return 0;
	}
	return SteamRemoteStorage()->GetCachedUGCHandle(content);
}

// Gets the total number of local files synchronized by Steam Cloud.
int32_t SteamServer::getFileCount() {
	if (SteamRemoteStorage() == NULL) {
		return 0;
	}
	return SteamRemoteStorage()->GetFileCount();
}

// Gets the file name and size of a file from the index.
Dictionary SteamServer::getFileNameAndSize(int file) {
	Dictionary d;
	String name = "";
	int32_t size = 0;
	if (SteamRemoteStorage() != NULL) {
		name = String(SteamRemoteStorage()->GetFileNameAndSize(file, &size));
	}
	d["name"] = name;
	d["size"] = size;
	return d;
}

// Get the size of a given file.
int32_t SteamServer::getFileSize(const String &file) {
	if (SteamRemoteStorage() == NULL) {
		return -1;
	}
	return SteamRemoteStorage()->GetFileSize(file.utf8().get_data());
}

// Get the timestamp of when the file was uploaded/changed.
int64_t SteamServer::getFileTimestamp(const String &file) {
	if (SteamRemoteStorage() == NULL) {
		return -1;
	}
	return SteamRemoteStorage()->GetFileTimestamp(file.utf8().get_data());
}

// Gets the number of bytes available, and used on the users Steam Cloud storage.
Dictionary SteamServer::getQuota() {
	Dictionary d;
	uint64_t total = 0;
	uint64_t available = 0;
	if (SteamRemoteStorage() != NULL) {
		SteamRemoteStorage()->GetQuota((uint64 *)&total, (uint64 *)&available);
	}
	d["total_bytes"] = total;
	d["available_bytes"] = available;
	return d;
}

// Obtains the platforms that the specified file will syncronize to.
Dictionary SteamServer::getSyncPlatforms(const String &file) {
	// Set dictionary to populate
	Dictionary platforms;
	if (SteamRemoteStorage() == NULL) {
		return platforms;
	}
	// Get the bitwise platform
	uint32_t platform = SteamRemoteStorage()->GetSyncPlatforms(file.utf8().get_data());
	// Assign this to bitwise in the dictionary
	platforms["bitwise"] = platform;
	// Now assign the verbose platform
	if (platform == 0) {
		platforms["verbose"] = "none";
	} else if (platform == (1 << 0)) {
		platforms["verbose"] = "windows";
	} else if (platform == (1 << 1)) {
		platforms["verbose"] = "osx";
	} else if (platform == (1 << 2)) {
		platforms["verbose"] = "playstation 3";
	} else if (platform == (1 << 3)) {
		platforms["verbose"] = "linux / steamos";
	} else if (platform == (1 << 4)) {
		platforms["verbose"] = "reserved";
	} else {
		platforms["verbose"] = "all";
	}
	// Return the dictionary
	return platforms;
}

// Gets metadata for a file after it has been downloaded. This is the same metadata given in the RemoteStorageDownloadUGCResult_t call result.
Dictionary SteamServer::getUGCDetails(uint64_t content) {
	// Create the return dictionary
	Dictionary ugc_details;
	if (SteamRemoteStorage() != NULL) {
		AppId_t app_id;
		char *filename;
		int32 file_size;
		CSteamID steam_owner_id;
		if (SteamRemoteStorage()->GetUGCDetails((UGCHandle_t)content, &app_id, &filename, &file_size, &steam_owner_id)) {
			ugc_details["handle"] = content;
			ugc_details["app_id"] = app_id;
			ugc_details["size"] = file_size;
			ugc_details["filename"] = filename;
			ugc_details["owner_id"] = (uint64_t)steam_owner_id.ConvertToUint64();
		}
	}
	return ugc_details;
}

// Gets the amount of data downloaded so far for a piece of content. pnBytesExpected can be 0 if function returns false or if the transfer hasn't started yet, so be careful to check for that before dividing to get a percentage.
Dictionary SteamServer::getUGCDownloadProgress(uint64_t content) {
	// Create the return dictionary
	Dictionary ugc_download_progress;
	if (SteamRemoteStorage() != NULL) {
		int32 bytes_downloaded;
		int32 bytes_expected;
		if (SteamRemoteStorage()->GetUGCDownloadProgress((UGCHandle_t)content, &bytes_downloaded, &bytes_expected)) {
			ugc_download_progress["bytes_downloaded"] = bytes_downloaded;
			ugc_download_progress["bytes_expected"] = bytes_expected;
		}
	}
	return ugc_download_progress;
}

// Is Steam Cloud enabled on the user's account?
bool SteamServer::isCloudEnabledForAccount() {
	if (SteamRemoteStorage() == NULL) {
		return false;
	}
	return SteamRemoteStorage()->IsCloudEnabledForAccount();
}

// Is Steam Cloud enabled for this application?
bool SteamServer::isCloudEnabledForApp() {
	if (SteamRemoteStorage() == NULL) {
		return false;
	}
	return SteamRemoteStorage()->IsCloudEnabledForApp();
}

// Set Steam Cloud enabled for this application.
void SteamServer::setCloudEnabledForApp(bool enabled) {
	if (SteamRemoteStorage() != NULL) {
		SteamRemoteStorage()->SetCloudEnabledForApp(enabled);
	}
}

// Allows you to specify which operating systems a file will be synchronized to. Use this if you have a multiplatform game but have data which is incompatible between platforms.
bool SteamServer::setSyncPlatforms(const String &file, int platform) {
	if (SteamRemoteStorage() == NULL) {
		return false;
	}
	return SteamRemoteStorage()->SetSyncPlatforms(file.utf8().get_data(), (ERemoteStoragePlatform)platform);
}

// Downloads a UGC file.  A priority value of 0 will download the file immediately, otherwise it will wait to download the file until all downloads with a lower priority value are completed.  Downloads with equal priority will occur simultaneously.
void SteamServer::ugcDownload(uint64_t content, uint32 priority) {
	if (SteamRemoteStorage() != NULL) {
		SteamAPICall_t api_call = SteamRemoteStorage()->UGCDownload((UGCHandle_t)content, priority);
		callResultDownloadUGCResult.Set(api_call, this, &SteamServer::download_ugc_result);
	}
}

// Downloads a UGC file to a specific location.
void SteamServer::ugcDownloadToLocation(uint64_t content, const String &location, uint32 priority) {
	if (SteamRemoteStorage() != NULL) {
		SteamAPICall_t api_call = SteamRemoteStorage()->UGCDownloadToLocation((UGCHandle_t)content, location.utf8().get_data(), priority);
		callResultDownloadUGCResult.Set(api_call, this, &SteamServer::download_ugc_result);
	}
}

// After download, gets the content of the file.
PackedByteArray SteamServer::ugcRead(uint64_t content, int32 data_size, uint32 offset, UGCReadAction action) {
	PackedByteArray file_contents;
	file_contents.resize(data_size);
	if (SteamRemoteStorage() != NULL) {
		SteamRemoteStorage()->UGCRead((UGCHandle_t)content, file_contents.ptrw(), data_size, offset, (EUGCReadAction)action);
	}
	return file_contents;
}

// Indicate to Steam the beginning / end of a set of local file operations - for example, writing a game save that requires updating two files.
bool SteamServer::beginFileWriteBatch() {
	if (SteamRemoteStorage() == NULL) {
		return false;
	}
	return SteamRemoteStorage()->BeginFileWriteBatch();
}

// Indicate to Steam the beginning / end of a set of local file operations - for example, writing a game save that requires updating two files.
bool SteamServer::endFileWriteBatch() {
	if (SteamRemoteStorage() == NULL) {
		return false;
	}
	return SteamRemoteStorage()->EndFileWriteBatch();
}

// Cloud dynamic state change notification, used to get the total number of files changed; paired with getLocalFileChange
uint32_t SteamServer::getLocalFileChangeCount() {
	if (SteamRemoteStorage() == NULL) {
		return 0;
	}
	return SteamRemoteStorage()->GetLocalFileChangeCount();
}

// Cloud dynamic state change notification, for iteration with getLocalFileChangeCount
Dictionary SteamServer::getLocalFileChange(int file) {
	Dictionary file_change;
	if (SteamRemoteStorage() != NULL) {
		ERemoteStorageLocalFileChange change_type;
		ERemoteStorageFilePathType file_path_type;
		String changed_file = SteamRemoteStorage()->GetLocalFileChange(file, &change_type, &file_path_type);
		// Add these to the dictionary
		file_change["file"] = changed_file;
		file_change["change_type"] = change_type;
		file_change["path_type"] = file_path_type;
	}
	return file_change;
}

/////////////////////////////////////////////////
///// UGC
/////////////////////////////////////////////////
//
// Adds a dependency between the given item and the appid. This list of dependencies can be retrieved by calling GetAppDependencies.
// This is a soft-dependency that is displayed on the web. It is up to the application to determine whether the item can actually be used or not.
void SteamServer::addAppDependency(uint64_t published_file_id, uint32_t app_id) {
	if (SteamUGC() != NULL) {
		PublishedFileId_t file_id = (uint64_t)published_file_id;
		AppId_t app = (uint32_t)app_id;
		SteamAPICall_t api_call = SteamUGC()->AddAppDependency(file_id, app);
		callResultAddAppDependency.Set(api_call, this, &SteamServer::add_app_dependency_result);
	}
}

bool SteamServer::addContentDescriptor(uint64_t update_handle, int descriptor_id) {
	if (SteamUGC() == NULL) {
		return false;
	}
	return SteamUGC()->AddContentDescriptor((UGCUpdateHandle_t)update_handle, (EUGCContentDescriptorID)descriptor_id);
}

// Adds a workshop item as a dependency to the specified item. If the nParentPublishedFileID item is of type k_EWorkshopFileTypeCollection, than the nChildPublishedFileID is simply added to that collection.
// Otherwise, the dependency is a soft one that is displayed on the web and can be retrieved via the ISteamUGC API using a combination of the m_unNumChildren member variable of the SteamUGCDetails_t struct and GetQueryUGCChildren.
void SteamServer::addDependency(uint64_t published_file_id, uint64_t child_published_file_id) {
	if (SteamUGC() != NULL) {
		PublishedFileId_t parent = (uint64_t)published_file_id;
		PublishedFileId_t child = (uint64_t)child_published_file_id;
		SteamAPICall_t api_call = SteamUGC()->AddDependency(parent, child);
		callResultAddUGCDependency.Set(api_call, this, &SteamServer::add_ugc_dependency_result);
	}
}

// Adds a excluded tag to a pending UGC Query. This will only return UGC without the specified tag.
bool SteamServer::addExcludedTag(uint64_t query_handle, const String &tag_name) {
	if (SteamUGC() == NULL) {
		return false;
	}
	UGCQueryHandle_t handle = (uint64_t)query_handle;
	return SteamUGC()->AddExcludedTag(handle, tag_name.utf8().get_data());
}

// Adds a key-value tag pair to an item. Keys can map to multiple different values (1-to-many relationship).
bool SteamServer::addItemKeyValueTag(uint64_t update_handle, const String &key, const String &value) {
	if (SteamUGC() == NULL) {
		return false;
	}
	UGCUpdateHandle_t handle = (uint64_t)update_handle;
	return SteamUGC()->AddItemKeyValueTag(handle, key.utf8().get_data(), value.utf8().get_data());
}

// Adds an additional preview file for the item.
bool SteamServer::addItemPreviewFile(uint64_t query_handle, const String &preview_file, ItemPreviewType type) {
	if (SteamUGC() == NULL) {
		return false;
	}
	EItemPreviewType previewType;
	UGCQueryHandle_t handle = (uint64_t)query_handle;
	if (type == 0) {
		previewType = k_EItemPreviewType_Image;
	} else if (type == 1) {
		previewType = k_EItemPreviewType_YouTubeVideo;
	} else if (type == 2) {
		previewType = k_EItemPreviewType_Sketchfab;
	} else if (type == 3) {
		previewType = k_EItemPreviewType_EnvironmentMap_HorizontalCross;
	} else if (type == 4) {
		previewType = k_EItemPreviewType_EnvironmentMap_LatLong;
	} else {
		previewType = k_EItemPreviewType_ReservedMax;
	}
	return SteamUGC()->AddItemPreviewFile(handle, preview_file.utf8().get_data(), previewType);
}

// Adds a workshop item to the users favorites list.
void SteamServer::addItemToFavorites(uint32_t app_id, uint64_t published_file_id) {
	if (SteamUGC() != NULL) {
		AppId_t app = (uint32_t)app_id;
		PublishedFileId_t file_id = (uint64_t)published_file_id;
		SteamAPICall_t api_call = SteamUGC()->AddItemToFavorites(app, file_id);
		callResultFavoriteItemListChanged.Set(api_call, this, &SteamServer::user_favorite_items_list_changed);
	}
}

// Adds a required key-value tag to a pending UGC Query. This will only return workshop items that have a key = pKey and a value = pValue.
bool SteamServer::addRequiredKeyValueTag(uint64_t query_handle, const String &key, const String &value) {
	if (SteamUGC() == NULL) {
		return false;
	}
	UGCQueryHandle_t handle = (uint64_t)query_handle;
	return SteamUGC()->AddRequiredKeyValueTag(handle, key.utf8().get_data(), value.utf8().get_data());
}

// Adds a required tag to a pending UGC Query. This will only return UGC with the specified tag.
bool SteamServer::addRequiredTag(uint64_t query_handle, const String &tag_name) {
	if (SteamUGC() == NULL) {
		return false;
	}
	UGCQueryHandle_t handle = (uint64_t)query_handle;
	return SteamUGC()->AddRequiredTag(handle, tag_name.utf8().get_data());
}

// Adds the requirement that the returned items from the pending UGC Query have at least one of the tags in the given set (logical "or"). For each tag group that is added, at least one tag from each group is required to be on the matching items.
bool SteamServer::addRequiredTagGroup(uint64_t query_handle, Array tag_array) {
	bool added_tag_group = false;
	if (SteamUGC() != NULL) {
		UGCQueryHandle_t handle = uint64(query_handle);
		SteamParamStringArray_t *tags = new SteamParamStringArray_t();
		tags->m_ppStrings = new const char *[tag_array.size()];
		uint32 strCount = tag_array.size();
		for (uint32 i = 0; i < strCount; i++) {
			String str = (String)tag_array[i];
			tags->m_ppStrings[i] = str.utf8().get_data();
		}
		tags->m_nNumStrings = tag_array.size();
		added_tag_group = SteamUGC()->AddRequiredTagGroup(handle, tags);
		delete tags;
	}
	return added_tag_group;
}

// Lets game servers set a specific workshop folder before issuing any UGC commands.
bool SteamServer::initWorkshopForGameServer(uint32_t workshop_depot_id) {
	bool initialized_workshop = false;
	if (SteamUGC() != NULL) {
		DepotId_t workshop = (uint32_t)workshop_depot_id;
		const char *folder = new char[256];
		initialized_workshop = SteamUGC()->BInitWorkshopForGameServer(workshop, (char *)folder);
		delete[] folder;
	}
	return initialized_workshop;
}

// Creates a new workshop item with no content attached yet.
void SteamServer::createItem(uint32 app_id, WorkshopFileType file_type) {
	if (SteamUGC() != NULL) {
		SteamAPICall_t api_call = SteamUGC()->CreateItem((AppId_t)app_id, (EWorkshopFileType)file_type);
		callResultItemCreate.Set(api_call, this, &SteamServer::item_created);
	}
}

// Query for all matching UGC. You can use this to list all of the available UGC for your app.
uint64_t SteamServer::createQueryAllUGCRequest(UGCQuery query_type, UGCMatchingUGCType matching_type, uint32_t creator_id, uint32_t consumer_id, uint32 page) {
	if (SteamUGC() == NULL) {
		return 0;
	}
	EUGCQuery query;
	if (query_type == 0) {
		query = k_EUGCQuery_RankedByVote;
	} else if (query_type == 1) {
		query = k_EUGCQuery_RankedByPublicationDate;
	} else if (query_type == 2) {
		query = k_EUGCQuery_AcceptedForGameRankedByAcceptanceDate;
	} else if (query_type == 3) {
		query = k_EUGCQuery_RankedByTrend;
	} else if (query_type == 4) {
		query = k_EUGCQuery_FavoritedByFriendsRankedByPublicationDate;
	} else if (query_type == 5) {
		query = k_EUGCQuery_CreatedByFriendsRankedByPublicationDate;
	} else if (query_type == 6) {
		query = k_EUGCQuery_RankedByNumTimesReported;
	} else if (query_type == 7) {
		query = k_EUGCQuery_CreatedByFollowedUsersRankedByPublicationDate;
	} else if (query_type == 8) {
		query = k_EUGCQuery_NotYetRated;
	} else if (query_type == 9) {
		query = k_EUGCQuery_RankedByTotalVotesAsc;
	} else if (query_type == 10) {
		query = k_EUGCQuery_RankedByVotesUp;
	} else if (query_type == 11) {
		query = k_EUGCQuery_RankedByTextSearch;
	} else if (query_type == 12) {
		query = k_EUGCQuery_RankedByTotalUniqueSubscriptions;
	} else if (query_type == 13) {
		query = k_EUGCQuery_RankedByPlaytimeTrend;
	} else if (query_type == 14) {
		query = k_EUGCQuery_RankedByTotalPlaytime;
	} else if (query_type == 15) {
		query = k_EUGCQuery_RankedByAveragePlaytimeTrend;
	} else if (query_type == 16) {
		query = k_EUGCQuery_RankedByLifetimeAveragePlaytime;
	} else if (query_type == 17) {
		query = k_EUGCQuery_RankedByPlaytimeSessionsTrend;
	} else {
		query = k_EUGCQuery_RankedByLifetimePlaytimeSessions;
	}
	EUGCMatchingUGCType match;
	if (matching_type == 0) {
		match = k_EUGCMatchingUGCType_All;
	} else if (matching_type == 1) {
		match = k_EUGCMatchingUGCType_Items_Mtx;
	} else if (matching_type == 2) {
		match = k_EUGCMatchingUGCType_Items_ReadyToUse;
	} else if (matching_type == 3) {
		match = k_EUGCMatchingUGCType_Collections;
	} else if (matching_type == 4) {
		match = k_EUGCMatchingUGCType_Artwork;
	} else if (matching_type == 5) {
		match = k_EUGCMatchingUGCType_Videos;
	} else if (matching_type == 6) {
		match = k_EUGCMatchingUGCType_Screenshots;
	} else if (matching_type == 7) {
		match = k_EUGCMatchingUGCType_AllGuides;
	} else if (matching_type == 8) {
		match = k_EUGCMatchingUGCType_WebGuides;
	} else if (matching_type == 9) {
		match = k_EUGCMatchingUGCType_IntegratedGuides;
	} else if (matching_type == 10) {
		match = k_EUGCMatchingUGCType_UsableInGame;
	} else if (matching_type == 11) {
		match = k_EUGCMatchingUGCType_ControllerBindings;
	} else {
		match = k_EUGCMatchingUGCType_GameManagedItems;
	}
	AppId_t creator = (uint32_t)creator_id;
	AppId_t consumer = (uint32_t)consumer_id;
	UGCQueryHandle_t handle = SteamUGC()->CreateQueryAllUGCRequest(query, match, creator, consumer, page);
	return (uint64_t)handle;
}

// Query for the details of specific workshop items.
uint64_t SteamServer::createQueryUGCDetailsRequest(Array published_file_ids) {
	uint64_t this_handle = 0;
	if (SteamUGC() != NULL) {
		uint32 fileCount = published_file_ids.size();
		if (fileCount != 0) {
			PublishedFileId_t *file_ids = new PublishedFileId_t[fileCount];
			for (uint32 i = 0; i < fileCount; i++) {
				file_ids[i] = (uint64_t)published_file_ids[i];
			}
			UGCQueryHandle_t handle = SteamUGC()->CreateQueryUGCDetailsRequest(file_ids, fileCount);
			delete[] file_ids;
			this_handle = (uint64_t)handle;
		}
	}
	return this_handle;
}

// Query UGC associated with a user. You can use this to list the UGC the user is subscribed to amongst other things.
uint64_t SteamServer::createQueryUserUGCRequest(uint64_t steam_id, UserUGCList list_type, UGCMatchingUGCType matching_ugc_type, UserUGCListSortOrder sort_order, uint32_t creator_id, uint32_t consumer_id, uint32 page) {
	if (SteamUGC() == NULL) {
		return 0;
	}
	// Get tue universe ID from the Steam ID
	CSteamID user_id = (uint64)steam_id;
	AccountID_t account = (AccountID_t)user_id.ConvertToUint64();
	EUserUGCList list;
	if (list_type == 0) {
		list = k_EUserUGCList_Published;
	} else if (list_type == 1) {
		list = k_EUserUGCList_VotedOn;
	} else if (list_type == 2) {
		list = k_EUserUGCList_VotedUp;
	} else if (list_type == 3) {
		list = k_EUserUGCList_VotedDown;
	} else if (list_type == 4) {
		list = k_EUserUGCList_WillVoteLater;
	} else if (list_type == 5) {
		list = k_EUserUGCList_Favorited;
	} else if (list_type == 6) {
		list = k_EUserUGCList_Subscribed;
	} else if (list_type == 7) {
		list = k_EUserUGCList_UsedOrPlayed;
	} else {
		list = k_EUserUGCList_Followed;
	}
	EUGCMatchingUGCType match;
	if (matching_ugc_type == 0) {
		match = k_EUGCMatchingUGCType_All;
	} else if (matching_ugc_type == 1) {
		match = k_EUGCMatchingUGCType_Items_Mtx;
	} else if (matching_ugc_type == 2) {
		match = k_EUGCMatchingUGCType_Items_ReadyToUse;
	} else if (matching_ugc_type == 3) {
		match = k_EUGCMatchingUGCType_Collections;
	} else if (matching_ugc_type == 4) {
		match = k_EUGCMatchingUGCType_Artwork;
	} else if (matching_ugc_type == 5) {
		match = k_EUGCMatchingUGCType_Videos;
	} else if (matching_ugc_type == 6) {
		match = k_EUGCMatchingUGCType_Screenshots;
	} else if (matching_ugc_type == 7) {
		match = k_EUGCMatchingUGCType_AllGuides;
	} else if (matching_ugc_type == 8) {
		match = k_EUGCMatchingUGCType_WebGuides;
	} else if (matching_ugc_type == 9) {
		match = k_EUGCMatchingUGCType_IntegratedGuides;
	} else if (matching_ugc_type == 10) {
		match = k_EUGCMatchingUGCType_UsableInGame;
	} else if (matching_ugc_type == 11) {
		match = k_EUGCMatchingUGCType_ControllerBindings;
	} else {
		match = k_EUGCMatchingUGCType_GameManagedItems;
	}
	EUserUGCListSortOrder sort;
	if (sort_order == 0) {
		sort = k_EUserUGCListSortOrder_CreationOrderDesc;
	} else if (sort_order == 1) {
		sort = k_EUserUGCListSortOrder_CreationOrderAsc;
	} else if (sort_order == 2) {
		sort = k_EUserUGCListSortOrder_TitleAsc;
	} else if (sort_order == 3) {
		sort = k_EUserUGCListSortOrder_LastUpdatedDesc;
	} else if (sort_order == 4) {
		sort = k_EUserUGCListSortOrder_SubscriptionDateDesc;
	} else if (sort_order == 5) {
		sort = k_EUserUGCListSortOrder_VoteScoreDesc;
	} else {
		sort = k_EUserUGCListSortOrder_ForModeration;
	}
	AppId_t creator = (int)creator_id;
	AppId_t consumer = (int)consumer_id;
	UGCQueryHandle_t handle = SteamUGC()->CreateQueryUserUGCRequest(account, list, match, sort, creator, consumer, page);
	return (uint64_t)handle;
}

// Deletes the item without prompting the user.
void SteamServer::deleteItem(uint64_t published_file_id) {
	if (SteamUGC() != NULL) {
		PublishedFileId_t file_id = (uint64_t)published_file_id;
		SteamAPICall_t api_call = SteamUGC()->DeleteItem(file_id);
		callResultDeleteItem.Set(api_call, this, &SteamServer::item_deleted);
	}
}

// Download new or update already installed item. If returns true, wait for DownloadItemResult_t. If item is already installed, then files on disk should not be used until callback received.
// If item is not subscribed to, it will be cached for some time. If bHighPriority is set, any other item download will be suspended and this item downloaded ASAP.
bool SteamServer::downloadItem(uint64_t published_file_id, bool high_priority) {
	if (SteamUGC() == NULL) {
		return false;
	}
	PublishedFileId_t file_id = (uint64_t)published_file_id;
	return SteamUGC()->DownloadItem(file_id, high_priority);
}

// Get info about a pending download of a workshop item that has k_EItemStateNeedsUpdate set.
Dictionary SteamServer::getItemDownloadInfo(uint64_t published_file_id) {
	Dictionary info;
	if (SteamUGC() == NULL) {
		return info;
	}
	uint64 downloaded = 0;
	uint64 total = 0;
	info["ret"] = SteamUGC()->GetItemDownloadInfo((PublishedFileId_t)published_file_id, &downloaded, &total);
	if (info["ret"]) {
		info["downloaded"] = uint64_t(downloaded);
		info["total"] = uint64_t(total);
	}
	return info;
}

// Gets info about currently installed content on the disc for workshop items that have k_EItemStateInstalled set.
Dictionary SteamServer::getItemInstallInfo(uint64_t published_file_id) {
	Dictionary info;
	if (SteamUGC() == NULL) {
		info["ret"] = false;
		return info;
	}
	PublishedFileId_t file_id = (uint64_t)published_file_id;
	uint64 sizeOnDisk;
	char folder[1024] = { 0 };
	uint32 timeStamp;
	info["ret"] = SteamUGC()->GetItemInstallInfo((PublishedFileId_t)file_id, &sizeOnDisk, folder, sizeof(folder), &timeStamp);
	if (info["ret"]) {
		info["size"] = (int)sizeOnDisk;
		info["folder"] = folder;
		info["foldersize"] = (uint32)sizeof(folder);
		info["timestamp"] = timeStamp;
	}
	return info;
}

// Gets the current state of a workshop item on this client.
uint32 SteamServer::getItemState(uint64_t published_file_id) {
	if (SteamUGC() == NULL) {
		return 0;
	}
	PublishedFileId_t file_id = (uint64_t)published_file_id;
	return SteamUGC()->GetItemState(file_id);
}

// Gets the progress of an item update.
Dictionary SteamServer::getItemUpdateProgress(uint64_t update_handle) {
	Dictionary updateProgress;
	if (SteamUGC() == NULL) {
		return updateProgress;
	}
	UGCUpdateHandle_t handle = (uint64_t)update_handle;
	uint64 processed = 0;
	uint64 total = 0;
	EItemUpdateStatus status = SteamUGC()->GetItemUpdateProgress(handle, &processed, &total);
	updateProgress["status"] = status;
	updateProgress["processed"] = uint64_t(processed);
	updateProgress["total"] = uint64_t(total);
	return updateProgress;
}

// Gets the total number of items the current user is subscribed to for the game or application.
uint32 SteamServer::getNumSubscribedItems() {
	if (SteamUser() == NULL) {
		return 0;
	}
	return SteamUGC()->GetNumSubscribedItems();
}

// Retrieve the details of an additional preview associated with an individual workshop item after receiving a querying UGC call result.
Dictionary SteamServer::getQueryUGCAdditionalPreview(uint64_t query_handle, uint32 index, uint32 preview_index) {
	Dictionary preview;
	if (SteamUGC() == NULL) {
		return preview;
	}
	UGCQueryHandle_t handle = (uint64_t)query_handle;
	char *url_or_video_id = new char[256];
	char *original_filename = new char[256];
	EItemPreviewType previewType;
	bool success = SteamUGC()->GetQueryUGCAdditionalPreview(handle, index, preview_index, (char *)url_or_video_id, 256, (char *)original_filename, 256, &previewType);
	if (success) {
		preview["success"] = success;
		preview["handle"] = (uint64_t)handle;
		preview["index"] = index;
		preview["preview"] = preview_index;
		preview["urlOrVideo"] = url_or_video_id;
		preview["filename"] = original_filename;
		preview["type"] = previewType;
	}
	delete[] url_or_video_id;
	delete[] original_filename;
	return preview;
}

// Retrieve the ids of any child items of an individual workshop item after receiving a querying UGC call result. These items can either be a part of a collection or some other dependency (see AddDependency).
Dictionary SteamServer::getQueryUGCChildren(uint64_t query_handle, uint32 index, uint32_t child_count) {
	Dictionary children;
	if (SteamUGC() == NULL) {
		return children;
	}
	UGCQueryHandle_t handle = (uint64_t)query_handle;
	PackedVector2Array vec;
	vec.resize(child_count);
	bool success = SteamUGC()->GetQueryUGCChildren(handle, index, (PublishedFileId_t *)vec.ptrw(), child_count);
	if (success) {
		Array godot_arr;
		godot_arr.resize(child_count);
		for (uint32_t i = 0; i < child_count; i++) {
			godot_arr[i] = vec[i];
		}

		children["success"] = success;
		children["handle"] = (uint64_t)handle;
		children["index"] = index;
		children["children"] = godot_arr;
	}
	return children;
}

Dictionary SteamServer::getQueryUGCContentDescriptors(uint64_t query_handle, uint32 index, uint32_t max_entries) {
	Dictionary descriptors;
	if (SteamUGC() != NULL) {
		UGCQueryHandle_t handle = (uint64_t)query_handle;
		PackedVector2Array vec;
		vec.resize(max_entries);
		uint32_t result = SteamUGC()->GetQueryUGCContentDescriptors(handle, index, (EUGCContentDescriptorID *)vec.ptrw(), max_entries);
		Array descriptor_array;
		descriptor_array.resize(max_entries);
		for (uint32_t i = 0; i < max_entries; i++) {
			descriptor_array[i] = vec[i];
		}
		descriptors["result"] = result;
		descriptors["handle"] = (uint64_t)handle;
		descriptors["index"] = index;
		descriptors["descriptors"] = descriptor_array;
	}
	return descriptors;
}

// Retrieve the details of a key-value tag associated with an individual workshop item after receiving a querying UGC call result.
Dictionary SteamServer::getQueryUGCKeyValueTag(uint64_t query_handle, uint32 index, uint32 key_value_tag_index) {
	Dictionary tag;
	if (SteamUGC() == NULL) {
		return tag;
	}
	UGCQueryHandle_t handle = (uint64_t)query_handle;
	char *key = new char[256];
	char *value = new char[256];
	bool success = SteamUGC()->GetQueryUGCKeyValueTag(handle, index, key_value_tag_index, (char *)key, 256, (char *)value, 256);
	if (success) {
		tag["success"] = success;
		tag["handle"] = (uint64_t)handle;
		tag["index"] = index;
		tag["tag"] = key_value_tag_index;
		tag["key"] = key;
		tag["value"] = value;
	}
	delete[] key;
	delete[] value;
	return tag;
}

// Retrieve the developer set metadata of an individual workshop item after receiving a querying UGC call result.
String SteamServer::getQueryUGCMetadata(uint64_t query_handle, uint32 index) {
	String query_ugc_metadata = "";
	if (SteamUGC() != NULL) {
		UGCQueryHandle_t handle = (uint64_t)query_handle;
		char *ugc_metadata = new char[5000];
		bool success = SteamUGC()->GetQueryUGCMetadata(handle, index, (char *)ugc_metadata, 5000);
		if (success) {
			query_ugc_metadata = ugc_metadata;
		}
		delete[] ugc_metadata;
	}
	return query_ugc_metadata;
}

// Retrieve the number of additional previews of an individual workshop item after receiving a querying UGC call result.
uint32 SteamServer::getQueryUGCNumAdditionalPreviews(uint64_t query_handle, uint32 index) {
	if (SteamUser() == NULL) {
		return 0;
	}
	UGCQueryHandle_t handle = (uint64_t)query_handle;
	return SteamUGC()->GetQueryUGCNumAdditionalPreviews(handle, index);
}

// Retrieve the number of key-value tags of an individual workshop item after receiving a querying UGC call result.
uint32 SteamServer::getQueryUGCNumKeyValueTags(uint64_t query_handle, uint32 index) {
	if (SteamUser() == NULL) {
		return 0;
	}
	UGCQueryHandle_t handle = (uint64_t)query_handle;
	return SteamUGC()->GetQueryUGCNumKeyValueTags(handle, index);
}

// Retrieve the number of tags for an individual workshop item after receiving a querying UGC call result. You should call this in a loop to get the details of all the workshop items returned.
uint32 SteamServer::getQueryUGCNumTags(uint64_t query_handle, uint32 index) {
	if (SteamUGC() == NULL) {
		return 0;
	}
	UGCQueryHandle_t handle = (uint64_t)query_handle;
	return SteamUGC()->GetQueryUGCNumTags(handle, index);
}

// Retrieve the URL to the preview image of an individual workshop item after receiving a querying UGC call result.
String SteamServer::getQueryUGCPreviewURL(uint64_t query_handle, uint32 index) {
	String query_ugc_preview_url = "";
	if (SteamUGC() != NULL) {
		UGCQueryHandle_t handle = (uint64_t)query_handle;
		char *url = new char[256];
		bool success = SteamUGC()->GetQueryUGCPreviewURL(handle, index, (char *)url, 256);
		if (success) {
			query_ugc_preview_url = url;
		}
		delete[] url;
	}
	return query_ugc_preview_url;
}

// Retrieve the details of an individual workshop item after receiving a querying UGC call result.
Dictionary SteamServer::getQueryUGCResult(uint64_t query_handle, uint32 index) {
	Dictionary ugcResult;
	if (SteamUGC() == NULL) {
		return ugcResult;
	}
	UGCQueryHandle_t handle = (uint64_t)query_handle;
	SteamUGCDetails_t pDetails;
	bool success = SteamUGC()->GetQueryUGCResult(handle, index, &pDetails);
	if (success) {
		ugcResult["result"] = (uint64_t)pDetails.m_eResult;
		ugcResult["file_id"] = (uint64_t)pDetails.m_nPublishedFileId;
		ugcResult["file_type"] = (uint64_t)pDetails.m_eFileType;
		ugcResult["creator_app_id"] = (uint32_t)pDetails.m_nCreatorAppID;
		ugcResult["consumer_app_id"] = (uint32_t)pDetails.m_nConsumerAppID;
		ugcResult["title"] = String::utf8(pDetails.m_rgchTitle);
		ugcResult["description"] = String::utf8(pDetails.m_rgchDescription);
		ugcResult["steam_id_owner"] = (uint64_t)pDetails.m_ulSteamIDOwner;
		ugcResult["time_created"] = pDetails.m_rtimeCreated;
		ugcResult["time_updated"] = pDetails.m_rtimeUpdated;
		ugcResult["time_added_to_user_list"] = pDetails.m_rtimeAddedToUserList;
		ugcResult["visibility"] = (uint64_t)pDetails.m_eVisibility;
		ugcResult["banned"] = pDetails.m_bBanned;
		ugcResult["accepted_for_use"] = pDetails.m_bAcceptedForUse;
		ugcResult["tags_truncated"] = pDetails.m_bTagsTruncated;
		ugcResult["tags"] = pDetails.m_rgchTags;
		ugcResult["handle_file"] = (uint64_t)pDetails.m_hFile;
		ugcResult["handle_preview_file"] = (uint64_t)pDetails.m_hPreviewFile;
		ugcResult["file_name"] = pDetails.m_pchFileName;
		ugcResult["file_size"] = pDetails.m_nFileSize;
		ugcResult["preview_file_size"] = pDetails.m_nPreviewFileSize;
		ugcResult["url"] = pDetails.m_rgchURL;
		ugcResult["votes_up"] = pDetails.m_unVotesUp;
		ugcResult["votes_down"] = pDetails.m_unVotesDown;
		ugcResult["score"] = pDetails.m_flScore;
		ugcResult["num_children"] = pDetails.m_unNumChildren;
	}
	return ugcResult;
}

// Retrieve various statistics of an individual workshop item after receiving a querying UGC call result.
Dictionary SteamServer::getQueryUGCStatistic(uint64_t query_handle, uint32 index, ItemStatistic stat_type) {
	Dictionary ugcStat;
	if (SteamUGC() == NULL) {
		return ugcStat;
	}
	UGCQueryHandle_t handle = (uint64_t)query_handle;
	EItemStatistic type;
	if (stat_type == 0) {
		type = k_EItemStatistic_NumSubscriptions;
	} else if (stat_type == 1) {
		type = k_EItemStatistic_NumFavorites;
	} else if (stat_type == 2) {
		type = k_EItemStatistic_NumFollowers;
	} else if (stat_type == 3) {
		type = k_EItemStatistic_NumUniqueSubscriptions;
	} else if (stat_type == 4) {
		type = k_EItemStatistic_NumUniqueFavorites;
	} else if (stat_type == 5) {
		type = k_EItemStatistic_NumUniqueFollowers;
	} else if (stat_type == 6) {
		type = k_EItemStatistic_NumUniqueWebsiteViews;
	} else if (stat_type == 7) {
		type = k_EItemStatistic_ReportScore;
	} else if (stat_type == 8) {
		type = k_EItemStatistic_NumSecondsPlayed;
	} else if (stat_type == 9) {
		type = k_EItemStatistic_NumPlaytimeSessions;
	} else if (stat_type == 10) {
		type = k_EItemStatistic_NumComments;
	} else if (stat_type == 11) {
		type = k_EItemStatistic_NumSecondsPlayedDuringTimePeriod;
	} else {
		type = k_EItemStatistic_NumPlaytimeSessionsDuringTimePeriod;
	}
	uint64 value = 0;
	bool success = SteamUGC()->GetQueryUGCStatistic(handle, index, type, &value);
	if (success) {
		ugcStat["success"] = success;
		ugcStat["handle"] = (uint64_t)handle;
		ugcStat["index"] = index;
		ugcStat["type"] = type;
		ugcStat["value"] = (uint64_t)value;
	}
	return ugcStat;
}

// Retrieve the "nth" tag associated with an individual workshop item after receiving a querying UGC call result.
// You should call this in a loop to get the details of all the workshop items returned.
String SteamServer::getQueryUGCTag(uint64_t query_handle, uint32 index, uint32 tag_index) {
	if (SteamUGC() == NULL) {
		return "";
	}
	// Set a default tag to return
	char *tag = new char[64];
	UGCQueryHandle_t handle = (uint64_t)query_handle;
	SteamUGC()->GetQueryUGCTag(handle, index, tag_index, tag, 64);
	tag[63] = '\0';
	String tag_name = tag;
	delete[] tag;
	return tag_name;
}

// Retrieve the "nth" display string (usually localized) for a tag, which is associated with an individual workshop item after receiving a querying UGC call result.
// You should call this in a loop to get the details of all the workshop items returned.
String SteamServer::getQueryUGCTagDisplayName(uint64_t query_handle, uint32 index, uint32 tag_index) {
	if (SteamUGC() == NULL) {
		return "";
	}
	// Set a default tag name to return
	char *tag = new char[256];
	UGCQueryHandle_t handle = (uint64_t)query_handle;
	SteamUGC()->GetQueryUGCTagDisplayName(handle, index, tag_index, tag, 256);
	tag[255] = '\0';
	String tagDisplay = tag;
	delete[] tag;
	return tagDisplay;
}

// Gets a list of all of the items the current user is subscribed to for the current game.
Array SteamServer::getSubscribedItems() {
	if (SteamUGC() == NULL) {
		return Array();
	}
	Array subscribed;
	uint32 num_items = SteamUGC()->GetNumSubscribedItems();
	PublishedFileId_t *items = new PublishedFileId_t[num_items];
	uint32 item_list = SteamUGC()->GetSubscribedItems(items, num_items);
	for (uint32 i = 0; i < item_list; i++) {
		subscribed.append((uint64_t)items[i]);
	}
	delete[] items;
	return subscribed;
}

// Return the user's community content descriptor preferences
// Information is unclear how this actually works so here goes nothing!
Array SteamServer::getUserContentDescriptorPreferences(uint32 max_entries){
	Array descriptors;
	if(SteamUGC() != NULL){
		EUGCContentDescriptorID *descriptor_list = new EUGCContentDescriptorID[max_entries];
		uint32 num_descriptors = SteamUGC()->GetUserContentDescriptorPreferences(descriptor_list, max_entries);
		for(uint32 i = 0; i < num_descriptors; i++){
			descriptors.append(descriptor_list[i]);
		}
	}
	return descriptors;
}

// Gets the users vote status on a workshop item.
void SteamServer::getUserItemVote(uint64_t published_file_id) {
	if (SteamUGC() != NULL) {
		PublishedFileId_t file_id = (uint64_t)published_file_id;
		SteamAPICall_t api_call = SteamUGC()->GetUserItemVote(file_id);
		callResultGetUserItemVote.Set(api_call, this, &SteamServer::get_item_vote_result);
	}
}

// Releases a UGC query handle when you are done with it to free up memory.
bool SteamServer::releaseQueryUGCRequest(uint64_t query_handle) {
	if (SteamUGC() == NULL) {
		return false;
	}
	UGCQueryHandle_t handle = (uint64_t)query_handle;
	return SteamUGC()->ReleaseQueryUGCRequest(handle);
}

// Removes the dependency between the given item and the appid. This list of dependencies can be retrieved by calling GetAppDependencies.
void SteamServer::removeAppDependency(uint64_t published_file_id, uint32_t app_id) {
	if (SteamUGC() != NULL) {
		PublishedFileId_t file_id = (uint64_t)published_file_id;
		AppId_t app = (uint32_t)app_id;
		SteamAPICall_t api_call = SteamUGC()->RemoveAppDependency(file_id, app);
		callResultRemoveAppDependency.Set(api_call, this, &SteamServer::remove_app_dependency_result);
	}
}

bool SteamServer::removeContentDescriptor(uint64_t update_handle, int descriptor_id) {
	if (SteamUGC() == NULL) {
		return false;
	}
	return SteamUGC()->RemoveContentDescriptor((UGCUpdateHandle_t)update_handle, (EUGCContentDescriptorID)descriptor_id);
}

// Removes a workshop item as a dependency from the specified item.
void SteamServer::removeDependency(uint64_t published_file_id, uint64_t child_published_file_id) {
	if (SteamUGC() != NULL) {
		PublishedFileId_t file_id = (uint64_t)published_file_id;
		PublishedFileId_t childID = (uint64_t)child_published_file_id;
		SteamAPICall_t api_call = SteamUGC()->RemoveDependency(file_id, childID);
		callResultRemoveUGCDependency.Set(api_call, this, &SteamServer::remove_ugc_dependency_result);
	}
}

// Removes a workshop item from the users favorites list.
void SteamServer::removeItemFromFavorites(uint32_t app_id, uint64_t published_file_id) {
	if (SteamUGC() != NULL) {
		PublishedFileId_t file_id = (uint64_t)published_file_id;
		AppId_t app = (uint32_t)app_id;
		SteamAPICall_t api_call = SteamUGC()->RemoveItemFromFavorites(app, file_id);
		callResultFavoriteItemListChanged.Set(api_call, this, &SteamServer::user_favorite_items_list_changed);
	}
}

// Removes an existing key value tag from an item.
bool SteamServer::removeItemKeyValueTags(uint64_t update_handle, const String &key) {
	if (SteamUGC() == NULL) {
		return false;
	}
	UGCUpdateHandle_t handle = uint64(update_handle);
	return SteamUGC()->RemoveItemKeyValueTags(handle, key.utf8().get_data());
}

// Removes an existing preview from an item.
bool SteamServer::removeItemPreview(uint64_t update_handle, uint32 index) {
	if (SteamUGC() == NULL) {
		return false;
	}
	UGCUpdateHandle_t handle = uint64(update_handle);
	return SteamUGC()->RemoveItemPreview(handle, index);
}

// Send a UGC query to Steam.
void SteamServer::sendQueryUGCRequest(uint64_t update_handle) {
	if (SteamUGC() != NULL) {
		UGCUpdateHandle_t handle = uint64(update_handle);
		SteamAPICall_t api_call = SteamUGC()->SendQueryUGCRequest(handle);
		callResultUGCQueryCompleted.Set(api_call, this, &SteamServer::ugc_query_completed);
	}
}

// Sets whether results will be returned from the cache for the specific period of time on a pending UGC Query.
bool SteamServer::setAllowCachedResponse(uint64_t update_handle, uint32 max_age_seconds) {
	if (SteamUGC() == NULL) {
		return false;
	}
	UGCUpdateHandle_t handle = uint64(update_handle);
	return SteamUGC()->SetAllowCachedResponse(handle, max_age_seconds);
}

// Sets to only return items that have a specific filename on a pending UGC Query.
bool SteamServer::setCloudFileNameFilter(uint64_t update_handle, const String &match_cloud_filename) {
	if (SteamUGC() == NULL) {
		return false;
	}
	UGCUpdateHandle_t handle = uint64(update_handle);
	return SteamUGC()->SetCloudFileNameFilter(handle, match_cloud_filename.utf8().get_data());
}

// Sets the folder that will be stored as the content for an item.
bool SteamServer::setItemContent(uint64_t update_handle, const String &content_folder) {
	if (SteamUGC() == NULL) {
		return false;
	}
	UGCUpdateHandle_t handle = uint64(update_handle);
	return SteamUGC()->SetItemContent(handle, content_folder.utf8().get_data());
}

// Sets a new description for an item.
bool SteamServer::setItemDescription(uint64_t update_handle, const String &description) {
	if (SteamUGC() == NULL) {
		return false;
	}
	if ((uint32_t)description.length() > (uint32_t)k_cchPublishedDocumentDescriptionMax) {
		printf("Description cannot have more than %d ASCII characters. Description not set.", k_cchPublishedDocumentDescriptionMax);
		return false;
	}
	UGCUpdateHandle_t handle = uint64(update_handle);
	return SteamUGC()->SetItemDescription(handle, description.utf8().get_data());
}

// Sets arbitrary metadata for an item. This metadata can be returned from queries without having to download and install the actual content.
bool SteamServer::setItemMetadata(uint64_t update_handle, const String &ugc_metadata) {
	if (SteamUGC() == NULL) {
		return false;
	}
	if (ugc_metadata.utf8().length() > 5000) {
		printf("Metadata cannot be more than %d bytes. Metadata not set.", 5000);
	}
	UGCUpdateHandle_t handle = uint64(update_handle);
	return SteamUGC()->SetItemMetadata(handle, ugc_metadata.utf8().get_data());
}

// Sets the primary preview image for the item.
bool SteamServer::setItemPreview(uint64_t update_handle, const String &preview_file) {
	if (SteamUGC() == NULL) {
		return false;
	}
	UGCUpdateHandle_t handle = uint64(update_handle);
	return SteamUGC()->SetItemPreview(handle, preview_file.utf8().get_data());
}

// Sets arbitrary developer specified tags on an item.
bool SteamServer::setItemTags(uint64_t update_handle, Array tag_array, bool allow_admin_tags) {
	bool tags_set = false;
	if (SteamUGC() != NULL) {
		UGCUpdateHandle_t handle = uint64(update_handle);
		SteamParamStringArray_t *tags = new SteamParamStringArray_t();
		tags->m_ppStrings = new const char *[tag_array.size()];
		uint32 strCount = tag_array.size();
		for (uint32 i = 0; i < strCount; i++) {
			String str = (String)tag_array[i];
			tags->m_ppStrings[i] = str.utf8().get_data();
		}
		tags->m_nNumStrings = tag_array.size();
		tags_set = SteamUGC()->SetItemTags(handle, tags, allow_admin_tags);
		delete tags;
	}
	return tags_set;
}

// Sets a new title for an item.
bool SteamServer::setItemTitle(uint64_t update_handle, const String &title) {
	if (SteamUGC() == NULL) {
		return false;
	}
	if (title.length() > 255) {
		printf("Title cannot have more than %d ASCII characters. Title not set.", 255);
		return false;
	}
	UGCUpdateHandle_t handle = uint64(update_handle);
	return SteamUGC()->SetItemTitle(handle, title.utf8().get_data());
}

// Sets the language of the title and description that will be set in this item update.
bool SteamServer::setItemUpdateLanguage(uint64_t update_handle, const String &language) {
	if (SteamUGC() == NULL) {
		return false;
	}
	UGCUpdateHandle_t handle = uint64(update_handle);
	return SteamUGC()->SetItemUpdateLanguage(handle, language.utf8().get_data());
}

// Sets the visibility of an item.
bool SteamServer::setItemVisibility(uint64_t update_handle, RemoteStoragePublishedFileVisibility visibility) {
	if (SteamUGC() == NULL) {
		return false;
	}
	UGCUpdateHandle_t handle = uint64(update_handle);
	return SteamUGC()->SetItemVisibility(handle, (ERemoteStoragePublishedFileVisibility)visibility);
}

// Sets the language to return the title and description in for the items on a pending UGC Query.
bool SteamServer::setLanguage(uint64_t query_handle, const String &language) {
	if (SteamUGC() == NULL) {
		return false;
	}
	UGCQueryHandle_t handle = (uint64_t)query_handle;
	return SteamUGC()->SetLanguage(handle, language.utf8().get_data());
}

// Sets whether workshop items will be returned if they have one or more matching tag, or if all tags need to match on a pending UGC Query.
bool SteamServer::setMatchAnyTag(uint64_t query_handle, bool match_any_tag) {
	if (SteamUGC() == NULL) {
		return false;
	}
	UGCQueryHandle_t handle = (uint64_t)query_handle;
	return SteamUGC()->SetMatchAnyTag(handle, match_any_tag);
}

// Sets whether the order of the results will be updated based on the rank of items over a number of days on a pending UGC Query.
bool SteamServer::setRankedByTrendDays(uint64_t query_handle, uint32 days) {
	if (SteamUGC() == NULL) {
		return false;
	}
	UGCQueryHandle_t handle = (uint64_t)query_handle;
	return SteamUGC()->SetRankedByTrendDays(handle, days);
}

// Sets whether to return any additional images/videos attached to the items on a pending UGC Query.
bool SteamServer::setReturnAdditionalPreviews(uint64_t query_handle, bool return_additional_previews) {
	if (SteamUGC() == NULL) {
		return false;
	}
	UGCQueryHandle_t handle = (uint64_t)query_handle;
	return SteamUGC()->SetReturnAdditionalPreviews(handle, return_additional_previews);
}

// Sets whether to return the IDs of the child items of the items on a pending UGC Query.
bool SteamServer::setReturnChildren(uint64_t query_handle, bool return_children) {
	if (SteamUGC() == NULL) {
		return false;
	}
	UGCQueryHandle_t handle = (uint64_t)query_handle;
	return SteamUGC()->SetReturnChildren(handle, return_children);
}

// Sets whether to return any key-value tags for the items on a pending UGC Query.
bool SteamServer::setReturnKeyValueTags(uint64_t query_handle, bool return_key_value_tags) {
	if (SteamUGC() == NULL) {
		return false;
	}
	UGCQueryHandle_t handle = (uint64_t)query_handle;
	return SteamUGC()->SetReturnKeyValueTags(handle, return_key_value_tags);
}

// Sets whether to return the full description for the items on a pending UGC Query.
bool SteamServer::setReturnLongDescription(uint64_t query_handle, bool return_long_description) {
	if (SteamUGC() == NULL) {
		return false;
	}
	UGCQueryHandle_t handle = (uint64_t)query_handle;
	return SteamUGC()->SetReturnLongDescription(handle, return_long_description);
}

// Sets whether to return the developer specified metadata for the items on a pending UGC Query.
bool SteamServer::setReturnMetadata(uint64_t query_handle, bool return_metadata) {
	if (SteamUGC() == NULL) {
		return false;
	}
	UGCQueryHandle_t handle = (uint64_t)query_handle;
	return SteamUGC()->SetReturnMetadata(handle, return_metadata);
}

// Sets whether to only return IDs instead of all the details on a pending UGC Query.
bool SteamServer::setReturnOnlyIDs(uint64_t query_handle, bool return_only_ids) {
	if (SteamUGC() == NULL) {
		return false;
	}
	UGCQueryHandle_t handle = (uint64_t)query_handle;
	return SteamUGC()->SetReturnOnlyIDs(handle, return_only_ids);
}

// Sets whether to return the the playtime stats on a pending UGC Query.
bool SteamServer::setReturnPlaytimeStats(uint64_t query_handle, uint32 days) {
	if (SteamUGC() == NULL) {
		return false;
	}
	UGCQueryHandle_t handle = (uint64_t)query_handle;
	return SteamUGC()->SetReturnPlaytimeStats(handle, days);
}

// Sets whether to only return the the total number of matching items on a pending UGC Query.
bool SteamServer::setReturnTotalOnly(uint64_t query_handle, bool return_total_only) {
	if (SteamUGC() == NULL) {
		return false;
	}
	UGCQueryHandle_t handle = (uint64_t)query_handle;
	return SteamUGC()->SetReturnTotalOnly(handle, return_total_only);
}

// Sets a string to that items need to match in either the title or the description on a pending UGC Query.
bool SteamServer::setSearchText(uint64_t query_handle, const String &search_text) {
	if (SteamUGC() == NULL) {
		return false;
	}
	UGCQueryHandle_t handle = (uint64_t)query_handle;
	return SteamUGC()->SetSearchText(handle, search_text.utf8().get_data());
}

// Allows the user to rate a workshop item up or down.
void SteamServer::setUserItemVote(uint64_t published_file_id, bool vote_up) {
	if (SteamUGC() != NULL) {
		PublishedFileId_t file_id = (uint64_t)published_file_id;
		SteamAPICall_t api_call = SteamUGC()->SetUserItemVote(file_id, vote_up);
		callResultSetUserItemVote.Set(api_call, this, &SteamServer::set_user_item_vote);
	}
}

// Starts the item update process.
uint64_t SteamServer::startItemUpdate(uint32_t app_id, uint64_t published_file_id) {
	if (SteamUGC() == NULL) {
		return 0;
	}
	AppId_t app = (uint32_t)app_id;
	PublishedFileId_t file_id = (uint64_t)published_file_id;
	return SteamUGC()->StartItemUpdate(app, file_id);
}

// Start tracking playtime on a set of workshop items.
void SteamServer::startPlaytimeTracking(Array published_file_ids) {
	if (SteamUGC() != NULL) {
		uint32 fileCount = published_file_ids.size();
		if (fileCount > 0) {
			PublishedFileId_t *file_ids = new PublishedFileId_t[fileCount];
			for (uint32 i = 0; i < fileCount; i++) {
				file_ids[i] = (uint64_t)published_file_ids[i];
			}
			SteamAPICall_t api_call = SteamUGC()->StartPlaytimeTracking(file_ids, fileCount);
			callResultStartPlaytimeTracking.Set(api_call, this, &SteamServer::start_playtime_tracking);
			delete[] file_ids;
		}
	}
}

// Stop tracking playtime on a set of workshop items.
void SteamServer::stopPlaytimeTracking(Array published_file_ids) {
	if (SteamUGC() != NULL) {
		uint32 fileCount = published_file_ids.size();
		if (fileCount > 0) {
			PublishedFileId_t *file_ids = new PublishedFileId_t[fileCount];
			Array files;
			for (uint32 i = 0; i < fileCount; i++) {
				file_ids[i] = (uint64_t)published_file_ids[i];
			}
			SteamAPICall_t api_call = SteamUGC()->StopPlaytimeTracking(file_ids, fileCount);
			callResultStopPlaytimeTracking.Set(api_call, this, &SteamServer::stop_playtime_tracking);
			delete[] file_ids;
		}
	}
}

// Stop tracking playtime of all workshop items.
void SteamServer::stopPlaytimeTrackingForAllItems() {
	if (SteamUGC() != NULL) {
		SteamAPICall_t api_call = SteamUGC()->StopPlaytimeTrackingForAllItems();
		callResultStopPlaytimeTracking.Set(api_call, this, &SteamServer::stop_playtime_tracking);
	}
}

// Returns any app dependencies that are associated with the given item.
void SteamServer::getAppDependencies(uint64_t published_file_id) {
	if (SteamUGC() != NULL) {
		PublishedFileId_t file_id = (uint64_t)published_file_id;
		SteamAPICall_t api_call = SteamUGC()->GetAppDependencies(file_id);
		callResultGetAppDependencies.Set(api_call, this, &SteamServer::get_app_dependencies_result);
	}
}

// Uploads the changes made to an item to the Steam Workshop; to be called after setting your changes.
void SteamServer::submitItemUpdate(uint64_t update_handle, const String &change_note) {
	if (SteamUGC() != NULL) {
		UGCUpdateHandle_t handle = uint64(update_handle);
		SteamAPICall_t api_call;
		if (change_note.length() == 0) {
			api_call = SteamUGC()->SubmitItemUpdate(handle, NULL);
		} else {
			api_call = SteamUGC()->SubmitItemUpdate(handle, change_note.utf8().get_data());
		}
		callResultItemUpdate.Set(api_call, this, &SteamServer::item_updated);
	}
}

// Subscribe to a workshop item. It will be downloaded and installed as soon as possible.
void SteamServer::subscribeItem(uint64_t published_file_id) {
	if (SteamUGC() != NULL) {
		PublishedFileId_t file_id = (uint64_t)published_file_id;
		SteamAPICall_t api_call = SteamUGC()->SubscribeItem(file_id);
		callResultSubscribeItem.Set(api_call, this, &SteamServer::subscribe_item);
	}
}

// SuspendDownloads( true ) will suspend all workshop downloads until SuspendDownloads( false ) is called or the game ends.
void SteamServer::suspendDownloads(bool suspend) {
	if (SteamUGC() != NULL) {
		SteamUGC()->SuspendDownloads(suspend);
	}
}

// Unsubscribe from a workshop item. This will result in the item being removed after the game quits.
void SteamServer::unsubscribeItem(uint64_t published_file_id) {
	if (SteamUGC() != NULL) {
		PublishedFileId_t file_id = (uint64_t)published_file_id;
		SteamAPICall_t api_call = SteamUGC()->UnsubscribeItem(file_id);
		callResultUnsubscribeItem.Set(api_call, this, &SteamServer::unsubscribe_item);
	}
}

// Updates an existing additional preview file for the item.
bool SteamServer::updateItemPreviewFile(uint64_t update_handle, uint32 index, const String &preview_file) {
	if (SteamUGC() == NULL) {
		return false;
	}
	UGCUpdateHandle_t handle = uint64(update_handle);
	return SteamUGC()->UpdateItemPreviewFile(handle, index, preview_file.utf8().get_data());
}

// Updates an additional video preview from YouTube for the item.
bool SteamServer::updateItemPreviewVideo(uint64_t update_handle, uint32 index, const String &video_id) {
	if (SteamUGC() == NULL) {
		return false;
	}
	UGCUpdateHandle_t handle = uint64(update_handle);
	return SteamUGC()->UpdateItemPreviewVideo(handle, index, video_id.utf8().get_data());
}

// Show the app's latest Workshop EULA to the user in an overlay window, where they can accept it or not.
bool SteamServer::showWorkshopEULA() {
	if (SteamUGC() == NULL) {
		return false;
	}
	return SteamUGC()->ShowWorkshopEULA();
}

// Retrieve information related to the user's acceptance or not of the app's specific Workshop EULA.
void SteamServer::getWorkshopEULAStatus() {
	if (SteamUGC() != NULL) {
		SteamAPICall_t api_call = SteamUGC()->GetWorkshopEULAStatus();
		callResultWorkshopEULAStatus.Set(api_call, this, &SteamServer::workshop_eula_status);
	}
}

// Set the time range this item was created.
bool SteamServer::setTimeCreatedDateRange(uint64_t update_handle, uint32 start, uint32 end) {
	if (SteamUGC() == NULL) {
		return false;
	}
	UGCUpdateHandle_t handle = uint64(update_handle);
	return SteamUGC()->SetTimeCreatedDateRange(handle, start, end);
}

// Set the time range this item was updated.
bool SteamServer::setTimeUpdatedDateRange(uint64_t update_handle, uint32 start, uint32 end) {
	if (SteamUGC() == NULL) {
		return false;
	}
	UGCUpdateHandle_t handle = uint64(update_handle);
	return SteamUGC()->SetTimeUpdatedDateRange(handle, start, end);
}

/////////////////////////////////////////////////
///// USERS
/////////////////////////////////////////////////
//
// Set the rich presence data for an unsecured game server that the user is playing on. This allows friends to be able to view the game info and join your game.
void SteamServer::advertiseGame(const String &server_ip, int port) {
	if (SteamUser() != NULL) {
		// Resolve address and convert it from IPAddress struct to uint32_t
		IPAddress address;
		if (server_ip.is_valid_ip_address()) {
			address = server_ip;
		} else {
			address = IP::get_singleton()->resolve_hostname(server_ip, IP::TYPE_IPV4);
		}
		// Resolution failed
		if (address.is_valid()) {
			uint32_t ip4 = *((uint32_t *)address.get_ipv4());
			CSteamID gameserverID = SteamUser()->GetSteamID();
			SteamUser()->AdvertiseGame(gameserverID, ip4, port);
		}
	}
}

// Authenticate the ticket from the entity Steam ID to be sure it is valid and isn't reused.
SteamServer::BeginAuthSessionResult SteamServer::beginAuthSession(PackedByteArray ticket, int ticket_size, uint64_t steam_id) {
	if (SteamUser() == NULL) {
		return BeginAuthSessionResult(-1);
	}
	CSteamID auth_steam_id = createSteamID(steam_id);
	return BeginAuthSessionResult(SteamUser()->BeginAuthSession(ticket.ptr(), ticket_size, auth_steam_id));
}

// Cancels an auth ticket.
void SteamServer::cancelAuthTicket(uint32_t auth_ticket) {
	if (SteamUser() != NULL) {
		SteamUser()->CancelAuthTicket(auth_ticket);
	}
}

// Decodes the compressed voice data returned by GetVoice.
Dictionary SteamServer::decompressVoice(const PackedByteArray &voice, uint32 voice_size, uint32 sample_rate) {
	Dictionary decompressed;
	if (SteamUser() != NULL) {
		uint32 written = 0;
		PackedByteArray outputBuffer;
		outputBuffer.resize(20480); // 20KiB buffer
		int result = SteamUser()->DecompressVoice(voice.ptr(), voice_size, outputBuffer.ptrw(), outputBuffer.size(), &written, sample_rate);
		if (result == 0) {
			decompressed["uncompressed"] = outputBuffer;
			decompressed["size"] = written;
		}
		decompressed["result"] = result; // Include result for debugging
	}
	return decompressed;
}

// Ends an auth session.
void SteamServer::endAuthSession(uint64_t steam_id) {
	if (SteamUser() != NULL) {
		CSteamID auth_steam_id = createSteamID(steam_id);
		SteamUser()->EndAuthSession(auth_steam_id);
	}
}

// Get the authentication ticket data.
Dictionary SteamServer::getUserAuthSessionTicket(const String &identity_reference) {
	// Create the dictionary to use
	Dictionary auth_ticket;
	if (SteamUser() != NULL) {
		uint32_t id = 0;
		uint32_t ticket_size = 1024;
		PackedByteArray buffer;
		buffer.resize(ticket_size);
		// If no reference is passed, just use NULL
		// Not pretty but will work for now
		if (identity_reference != "") {
			const SteamNetworkingIdentity identity = networking_identities[identity_reference.utf8().get_data()];
			id = SteamUser()->GetAuthSessionTicket(buffer.ptrw(), ticket_size, &ticket_size, &identity);
		} else {
			id = SteamUser()->GetAuthSessionTicket(buffer.ptrw(), ticket_size, &ticket_size, NULL);
		}
		auth_ticket["id"] = id;
		auth_ticket["buffer"] = buffer;
		auth_ticket["size"] = ticket_size;
	}
	return auth_ticket;
}

// Request a ticket which will be used for webapi "ISteamUserAuth\AuthenticateUserTicket" pchIdentity is an optional input parameter to identify the service the ticket will be sent to the ticket will be returned in callback GetTicketForWebApiResponse_t
uint32 SteamServer::getAuthTicketForWebApi(const String &service_identity) {
	uint32 auth_ticket_handle = 0;
	if (SteamUser() != NULL) {
		if (service_identity != "") {
			auth_ticket_handle = SteamUser()->GetAuthTicketForWebApi(service_identity.utf8().get_data());
		} else {
			auth_ticket_handle = SteamUser()->GetAuthTicketForWebApi(NULL);
		}
	}
	return auth_ticket_handle;
}

// Checks to see if there is captured audio data available from GetVoice, and gets the size of the data.
Dictionary SteamServer::getAvailableVoice() {
	Dictionary voice_data;
	if (SteamUser() != NULL) {
		uint32 bytes_available = 0;
		int result = SteamUser()->GetAvailableVoice(&bytes_available, NULL, 0);
		// Add this data to the dictionary
		voice_data["result"] = result;
		voice_data["buffer"] = bytes_available;
	}
	return voice_data;
}

// Retrieves anti indulgence / duration control for current user / game combination.
void SteamServer::getDurationControl() {
	if (SteamUser() != NULL) {
		SteamAPICall_t api_call = SteamUser()->GetDurationControl();
		callResultDurationControl.Set(api_call, this, &SteamServer::duration_control);
	}
}

// Retrieve an encrypted ticket. This should be called after requesting an encrypted app ticket with RequestEncryptedAppTicket and receiving the EncryptedAppTicketResponse_t call result.
Dictionary SteamServer::getEncryptedAppTicket() {
	Dictionary encrypted;
	if (SteamUser() != NULL) {
		uint32_t ticket_size = 1024;
		PackedByteArray buffer;
		buffer.resize(ticket_size);
		if (SteamUser()->GetEncryptedAppTicket(buffer.ptrw(), ticket_size, &ticket_size)) {
			encrypted["buffer"] = buffer;
			encrypted["size"] = ticket_size;
		}
	}
	return encrypted;
}

// Trading Card badges data access, if you only have one set of cards, the series will be 1.
// The user has can have two different badges for a series; the regular (max level 5) and the foil (max level 1).
int SteamServer::getGameBadgeLevel(int series, bool foil) {
	if (SteamUser() == NULL) {
		return 0;
	}
	return SteamUser()->GetGameBadgeLevel(series, foil);
}

// Get the user's Steam level.
int SteamServer::getPlayerSteamLevel() {
	if (SteamUser() == NULL) {
		return 0;
	}
	return SteamUser()->GetPlayerSteamLevel();
}

// Get user's Steam ID.
uint64_t SteamServer::getSteamID() {
	if (SteamUser() == NULL) {
		return 0;
	}
	CSteamID steam_id = SteamUser()->GetSteamID();
	return steam_id.ConvertToUint64();
}

// Read captured audio data from the microphone buffer.
Dictionary SteamServer::getVoice() {
	Dictionary voice_data;
	if (SteamUser() != NULL) {
		uint32 written = 0;
		PackedByteArray buffer = PackedByteArray();
		buffer.resize(8192); // Steam suggests 8Kb of buffer space per call
		int result = SteamUser()->GetVoice(true, buffer.ptrw(), 8192, &written, false, NULL, 0, NULL, 0);
		buffer.resize(written);
		// Add the data to the dictionary
		voice_data["result"] = result;
		voice_data["written"] = written;
		voice_data["buffer"] = buffer;
	}
	return voice_data;
}

// Gets the native sample rate of the Steam voice decoder.
uint32 SteamServer::getVoiceOptimalSampleRate() {
	if (SteamUser() == NULL) {
		return 0;
	}
	return SteamUser()->GetVoiceOptimalSampleRate();
}

// This starts the state machine for authenticating the game client with the game server. It is the client portion of a three-way handshake between the client, the game server, and the steam servers.
Dictionary SteamServer::initiateGameConnection(uint64_t server_id, uint32 server_ip, uint16 server_port, bool secure) {
	Dictionary connection;
	if (SteamUser() != NULL) {
		PackedByteArray auth;
		int authSize = 2048;
		auth.resize(authSize);
		CSteamID server = (uint64)server_id;
		if (SteamUser()->InitiateGameConnection_DEPRECATED(&auth, authSize, server, server_ip, server_port, secure) > 0) {
			connection["auth_blob"] = auth;
			connection["server_id"] = server_id;
			connection["server_ip"] = server_ip;
			connection["server_port"] = server_port;
		}
	}
	return connection;
}

// Checks if the current users looks like they are behind a NAT device.
bool SteamServer::isBehindNAT() {
	if (SteamUser() == NULL) {
		return false;
	}
	return SteamUser()->BIsBehindNAT();
}

// Checks whether the user's phone number is used to uniquely identify them.
bool SteamServer::isPhoneIdentifying() {
	if (SteamUser() == NULL) {
		return false;
	}
	return SteamUser()->BIsPhoneIdentifying();
}

// Checks whether the current user's phone number is awaiting (re)verification.
bool SteamServer::isPhoneRequiringVerification() {
	if (SteamUser() == NULL) {
		return false;
	}
	return SteamUser()->BIsPhoneRequiringVerification();
}

// Checks whether the current user has verified their phone number.
bool SteamServer::isPhoneVerified() {
	if (SteamUser() == NULL) {
		return false;
	}
	return SteamUser()->BIsPhoneVerified();
}

// Checks whether the current user has Steam Guard two factor authentication enabled on their account.
bool SteamServer::isTwoFactorEnabled() {
	if (SteamUser() == NULL) {
		return false;
	}
	return SteamUser()->BIsTwoFactorEnabled();
}

// Check, true/false, if user is logged into Steam currently.
bool SteamServer::userLoggedOn() {
	if (SteamUser() == NULL) {
		return false;
	}
	return SteamUser()->BLoggedOn();
}

// Requests an application ticket encrypted with the secret "encrypted app ticket key".
void SteamServer::requestEncryptedAppTicket(const String &secret) {
	if (SteamUser() != NULL) {
		SteamAPICall_t api_call = SteamUser()->RequestEncryptedAppTicket((void *)secret.utf8().get_data(), sizeof(&secret));
		callResultEncryptedAppTicketResponse.Set(api_call, this, &SteamServer::encrypted_app_ticket_response);
	}
}

// Requests a URL which authenticates an in-game browser for store check-out, and then redirects to the specified URL.
void SteamServer::requestStoreAuthURL(const String &redirect) {
	if (SteamUser() != NULL) {
		SteamAPICall_t api_call = SteamUser()->RequestStoreAuthURL(redirect.utf8().get_data());
		callResultStoreAuthURLResponse.Set(api_call, this, &SteamServer::store_auth_url_response);
	}
}

// Starts voice recording.
void SteamServer::startVoiceRecording() {
	if (SteamUser() != NULL) {
		SteamUser()->StartVoiceRecording();
	}
}

//
bool SteamServer::setDurationControlOnlineState(int new_state) {
	if (SteamUser() == NULL) {
		return false;
	}
	return SteamUser()->BSetDurationControlOnlineState((EDurationControlOnlineState)new_state);
}

// Stops voice recording.
void SteamServer::stopVoiceRecording() {
	if (SteamUser() != NULL) {
		SteamUser()->StopVoiceRecording();
	}
}

// Notify the game server that we are disconnecting. NOTE: This is part of the old user authentication API and should not be mixed with the new API.
void SteamServer::terminateGameConnection(uint32 server_ip, uint16 server_port) {
	if (SteamUser() != NULL) {
		SteamUser()->TerminateGameConnection_DEPRECATED(server_ip, server_port);
	}
}

// Checks if the user owns a specific piece of Downloadable Content (DLC). This can only be called after sending the users auth ticket to ISteamGameServer::BeginAuthSession.
int SteamServer::userHasLicenseForApp(uint64_t steam_id, uint32_t app_id) {
	if (SteamUser() == NULL) {
		return 2;
	}
	CSteamID user_id = (uint64)steam_id;
	return SteamUser()->UserHasLicenseForApp(user_id, (AppId_t)app_id);
}

/////////////////////////////////////////////////
///// USER STATS
/////////////////////////////////////////////////
//
// Attaches a piece of user generated content the current user's entry on a leaderboard.
void SteamServer::attachLeaderboardUGC(uint64_t ugc_handle, uint64_t this_leaderboard) {
	if (SteamUserStats() != NULL) {
		// If no leaderboard is passed, use internal one
		if (this_leaderboard == 0) {
			this_leaderboard = leaderboard_handle;
		}
		SteamAPICall_t api_call = SteamUserStats()->AttachLeaderboardUGC((SteamLeaderboard_t)this_leaderboard, (UGCHandle_t)ugc_handle);
		callResultLeaderboardUGCSet.Set(api_call, this, &SteamServer::leaderboard_ugc_set);
	}
}

// Clears a given achievement.
bool SteamServer::clearAchievement(const String &name) {
	if (SteamUserStats() == NULL) {
		return false;
	}
	return SteamUserStats()->ClearAchievement(name.utf8().get_data());
}

// Request all rows for friends of user.
void SteamServer::downloadLeaderboardEntries(int start, int end, LeaderboardDataRequest type, uint64_t this_leaderboard) {
	if (SteamUserStats() != NULL) {
		// If no leaderboard is passed, use internal one
		if (this_leaderboard == 0) {
			this_leaderboard = leaderboard_handle;
		}
		SteamAPICall_t api_call = SteamUserStats()->DownloadLeaderboardEntries((SteamLeaderboard_t)this_leaderboard, ELeaderboardDataRequest(type), start, end);
		callResultEntries.Set(api_call, this, &SteamServer::leaderboard_scores_downloaded);
	}
}

// Request a maximum of 100 users with only one outstanding call at a time.
void SteamServer::downloadLeaderboardEntriesForUsers(Array users_id, uint64_t this_leaderboard) {
	if (SteamUserStats() != NULL) {
		// If no leaderboard is passed, use internal one
		if (this_leaderboard == 0) {
			this_leaderboard = leaderboard_handle;
		}
		int usersCount = users_id.size();
		if (usersCount > 0) {
			CSteamID *users = new CSteamID[usersCount];
			for (int i = 0; i < usersCount; i++) {
				CSteamID user = createSteamID(users_id[i]);
				users[i] = user;
			}
			SteamAPICall_t api_call = SteamUserStats()->DownloadLeaderboardEntriesForUsers((SteamLeaderboard_t)this_leaderboard, users, usersCount);
			callResultEntries.Set(api_call, this, &SteamServer::leaderboard_scores_downloaded);
			delete[] users;
		}
	}
}

// Find a given leaderboard, by name.
void SteamServer::findLeaderboard(const String &name) {
	if (SteamUserStats() != NULL) {
		SteamAPICall_t api_call = SteamUserStats()->FindLeaderboard(name.utf8().get_data());
		callResultFindLeaderboard.Set(api_call, this, &SteamServer::leaderboard_find_result);
	}
}

// Gets a leaderboard by name, it will create it if it's not yet created.
void SteamServer::findOrCreateLeaderboard(const String &name, LeaderboardSortMethod sort_method, LeaderboardDisplayType display_type) {
	if (SteamUserStats() != NULL) {
		SteamAPICall_t api_call = SteamUserStats()->FindOrCreateLeaderboard(name.utf8().get_data(), (ELeaderboardSortMethod)sort_method, (ELeaderboardDisplayType)display_type);
		callResultFindLeaderboard.Set(api_call, this, &SteamServer::leaderboard_find_result);
	}
}

// Return true/false if user has given achievement and the bool status of it being achieved or not.
Dictionary SteamServer::getAchievement(const String &name) {
	Dictionary achieve;
	bool achieved = false;
	if (SteamUserStats() == NULL) {
		achieve["ret"] = false;
	} else {
		achieve["ret"] = SteamUserStats()->GetAchievement(name.utf8().get_data(), &achieved);
	}
	achieve["achieved"] = achieved;
	return achieve;
}

// Returns the percentage of users who have unlocked the specified achievement.
Dictionary SteamServer::getAchievementAchievedPercent(const String &name) {
	Dictionary achieve;
	float percent = 0.f;
	if (SteamUserStats() == NULL) {
		achieve["ret"] = false;
	} else {
		achieve["ret"] = SteamUserStats()->GetAchievementAchievedPercent(name.utf8().get_data(), &percent);
	}
	achieve["percent"] = percent;
	return achieve;
}

// Get the achievement status, and the time it was unlocked if unlocked (in seconds since January 1, 19).
Dictionary SteamServer::getAchievementAndUnlockTime(const String &name) {
	Dictionary achieve;
	if (SteamUserStats() == NULL) {
		return achieve;
	}
	bool achieved = false;
	uint32 unlockTime = 0;
	// Get the data from Steam
	bool retrieved = SteamUserStats()->GetAchievementAndUnlockTime(name.utf8().get_data(), &achieved, &unlockTime);
	if (retrieved) {
		achieve["retrieve"] = retrieved;
		achieve["achieved"] = achieved;
		achieve["unlocked"] = unlockTime;
	}
	return achieve;
}

// Get general attributes for an achievement
String SteamServer::getAchievementDisplayAttribute(const String &name, const String &key) {
	if (SteamUserStats() == NULL) {
		return "";
	}
	return SteamUserStats()->GetAchievementDisplayAttribute(name.utf8().get_data(), key.utf8().get_data());
}

// Gets the icon for an achievement
int SteamServer::getAchievementIcon(const String &name) {
	if (SteamUserStats() == NULL) {
		return 0;
	}
	return SteamUserStats()->GetAchievementIcon(name.utf8().get_data());
}

// Gets the 'API name' for an achievement index
String SteamServer::getAchievementName(uint32_t achievement) {
	if (SteamUserStats() == NULL) {
		return "";
	}
	return SteamUserStats()->GetAchievementName(achievement);
}

// For achievements that have related Progress stats, use this to query what the bounds of that progress are. You may want this info to selectively call IndicateAchievementProgress when appropriate milestones of progress have been made, to show a progress notification to the user.
Dictionary SteamServer::getAchievementProgressLimitsInt(const String &name) {
	Dictionary progress;
	if (SteamUserStats() != NULL) {
		int32 min = 0;
		int32 max = 0;
		if (SteamUserStats()->GetAchievementProgressLimits(name.utf8().get_data(), &min, &max)) {
			progress["name"] = name;
			progress["min"] = min;
			progress["max"] = max;
		}
	}
	return progress;
}

// For achievements that have related Progress stats, use this to query what the bounds of that progress are. You may want this info to selectively call IndicateAchievementProgress when appropriate milestones of progress have been made, to show a progress notification to the user.
Dictionary SteamServer::getAchievementProgressLimitsFloat(const String &name) {
	Dictionary progress;
	if (SteamUserStats() != NULL) {
		float min = 0.0;
		float max = 0.0;
		if (SteamUserStats()->GetAchievementProgressLimits(name.utf8().get_data(), &min, &max)) {
			progress["name"] = name;
			progress["min"] = min;
			progress["max"] = max;
		}
	}
	return progress;
}

// Gets the lifetime totals for an aggregated stat; as an int
uint64_t SteamServer::getGlobalStatInt(const String &name) {
	if (SteamUserStats() == NULL) {
		return 0;
	}
	int64 stat = 0;
	SteamUserStats()->GetGlobalStat(name.utf8().get_data(), &stat);
	return (uint64_t)stat;
}

// Gets the lifetime totals for an aggregated stat; as an int
double SteamServer::getGlobalStatFloat(const String &name) {
	if (SteamUserStats() == NULL) {
		return 0;
	}
	double stat = 0;
	SteamUserStats()->GetGlobalStat(name.utf8().get_data(), &stat);
	return stat;
}

// Gets the daily history for an aggregated stat; int.
uint64_t SteamServer::getGlobalStatIntHistory(const String &name) {
	if (SteamUserStats() == NULL) {
		return 0;
	}
	int64 history = 0;
	SteamUserStats()->GetGlobalStatHistory(name.utf8().get_data(), &history, 60);
	return (uint64_t)history;
}

// Gets the daily history for an aggregated stat; float / double.
double SteamServer::getGlobalStatFloatHistory(const String &name) {
	if (SteamUserStats() == NULL) {
		return 0;
	}
	double history = 0;
	SteamUserStats()->GetGlobalStatHistory(name.utf8().get_data(), &history, 60);
	return history;
}

// Returns the display type of a leaderboard handle.
Dictionary SteamServer::getLeaderboardDisplayType(uint64_t this_leaderboard) {
	// Create a dictionary to return
	Dictionary display;
	if (SteamUserStats() != NULL) {
		// If no leaderboard is passed, use internal one
		if (this_leaderboard == 0) {
			this_leaderboard = leaderboard_handle;
		}
		int display_type = SteamUserStats()->GetLeaderboardDisplayType((SteamLeaderboard_t)this_leaderboard);
		// Add a verbal response
		if (display_type == 3) {
			display["result"] = 3;
			display["verbal"] = "Display is time in milliseconds";
		} else if (display_type == 2) {
			display["result"] = 2;
			display["verbal"] = "Display is time in seconds";
		} else if (display_type == 1) {
			display["result"] = 1;
			display["verbal"] = "Display is simple numerical value";
		} else {
			display["result"] = 0;
			display["verbal"] = "Display type or leaderboard handle is invalid";
		}
	}
	return display;
}

// Get the total number of entries in a leaderboard, as of the last request.
int SteamServer::getLeaderboardEntryCount(uint64_t this_leaderboard) {
	if (SteamUserStats() == NULL) {
		return -1;
	}
	// If no leaderboard is passed, use internal one
	if (this_leaderboard == 0) {
		this_leaderboard = leaderboard_handle;
	}
	return SteamUserStats()->GetLeaderboardEntryCount((SteamLeaderboard_t)this_leaderboard);
}

// Get the name of a leaderboard.
String SteamServer::getLeaderboardName(uint64_t this_leaderboard) {
	if (SteamUserStats() == NULL) {
		return "";
	}
	// If no leaderboard is passed, use internal one
	if (this_leaderboard == 0) {
		this_leaderboard = leaderboard_handle;
	}
	return SteamUserStats()->GetLeaderboardName((SteamLeaderboard_t)this_leaderboard);
}

// Returns the sort order of a leaderboard handle.
Dictionary SteamServer::getLeaderboardSortMethod(uint64_t this_leaderboard) {
	// Create a dictionary to return
	Dictionary sort;
	if (SteamUserStats() != NULL) {
		// If no leaderboard is passed, use internal one
		if (this_leaderboard == 0) {
			this_leaderboard = leaderboard_handle;
		}
		// Get the sort method int
		int sort_method = SteamUserStats()->GetLeaderboardSortMethod((SteamLeaderboard_t)this_leaderboard);
		// Add a verbal response
		if (sort_method == 2) {
			sort["result"] = 2;
			sort["verbal"] = "Top score is highest number";
		} else if (sort_method == 1) {
			sort["result"] = 1;
			sort["verbal"] = "Top score is lowest number";
		} else {
			sort["result"] = 0;
			sort["verbal"] = "Sort method or leaderboard handle is invalid";
		}
	}
	return sort;
}

// Gets the info on the most achieved achievement for the game.
Dictionary SteamServer::getMostAchievedAchievementInfo() {
	Dictionary entry;
	if (SteamUserStats() == NULL) {
		return entry;
	}
	char *name = new char[64];
	float percent = 0;
	bool achieved = false;
	// Get the data from Steam
	int result = SteamUserStats()->GetMostAchievedAchievementInfo(name, 64, &percent, &achieved);
	if (result > -1) {
		entry["rank"] = result;
		entry["name"] = name;
		entry["percent"] = percent;
		entry["achieved"] = achieved;
	}
	delete[] name;
	return entry;
}

// Gets the info on the next most achieved achievement for the game.
Dictionary SteamServer::getNextMostAchievedAchievementInfo(int iterator) {
	Dictionary entry;
	if (SteamUserStats() == NULL) {
		return entry;
	}
	char *name = new char[64];
	float percent = 0;
	bool achieved = false;
	// Get the data from Steam
	int result = SteamUserStats()->GetNextMostAchievedAchievementInfo(iterator, name, 64, &percent, &achieved);
	if (result > -1) {
		entry["rank"] = result;
		entry["name"] = name;
		entry["percent"] = percent;
		entry["achieved"] = achieved;
	}
	delete[] name;
	return entry;
}

// Get the number of achievements.
uint32_t SteamServer::getNumAchievements() {
	if (SteamUserStats() == NULL) {
		return 0;
	}
	return SteamUserStats()->GetNumAchievements();
}

//  Get the amount of players currently playing the current game (online + offline).
void SteamServer::getNumberOfCurrentPlayers() {
	if (SteamUserStats() != NULL) {
		SteamAPICall_t api_call = SteamUserStats()->GetNumberOfCurrentPlayers();
		callResultNumberOfCurrentPlayers.Set(api_call, this, &SteamServer::number_of_current_players);
	}
}

// Get the value of a float statistic.
float SteamServer::getStatFloat(const String &name) {
	if (SteamUserStats() == NULL) {
		return 0;
	}
	float statValue = 0;
	SteamUserStats()->GetStat(name.utf8().get_data(), &statValue);
	return statValue;
}

// Get the value of an integer statistic.
int SteamServer::getStatInt(const String &name) {
	if (SteamUserStats() == NULL) {
		return 0;
	}
	int32_t statValue = 0;
	SteamUserStats()->GetStat(name.utf8().get_data(), &statValue);
	return statValue;
}

// Gets the unlock status of the Achievement.
Dictionary SteamServer::getUserAchievement(uint64_t steam_id, const String &name) {
	Dictionary achieve;
	if (SteamUserStats() == NULL) {
		return achieve;
	}
	bool achieved = false;
	CSteamID user_id = (uint64)steam_id;
	// Get the data from Steam
	bool success = SteamUserStats()->GetUserAchievement(user_id, name.utf8().get_data(), &achieved);
	if (success) {
		achieve["steam_id"] = steam_id;
		achieve["retrieved"] = success;
		achieve["name"] = name;
		achieve["achieved"] = achieved;
	}
	return achieve;
}

// Gets the achievement status, and the time it was unlocked if unlocked.
Dictionary SteamServer::getUserAchievementAndUnlockTime(uint64_t steam_id, const String &name) {
	Dictionary achieve;
	if (SteamUserStats() == NULL) {
		return achieve;
	}
	bool achieved = false;
	uint32 unlocked = 0;
	CSteamID user_id = (uint64)steam_id;
	// Get the data from Steam
	bool success = SteamUserStats()->GetUserAchievementAndUnlockTime(user_id, name.utf8().get_data(), &achieved, &unlocked);
	if (success) {
		achieve["retrieved"] = success;
		achieve["name"] = name;
		achieve["achieved"] = achieved;
		achieve["unlocked"] = unlocked;
	}
	return achieve;
}

// Gets the current value of a float stat for the specified user.
float SteamServer::getUserStatFloat(uint64_t steam_id, const String &name) {
	if (SteamUserStats() == NULL) {
		return 0;
	}
	float statValue = 0;
	CSteamID user_id = (uint64)steam_id;
	SteamUserStats()->GetUserStat(user_id, name.utf8().get_data(), &statValue);
	return statValue;
}

// Gets the current value of an integer stat for the specified user.
int SteamServer::getUserStatInt(uint64_t steam_id, const String &name) {
	if (SteamUserStats() == NULL) {
		return 0;
	}
	int32_t statValue = 0;
	CSteamID user_id = (uint64)steam_id;
	SteamUserStats()->GetUserStat(user_id, name.utf8().get_data(), &statValue);
	return statValue;
}

// Achievement progress, triggers an AchievementProgress callback, that is all.
// Calling this with X out of X progress will NOT set the achievement, the game must still do that.
bool SteamServer::indicateAchievementProgress(const String &name, int current_progress, int max_progress) {
	if (SteamUserStats() == NULL) {
		return 0;
	}
	return SteamUserStats()->IndicateAchievementProgress(name.utf8().get_data(), current_progress, max_progress);
}

// Request all statistics and achievements from Steam servers.
bool SteamServer::requestCurrentStats() {
	if (SteamUserStats() == NULL) {
		return false;
	}
	// If the user isn't logged in, you can't get stats
	if (!userLoggedOn()) {
		return false;
	}
	return SteamUserStats()->RequestCurrentStats();
}

// Asynchronously fetch the data for the percentages.
void SteamServer::requestGlobalAchievementPercentages() {
	if (SteamUserStats() != NULL) {
		SteamAPICall_t api_call = SteamUserStats()->RequestGlobalAchievementPercentages();
		callResultGlobalAchievementPercentagesReady.Set(api_call, this, &SteamServer::global_achievement_percentages_ready);
	}
}

// Asynchronously fetches global stats data, which is available for stats marked as "aggregated" in the App Admin panel of the Steamworks website.  The limit is 60.
void SteamServer::requestGlobalStats(int history_days) {
	if (SteamUserStats() != NULL) {
		SteamAPICall_t api_call = SteamUserStats()->RequestGlobalStats(history_days);
		callResultGetGlobalStatsReceived.Set(api_call, this, &SteamServer::global_stats_received);
	}
}

// Asynchronously downloads stats and achievements for the specified user from the server.
void SteamServer::requestUserStats(uint64_t steam_id) {
	if (SteamUserStats() != NULL) {
		CSteamID user_id = (uint64)steam_id;
		SteamAPICall_t api_call = SteamUserStats()->RequestUserStats(user_id);
		callResultUserStatsReceived.Set(api_call, this, &SteamServer::user_stats_received);
	}
}

// Reset all Steam statistics; optional to reset achievements.
bool SteamServer::resetAllStats(bool achievements_too) {
	if (SteamUserStats() == NULL) {
		return false;
	}
	return SteamUserStats()->ResetAllStats(achievements_too);
}

// Set a given achievement.
bool SteamServer::setAchievement(const String &name) {
	if (SteamUserStats() == NULL) {
		return false;
	}
	return SteamUserStats()->SetAchievement(name.utf8().get_data());
}

// Set the maximum number of details to return for leaderboard entries
int SteamServer::setLeaderboardDetailsMax(int max) {
	// If the user submitted too high of a maximum, set to the real max
	if (max > k_cLeaderboardDetailsMax) {
		max = k_cLeaderboardDetailsMax;
	}
	// If the user submitted too low of a maximum, set it to 0
	if (max < 0) {
		max = 0;
	}
	// Now set the internal variable
	leaderboard_details_max = max;
	return leaderboard_details_max;
}

// Set a float statistic.
bool SteamServer::setStatFloat(const String &name, float value) {
	if (SteamUserStats() == NULL) {
		return false;
	}
	return SteamUserStats()->SetStat(name.utf8().get_data(), value);
}

// Set an integer statistic.
bool SteamServer::setStatInt(const String &name, int value) {
	if (SteamUserStats() == NULL) {
		return false;
	}
	return SteamUserStats()->SetStat(name.utf8().get_data(), value);
}

// Store all statistics, and achievements, on Steam servers; must be called to "pop" achievements.
bool SteamServer::storeStats() {
	if (SteamUserStats() == NULL) {
		return 0;
	}
	return SteamUserStats()->StoreStats();
}

// Updates an AVGRATE stat with new values.
bool SteamServer::updateAvgRateStat(const String &name, float this_session, double session_length) {
	if (SteamUserStats() == NULL) {
		return false;
	}
	return SteamUserStats()->UpdateAvgRateStat(name.utf8().get_data(), this_session, session_length);
}

// Upload a leaderboard score for the user.
void SteamServer::uploadLeaderboardScore(int score, bool keep_best, PackedInt32Array details, uint64_t this_leaderboard) {
	if (SteamUserStats() != NULL) {
		// If no leaderboard is passed, use internal one
		if (this_leaderboard == 0) {
			this_leaderboard = leaderboard_handle;
		}
		ELeaderboardUploadScoreMethod method = keep_best ? k_ELeaderboardUploadScoreMethodKeepBest : k_ELeaderboardUploadScoreMethodForceUpdate;
		int detailsSize = details.size();
		const int32 *details_pointer = NULL;
		if (detailsSize > 0) {
			details_pointer = details.ptr();
		}
		SteamAPICall_t api_call = SteamUserStats()->UploadLeaderboardScore((SteamLeaderboard_t)this_leaderboard, method, (int32)score, details_pointer, detailsSize);
		callResultUploadScore.Set(api_call, this, &SteamServer::leaderboard_score_uploaded);
	}
}

// Get the currently used leaderboard entries.
Array SteamServer::getLeaderboardEntries() {
	if (SteamUserStats() == NULL) {
		return Array();
	}
	return leaderboard_entries_array;
}

/////////////////////////////////////////////////
///// UTILS
/////////////////////////////////////////////////
//
// Filters the provided input message and places the filtered result into pchOutFilteredText.
String SteamServer::filterText(TextFilteringContext context, uint64_t steam_id, const String &message) {
	String new_message = "";
	if (SteamUtils() != NULL) {
		auto utf8_input = message.utf8();
		char *filtered = new char[utf8_input.length() + 1];
		CSteamID source_id = (uint64)steam_id;
		SteamUtils()->FilterText((ETextFilteringContext)context, source_id, utf8_input.get_data(), filtered, utf8_input.length() + 1);
		new_message = filtered;
		delete[] filtered;
	}
	return new_message;
}

// Used to get the failure reason of a call result. The primary usage for this function is debugging. The failure reasons are typically out of your control and tend to not be very important. Just keep retrying your API Call until it works.
String SteamServer::getAPICallFailureReason() {
	if (SteamUtils() == NULL) {
		return "ERROR: Steam Utils not present.";
	}
	int failure = SteamUtils()->GetAPICallFailureReason(api_handle);
	// Parse the failure
	if (failure == k_ESteamAPICallFailureSteamGone) {
		return "The local Steam process has stopped responding, it may have been forcefully closed or is frozen.";
	} else if (failure == k_ESteamAPICallFailureNetworkFailure) {
		return "The network connection to the Steam servers has been lost, or was already broken.";
	} else if (failure == k_ESteamAPICallFailureInvalidHandle) {
		return "The SteamAPICall_t handle passed in no longer exists.";
	} else if (failure == k_ESteamAPICallFailureMismatchedCallback) {
		return "GetAPICallResult was called with the wrong callback type for this API call.";
	} else {
		return "No failure.";
	}
}

// Get the Steam ID of the running application/game.
uint32_t SteamServer::getAppID() {
	if (SteamUtils() == NULL) {
		return 0;
	}
	return SteamUtils()->GetAppID();
}

// Get the amount of battery power, clearly for laptops.
int SteamServer::getCurrentBatteryPower() {
	if (SteamUtils() == NULL) {
		return 0;
	}
	return SteamUtils()->GetCurrentBatteryPower();
}

// Gets the image bytes from an image handle.
Dictionary SteamServer::getImageRGBA(int image) {
	Dictionary d;
	bool success = false;
	if (SteamUtils() != NULL) {
		uint32 width;
		uint32 height;
		success = SteamUtils()->GetImageSize(image, &width, &height);
		if (success) {
			PackedByteArray data;
			data.resize(width * height * 4);
			success = SteamUtils()->GetImageRGBA(image, data.ptrw(), data.size());
			if (success) {
				d["buffer"] = data;
			}
		}
	}
	d["success"] = success;
	return d;
}

// Gets the size of a Steam image handle.
Dictionary SteamServer::getImageSize(int image) {
	Dictionary d;
	bool success = false;
	if (SteamUtils() != NULL) {
		uint32 width;
		uint32 height;
		success = SteamUtils()->GetImageSize(image, &width, &height);
		d["width"] = width;
		d["height"] = height;
	}
	d["success"] = success;
	return d;
}

// Returns the number of IPC calls made since the last time this function was called.
uint32 SteamServer::getIPCCallCount() {
	if (SteamUtils() == NULL) {
		return 0;
	}
	return SteamUtils()->GetIPCCallCount();
}

// Get the user's country by IP.
String SteamServer::getIPCountry() {
	if (SteamUtils() == NULL) {
		return "";
	}
	return SteamUtils()->GetIPCountry();
}

// Return amount of time, in seconds, user has spent in this session.
int SteamServer::getSecondsSinceAppActive() {
	if (SteamUtils() == NULL) {
		return 0;
	}
	return SteamUtils()->GetSecondsSinceAppActive();
}

// Returns the number of seconds since the user last moved the mouse.
int SteamServer::getSecondsSinceComputerActive() {
	if (SteamUtils() == NULL) {
		return 0;
	}
	return SteamUtils()->GetSecondsSinceComputerActive();
}

// Get the actual time.
int SteamServer::getServerRealTime() {
	if (SteamUtils() == NULL) {
		return 0;
	}
	return SteamUtils()->GetServerRealTime();
}

// Get the Steam user interface language.
String SteamServer::getSteamUILanguage() {
	if (SteamUtils() == NULL) {
		return "";
	}
	return SteamUtils()->GetSteamUILanguage();
}

// Initializes text filtering. Returns false if filtering is unavailable for the language the user is currently running in. If the language is unsupported, the FilterText API will act as a passthrough.
bool SteamServer::initFilterText() {
	if (SteamUtils() == NULL) {
		return false;
	}
	return SteamUtils()->InitFilterText();
}

// Checks if an API Call is completed. Provides the backend of the CallResult wrapper.
Dictionary SteamServer::isAPICallCompleted() {
	Dictionary completed;
	if (SteamUtils() != NULL) {
		bool failed = false;
		bool valid = SteamUtils()->IsAPICallCompleted(api_handle, &failed);
		// Populate the dictionary
		completed["completed"] = valid;
		completed["failed"] = failed;
	}
	return completed;
}

// Returns true/false if Steam overlay is enabled.
bool SteamServer::isOverlayEnabled() {
	if (SteamUtils() == NULL) {
		return false;
	}
	return SteamUtils()->IsOverlayEnabled();
}

// Returns whether the current launcher is a Steam China launcher. You can cause the client to behave as the Steam China launcher by adding -dev -steamchina to the command line when running Steam.
bool SteamServer::isSteamChinaLauncher() {
	if (SteamUtils() == NULL) {
		return false;
	}
	return SteamUtils()->IsSteamChinaLauncher();
}

// Returns true if currently running on the Steam Deck device
bool SteamServer::isSteamRunningOnSteamDeck() {
	if (SteamUtils() == NULL) {
		return false;
	}
	return SteamUtils()->IsSteamRunningOnSteamDeck();
}

// Returns true if Steam & the Steam Overlay are running in Big Picture mode.
bool SteamServer::isSteamInBigPictureMode() {
	if (SteamUtils() == NULL) {
		return false;
	}
	return SteamUtils()->IsSteamInBigPictureMode();
}

// Is Steam running in VR?
bool SteamServer::isSteamRunningInVR() {
	if (SteamUtils() == NULL) {
		return 0;
	}
	return SteamUtils()->IsSteamRunningInVR();
}

// Checks if the HMD view will be streamed via Steam In-Home Streaming.
bool SteamServer::isVRHeadsetStreamingEnabled() {
	if (SteamUtils() == NULL) {
		return false;
	}
	return SteamUtils()->IsVRHeadsetStreamingEnabled();
}

// Checks if the Overlay needs a present. Only required if using event driven render updates.
bool SteamServer::overlayNeedsPresent() {
	if (SteamUtils() == NULL) {
		return false;
	}
	return SteamUtils()->BOverlayNeedsPresent();
}

// Sets the inset of the overlay notification from the corner specified by SetOverlayNotificationPosition.
void SteamServer::setOverlayNotificationInset(int horizontal, int vertical) {
	if (SteamUtils() != NULL) {
		SteamUtils()->SetOverlayNotificationInset(horizontal, vertical);
	}
}

// Set the position where overlay shows notifications.
void SteamServer::setOverlayNotificationPosition(int pos) {
	if ((pos >= 0) && (pos < 4) && (SteamUtils() != NULL)) {
		SteamUtils()->SetOverlayNotificationPosition(ENotificationPosition(pos));
	}
}

// Set whether the HMD content will be streamed via Steam In-Home Streaming.
void SteamServer::setVRHeadsetStreamingEnabled(bool enabled) {
	if (SteamUtils() != NULL) {
		SteamUtils()->SetVRHeadsetStreamingEnabled(enabled);
	}
}

// In game launchers that don't have controller support you can call this to have Steam Input translate the controller input into mouse/kb to navigate the launcher
void SteamServer::setGameLauncherMode(bool mode) {
	if (SteamUtils() == NULL) {
		return;
	}
	SteamUtils()->SetGameLauncherMode(mode);
}

// Ask SteamUI to create and render its OpenVR dashboard.
void SteamServer::startVRDashboard() {
	if (SteamUtils() != NULL) {
		SteamUtils()->StartVRDashboard();
	}
}

/////////////////////////////////////////////////
///// VIDEO
/////////////////////////////////////////////////
//
// Get the OPF details for 360 video playback.
void SteamServer::getOPFSettings(uint32_t app_id) {
	if (SteamVideo() != NULL) {
		SteamVideo()->GetOPFSettings((AppId_t)app_id);
	}
}

// Gets the OPF string for the specified video App ID.
String SteamServer::getOPFStringForApp(uint32_t app_id) {
	String opf_string = "";
	if (SteamVideo() != NULL) {
		int32 size = 48000;
		char *buffer = new char[size];
		if (SteamVideo()->GetOPFStringForApp((AppId_t)app_id, buffer, &size)) {
			opf_string = buffer;
		}
		delete[] buffer;
	}
	return opf_string;
}

// Asynchronously gets the URL suitable for streaming the video associated with the specified video app ID.
void SteamServer::getVideoURL(uint32_t app_id) {
	if (SteamVideo() != NULL) {
		SteamVideo()->GetVideoURL((AppId_t)app_id);
	}
}

// Checks if the user is currently live broadcasting and gets the number of users.
Dictionary SteamServer::isBroadcasting() {
	Dictionary broadcast;
	if (SteamVideo() != NULL) {
		int viewers = 0;
		bool broadcasting = SteamVideo()->IsBroadcasting(&viewers);
		// Populate the dictionary
		broadcast["broadcasting"] = broadcasting;
		broadcast["viewers"] = viewers;
	}
	return broadcast;
}

/////////////////////////////////////////////////
///// SIGNALS / CALLBACKS
/////////////////////////////////////////////////
//
// APPS CALLBACKS ///////////////////////////////
//
// Triggered after the current user gains ownership of DLC and that DLC is installed.
void SteamServer::dlc_installed(DlcInstalled_t *call_data) {
	uint32_t app_id = (AppId_t)call_data->m_nAppID;
	emit_signal("dlc_installed", app_id);
}

// Called after requesting the details of a specific file.
void SteamServer::file_details_result(FileDetailsResult_t *file_data) {
	uint32_t result = file_data->m_eResult;
	uint64_t file_size = file_data->m_ulFileSize;
	uint32_t flags = file_data->m_unFlags;
	uint8 *file_hash = new uint8[20];
	file_hash = file_data->m_FileSHA;
	emit_signal("file_details_result", result, file_size, file_hash, flags);
	delete[] file_hash;
}

// Posted after the user executes a steam url with command line or query parameters such as steam://run/<appid>//?param1=value1;param2=value2;param3=value3; while the game is already running. The new params can be queried with getLaunchCommandLine and getLaunchQueryParam.
void SteamServer::new_launch_url_parameters(NewUrlLaunchParameters_t *call_data) {
	emit_signal("new_launch_url_parameters");
}

// Purpose: called for games in Timed Trial mode
void SteamServer::timed_trial_status(TimedTrialStatus_t *call_data) {
	uint32_t app_id = call_data->m_unAppID;
	bool is_offline = call_data->m_bIsOffline;
	uint32 seconds_allowed = call_data->m_unSecondsAllowed;
	uint32 seconds_played = call_data->m_unSecondsPlayed;
	emit_signal("timed_trial_status", app_id, is_offline, seconds_allowed, seconds_played);
}

// APPS LIST CALLBACKS ///////////////////////////
//
// Sent when a new app is installed.
void SteamServer::app_installed(SteamAppInstalled_t *call_data) {
	uint32_t app_id = (uint32_t)call_data->m_nAppID;
	uint32_t install_folder_index = call_data->m_iInstallFolderIndex;
	emit_signal("app_installed", app_id, install_folder_index);
}

// Sent when an app is uninstalled.
void SteamServer::app_uninstalled(SteamAppUninstalled_t *call_data) {
	uint32_t app_id = (uint32_t)call_data->m_nAppID;
	uint32_t install_folder_index = call_data->m_iInstallFolderIndex;
	emit_signal("app_uninstalled", app_id, install_folder_index);
}

// FRIENDS CALLBACKS ////////////////////////////
//
// Called when a large avatar is loaded if you have tried requesting it when it was unavailable.
void SteamServer::avatar_loaded(AvatarImageLoaded_t *avatarData) {
	uint32 width, height;
	bool success = SteamUtils()->GetImageSize(avatarData->m_iImage, &width, &height);
	if (!success) {
		printf("[Steam] Failed to get image size.\n");
		return;
	}
	PackedByteArray data;
	data.resize(width * height * 4);
	success = SteamUtils()->GetImageRGBA(avatarData->m_iImage, data.ptrw(), data.size());
	if (!success) {
		printf("[Steam] Failed to load image buffer from callback\n");
		return;
	}
	CSteamID steam_id = avatarData->m_steamID;
	uint64_t avatar_id = steam_id.ConvertToUint64();
	call_deferred("emit_signal", "avatar_loaded", avatar_id, width, data);
}

// Called when a large avatar is loaded if you have tried requesting it when it was unavailable.
void SteamServer::avatar_image_loaded(AvatarImageLoaded_t *avatarData) {
	uint32 width = avatarData->m_iWide;
	uint32 height = avatarData->m_iTall;
	int avatar_index = avatarData->m_iImage;
	CSteamID steam_id = avatarData->m_steamID;
	uint64_t avatar_id = steam_id.ConvertToUint64();
	call_deferred("emit_signal", "avatar_image_loaded", avatar_id, avatar_index, width, height);
}

// Called when a Steam group activity has received.
void SteamServer::clan_activity_downloaded(DownloadClanActivityCountsResult_t *call_data) {
	bool success = call_data->m_bSuccess;
	// Set up the dictionary to populate
	Dictionary activity;
	if (success) {
		int online = 0;
		int in_game = 0;
		int chatting = 0;
		activity["ret"] = SteamFriends()->GetClanActivityCounts(clan_activity, &online, &in_game, &chatting);
		if (activity["ret"]) {
			activity["online"] = online;
			activity["ingame"] = in_game;
			activity["chatting"] = chatting;
		}
	}
	emit_signal("clan_activity_downloaded", activity);
}

// Called when Rich Presence data has been updated for a user, this can happen automatically when friends in the same game update their rich presence, or after a call to requestFriendRichPresence.
void SteamServer::friend_rich_presence_update(FriendRichPresenceUpdate_t *call_data) {
	uint64_t steam_id = call_data->m_steamIDFriend.ConvertToUint64();
	AppId_t app_id = call_data->m_nAppID;
	emit_signal("friend_rich_presence_updated", steam_id, app_id);
}

// Called when a user has joined a Steam group chat that the we are in.
void SteamServer::connected_chat_join(GameConnectedChatJoin_t *call_data) {
	uint64_t chat_id = call_data->m_steamIDClanChat.ConvertToUint64();
	uint64_t steam_id = call_data->m_steamIDUser.ConvertToUint64();
	emit_signal("chat_joined", chat_id, steam_id);
}

// Called when a user has left a Steam group chat that the we are in.
void SteamServer::connected_chat_leave(GameConnectedChatLeave_t *call_data) {
	uint64_t chat_id = call_data->m_steamIDClanChat.ConvertToUint64();
	uint64_t steam_id = call_data->m_steamIDUser.ConvertToUint64();
	bool kicked = call_data->m_bKicked;
	bool dropped = call_data->m_bDropped;
	emit_signal("chat_left", chat_id, steam_id, kicked, dropped);
}

// Called when a chat message has been received in a Steam group chat that we are in.
void SteamServer::connected_clan_chat_message(GameConnectedClanChatMsg_t *call_data) {
	Dictionary chat;
	char text[2048];
	EChatEntryType type = k_EChatEntryTypeInvalid;
	CSteamID user_id;
	chat["ret"] = SteamFriends()->GetClanChatMessage(call_data->m_steamIDClanChat, call_data->m_iMessageID, text, 2048, &type, &user_id);
	chat["text"] = String(text);
	chat["type"] = type;
	chat["chatter"] = uint64_t(user_id.ConvertToUint64());
	emit_signal("clan_chat_message", chat);
}

// Called when chat message has been received from a friend
void SteamServer::connected_friend_chat_message(GameConnectedFriendChatMsg_t *call_data) {
	uint64_t steam_id = call_data->m_steamIDUser.ConvertToUint64();
	int message = call_data->m_iMessageID;
	Dictionary chat;
	char text[2048];
	EChatEntryType type = k_EChatEntryTypeInvalid;
	chat["ret"] = SteamFriends()->GetFriendMessage(createSteamID(steam_id), message, text, 2048, &type);
	chat["text"] = String(text);
	emit_signal("friend_chat_message", chat);
}

// Called when the user tries to join a lobby from their friends list or from an invite. The game client should attempt to connect to specified lobby when this is received. If the game isn't running yet then the game will be automatically launched with the command line parameter +connect_lobby <64-bit lobby Steam ID> instead.
void SteamServer::join_requested(GameLobbyJoinRequested_t *call_data) {
	CSteamID lobby_id = call_data->m_steamIDLobby;
	uint64_t lobby = lobby_id.ConvertToUint64();
	CSteamID friend_id = call_data->m_steamIDFriend;
	uint64_t steam_id = friend_id.ConvertToUint64();
	emit_signal("join_requested", lobby, steam_id);
}

// Posted when the Steam Overlay activates or deactivates. The game can use this to be pause or resume single player games.
void SteamServer::overlay_toggled(GameOverlayActivated_t *call_data) {
	bool user_initiated = call_data->m_bUserInitiated;
	uint32_t app_id = call_data->m_nAppID;
	if (call_data->m_bActive) {
		emit_signal("overlay_toggled", true, user_initiated, app_id);
	} else {
		emit_signal("overlay_toggled", false, user_initiated, app_id);
	}
}

// Called when the user tries to join a game from their friends list or after a user accepts an invite by a friend with inviteUserToGame.
void SteamServer::join_game_requested(GameRichPresenceJoinRequested_t *call_data) {
	CSteamID steam_id = call_data->m_steamIDFriend;
	uint64_t user = steam_id.ConvertToUint64();
	String connect = call_data->m_rgchConnect;
	emit_signal("join_game_requested", user, connect);
}

// This callback is made when joining a game. If the user is attempting to join a lobby, then the callback GameLobbyJoinRequested_t will be made.
void SteamServer::change_server_requested(GameServerChangeRequested_t *call_data) {
	String server = call_data->m_rgchServer;
	String password = call_data->m_rgchPassword;
	emit_signal("change_server_requested", server, password);
}

//
void SteamServer::join_clan_chat_complete(JoinClanChatRoomCompletionResult_t *call_data) {
	uint64_t chat_id = call_data->m_steamIDClanChat.ConvertToUint64();
	EChatRoomEnterResponse response = call_data->m_eChatRoomEnterResponse;
	emit_signal("chat_join_complete", chat_id, response);
}

// Signal for a user change
void SteamServer::persona_state_change(PersonaStateChange_t *call_data) {
	uint64_t steam_id = call_data->m_ulSteamID;
	int flags = call_data->m_nChangeFlags;
	emit_signal("persona_state_change", steam_id, flags);
}

// Reports the result of an attempt to change the user's persona name.
void SteamServer::name_changed(SetPersonaNameResponse_t *call_data) {
	bool success = call_data->m_bSuccess;
	bool local_success = call_data->m_bLocalSuccess;
	EResult result = call_data->m_result;
	emit_signal("name_changed", success, local_success, result);
}

// Dispatched when an overlay browser instance is navigated to a protocol/scheme registered by RegisterProtocolInOverlayBrowser().
void SteamServer::overlay_browser_protocol(OverlayBrowserProtocolNavigation_t *call_data) {
	String uri = call_data->rgchURI;
	emit_signal("overlay_browser_protocol", uri);
}

// Purpose: Invoked when the status of unread messages changes
void SteamServer::unread_chat_messages_changed(UnreadChatMessagesChanged_t *call_data) {
	emit_signal("unread_chat_messages_changed");
}

// Callback for when a user's equipped Steam Commuity profile items have changed. This can be for the current user or their friends.
void SteamServer::equipped_profile_items_changed(EquippedProfileItemsChanged_t *call_data) {
	CSteamID this_steam_id = call_data->m_steamID;
	uint64_t steam_id = this_steam_id.ConvertToUint64();
	emit_signal("equipped_profile_items_changed", steam_id);
}

// GAME SEARCH CALLBACKS ////////////////////////
//
// There are no notes about this in Valve's header files or documentation.
void SteamServer::search_for_game_progress(SearchForGameProgressCallback_t *call_data) {
	EResult result = call_data->m_eResult;
	uint64_t search_id = call_data->m_ullSearchID;
	uint64_t lobby_id = call_data->m_lobbyID.ConvertToUint64();
	uint64_t steamIDEndedSearch = call_data->m_steamIDEndedSearch.ConvertToUint64();
	// Create a dictionary for search progress
	Dictionary search_progress;
	search_progress["lobby_id"] = lobby_id;
	search_progress["ended_search_id"] = steamIDEndedSearch;
	search_progress["seconds_remaining_estimate"] = call_data->m_nSecondsRemainingEstimate;
	search_progress["players_searching"] = call_data->m_cPlayersSearching;
	emit_signal("search_for_game_progress", result, search_id, search_progress);
}

// Notification to all players searching that a game has been found.
void SteamServer::search_for_game_result(SearchForGameResultCallback_t *call_data) {
	EResult result = call_data->m_eResult;
	uint64_t search_id = call_data->m_ullSearchID;
	uint64_t host_id = call_data->m_steamIDHost.ConvertToUint64();
	// Create a dictionary for search results
	Dictionary search_result;
	search_result["count_players_ingame"] = call_data->m_nCountPlayersInGame;
	search_result["count_accepted_game"] = call_data->m_nCountAcceptedGame;
	search_result["host_id"] = host_id;
	search_result["final_callback"] = call_data->m_bFinalCallback;
	emit_signal("search_for_game_result", result, search_id, search_result);
}

// Callback from RequestPlayersForGame when the matchmaking service has started or ended search; callback will also follow a call from CancelRequestPlayersForGame - m_bSearchInProgress will be false.
void SteamServer::request_players_for_game_progress(RequestPlayersForGameProgressCallback_t *call_data) {
	EResult result = call_data->m_eResult;
	uint64_t search_id = call_data->m_ullSearchID;
	emit_signal("request_players_for_game_progress", result, search_id);
}

// Callback from RequestPlayersForGame, one of these will be sent per player followed by additional callbacks when players accept or decline the game.
void SteamServer::request_players_for_game_result(RequestPlayersForGameResultCallback_t *call_data) {
	EResult result = call_data->m_eResult;
	uint64_t search_id = call_data->m_ullSearchID;
	uint64_t player_id = call_data->m_SteamIDPlayerFound.ConvertToUint64();
	uint64_t lobby_id = call_data->m_SteamIDLobby.ConvertToUint64();
	uint64_t unique_game_id = call_data->m_ullUniqueGameID;
	// Create a dictionary for the player data
	Dictionary player_data;
	player_data["player_id"] = player_id;
	player_data["lobby_id"] = lobby_id;
	player_data["player_accept_state"] = call_data->m_ePlayerAcceptState;
	player_data["player_index"] = call_data->m_nPlayerIndex;
	player_data["total_players"] = call_data->m_nTotalPlayersFound;
	player_data["total_players_accepted_game"] = call_data->m_nTotalPlayersAcceptedGame;
	player_data["suggested_team_index"] = call_data->m_nSuggestedTeamIndex;
	player_data["unique_game_id"] = unique_game_id;
	// Send the data back via signal
	emit_signal("request_players_for_game_result", result, search_id, player_data);
}

// There are no notes about this in Valve's header files or documentation.
void SteamServer::request_players_for_game_final_result(RequestPlayersForGameFinalResultCallback_t *call_data) {
	EResult result = call_data->m_eResult;
	uint64_t search_id = call_data->m_ullSearchID;
	uint64_t game_id = call_data->m_ullUniqueGameID;
	emit_signal("request_players_for_game_final", result, search_id, game_id);
}

// This callback confirms that results were received by the matchmaking service for this player.
void SteamServer::submit_player_result(SubmitPlayerResultResultCallback_t *call_data) {
	EResult result = call_data->m_eResult;
	uint64_t game_id = call_data->ullUniqueGameID;
	uint64_t player_id = call_data->steamIDPlayer.ConvertToUint64();
	emit_signal("submit_player_result", result, game_id, player_id);
}

// This callback confirms that the game is recorded as complete on the matchmaking service, the next call to RequestPlayersForGame will generate a new unique game ID.
void SteamServer::end_game_result(EndGameResultCallback_t *call_data) {
	EResult result = call_data->m_eResult;
	uint64_t game_id = call_data->ullUniqueGameID;
	emit_signal("end_game_result", result, game_id);
}

// GAME SERVER CALLBACKS ////////////////////////
//
//! Emits signal in response to function sendUserConnectAndAuthenticate
void SteamServer::client_approved(GSClientApprove_t *call_data) {
	uint64_t steam_id = call_data->m_SteamID.ConvertToUint64();
	uint64_t owner_id = call_data->m_OwnerSteamID.ConvertToUint64();
	emit_signal("client_approved", steam_id, owner_id);
}

//! Emits signal in response to function sendUserConnectAndAuthenticate
void SteamServer::client_denied(GSClientDeny_t *call_data) {
	uint64_t steam_id = call_data->m_SteamID.ConvertToUint64();
	int reason = call_data->m_eDenyReason;
	emit_signal("client_denied", steam_id, reason);
}

//! Called when the game server should kick the user
void SteamServer::client_kick(GSClientKick_t *call_data) {
	uint64_t steam_id = call_data->m_SteamID.ConvertToUint64();
	int reason = call_data->m_eDenyReason;
	emit_signal("client_kick", steam_id, reason);
}

//! Sent as a reply to computeNewPlayerCompatibility.
void SteamServer::player_compat(ComputeNewPlayerCompatibilityResult_t *call_data) {
	int result = call_data->m_eResult;
	int players_that_dont_like_candidate = call_data->m_cPlayersThatDontLikeCandidate;
	int players_that_candidate_doesnt_like = call_data->m_cPlayersThatCandidateDoesntLike;
	int clan_players_that_dont_like_candidate = call_data->m_cClanPlayersThatDontLikeCandidate;
	uint64_t steam_id = call_data->m_SteamIDCandidate.ConvertToUint64();
	emit_signal("player_compat", result, players_that_dont_like_candidate, players_that_candidate_doesnt_like, clan_players_that_dont_like_candidate, steam_id);
}

//! Received when the server requests to be displayed as secure (VAC protection)
void SteamServer::policy_response(GSPolicyResponse_t *call_data) {
	uint8 secure = call_data->m_bSecure;
	emit_signal("policy_response", secure);
}

//! Server has connected to the Steam back-end. Emits signal in response to functions logOff, logOn, or LogOnAnonymous.
void SteamServer::server_connected(SteamServersConnected_t *call_data) {
	emit_signal("server_connected");
}

//! Called if the client has lost connection to the Steam servers. Real-time services will be disabled until a matching server_connected has been posted.
void SteamServer::server_disconnected(SteamServersDisconnected_t *call_data) {
	int result = call_data->m_eResult;
	emit_signal("server_disconnected", result);
}

//! Logging the game server onto Steam
void SteamServer::server_connect_failure(SteamServerConnectFailure_t *call_data) {
	int result = call_data->m_eResult;
	bool retrying = call_data->m_bStillRetrying;
	emit_signal("server_connect_failure", result, retrying);
}

// HTTP CALLBACKS ///////////////////////////////
//
// Result when an HTTP request completes. If you're using GetHTTPStreamingResponseBodyData then you should be using the HTTPRequestHeadersReceived_t or HTTPRequestDataReceived_t.
void SteamServer::http_request_completed(HTTPRequestCompleted_t *call_data) {
	uint32 cookie_handle = call_data->m_hRequest;
	uint64_t context_value = call_data->m_ulContextValue;
	bool request_success = call_data->m_bRequestSuccessful;
	int status_code = call_data->m_eStatusCode;
	uint32 body_size = call_data->m_unBodySize;
	emit_signal("http_request_completed", cookie_handle, context_value, request_success, status_code, body_size);
}

// Triggered when a chunk of data is received from a streaming HTTP request.
void SteamServer::http_request_data_received(HTTPRequestDataReceived_t *call_data) {
	uint32 cookie_handle = call_data->m_hRequest;
	uint64_t context_value = call_data->m_ulContextValue;
	uint32 offset = call_data->m_cOffset;
	uint32 bytes_received = call_data->m_cBytesReceived;
	emit_signal("http_request_data_received", cookie_handle, context_value, offset, bytes_received);
}

// Triggered when HTTP headers are received from a streaming HTTP request.
void SteamServer::http_request_headers_received(HTTPRequestHeadersReceived_t *call_data) {
	uint32 cookie_handle = call_data->m_hRequest;
	uint64_t context_value = call_data->m_ulContextValue;
	emit_signal("http_request_headers_received", cookie_handle, context_value);
}

// INVENTORY CALLBACKS //////////////////////////
//
// This callback is triggered whenever item definitions have been updated, which could be in response to LoadItemDefinitions or any time new item definitions are available (eg, from the dynamic addition of new item types while players are still in-game).
void SteamServer::inventory_definition_update(SteamInventoryDefinitionUpdate_t *call_data) {
	// Create the return array
	Array definitions;
	// Set the array size variable
	uint32 size = 0;
	// Get the item defition IDs
	if (SteamInventory()->GetItemDefinitionIDs(NULL, &size)) {
		SteamItemDef_t *id_array = new SteamItemDef_t[size];
		if (SteamInventory()->GetItemDefinitionIDs(id_array, &size)) {
			// Loop through the temporary array and populate the return array
			for (uint32 i = 0; i < size; i++) {
				definitions.append(id_array[i]);
			}
		}
		// Delete the temporary array
		delete[] id_array;
	}
	// Return the item array as a signal
	emit_signal("inventory_defintion_update", definitions);
}

// Triggered when GetAllItems successfully returns a result which is newer / fresher than the last known result. (It will not trigger if the inventory hasn't changed, or if results from two overlapping calls are reversed in flight and the earlier result is already known to be stale/out-of-date.)
// The regular SteamInventoryResultReady_t callback will still be triggered immediately afterwards; this is an additional notification for your convenience.
void SteamServer::inventory_full_update(SteamInventoryFullUpdate_t *call_data) {
	// Set the handle
	inventory_handle = call_data->m_handle;
	// Send the handle back to the user
	emit_signal("inventory_full_update", call_data->m_handle);
}

// This is fired whenever an inventory result transitions from k_EResultPending to any other completed state, see GetResultStatus for the complete list of states. There will always be exactly one callback per handle.
void SteamServer::inventory_result_ready(SteamInventoryResultReady_t *call_data) {
	// Get the result
	int result = call_data->m_result;
	// Get the handle and pass it over
	inventory_handle = call_data->m_handle;
	emit_signal("inventory_result_ready", result, inventory_handle);
}

// MATCHMAKING CALLBACKS ////////////////////////
//
// Called when an account on your favorites list is updated
void SteamServer::favorites_list_accounts_updated(FavoritesListAccountsUpdated_t *call_data) {
	int result = call_data->m_eResult;
	emit_signal("favorites_list_accounts_updated", result);
}

// A server was added/removed from the favorites list, you should refresh now.
void SteamServer::favorites_list_changed(FavoritesListChanged_t *call_data) {
	Dictionary favorite;
	// Convert the IP address back to a string
	const int NBYTES = 4;
	uint8 octet[NBYTES];
	char favoriteIP[16];
	for (int j = 0; j < NBYTES; j++) {
		octet[j] = call_data->m_nIP >> (j * 8);
	}
	sprintf(favoriteIP, "%d.%d.%d.%d", octet[0], octet[1], octet[2], octet[3]);
	favorite["ip"] = favoriteIP;
	favorite["query_port"] = call_data->m_nQueryPort;
	favorite["connection_port"] = call_data->m_nConnPort;
	favorite["app_id"] = call_data->m_nAppID;
	favorite["flags"] = call_data->m_nFlags;
	favorite["add"] = call_data->m_bAdd;
	favorite["account_id"] = call_data->m_unAccountId;
	emit_signal("favorites_list_changed", favorite);
}

// Signal when a lobby chat message is received
void SteamServer::lobby_message(LobbyChatMsg_t *call_data) {
	CSteamID lobby_id = call_data->m_ulSteamIDLobby;
	CSteamID user_id = call_data->m_ulSteamIDUser;
	uint8 chat_type = call_data->m_eChatEntryType;
	// Convert the chat type over
	EChatEntryType type = (EChatEntryType)chat_type;
	// Get the chat message data
	char buffer[STEAM_LARGE_BUFFER_SIZE];
	int size = SteamMatchmaking()->GetLobbyChatEntry(lobby_id, call_data->m_iChatID, &user_id, &buffer, STEAM_LARGE_BUFFER_SIZE, &type);
	uint64_t lobby = lobby_id.ConvertToUint64();
	uint64_t user = user_id.ConvertToUint64();
	emit_signal("lobby_message", lobby, user, String::utf8(buffer, size), chat_type);
}

// A lobby chat room state has changed, this is usually sent when a user has joined or left the lobby.
void SteamServer::lobby_chat_update(LobbyChatUpdate_t *call_data) {
	uint64_t lobby_id = call_data->m_ulSteamIDLobby;
	uint64_t changed_id = call_data->m_ulSteamIDUserChanged;
	uint64_t making_change_id = call_data->m_ulSteamIDMakingChange;
	uint32 chat_state = call_data->m_rgfChatMemberStateChange;
	emit_signal("lobby_chat_update", lobby_id, changed_id, making_change_id, chat_state);
}

// The lobby metadata has changed.
void SteamServer::lobby_data_update(LobbyDataUpdate_t *call_data) {
	uint64_t member_id = call_data->m_ulSteamIDMember;
	uint64_t lobby_id = call_data->m_ulSteamIDLobby;
	uint8 success = call_data->m_bSuccess;
	emit_signal("lobby_data_update", success, lobby_id, member_id);
}

// Posted if a user is forcefully removed from a lobby; can occur if a user loses connection to Steam.
void SteamServer::lobby_kicked(LobbyKicked_t *call_data) {
	CSteamID steam_lobby_id = call_data->m_ulSteamIDLobby;
	uint64_t lobby_id = steam_lobby_id.ConvertToUint64();
	CSteamID steam_admin_id = call_data->m_ulSteamIDAdmin;
	uint64_t admin_id = steam_admin_id.ConvertToUint64();
	uint8 due_to_disconnect = call_data->m_bKickedDueToDisconnect;
	emit_signal("lobby_kicked", lobby_id, admin_id, due_to_disconnect);
}

// Received upon attempting to enter a lobby. Lobby metadata is available to use immediately after receiving this.
void SteamServer::lobby_joined(LobbyEnter_t *lobbyData) {
	CSteamID steam_lobby_id = lobbyData->m_ulSteamIDLobby;
	uint64_t lobby_id = steam_lobby_id.ConvertToUint64();
	uint32_t permissions = lobbyData->m_rgfChatPermissions;
	bool locked = lobbyData->m_bLocked;
	uint32_t response = lobbyData->m_EChatRoomEnterResponse;
	emit_signal("lobby_joined", lobby_id, permissions, locked, response);
}

// A game server has been set via SetLobbyGameServer for all of the members of the lobby to join. It's up to the individual clients to take action on this; the typical game behavior is to leave the lobby and connect to the specified game server; but the lobby may stay open throughout the session if desired.
void SteamServer::lobby_game_created(LobbyGameCreated_t *call_data) {
	uint64_t lobby_id = call_data->m_ulSteamIDLobby;
	uint64_t server_id = call_data->m_ulSteamIDGameServer;
	uint32 ip = call_data->m_unIP;
	uint16 port = call_data->m_usPort;
	// Convert the IP address back to a string
	const int NBYTES = 4;
	uint8 octet[NBYTES];
	char server_ip[16];
	for (int i = 0; i < NBYTES; i++) {
		octet[i] = ip >> (i * 8);
	}
	sprintf(server_ip, "%d.%d.%d.%d", octet[0], octet[1], octet[2], octet[3]);
	emit_signal("lobby_game_created", lobby_id, server_id, server_ip, port);
}

// Someone has invited you to join a Lobby. Normally you don't need to do anything with this, as the Steam UI will also display a '<user> has invited you to the lobby, join?' notification and message. If the user outside a game chooses to join, your game will be launched with the parameter +connect_lobby <64-bit lobby id>, or with the callback GameLobbyJoinRequested_t if they're already in-game.
void SteamServer::lobby_invite(LobbyInvite_t *lobbyData) {
	CSteamID inviter_id = lobbyData->m_ulSteamIDUser;
	uint64_t inviter = inviter_id.ConvertToUint64();
	CSteamID lobby_id = lobbyData->m_ulSteamIDLobby;
	uint64_t lobby = lobby_id.ConvertToUint64();
	CSteamID game_id = lobbyData->m_ulGameID;
	uint64_t game = game_id.ConvertToUint64();
	emit_signal("lobby_invite", inviter, lobby, game);
}

// NETWORKING CALLBACKS /////////////////////////
//
// Called when packets can't get through to the specified user. All queued packets unsent at this point will be dropped, further attempts to send will retry making the connection (but will be dropped if we fail again).
void SteamServer::p2p_session_connect_fail(P2PSessionConnectFail_t *call_data) {
	uint64_t steam_id_remote = call_data->m_steamIDRemote.ConvertToUint64();
	uint8_t session_error = call_data->m_eP2PSessionError;
	emit_signal("p2p_session_connect_fail", steam_id_remote, session_error);
}

// A user wants to communicate with us over the P2P channel via the sendP2PPacket. In response, a call to acceptP2PSessionWithUser needs to be made, if you want to open the network channel with them.
void SteamServer::p2p_session_request(P2PSessionRequest_t *call_data) {
	uint64_t steam_id_remote = call_data->m_steamIDRemote.ConvertToUint64();
	emit_signal("p2p_session_request", steam_id_remote);
}

// NETWORKING MESSAGES CALLBACKS ////////////////
//
// Posted when a remote host is sending us a message, and we do not already have a session with them.
void SteamServer::network_messages_session_request(SteamNetworkingMessagesSessionRequest_t *call_data) {
	SteamNetworkingIdentity remote = call_data->m_identityRemote;
	char identity[STEAM_BUFFER_SIZE];
	remote.ToString(identity, STEAM_BUFFER_SIZE);
	emit_signal("network_messages_session_request", identity);
}

// Posted when we fail to establish a connection, or we detect that communications have been disrupted it an unusual way.
void SteamServer::network_messages_session_failed(SteamNetworkingMessagesSessionFailed_t *call_data) {
	SteamNetConnectionInfo_t info = call_data->m_info;
	// Parse out the reason for failure
	int reason = info.m_eEndReason;
	emit_signal("network_messages_session_failed", reason);
}

// NETWORKING SOCKETS CALLBACKS /////////////////
//
// This callback is posted whenever a connection is created, destroyed, or changes state. The m_info field will contain a complete description of the connection at the time the change occurred and the callback was posted. In particular, m_info.m_eState will have the new connection state.
void SteamServer::network_connection_status_changed(SteamNetConnectionStatusChangedCallback_t *call_data) {
	// Connection handle.
	uint64_t connect_handle = call_data->m_hConn;
	// Full connection info.
	SteamNetConnectionInfo_t connection_info = call_data->m_info;
	// Move connection info into a dictionary
	Dictionary connection;
	char identity[STEAM_BUFFER_SIZE];
	connection_info.m_identityRemote.ToString(identity, STEAM_BUFFER_SIZE);
	connection["identity"] = identity;
	connection["user_data"] = (uint64_t)connection_info.m_nUserData;
	connection["listen_socket"] = connection_info.m_hListenSocket;
	char ip_address[STEAM_BUFFER_SIZE];
	connection_info.m_addrRemote.ToString(ip_address, STEAM_BUFFER_SIZE, true);
	connection["remote_address"] = ip_address;
	connection["remote_pop"] = connection_info.m_idPOPRemote;
	connection["pop_relay"] = connection_info.m_idPOPRelay;
	connection["connection_state"] = connection_info.m_eState;
	connection["end_reason"] = connection_info.m_eEndReason;
	connection["end_debug"] = connection_info.m_szEndDebug;
	connection["debug_description"] = connection_info.m_szConnectionDescription;
	// Previous state (current state is in m_info.m_eState).
	int old_state = call_data->m_eOldState;
	// Send the data back via signal
	emit_signal("network_connection_status_changed", connect_handle, connection, old_state);
}

// This callback is posted whenever the state of our readiness changes.
void SteamServer::network_authentication_status(SteamNetAuthenticationStatus_t *call_data) {
	// Status.
	int available = call_data->m_eAvail;
	// Non-localized English language status. For diagnostic / debugging purposes only.
	char *debug_message = new char[256];
	sprintf(debug_message, "%s", call_data->m_debugMsg);
	// Send the data back via signal
	emit_signal("network_authentication_status", available, debug_message);
	delete[] debug_message;
}

// A struct used to describe a "fake IP" we have been assigned to use as an identifier.
// This callback is posted when ISteamNetworkingSoockets::BeginAsyncRequestFakeIP completes.
void SteamServer::fake_ip_result(SteamNetworkingFakeIPResult_t *call_data) {
	int result = call_data->m_eResult;
	// Pass this new networking identity to the map
	networking_identities["fake_ip_identity"] = call_data->m_identity;
	uint32 ip = call_data->m_unIP;
	// Convert the IP address back to a string
	const int NBYTES = 4;
	uint8 octet[NBYTES];
	char fake_ip[16];
	for (int i = 0; i < NBYTES; i++) {
		octet[i] = ip >> (i * 8);
	}
	sprintf(fake_ip, "%d.%d.%d.%d", octet[0], octet[1], octet[2], octet[3]);
	// Get the ports as an array
	Array port_list;
	uint16 *ports = call_data->m_unPorts;
	for (uint16 i = 0; i < sizeof(ports); i++) {
		port_list.append(ports[i]);
	}
	emit_signal("fake_ip_result", result, "fake_ip_identity", fake_ip, port_list);
}

// NETWORKING UTILS CALLBACKS ///////////////////
//
// A struct used to describe our readiness to use the relay network.
void SteamServer::relay_network_status(SteamRelayNetworkStatus_t *call_data) {
	int available = call_data->m_eAvail;
	int ping_measurement = call_data->m_bPingMeasurementInProgress;
	int available_config = call_data->m_eAvailNetworkConfig;
	int available_relay = call_data->m_eAvailAnyRelay;
	char *debug_message = new char[256];
	sprintf(debug_message, "%s", call_data->m_debugMsg);
	//	debug_message = call_data->m_debugMsg;
	emit_signal("relay_network_status", available, ping_measurement, available_config, available_relay, debug_message);
	delete[] debug_message;
}

// PARTIES CALLBACKS ////////////////////////////
//
// After creating a beacon, when a user "follows" that beacon Steam will send you this callback to know that you should be prepared for the user to join your game. When they do join, be sure to call ISteamParties::OnReservationCompleted to let Steam know.
void SteamServer::reservation_notification(ReservationNotificationCallback_t *call_data) {
	uint64_t beacon_id = call_data->m_ulBeaconID;
	uint64_t steam_id = call_data->m_steamIDJoiner.ConvertToUint64();
	emit_signal("reservation_notifications", beacon_id, steam_id);
}

// Notification that the list of available locations for posting a beacon has been updated.
void SteamServer::available_beacon_locations_updated(AvailableBeaconLocationsUpdated_t *call_data) {
	emit_signal("available_beacon_locations_updated");
}

// Notification that the list of active beacons visible to the current user has changed.
void SteamServer::active_beacons_updated(ActiveBeaconsUpdated_t *call_data) {
	emit_signal("active_beacons_updated");
}

// REMOTE STORAGE CALLBACKS /////////////////////
//
// Purpose: one or more files for this app have changed locally after syncing to remote session changes.
// Note: only posted if this happens DURING the local app session.
void SteamServer::local_file_changed(RemoteStorageLocalFileChange_t *call_data) {
	emit_signal("local_file_changed");
}

// UGC CALLBACKS ////////////////////////////////
//
// Called when a workshop item has been downloaded.
void SteamServer::item_downloaded(DownloadItemResult_t *call_data) {
	EResult result = call_data->m_eResult;
	PublishedFileId_t file_id = call_data->m_nPublishedFileId;
	AppId_t app_id = call_data->m_unAppID;
	emit_signal("item_downloaded", result, (uint64_t)file_id, (uint32_t)app_id);
}

// Called when a workshop item has been installed or updated.
void SteamServer::item_installed(ItemInstalled_t *call_data) {
	AppId_t app_id = call_data->m_unAppID;
	PublishedFileId_t file_id = call_data->m_nPublishedFileId;
	emit_signal("item_installed", app_id, (uint64_t)file_id);
}

// Purpose: signal that the list of subscribed items changed.
void SteamServer::user_subscribed_items_list_changed(UserSubscribedItemsListChanged_t *call_data) {
	uint32 app_id = call_data->m_nAppID;
	emit_signal("user_subscribed_items_list_changed", app_id);
}

// USER CALLBACKS ///////////////////////////////
//
// Sent by the Steam server to the client telling it to disconnect from the specified game server, which it may be in the process of or already connected to. The game client should immediately disconnect upon receiving this message. This can usually occur if the user doesn't have rights to play on the game server.
void SteamServer::client_game_server_deny(ClientGameServerDeny_t *call_data) {
	uint32 app_id = call_data->m_uAppID;
	uint32 server_ip = call_data->m_unGameServerIP;
	uint16 server_port = call_data->m_usGameServerPort;
	uint16 secure = call_data->m_bSecure;
	uint32 reason = call_data->m_uReason;
	// Convert the IP address back to a string
	const int NBYTES = 4;
	uint8 octet[NBYTES];
	char ip[16];
	for (int j = 0; j < NBYTES; j++) {
		octet[j] = server_ip >> (j * 8);
	}
	sprintf(ip, "%d.%d.%d.%d", octet[0], octet[1], octet[2], octet[3]);
	emit_signal("client_game_server_deny", app_id, ip, server_port, secure, reason);
}

// Sent to your game in response to a steam://gamewebcallback/ command from a user clicking a link in the Steam overlay browser. You can use this to add support for external site signups where you want to pop back into the browser after some web page signup sequence, and optionally get back some detail about that.
void SteamServer::game_web_callback(GameWebCallback_t *call_data) {
	String url = call_data->m_szURL;
	emit_signal("game_web_callback", url);
}

// Result when creating an auth session ticket.
void SteamServer::get_auth_session_ticket_response(GetAuthSessionTicketResponse_t *call_data) {
	uint32 auth_ticket = call_data->m_hAuthTicket;
	int result = call_data->m_eResult;
	emit_signal("get_auth_session_ticket_response", auth_ticket, result);
}

// Result when creating an webapi ticket from GetAuthTicketForWebApi.
void SteamServer::get_ticket_for_web_api(GetTicketForWebApiResponse_t *call_data) {
	uint32 auth_ticket = call_data->m_hAuthTicket;
	int result = call_data->m_eResult;
	int ticket_size = call_data->m_cubTicket;
	uint8 ticket_buffer = call_data->m_rgubTicket[2560];
	emit_signal("get_ticket_for_web_api", auth_ticket, result, ticket_size, ticket_buffer);
}

// Called when the callback system for this client is in an error state (and has flushed pending callbacks). When getting this message the client should disconnect from Steam, reset any stored Steam state and reconnect. This usually occurs in the rare event the Steam client has some kind of fatal error.
void SteamServer::ipc_failure(IPCFailure_t *call_data) {
	uint8 type = call_data->m_eFailureType;
	emit_signal("ipc_failure", type);
}

// Called whenever the users licenses (owned packages) changes.
void SteamServer::licenses_updated(LicensesUpdated_t *call_data) {
	emit_signal("licenses_updated");
}

// Called when a user has responded to a microtransaction authorization request.
void SteamServer::microtransaction_auth_response(MicroTxnAuthorizationResponse_t *call_data) {
	uint32 app_id = call_data->m_unAppID;
	uint64_t order_id = call_data->m_ulOrderID;
	bool authorized;
	if (call_data->m_bAuthorized == 1) {
		authorized = true;
	} else {
		authorized = false;
	}
	emit_signal("microtransaction_auth_response", app_id, order_id, authorized);
}

// Called when a connections to the Steam back-end has been established. This means the Steam client now has a working connection to the Steam servers. Usually this will have occurred before the game has launched, and should only be seen if the user has dropped connection due to a networking issue or a Steam server update.
void SteamServer::steam_server_connected(SteamServersConnected_t *connectData) {
	emit_signal("steam_server_connected");
}

// Called if the client has lost connection to the Steam servers. Real-time services will be disabled until a matching SteamServersConnected_t has been posted.
void SteamServer::steam_server_disconnected(SteamServersDisconnected_t *connectData) {
	emit_signal("steam_server_disconnected");
}

// Called when an auth ticket has been validated.
void SteamServer::validate_auth_ticket_response(ValidateAuthTicketResponse_t *call_data) {
	uint64_t auth_id = call_data->m_SteamID.ConvertToUint64();
	uint32_t response = call_data->m_eAuthSessionResponse;
	uint64_t owner_id = call_data->m_OwnerSteamID.ConvertToUint64();
	emit_signal("validate_auth_ticket_response", auth_id, response, owner_id);
}

// USER STATS CALLBACKS /////////////////////////
//
// Result of a request to store the achievements on the server, or an "indicate progress" call. If both m_nCurProgress and m_nMaxProgress are zero, that means the achievement has been fully unlocked.
void SteamServer::user_achievement_stored(UserAchievementStored_t *call_data) {
	CSteamID game_id = call_data->m_nGameID;
	uint64_t game = game_id.ConvertToUint64();
	bool group_achieve = call_data->m_bGroupAchievement;
	String name = call_data->m_rgchAchievementName;
	uint32_t current_progress = call_data->m_nCurProgress;
	uint32_t max_progress = call_data->m_nMaxProgress;
	emit_signal("user_achievement_stored", game, group_achieve, name, current_progress, max_progress);
}

// Called when the latest stats and achievements for the local user have been received from the server.
void SteamServer::current_stats_received(UserStatsReceived_t *call_data) {
	CSteamID game_id = call_data->m_nGameID;
	uint64_t game = game_id.ConvertToUint64();
	uint32_t result = call_data->m_eResult;
	CSteamID user_id = call_data->m_steamIDUser;
	uint64_t user = user_id.ConvertToUint64();
	emit_signal("current_stats_received", game, result, user);
}

// Result of a request to store the user stats.
void SteamServer::user_stats_stored(UserStatsStored_t *call_data) {
	CSteamID game_id = call_data->m_nGameID;
	uint64_t game = game_id.ConvertToUint64();
	uint32_t result = call_data->m_eResult;
	emit_signal("user_stats_stored", game, result);
}

// Callback indicating that a user's stats have been unloaded. Call RequestUserStats again before accessing stats for this user.
void SteamServer::user_stats_unloaded(UserStatsUnloaded_t *call_data) {
	CSteamID steam_id = call_data->m_steamIDUser;
	uint64_t user = steam_id.ConvertToUint64();
	emit_signal("user_stats_unloaded", user);
}

// UTILITY CALLBACKS ////////////////////////////
//
// Called when the big picture gamepad text input has been closed.
void SteamServer::gamepad_text_input_dismissed(GamepadTextInputDismissed_t *call_data) {
	bool was_submitted = call_data->m_bSubmitted;
	const uint32 buffer_length = 1024 + 1;
	char *text = new char[buffer_length];
	uint32 length = buffer_length;
	uint32_t app_id = call_data->m_unAppID;
	if (was_submitted) {
		SteamUtils()->GetEnteredGamepadTextInput(text, buffer_length);
		length = SteamUtils()->GetEnteredGamepadTextLength();
	}
	emit_signal("gamepad_text_input_dismissed", was_submitted, String::utf8(text, (int)length), app_id);
	delete[] text;
}

// Called when the country of the user changed. The country should be updated with getIPCountry.
void SteamServer::ip_country(IPCountry_t *call_data) {
	emit_signal("ip_country");
}

// Called when running on a laptop and less than 10 minutes of battery is left, and then fires then every minute afterwards.
void SteamServer::low_power(LowBatteryPower_t *timeLeft) {
	uint8 power = timeLeft->m_nMinutesBatteryLeft;
	emit_signal("low_power", power);
}

// Called when a SteamAPICall_t has completed (or failed)
void SteamServer::steam_api_call_completed(SteamAPICallCompleted_t *call_data) {
	uint64_t async_call = call_data->m_hAsyncCall;
	int callback = call_data->m_iCallback;
	uint32 parameter = call_data->m_cubParam;
	emit_signal("steam_api_call_completed", async_call, callback, parameter);
}

// Called when Steam wants to shutdown.
void SteamServer::steam_shutdown(SteamShutdown_t *call_data) {
	emit_signal("steam_shutdown");
}

// Sent after the device returns from sleep/suspend mode.
void SteamServer::app_resuming_from_suspend(AppResumingFromSuspend_t *call_data) {
	emit_signal("app_resuming_from_suspend");
}

// Sent after the device returns from sleep/suspend mode.
void SteamServer::floating_gamepad_text_input_dismissed(FloatingGamepadTextInputDismissed_t *call_data) {
	emit_signal("floating_gamepad_text_input_dismissed");
}

// The text filtering dictionary has changed, obviously.
void SteamServer::filter_text_dictionary_changed(FilterTextDictionaryChanged_t *call_data) {
	int language = call_data->m_eLanguage;
	emit_signal("filter_text_dictionary_changed", language);
}

// VIDEO CALLBACKS //////////////////////////////
//
// Triggered when the OPF Details for 360 video playback are retrieved. After receiving this you can use GetOPFStringForApp to access the OPF details.
void SteamServer::get_opf_settings_result(GetOPFSettingsResult_t *call_data) {
	int result = call_data->m_eResult;
	uint32 app_id = call_data->m_unVideoAppID;
	emit_signal("broadcast_upload_stop", result, app_id);
}

// Provides the result of a call to GetVideoURL.
void SteamServer::get_video_result(GetVideoURLResult_t *call_data) {
	int result = call_data->m_eResult;
	uint32 app_id = call_data->m_unVideoAppID;
	String url = call_data->m_rgchURL;
	emit_signal("get_video_result", result, app_id, url);
}

/////////////////////////////////////////////////
///// SIGNALS / CALL RESULTS ////////////////////
/////////////////////////////////////////////////
//
// STEAMWORKS ERROR SIGNAL //////////////////////
//
// Intended to serve as generic error messaging for failed call results
void SteamServer::steamworksError(const String &failed_signal) {
	// Emit the signal to inform the user of the failure
	emit_signal("steamworks_error", failed_signal, "io_failure");
}

// FRIENDS CALL RESULTS /////////////////////////
//
// Marks the return of a request officer list call.
void SteamServer::request_clan_officer_list(ClanOfficerListResponse_t *call_data, bool io_failure) {
	if (io_failure) {
		steamworksError("request_clan_officer_list");
	} else {
		Array officers_list;
		String message;
		if (!call_data->m_bSuccess) {
			message = "Clan officer list response failed.";
		} else {
			CSteamID owner_steam_id = SteamFriends()->GetClanOwner(call_data->m_steamIDClan);
			int officers = SteamFriends()->GetClanOfficerCount(call_data->m_steamIDClan);
			message = "The owner of the clan is: " + (String)String::utf8(SteamFriends()->GetFriendPersonaName(owner_steam_id)) + " (" + itos(owner_steam_id.ConvertToUint64()) + ") and there are " + itos(call_data->m_cOfficers) + " officers.";
			for (int i = 0; i < officers; i++) {
				Dictionary officer;
				CSteamID officerSteamID = SteamFriends()->GetClanOfficerByIndex(call_data->m_steamIDClan, i);
				uint64_t id = officerSteamID.ConvertToUint64();
				officer["id"] = id;
				officer["name"] = String::utf8(SteamFriends()->GetFriendPersonaName(officerSteamID));
				officers_list.append(officer);
			}
		}
		emit_signal("request_clan_officer_list", message, officers_list);
	}
}

// Returns the result of enumerateFollowingList.
void SteamServer::enumerate_following_list(FriendsEnumerateFollowingList_t *call_data, bool io_failure) {
	if (io_failure) {
		steamworksError("enumerate_following_list");
	} else {
		Array following;
		String message;
		if (call_data->m_eResult != k_EResultOK) {
			message = "Failed to acquire list.";
		} else {
			int followers_parsed = 0;
			message = "Retrieved " + itos(call_data->m_nResultsReturned) + " of " + itos(call_data->m_nTotalResultCount) + " people followed.";
			int32 count = call_data->m_nTotalResultCount;
			for (int i = 0; i < count; i++) {
				Dictionary follow;
				int num = i;
				uint64_t id = call_data->m_rgSteamID[i].ConvertToUint64();
				follow["num"] = num;
				follow["id"] = id;
				following.append(follow);
			}
			followers_parsed += call_data->m_nResultsReturned;
			// There are more followers so make another callback.
			if (followers_parsed < count) {
				SteamAPICall_t api_call = SteamFriends()->EnumerateFollowingList(call_data->m_nResultsReturned);
				callResultEnumerateFollowingList.Set(api_call, this, &SteamServer::enumerate_following_list);
			}
		}
		emit_signal("enumerate_following_list", message, following);
	}
}

// Call from RequestEquippedProfileItems... nice.
void SteamServer::equipped_profile_items(EquippedProfileItems_t *call_data, bool io_failure) {
	if (io_failure) {
		steamworksError("equipped_profile_items");
	} else {
		int result = call_data->m_eResult;
		CSteamID this_steam_id = call_data->m_steamID;
		uint64_t steam_id = this_steam_id.ConvertToUint64();
		bool has_animated_avatar = call_data->m_bHasAnimatedAvatar;
		bool has_avatar_frame = call_data->m_bHasAvatarFrame;
		bool has_profile_modifier = call_data->m_bHasProfileModifier;
		bool has_profile_background = call_data->m_bHasProfileBackground;
		bool has_mini_profile_background = call_data->m_bHasMiniProfileBackground;
		// Pass all profile data to a dictionary
		Dictionary profile_data;
		profile_data["avatar_animated"] = has_animated_avatar;
		profile_data["avatar_frame"] = has_avatar_frame;
		profile_data["profile_modifier"] = has_profile_modifier;
		profile_data["profile_background"] = has_profile_background;
		profile_data["profile_mini_background"] = has_mini_profile_background;
		emit_signal("equipped_profile_items", result, steam_id, profile_data);
	}
}

// Returns the result of getFollowerCount.
void SteamServer::get_follower_count(FriendsGetFollowerCount_t *call_data, bool io_failure) {
	if (io_failure) {
		steamworksError("get_follower_count");
	} else {
		EResult result = call_data->m_eResult;
		uint64_t steam_id = call_data->m_steamID.ConvertToUint64();
		int count = call_data->m_nCount;
		emit_signal("follower_count", result, steam_id, count);
	}
}

// Returns the result of isFollowing.
void SteamServer::is_following(FriendsIsFollowing_t *call_data, bool io_failure) {
	if (io_failure) {
		steamworksError("is_following");
	} else {
		EResult result = call_data->m_eResult;
		uint64_t steam_id = call_data->m_steamID.ConvertToUint64();
		bool following = call_data->m_bIsFollowing;
		emit_signal("is_following", result, steam_id, following);
	}
}

// INVENTORY CALL RESULTS ///////////////////////
//
// Returned when you have requested the list of "eligible" promo items that can be manually granted to the given user. These are promo items of type "manual" that won't be granted automatically.
void SteamServer::inventory_eligible_promo_item(SteamInventoryEligiblePromoItemDefIDs_t *call_data, bool io_failure) {
	if (io_failure) {
		steamworksError("inventory_eligible_promo_item");
	} else {
		// Clean up call data
		CSteamID steam_id = call_data->m_steamID;
		int result = call_data->m_result;
		int eligible = call_data->m_numEligiblePromoItemDefs;
		bool cached = call_data->m_bCachedData;
		// Create the return array
		Array definitions;
		// Create the temporary ID array
		SteamItemDef_t *id_array = new SteamItemDef_t[eligible];
		// Convert eligible size
		uint32 array_size = (int)eligible;
		// Get the list
		if (SteamInventory()->GetEligiblePromoItemDefinitionIDs(steam_id, id_array, &array_size)) {
			// Loop through the temporary array and populate the return array
			for (int i = 0; i < eligible; i++) {
				definitions.append(id_array[i]);
			}
		}
		// Delete the temporary array
		delete[] id_array;
		// Return the item array as a signal
		emit_signal("inventory_eligible_promo_Item", result, cached, definitions);
	}
}

// Returned after StartPurchase is called.
void SteamServer::inventory_start_purchase_result(SteamInventoryStartPurchaseResult_t *call_data, bool io_failure) {
	if (io_failure) {
		steamworksError("inventory_start_purchase_result");
	} else {
		if (call_data->m_result == k_EResultOK) {
			uint64_t order_id = call_data->m_ulOrderID;
			uint64_t transaction_id = call_data->m_ulTransID;
			emit_signal("inventory_start_purchase_result", "success", order_id, transaction_id);
		} else {
			emit_signal("inventory_start_purchase_result", "failure", 0, 0);
		}
	}
}

// Returned after RequestPrices is called.
void SteamServer::inventory_request_prices_result(SteamInventoryRequestPricesResult_t *call_data, bool io_failure) {
	if (io_failure) {
		steamworksError("inventory_request_prices_result");
	} else {
		int result = call_data->m_result;
		String currency = call_data->m_rgchCurrency;
		emit_signal("inventory_request_prices_result", result, currency);
	}
}

// MATCHMAKING CALL RESULTS /////////////////////
//
// Signal the lobby has been created.
void SteamServer::lobby_created(LobbyCreated_t *lobbyData, bool io_failure) {
	if (io_failure) {
		steamworksError("lobby_created");
	} else {
		int connect = lobbyData->m_eResult;
		CSteamID lobby_id = lobbyData->m_ulSteamIDLobby;
		uint64_t lobby = lobby_id.ConvertToUint64();
		emit_signal("lobby_created", connect, lobby);
	}
}

// Result when requesting the lobby list. You should iterate over the returned lobbies with getLobbyByIndex, from 0 to m_nLobbiesMatching-1.
void SteamServer::lobby_match_list(LobbyMatchList_t *call_data, bool io_failure) {
	if (io_failure) {
		steamworksError("lobby_match_list");
	} else {
		int lobby_count = call_data->m_nLobbiesMatching;
		Array lobbies;
		for (int i = 0; i < lobby_count; i++) {
			CSteamID lobby_id = SteamMatchmaking()->GetLobbyByIndex(i);
			uint64_t lobby = lobby_id.ConvertToUint64();
			lobbies.append(lobby);
		}
		emit_signal("lobby_match_list", lobbies);
	}
}

// MATCHMAKING SERVER CALL RESULTS //////////////
//
void SteamServer::server_Responded(gameserveritem_t server) {
	emit_signal("server_responded");
}

//
void SteamServer::server_Failed_To_Respond() {
	emit_signal("server_failed_to_respond");
}

// PARTIES CALL RESULTS /////////////////////////
//
// This callback is used as a call response for ISteamParties::JoinParty. On success, you will have reserved a slot in the beacon-owner's party, and should use m_rgchConnectString to connect to their game and complete the process.
void SteamServer::join_party(JoinPartyCallback_t *call_data, bool io_failure) {
	if (io_failure) {
		steamworksError("join_party");
	} else {
		int result = call_data->m_eResult;
		uint64_t beacon_id = call_data->m_ulBeaconID;
		uint64_t steam_id = call_data->m_SteamIDBeaconOwner.ConvertToUint64();
		String connect_string = call_data->m_rgchConnectString;
		emit_signal("join_party", result, beacon_id, steam_id, connect_string);
	}
}

// This callback is used as a call response for ISteamParties::CreateBeacon. If successful, your beacon has been posted in the desired location and you may start receiving ISteamParties::ReservationNotificationCallback_t callbacks for users following the beacon.
void SteamServer::create_beacon(CreateBeaconCallback_t *call_data, bool io_failure) {
	if (io_failure) {
		steamworksError("create_beacon");
	} else {
		int result = call_data->m_eResult;
		uint64_t beacon_id = call_data->m_ulBeaconID;
		emit_signal("create_beacon", result, beacon_id);
	}
}

// Call result for ISteamParties::ChangeNumOpenSlots.
void SteamServer::change_num_open_slots(ChangeNumOpenSlotsCallback_t *call_data, bool io_failure) {
	if (io_failure) {
		steamworksError("change_num_open_slots");
	} else {
		int result = call_data->m_eResult;
		emit_signal("change_num_open_slots", result);
	}
}

// REMOTE STORAGE CALL RESULTS //////////////////
//
// Response when reading a file asyncrounously with FileReadAsync.
void SteamServer::file_read_async_complete(RemoteStorageFileReadAsyncComplete_t *call_data, bool io_failure) {
	if (io_failure) {
		steamworksError("file_read_async_complete");
	} else {
		uint64_t handle = call_data->m_hFileReadAsync;
		int result = call_data->m_eResult;
		uint32 offset = call_data->m_nOffset;
		uint32 read = call_data->m_cubRead;
		// Was read complete?
		PackedByteArray buffer;
		buffer.resize(read);
		bool complete = SteamRemoteStorage()->FileReadAsyncComplete(handle, buffer.ptrw(), read);
		// Create a dictionary and populate it with the results
		Dictionary file_read;
		file_read["result"] = result;
		file_read["handle"] = handle;
		file_read["buffer"] = buffer;
		file_read["offset"] = offset;
		file_read["read"] = read;
		file_read["complete"] = complete;
		emit_signal("file_read_async_complete", file_read);
	}
}

// Response to a file being shared.
void SteamServer::file_share_result(RemoteStorageFileShareResult_t *call_data, bool io_failure) {
	if (io_failure) {
		steamworksError("file_share_result");
	} else {
		int result = call_data->m_eResult;
		uint64_t handle = call_data->m_hFile;
		char name[k_cchFilenameMax];
		strcpy(name, call_data->m_rgchFilename);
		emit_signal("file_share_result", result, handle, name);
	}
}

// Response when writing a file asyncrounously with FileWriteAsync.
void SteamServer::file_write_async_complete(RemoteStorageFileWriteAsyncComplete_t *call_data, bool io_failure) {
	if (io_failure) {
		steamworksError("file_write_async_complete");
	} else {
		int result = call_data->m_eResult;
		emit_signal("file_write_async_complete", result);
	}
}

// Response when downloading UGC
void SteamServer::download_ugc_result(RemoteStorageDownloadUGCResult_t *call_data, bool io_failure) {
	if (io_failure) {
		steamworksError("download_ugc_result");
	} else {
		int result = call_data->m_eResult;
		uint64_t handle = call_data->m_hFile;
		uint32_t app_id = call_data->m_nAppID;
		int32 size = call_data->m_nSizeInBytes;
		char filename[k_cchFilenameMax];
		strcpy(filename, call_data->m_pchFileName);
		uint64_t owner_id = call_data->m_ulSteamIDOwner;
		// Pass some variable to download dictionary to bypass argument limit
		Dictionary download_data;
		download_data["handle"] = handle;
		download_data["app_id"] = app_id;
		download_data["size"] = size;
		download_data["filename"] = filename;
		download_data["owner_id"] = owner_id;
		emit_signal("download_ugc_result", result, download_data);
	}
}

// Called when the user has unsubscribed from a piece of UGC. Result from ISteamUGC::UnsubscribeItem.
void SteamServer::unsubscribe_item(RemoteStorageUnsubscribePublishedFileResult_t *call_data, bool io_failure) {
	if (io_failure) {
		steamworksError("unsubscribe_item");
	} else {
		int result = call_data->m_eResult;
		int file_id = call_data->m_nPublishedFileId;
		emit_signal("unsubscribe_item", result, file_id);
	}
}

// Called when the user has subscribed to a piece of UGC. Result from ISteamUGC::SubscribeItem.
void SteamServer::subscribe_item(RemoteStorageSubscribePublishedFileResult_t *call_data, bool io_failure) {
	if (io_failure) {
		steamworksError("subscribe_item");
	} else {
		int result = call_data->m_eResult;
		int file_id = call_data->m_nPublishedFileId;
		emit_signal("subscribe_item", result, file_id);
	}
}

// UGC CALL RESULTS /////////////////////////////
//
// The result of a call to AddAppDependency.
void SteamServer::add_app_dependency_result(AddAppDependencyResult_t *call_data, bool io_failure) {
	if (io_failure) {
		steamworksError("add_app_dependency_result");
	} else {
		EResult result = call_data->m_eResult;
		PublishedFileId_t file_id = call_data->m_nPublishedFileId;
		AppId_t app_id = call_data->m_nAppID;
		emit_signal("add_app_dependency_result", result, (uint64_t)file_id, (uint32_t)app_id);
	}
}

// The result of a call to AddDependency.
void SteamServer::add_ugc_dependency_result(AddUGCDependencyResult_t *call_data, bool io_failure) {
	if (io_failure) {
		steamworksError("add_ugc_dependency_result");
	} else {
		EResult result = call_data->m_eResult;
		PublishedFileId_t file_id = call_data->m_nPublishedFileId;
		PublishedFileId_t child_id = call_data->m_nChildPublishedFileId;
		emit_signal("add_ugc_dependency_result", result, (uint64_t)file_id, (uint64_t)child_id);
	}
}

// Result of a workshop item being created.
void SteamServer::item_created(CreateItemResult_t *call_data, bool io_failure) {
	if (io_failure) {
		steamworksError("item_created");
	} else {
		EResult result = call_data->m_eResult;
		PublishedFileId_t file_id = call_data->m_nPublishedFileId;
		bool accept_tos = call_data->m_bUserNeedsToAcceptWorkshopLegalAgreement;
		emit_signal("item_created", result, (uint64_t)file_id, accept_tos);
	}
}

// Called when getting the app dependencies for an item.
void SteamServer::get_app_dependencies_result(GetAppDependenciesResult_t *call_data, bool io_failure) {
	if (io_failure) {
		steamworksError("get_app_dependencies_result");
	} else {
		EResult result = call_data->m_eResult;
		PublishedFileId_t file_id = call_data->m_nPublishedFileId;
		//		AppId_t app_id = call_data->m_rgAppIDs;
		uint32 app_dependencies = call_data->m_nNumAppDependencies;
		uint32 total_app_dependencies = call_data->m_nTotalNumAppDependencies;
		//		emit_signal("get_app_dependencies_result", result, (uint64_t)file_id, app_id, appDependencies, totalAppDependencies);
		emit_signal("get_app_dependencies_result", result, (uint64_t)file_id, app_dependencies, total_app_dependencies);
	}
}

// Called when an attempt at deleting an item completes.
void SteamServer::item_deleted(DeleteItemResult_t *call_data, bool io_failure) {
	if (io_failure) {
		steamworksError("item_deleted");
	} else {
		EResult result = call_data->m_eResult;
		PublishedFileId_t file_id = call_data->m_nPublishedFileId;
		emit_signal("item_deleted", result, (uint64_t)file_id);
	}
}

// Called when getting the users vote status on an item.
void SteamServer::get_item_vote_result(GetUserItemVoteResult_t *call_data, bool io_failure) {
	if (io_failure) {
		steamworksError("get_item_vote_result");
	} else {
		EResult result = call_data->m_eResult;
		PublishedFileId_t file_id = call_data->m_nPublishedFileId;
		bool vote_up = call_data->m_bVotedUp;
		bool vote_down = call_data->m_bVotedDown;
		bool vote_skipped = call_data->m_bVoteSkipped;
		emit_signal("get_item_vote_result", result, (uint64_t)file_id, vote_up, vote_down, vote_skipped);
	}
}

// Purpose: The result of a call to RemoveAppDependency.
void SteamServer::remove_app_dependency_result(RemoveAppDependencyResult_t *call_data, bool io_failure) {
	if (io_failure) {
		steamworksError("remove_app_dependency_result");
	} else {
		EResult result = call_data->m_eResult;
		PublishedFileId_t file_id = call_data->m_nPublishedFileId;
		AppId_t app_id = call_data->m_nAppID;
		emit_signal("remove_app_dependency_result", result, (uint64_t)file_id, (uint32_t)app_id);
	}
}

// Purpose: The result of a call to RemoveDependency.
void SteamServer::remove_ugc_dependency_result(RemoveUGCDependencyResult_t *call_data, bool io_failure) {
	if (io_failure) {
		steamworksError("remove_ugc_dependency_result");
	} else {
		EResult result = call_data->m_eResult;
		PublishedFileId_t file_id = call_data->m_nPublishedFileId;
		PublishedFileId_t child_id = call_data->m_nChildPublishedFileId;
		emit_signal("remove_ugc_dependency_result", result, (uint64_t)file_id, (uint64_t)child_id);
	}
}

// Called when the user has voted on an item.
void SteamServer::set_user_item_vote(SetUserItemVoteResult_t *call_data, bool io_failure) {
	if (io_failure) {
		steamworksError("set_user_item_vote");
	} else {
		EResult result = call_data->m_eResult;
		PublishedFileId_t file_id = call_data->m_nPublishedFileId;
		bool vote_up = call_data->m_bVoteUp;
		emit_signal("set_user_item_vote", result, (uint64_t)file_id, vote_up);
	}
}

// Called when workshop item playtime tracking has started.
void SteamServer::start_playtime_tracking(StartPlaytimeTrackingResult_t *call_data, bool io_failure) {
	if (io_failure) {
		steamworksError("start_playtime_tracking");
	} else {
		EResult result = call_data->m_eResult;
		emit_signal("start_playtime_tracking", result);
	}
}

// Called when a UGC query request completes.
void SteamServer::ugc_query_completed(SteamUGCQueryCompleted_t *call_data, bool io_failure) {
	if (io_failure) {
		steamworksError("ugc_query_completed");
	} else {
		UGCQueryHandle_t handle = call_data->m_handle;
		EResult result = call_data->m_eResult;
		uint32 results_returned = call_data->m_unNumResultsReturned;
		uint32 total_matching = call_data->m_unTotalMatchingResults;
		bool cached = call_data->m_bCachedData;
		emit_signal("ugc_query_completed", (uint64_t)handle, result, results_returned, total_matching, cached);
	}
}

// Called when workshop item playtime tracking has stopped.
void SteamServer::stop_playtime_tracking(StopPlaytimeTrackingResult_t *call_data, bool io_failure) {
	if (io_failure) {
		steamworksError("stop_playtime_tracking");
	} else {
		EResult result = call_data->m_eResult;
		emit_signal("stop_playtime_tracking", result);
	}
}

// Result of a workshop item being updated.
void SteamServer::item_updated(SubmitItemUpdateResult_t *call_data, bool io_failure) {
	if (io_failure) {
		steamworksError("item_updated");
	} else {
		EResult result = call_data->m_eResult;
		bool accept_tos = call_data->m_bUserNeedsToAcceptWorkshopLegalAgreement;
		emit_signal("item_updated", result, accept_tos);
	}
}

// Called when the user has added or removed an item to/from their favorites.
void SteamServer::user_favorite_items_list_changed(UserFavoriteItemsListChanged_t *call_data, bool io_failure) {
	if (io_failure) {
		steamworksError("user_favorite_items_list_changed");
	} else {
		EResult result = call_data->m_eResult;
		PublishedFileId_t file_id = call_data->m_nPublishedFileId;
		bool was_add_request = call_data->m_bWasAddRequest;
		emit_signal("user_favorite_items_list_changed", result, (uint64_t)file_id, was_add_request);
	}
}

// Purpose: Status of the user's acceptable/rejection of the app's specific Workshop EULA.
void SteamServer::workshop_eula_status(WorkshopEULAStatus_t *call_data, bool io_failure) {
	if (io_failure) {
		steamworksError("workshop_eula_status");
	} else {
		int result = call_data->m_eResult;
		uint32 app_id = call_data->m_nAppID;
		// Slim down signal arguments since Godot seems to limit them to six max
		Dictionary eula_data;
		eula_data["version"] = call_data->m_unVersion; // int
		eula_data["action"] = call_data->m_rtAction; // int
		eula_data["accepted"] = call_data->m_bAccepted; // bool
		eula_data["needs_action"] = call_data->m_bNeedsAction; // bool
		emit_signal("workshop_eula_status", result, app_id, eula_data);
	}
}

// USERS CALL RESULTS ///////////////////////////
//
// Sent for games with enabled anti indulgence / duration control, for enabled users. Lets the game know whether persistent rewards or XP should be granted at normal rate, half rate, or zero rate.
void SteamServer::duration_control(DurationControl_t *call_data, bool io_failure) {
	if (io_failure) {
		steamworksError("duration_control");
	} else {
		int result = call_data->m_eResult;
		uint32 app_id = call_data->m_appid;
		bool applicable = call_data->m_bApplicable;
		int32 seconds_last = call_data->m_csecsLast5h;
		int progress = call_data->m_progress;
		int notification = call_data->m_notification;
		String verbal = "";
		// Get a more verbal response
		if (notification == 1) {
			verbal = "you've been playing for an hour";
		} else if (notification == 2) {
			verbal = "you've been playing for 3 hours; take a break";
		} else if (notification == 3) {
			verbal = "your xp / progress is half normal";
		} else if (notification == 4) {
			verbal = "your xp / progress is zero";
		} else {
			verbal = "no notification";
		}
		// Create dictionary due to "too many arguments" issue
		Dictionary duration;
		duration["app_id"] = app_id;
		duration["applicable"] = applicable;
		duration["seconds_last_5hrs"] = seconds_last;
		duration["progress"] = progress;
		duration["notification"] = notification;
		duration["notification_verbal"] = verbal;
		emit_signal("duration_control", result, duration);
	}
}

// Called when an encrypted application ticket has been received.
void SteamServer::encrypted_app_ticket_response(EncryptedAppTicketResponse_t *call_data, bool io_failure) {
	if (io_failure) {
		steamworksError("encrypted_app_ticket_response");
	} else {
		String result;
		if (call_data->m_eResult == k_EResultOK) {
			result = "ok";
		} else if (call_data->m_eResult == k_EResultNoConnection) {
			result = "no connection";
		} else if (call_data->m_eResult == k_EResultDuplicateRequest) {
			result = "duplicate request";
		} else {
			result = "limit exceeded";
		}
		emit_signal("encrypted_app_ticket_response", result);
	}
}

// Called when a connection attempt has failed. This will occur periodically if the Steam client is not connected, and has failed when retrying to establish a connection.
void SteamServer::steam_server_connect_failed(SteamServerConnectFailure_t *call_data, bool io_failure) {
	if (io_failure) {
		steamworksError("steam_server_connect_failed");
	} else {
		int result = call_data->m_eResult;
		bool retrying = call_data->m_bStillRetrying;
		emit_signal("steam_server_connected_failed", result, retrying);
	}
}

// Response when we have recieved the authentication URL after a call to requestStoreAuthURL.
void SteamServer::store_auth_url_response(StoreAuthURLResponse_t *call_data, bool io_failure) {
	if (io_failure) {
		steamworksError("store_auth_url_response");
	} else {
		String url = call_data->m_szURL;
		emit_signal("store_auth_url_response", url);
	}
}

// USER STATS CALL RESULTS //////////////////////
//
// Global achievements percentages are ready.
void SteamServer::global_achievement_percentages_ready(GlobalAchievementPercentagesReady_t *call_data, bool io_failure) {
	if (io_failure) {
		steamworksError("global_achievement_percentages_ready");
	} else {
		CSteamID game_id = call_data->m_nGameID;
		uint64_t game = game_id.ConvertToUint64();
		uint32_t result = call_data->m_eResult;
		emit_signal("global_achievement_percentages_ready", game, result);
	}
}

// Called when the global stats have been received from the server.
void SteamServer::global_stats_received(GlobalStatsReceived_t *call_data, bool io_failure) {
	if (io_failure) {
		steamworksError("global_stats_received");
	} else {
		uint64_t game_id = call_data->m_nGameID;
		String result;
		if (call_data->m_eResult == k_EResultOK) {
			result = "ok";
		} else if (call_data->m_eResult == k_EResultInvalidState) {
			result = "invalid";
		} else {
			result = "fail";
		}
		emit_signal("global_stats_received", game_id, result);
	}
}

// Result when finding a leaderboard.
void SteamServer::leaderboard_find_result(LeaderboardFindResult_t *call_data, bool io_failure) {
	if (io_failure) {
		steamworksError("leaderboard_find_result");
	} else {
		leaderboard_handle = call_data->m_hSteamLeaderboard;
		uint8_t found = call_data->m_bLeaderboardFound;
		emit_signal("leaderboard_find_result", (uint64_t)leaderboard_handle, found);
	}
}

// Called when scores for a leaderboard have been downloaded and are ready to be retrieved. After calling you must use GetDownloadedLeaderboardEntry to retrieve the info for each downloaded entry.
void SteamServer::leaderboard_scores_downloaded(LeaderboardScoresDownloaded_t *call_data, bool io_failure) {
	if (io_failure) {
		steamworksError("leaderboard_scores_downloaded");
	} else {
		// Set up a message to fill in
		String message;
		// Get this download's handle
		uint64_t this_handle = call_data->m_hSteamLeaderboard;
		// Clear previous leaderboard entries
		leaderboard_entries_array.clear();
		// Create the entry pointer and details array
		LeaderboardEntry_t *entry = memnew(LeaderboardEntry_t);
		PackedInt32Array details;
		int32 *details_pointer = NULL;
		// Resize array
		if (leaderboard_details_max > 0) {
			details.resize(leaderboard_details_max);
			details_pointer = details.ptrw();
			for (int i = 0; i < leaderboard_details_max; i++) {
				details_pointer[i] = 0;
			}
		}
		// Loop through the entries and add them as dictionaries to the array
		for (int i = 0; i < call_data->m_cEntryCount; i++) {
			if (SteamUserStats()->GetDownloadedLeaderboardEntry(call_data->m_hSteamLeaderboardEntries, i, entry, details_pointer, leaderboard_details_max)) {
				Dictionary entry_dict;
				entry_dict["score"] = entry->m_nScore;
				entry_dict["steam_id"] = uint64_t(entry->m_steamIDUser.ConvertToUint64());
				entry_dict["global_rank"] = entry->m_nGlobalRank;
				entry_dict["ugc_handle"] = uint64_t(entry->m_hUGC);
				if (leaderboard_details_max > 0) {
					PackedInt32Array array;
					array.resize(leaderboard_details_max);
					int32_t *ptr = array.ptrw();
					for (int j = 0; j < leaderboard_details_max; j++) {
						ptr[j] = details_pointer[j];
					}
					entry_dict["details"] = array;
				}
				leaderboard_entries_array.append(entry_dict);
			}
			message = "Leaderboard entries successfully retrieved";
		}
		memdelete(entry);
		// Emit the signal, with array, back
		emit_signal("leaderboard_scores_downloaded", message, this_handle, leaderboard_entries_array);
	}
}

// Result indicating that a leaderboard score has been uploaded.
void SteamServer::leaderboard_score_uploaded(LeaderboardScoreUploaded_t *call_data, bool io_failure) {
	if (io_failure) {
		steamworksError("leaderboard_scores_uploaded");
	} else {
		uint64_t this_handle = call_data->m_hSteamLeaderboard;
		uint8 success = call_data->m_bSuccess;
		// Create dictionary since Godot will not allow more than six properties to be sent back
		Dictionary this_score;
		int32 score = call_data->m_nScore;
		uint8 score_changed = call_data->m_bScoreChanged;
		int global_rank_new = call_data->m_nGlobalRankNew;
		int global_rank_prev = call_data->m_nGlobalRankPrevious;
		// Populate dictionary
		this_score["score"] = score;
		this_score["score_changed"] = score_changed;
		this_score["global_rank_new"] = global_rank_new;
		this_score["global_rank_prev"] = global_rank_prev;
		emit_signal("leaderboard_score_uploaded", success, this_handle, this_score);
	}
}

// Result indicating that user generated content has been attached to one of the current user's leaderboard entries.
void SteamServer::leaderboard_ugc_set(LeaderboardUGCSet_t *call_data, bool io_failure) {
	if (io_failure) {
		steamworksError("leaderboard_ugc_set");
	} else {
		leaderboard_handle = call_data->m_hSteamLeaderboard;
		String result;
		if (call_data->m_eResult == k_EResultOK) {
			result = "ok";
		} else if (call_data->m_eResult == k_EResultTimeout) {
			result = "timeout";
		} else {
			result = "invalid";
		}
		emit_signal("leaderboard_ugc_set", (uint64_t)leaderboard_handle, result);
	}
}

// Gets the current number of players for the current AppId.
void SteamServer::number_of_current_players(NumberOfCurrentPlayers_t *call_data, bool io_failure) {
	if (io_failure) {
		steamworksError("number_of_current_players");
	} else {
		uint8 success = call_data->m_bSuccess;
		int32 players = call_data->m_cPlayers;
		emit_signal("number_of_current_players", success, players);
	}
}

// Called when the latest stats and achievements for a specific user (including the local user) have been received from the server.
void SteamServer::user_stats_received(UserStatsReceived_t *call_data, bool io_failure) {
	if (io_failure) {
		steamworksError("user_stats_received");
	} else {
		CSteamID game_id = call_data->m_nGameID;
		uint64_t game = game_id.ConvertToUint64();
		uint32_t result = call_data->m_eResult;
		CSteamID user_id = call_data->m_steamIDUser;
		uint64_t user = user_id.ConvertToUint64();
		emit_signal("user_stats_received", game, result, user);
	}
}

// UTILITY CALL RESULTS /////////////////////////
//
// CallResult for checkFileSignature.
void SteamServer::check_file_signature(CheckFileSignature_t *call_data, bool io_failure) {
	if (io_failure) {
		steamworksError("check_file_signature");
	} else {
		String signature;
		if (call_data->m_eCheckFileSignature == k_ECheckFileSignatureNoSignaturesFoundForThisApp) {
			signature = "app not signed";
		} else if (call_data->m_eCheckFileSignature == k_ECheckFileSignatureNoSignaturesFoundForThisFile) {
			signature = "file not signed";
		} else if (call_data->m_eCheckFileSignature == k_ECheckFileSignatureFileNotFound) {
			signature = "file does not exist";
		} else if (call_data->m_eCheckFileSignature == k_ECheckFileSignatureInvalidSignature) {
			signature = "signature invalid";
		} else if (call_data->m_eCheckFileSignature == k_ECheckFileSignatureValidSignature) {
			signature = "valid";
		} else {
			signature = "invalid response";
		}
		emit_signal("check_file_signature", signature);
	}
}

/////////////////////////////////////////////////
///// BIND METHODS
/////////////////////////////////////////////////
//
void SteamServer::_bind_methods() {
	/////////////////////////////////////////////
	// FUNCTION BINDS
	/////////////////////////////////////////////
	//
	// STEAM MAIN BIND METHODS //////////////////
	ClassDB::bind_method("run_callbacks", &SteamServer::run_callbacks);
	ClassDB::bind_method("loggedOn", &SteamServer::isLoggedOn);
	ClassDB::bind_method(D_METHOD("serverInit", "bind_ip", "game_port", "query_port", "server_mode", "version_string"), &SteamServer::serverInit);
	ClassDB::bind_method("steamShutdown", &SteamServer::steamShutdown);

	// APPS BIND METHODS ////////////////////////
	ClassDB::bind_method("getDLCDataByIndex", &SteamServer::getDLCDataByIndex);
	ClassDB::bind_method(D_METHOD("isAppInstalled", "app_id"), &SteamServer::isAppInstalled);
	ClassDB::bind_method("isCybercafe", &SteamServer::isCybercafe);
	ClassDB::bind_method(D_METHOD("isDLCInstalled", "dlc_id"), &SteamServer::isDLCInstalled);
	ClassDB::bind_method("isLowViolence", &SteamServer::isLowViolence);
	ClassDB::bind_method("isSubscribed", &SteamServer::isSubscribed);
	ClassDB::bind_method(D_METHOD("isSubscribedApp", "app_id"), &SteamServer::isSubscribedApp);
	ClassDB::bind_method("isSubscribedFromFamilySharing", &SteamServer::isSubscribedFromFamilySharing);
	ClassDB::bind_method("isSubscribedFromFreeWeekend", &SteamServer::isSubscribedFromFreeWeekend);
	ClassDB::bind_method("isTimedTrial", &SteamServer::isTimedTrial);
	ClassDB::bind_method("isVACBanned", &SteamServer::isVACBanned);
	ClassDB::bind_method("getAppBuildId", &SteamServer::getAppBuildId);
	ClassDB::bind_method(D_METHOD("getAppInstallDir", "app_id"), &SteamServer::getAppInstallDir);
	ClassDB::bind_method("getAppOwner", &SteamServer::getAppOwner);
	ClassDB::bind_method("getAvailableGameLanguages", &SteamServer::getAvailableGameLanguages);
	ClassDB::bind_method("getCurrentBetaName", &SteamServer::getCurrentBetaName);
	ClassDB::bind_method("getCurrentGameLanguage", &SteamServer::getCurrentGameLanguage);
	ClassDB::bind_method("getDLCCount", &SteamServer::getDLCCount);
	ClassDB::bind_method(D_METHOD("getDLCDownloadProgress", "dlc_id"), &SteamServer::getDLCDownloadProgress);
	ClassDB::bind_method(D_METHOD("getEarliestPurchaseUnixTime", "app_id"), &SteamServer::getEarliestPurchaseUnixTime);
	ClassDB::bind_method(D_METHOD("getFileDetails", "filename"), &SteamServer::getFileDetails);
	ClassDB::bind_method(D_METHOD("getInstalledDepots", "app_id"), &SteamServer::getInstalledDepots);
	ClassDB::bind_method("getLaunchCommandLine", &SteamServer::getLaunchCommandLine);
	ClassDB::bind_method(D_METHOD("getLaunchQueryParam", "key"), &SteamServer::getLaunchQueryParam);
	ClassDB::bind_method(D_METHOD("installDLC", "dlc_id"), &SteamServer::installDLC);
	ClassDB::bind_method(D_METHOD("markContentCorrupt", "missing_files_only"), &SteamServer::markContentCorrupt);
	ClassDB::bind_method(D_METHOD("setDLCContext", "app_id"), &SteamServer::setDLCContext);
	ClassDB::bind_method(D_METHOD("uninstallDLC", "dlc_id"), &SteamServer::uninstallDLC);

	// APP LIST BIND METHODS ////////////////////
	ClassDB::bind_method("getNumInstalledApps", &SteamServer::getNumInstalledApps);
	ClassDB::bind_method(D_METHOD("getInstalledApps", "max_app_ids"), &SteamServer::getInstalledApps);
	ClassDB::bind_method(D_METHOD("getAppName", "app_id", "name_max"), &SteamServer::getAppName);
	ClassDB::bind_method(D_METHOD("getAppListInstallDir", "app_id", "name_max"), &SteamServer::getAppListInstallDir);
	ClassDB::bind_method(D_METHOD("getAppListBuildId", "app_id"), &SteamServer::getAppListInstallDir);

	// FRIENDS BIND METHODS /////////////////////
	ClassDB::bind_method(D_METHOD("activateGameOverlay", "type"), &SteamServer::activateGameOverlay, DEFVAL(""));
	ClassDB::bind_method(D_METHOD("activateGameOverlayInviteDialog", "steam_id"), &SteamServer::activateGameOverlayInviteDialog);
	ClassDB::bind_method(D_METHOD("activateGameOverlayInviteDialogConnectString", "connect_string"), &SteamServer::activateGameOverlayInviteDialogConnectString);
	ClassDB::bind_method(D_METHOD("activateGameOverlayToStore", "app_id"), &SteamServer::activateGameOverlayToStore, DEFVAL(0));
	ClassDB::bind_method(D_METHOD("activateGameOverlayToUser", "type", "steam_id"), &SteamServer::activateGameOverlayToUser, DEFVAL(""), DEFVAL(0));
	ClassDB::bind_method(D_METHOD("activateGameOverlayToWebPage", "url"), &SteamServer::activateGameOverlayToWebPage);
	ClassDB::bind_method("clearRichPresence", &SteamServer::clearRichPresence);
	ClassDB::bind_method(D_METHOD("closeClanChatWindowInSteam", "chat_id"), &SteamServer::closeClanChatWindowInSteam);
	ClassDB::bind_method(D_METHOD("downloadClanActivityCounts", "chat_id", "clans_to_request"), &SteamServer::downloadClanActivityCounts);
	ClassDB::bind_method(D_METHOD("enumerateFollowingList", "start_index"), &SteamServer::enumerateFollowingList);
	ClassDB::bind_method(D_METHOD("getChatMemberByIndex", "clan_id", "user"), &SteamServer::getChatMemberByIndex);
	ClassDB::bind_method(D_METHOD("getClanActivityCounts", "clan_id"), &SteamServer::getClanActivityCounts);
	ClassDB::bind_method(D_METHOD("getClanByIndex", "clan"), &SteamServer::getClanByIndex);
	ClassDB::bind_method(D_METHOD("getClanChatMemberCount", "clan_id"), &SteamServer::getClanChatMemberCount);
	ClassDB::bind_method(D_METHOD("getClanChatMessage", "chat_id", "message"), &SteamServer::getClanChatMessage);
	ClassDB::bind_method("getClanCount", &SteamServer::getClanCount);
	ClassDB::bind_method(D_METHOD("getClanName", "clan_id"), &SteamServer::getClanName);
	ClassDB::bind_method(D_METHOD("getClanOfficerByIndex", "clan_id", "officer"), &SteamServer::getClanOfficerByIndex);
	ClassDB::bind_method(D_METHOD("getClanOfficerCount", "clan_id"), &SteamServer::getClanOfficerCount);
	ClassDB::bind_method(D_METHOD("getClanOwner", "clan_id"), &SteamServer::getClanOwner);
	ClassDB::bind_method(D_METHOD("getClanTag", "clan_id"), &SteamServer::getClanTag);
	ClassDB::bind_method(D_METHOD("getCoplayFriend", "friend_number"), &SteamServer::getCoplayFriend);
	ClassDB::bind_method("getCoplayFriendCount", &SteamServer::getCoplayFriendCount);
	ClassDB::bind_method(D_METHOD("getFollowerCount", "steam_id"), &SteamServer::getFollowerCount);
	ClassDB::bind_method(D_METHOD("getFriendByIndex", "friend_number", "friend_flags"), &SteamServer::getFriendByIndex);
	ClassDB::bind_method(D_METHOD("getFriendCoplayGame", "friend_id"), &SteamServer::getFriendCoplayGame);
	ClassDB::bind_method(D_METHOD("getFriendCoplayTime", "friend_id"), &SteamServer::getFriendCoplayTime);
	ClassDB::bind_method("getFriendCount", &SteamServer::getFriendCount, DEFVAL(0x04));
	ClassDB::bind_method(D_METHOD("getFriendCountFromSource", "source_id"), &SteamServer::getFriendCountFromSource);
	ClassDB::bind_method(D_METHOD("getFriendFromSourceByIndex", "source_id", "friend_number"), &SteamServer::getFriendFromSourceByIndex);
	ClassDB::bind_method(D_METHOD("getFriendGamePlayed", "steam_id"), &SteamServer::getFriendGamePlayed);
	ClassDB::bind_method(D_METHOD("getFriendMessage", "friend_id", "message"), &SteamServer::getFriendMessage);
	ClassDB::bind_method(D_METHOD("getFriendPersonaName", "steam_id"), &SteamServer::getFriendPersonaName);
	ClassDB::bind_method(D_METHOD("getFriendPersonaNameHistory", "steam_id", "name_history"), &SteamServer::getFriendPersonaNameHistory);
	ClassDB::bind_method(D_METHOD("getFriendPersonaState", "steam_id"), &SteamServer::getFriendPersonaState);
	ClassDB::bind_method(D_METHOD("getFriendRelationship", "steam_id"), &SteamServer::getFriendRelationship);
	ClassDB::bind_method(D_METHOD("getFriendRichPresence", "friend_id", "key"), &SteamServer::getFriendRichPresence);
	ClassDB::bind_method(D_METHOD("getFriendRichPresenceKeyCount", "friend_id"), &SteamServer::getFriendRichPresenceKeyCount);
	ClassDB::bind_method(D_METHOD("getFriendRichPresenceKeyByIndex", "friend_id", "key"), &SteamServer::getFriendRichPresenceKeyByIndex);
	ClassDB::bind_method("getFriendsGroupCount", &SteamServer::getFriendsGroupCount);
	ClassDB::bind_method(D_METHOD("getFriendsGroupIDByIndex", "friend_group"), &SteamServer::getFriendsGroupIDByIndex);
	ClassDB::bind_method(D_METHOD("getFriendsGroupMembersCount", "friend_group"), &SteamServer::getFriendsGroupMembersCount);
	ClassDB::bind_method(D_METHOD("getFriendsGroupMembersList", "friend_group", "member_count"), &SteamServer::getFriendsGroupMembersList);
	ClassDB::bind_method(D_METHOD("getFriendsGroupName", "friend_group"), &SteamServer::getFriendsGroupName);
	ClassDB::bind_method(D_METHOD("getFriendSteamLevel", "steam_id"), &SteamServer::getFriendSteamLevel);
	ClassDB::bind_method(D_METHOD("getLargeFriendAvatar", "steam_id"), &SteamServer::getLargeFriendAvatar);
	ClassDB::bind_method(D_METHOD("getMediumFriendAvatar", "steam_id"), &SteamServer::getMediumFriendAvatar);
	ClassDB::bind_method("getPersonaName", &SteamServer::getPersonaName);
	ClassDB::bind_method("getPersonaState", &SteamServer::getPersonaState);
	ClassDB::bind_method(D_METHOD("getPlayerAvatar", "size", "steam_id"), &SteamServer::getPlayerAvatar, DEFVAL(2), DEFVAL(0));
	ClassDB::bind_method(D_METHOD("getPlayerNickname", "steam_id"), &SteamServer::getPlayerNickname);
	ClassDB::bind_method(D_METHOD("getProfileItemPropertyString", "steam_id", "item_type", "item_property"), &SteamServer::getProfileItemPropertyString);
	ClassDB::bind_method(D_METHOD("getProfileItemPropertyInt", "steam_id", "item_type", "item_propery"), &SteamServer::getProfileItemPropertyInt);
	ClassDB::bind_method("getRecentPlayers", &SteamServer::getRecentPlayers);
	ClassDB::bind_method(D_METHOD("getSmallFriendAvatar", "steam_id"), &SteamServer::getSmallFriendAvatar);
	ClassDB::bind_method("getUserFriendsGroups", &SteamServer::getUserFriendsGroups);
	ClassDB::bind_method("getUserRestrictions", &SteamServer::getUserRestrictions);
	ClassDB::bind_method("getUserSteamFriends", &SteamServer::getUserSteamFriends);
	ClassDB::bind_method("getUserSteamGroups", &SteamServer::getUserSteamGroups);
	ClassDB::bind_method(D_METHOD("hasEquippedProfileItem", "steam_id", "friend_flags"), &SteamServer::hasEquippedProfileItem);
	ClassDB::bind_method(D_METHOD("hasFriend", "steam_id", "friend_flags"), &SteamServer::hasFriend);
	ClassDB::bind_method(D_METHOD("inviteUserToGame", "friend_id", "connect_string"), &SteamServer::inviteUserToGame);
	ClassDB::bind_method(D_METHOD("isClanChatAdmin", "chat_id", "steam_id"), &SteamServer::isClanChatAdmin);
	ClassDB::bind_method(D_METHOD("isClanPublic", "clan_id"), &SteamServer::isClanPublic);
	ClassDB::bind_method(D_METHOD("isClanOfficialGameGroup", "clan_id"), &SteamServer::isClanOfficialGameGroup);
	ClassDB::bind_method(D_METHOD("isClanChatWindowOpenInSteam", "chat_id"), &SteamServer::isClanChatWindowOpenInSteam);
	ClassDB::bind_method(D_METHOD("isFollowing", "steam_id"), &SteamServer::isFollowing);
	ClassDB::bind_method(D_METHOD("isUserInSource", "steam_id", "source_id"), &SteamServer::isUserInSource);
	ClassDB::bind_method(D_METHOD("joinClanChatRoom", "clan_id"), &SteamServer::joinClanChatRoom);
	ClassDB::bind_method(D_METHOD("leaveClanChatRoom", "clan_id"), &SteamServer::leaveClanChatRoom);
	ClassDB::bind_method(D_METHOD("openClanChatWindowInSteam", "chat_id"), &SteamServer::openClanChatWindowInSteam);
	ClassDB::bind_method(D_METHOD("registerProtocolInOverlayBrowser", "protocol"), &SteamServer::registerProtocolInOverlayBrowser);
	ClassDB::bind_method(D_METHOD("replyToFriendMessage", "steam_id", "message"), &SteamServer::replyToFriendMessage);
	ClassDB::bind_method(D_METHOD("requestClanOfficerList", "clan_id"), &SteamServer::requestClanOfficerList);
	ClassDB::bind_method(D_METHOD("requestFriendRichPresence", "friend_id"), &SteamServer::requestFriendRichPresence);
	ClassDB::bind_method(D_METHOD("requestUserInformation", "steam_id", "require_name_only"), &SteamServer::requestUserInformation);
	ClassDB::bind_method(D_METHOD("sendClanChatMessage", "chat_id", "text"), &SteamServer::sendClanChatMessage);
	ClassDB::bind_method(D_METHOD("setInGameVoiceSpeaking", "steam_id", "speaking"), &SteamServer::setInGameVoiceSpeaking);
	ClassDB::bind_method(D_METHOD("setListenForFriendsMessages", "intercept"), &SteamServer::setListenForFriendsMessages);
	ClassDB::bind_method(D_METHOD("setPersonaName", "name"), &SteamServer::setPersonaName);
	ClassDB::bind_method(D_METHOD("setPlayedWith", "steam_id"), &SteamServer::setPlayedWith);
	ClassDB::bind_method(D_METHOD("setRichPresence", "key", "value"), &SteamServer::setRichPresence);

	// GAME SEARCH BIND METHODS /////////////////
	ClassDB::bind_method(D_METHOD("addGameSearchParams", "key", "values"), &SteamServer::addGameSearchParams);
	ClassDB::bind_method(D_METHOD("searchForGameWithLobby", "lobby_id", "player_min", "player_max"), &SteamServer::searchForGameWithLobby);
	ClassDB::bind_method(D_METHOD("searchForGameSolo", "player_min", "player_max"), &SteamServer::searchForGameSolo);
	ClassDB::bind_method("acceptGame", &SteamServer::acceptGame);
	ClassDB::bind_method("declineGame", &SteamServer::declineGame);
	ClassDB::bind_method(D_METHOD("retrieveConnectionDetails", "host_id"), &SteamServer::retrieveConnectionDetails);
	ClassDB::bind_method("endGameSearch", &SteamServer::endGameSearch);
	ClassDB::bind_method(D_METHOD("setGameHostParams", "key", "value"), &SteamServer::setGameHostParams);
	ClassDB::bind_method(D_METHOD("setConnectionDetails", "details", "connection_details"), &SteamServer::setConnectionDetails);
	ClassDB::bind_method(D_METHOD("requestPlayersForGame", "player_min", "player_max", "max_team_size"), &SteamServer::requestPlayersForGame);
	ClassDB::bind_method(D_METHOD("hostConfirmGameStart", "game_id"), &SteamServer::hostConfirmGameStart);
	ClassDB::bind_method("cancelRequestPlayersForGame", &SteamServer::cancelRequestPlayersForGame);
	ClassDB::bind_method(D_METHOD("submitPlayerResult", "game_id", "player_id", "player_result"), &SteamServer::submitPlayerResult);
	ClassDB::bind_method(D_METHOD("endGame", "game_id"), &SteamServer::endGame);

	// HTTP BIND METHODS ////////////////////////
	ClassDB::bind_method(D_METHOD("createCookieContainer", "allow_response_to_modify"), &SteamServer::createCookieContainer);
	ClassDB::bind_method(D_METHOD("createHTTPRequest", "request_method", "absolute_url"), &SteamServer::createHTTPRequest);
	ClassDB::bind_method(D_METHOD("deferHTTPRequest", "request_handle"), &SteamServer::deferHTTPRequest);
	ClassDB::bind_method(D_METHOD("getHTTPDownloadProgressPct", "request_handle"), &SteamServer::getHTTPDownloadProgressPct);
	ClassDB::bind_method(D_METHOD("getHTTPRequestWasTimedOut", "request_handle"), &SteamServer::getHTTPRequestWasTimedOut);
	ClassDB::bind_method(D_METHOD("getHTTPResponseBodyData", "request_handle", "buffer_size"), &SteamServer::getHTTPResponseBodyData);
	ClassDB::bind_method(D_METHOD("getHTTPResponseBodySize", "request_handle"), &SteamServer::getHTTPResponseBodySize);
	ClassDB::bind_method(D_METHOD("getHTTPResponseHeaderSize", "request_handle", "header_name"), &SteamServer::getHTTPResponseHeaderSize);
	ClassDB::bind_method(D_METHOD("getHTTPResponseHeaderValue", "request_handle", "header_name", "buffer_size"), &SteamServer::getHTTPResponseHeaderValue);
	ClassDB::bind_method(D_METHOD("getHTTPStreamingResponseBodyData", "request_handle", "offset", "buffer_size"), &SteamServer::getHTTPStreamingResponseBodyData);
	ClassDB::bind_method(D_METHOD("prioritizeHTTPRequest", "request_handle"), &SteamServer::prioritizeHTTPRequest);
	ClassDB::bind_method(D_METHOD("releaseCookieContainer", "cookie_handle"), &SteamServer::releaseCookieContainer);
	ClassDB::bind_method(D_METHOD("releaseHTTPRequest", "request_handle"), &SteamServer::releaseHTTPRequest);
	ClassDB::bind_method(D_METHOD("sendHTTPRequest", "request_handle"), &SteamServer::sendHTTPRequest);
	ClassDB::bind_method(D_METHOD("sendHTTPRequestAndStreamResponse", "request_handle"), &SteamServer::sendHTTPRequestAndStreamResponse);
	ClassDB::bind_method(D_METHOD("setHTTPCookie", "cookie_handle", "host", "url", "cookie"), &SteamServer::setHTTPCookie);
	ClassDB::bind_method(D_METHOD("setHTTPRequestAbsoluteTimeoutMS", "request_handle", "milliseconds"), &SteamServer::setHTTPRequestAbsoluteTimeoutMS);
	ClassDB::bind_method(D_METHOD("setHTTPRequestContextValue", "request_handle", "context_value"), &SteamServer::setHTTPRequestContextValue);
	ClassDB::bind_method(D_METHOD("setHTTPRequestCookieContainer", "request_handle", "cookie_handle"), &SteamServer::setHTTPRequestCookieContainer);
	ClassDB::bind_method(D_METHOD("setHTTPRequestGetOrPostParameter", "request_handle", "name", "value"), &SteamServer::setHTTPRequestGetOrPostParameter);
	ClassDB::bind_method(D_METHOD("setHTTPRequestHeaderValue", "request_handle", "header_name", "header_value"), &SteamServer::setHTTPRequestHeaderValue);
	ClassDB::bind_method(D_METHOD("setHTTPRequestNetworkActivityTimeout", "request_handle", "timeout_seconds"), &SteamServer::setHTTPRequestNetworkActivityTimeout);
	ClassDB::bind_method(D_METHOD("setHTTPRequestRawPostBody", "request_handle", "content_type", "body_length"), &SteamServer::setHTTPRequestRawPostBody);
	ClassDB::bind_method(D_METHOD("setHTTPRequestRequiresVerifiedCertificate", "request_handle", "require_verified_certificate"), &SteamServer::setHTTPRequestRequiresVerifiedCertificate);
	ClassDB::bind_method(D_METHOD("setHTTPRequestUserAgentInfo", "request_handle", "user_agent_info"), &SteamServer::setHTTPRequestUserAgentInfo);

	// INVENTORY BIND METHODS ///////////////////
	ClassDB::bind_method(D_METHOD("addPromoItem", "item"), &SteamServer::addPromoItem);
	ClassDB::bind_method(D_METHOD("addPromoItems", "items"), &SteamServer::addPromoItems);
	ClassDB::bind_method(D_METHOD("checkResultSteamID", "steam_id_expected", "this_inventory_handle"), &SteamServer::checkResultSteamID, DEFVAL(0));
	ClassDB::bind_method(D_METHOD("consumeItem", "item_consume", "quantity"), &SteamServer::consumeItem);
	ClassDB::bind_method(D_METHOD("deserializeResult", "buffer"), &SteamServer::deserializeResult);
	ClassDB::bind_method(D_METHOD("destroyResult", "this_inventory_handle"), &SteamServer::destroyResult, DEFVAL(0));
	ClassDB::bind_method(D_METHOD("exchangeItems", "output_items", "output_quantity", "input_items", "input_quantity"), &SteamServer::exchangeItems);
	ClassDB::bind_method(D_METHOD("generateItems", "items", "quantity"), &SteamServer::generateItems);
	ClassDB::bind_method("getAllItems", &SteamServer::getAllItems);
	ClassDB::bind_method(D_METHOD("getItemDefinitionProperty", "definition", "name"), &SteamServer::getItemDefinitionProperty);
	ClassDB::bind_method(D_METHOD("getItemsByID", "id_array", "count"), &SteamServer::getItemsByID);
	ClassDB::bind_method(D_METHOD("getItemPrice", "definition"), &SteamServer::getItemPrice);
	ClassDB::bind_method(D_METHOD("getItemsWithPrices", "length"), &SteamServer::getItemsWithPrices);
	ClassDB::bind_method("getNumItemsWithPrices", &SteamServer::getNumItemsWithPrices);
	ClassDB::bind_method(D_METHOD("getResultItemProperty", "index", "name", "this_inventory_handle"), &SteamServer::getResultItemProperty, DEFVAL(0));
	ClassDB::bind_method(D_METHOD("getResultItems", "this_inventory_handle"), &SteamServer::getResultItems, DEFVAL(0));
	ClassDB::bind_method(D_METHOD("getResultStatus", "this_inventory_handle"), &SteamServer::getResultStatus, DEFVAL(0));
	ClassDB::bind_method(D_METHOD("getResultTimestamp", "this_inventory_handle"), &SteamServer::getResultTimestamp, DEFVAL(0));
	ClassDB::bind_method("grantPromoItems", &SteamServer::grantPromoItems);
	ClassDB::bind_method("loadItemDefinitions", &SteamServer::loadItemDefinitions);
	ClassDB::bind_method(D_METHOD("requestEligiblePromoItemDefinitionsIDs", "steam_id"), &SteamServer::requestEligiblePromoItemDefinitionsIDs);
	ClassDB::bind_method("requestPrices", &SteamServer::requestPrices);
	ClassDB::bind_method(D_METHOD("serializeResult", "this_inventory_handle"), &SteamServer::serializeResult, DEFVAL(0));
	ClassDB::bind_method(D_METHOD("startPurchase", "items", "quantity"), &SteamServer::startPurchase);
	ClassDB::bind_method(D_METHOD("transferItemQuantity", "item_id", "quantity", "item_destination", "split"), &SteamServer::transferItemQuantity);
	ClassDB::bind_method(D_METHOD("triggerItemDrop", "definition"), &SteamServer::triggerItemDrop);
	ClassDB::bind_method("startUpdateProperties", &SteamServer::startUpdateProperties);
	ClassDB::bind_method(D_METHOD("submitUpdateProperties", "this_inventory_update_handle"), &SteamServer::submitUpdateProperties, DEFVAL(0));
	ClassDB::bind_method(D_METHOD("removeProperty", "item_id", "name", "this_inventory_update_handle"), &SteamServer::removeProperty, DEFVAL(0));
	ClassDB::bind_method(D_METHOD("setPropertyString", "item_id", "name", "value", "this_inventory_update_handle"), &SteamServer::setPropertyString, DEFVAL(0));
	ClassDB::bind_method(D_METHOD("setPropertyBool", "item_id", "name", "value", "this_inventory_update_handle"), &SteamServer::setPropertyBool, DEFVAL(0));
	ClassDB::bind_method(D_METHOD("setPropertyInt", "item_id", "name", "value", "this_inventory_update_handle"), &SteamServer::setPropertyInt, DEFVAL(0));
	ClassDB::bind_method(D_METHOD("setPropertyFloat", "item_id", "name", "value", "this_inventory_update_handle"), &SteamServer::setPropertyFloat, DEFVAL(0));

	// MATCHMAKING BIND METHODS /////////////////
	ClassDB::bind_method("getFavoriteGames", &SteamServer::getFavoriteGames);
	ClassDB::bind_method(D_METHOD("addFavoriteGame", "ip", "port", "query_port", "flags", "last_played"), &SteamServer::addFavoriteGame);
	ClassDB::bind_method(D_METHOD("removeFavoriteGame", "app_id", "ip", "port", "query_port", "flags"), &SteamServer::removeFavoriteGame);
	ClassDB::bind_method("requestLobbyList", &SteamServer::requestLobbyList);
	ClassDB::bind_method(D_METHOD("addRequestLobbyListStringFilter", "key_to_match", "value_to_match", "comparison_type"), &SteamServer::addRequestLobbyListStringFilter);
	ClassDB::bind_method(D_METHOD("addRequestLobbyListNumericalFilter", "key_to_match", "value_to_match", "comparison_type"), &SteamServer::addRequestLobbyListNumericalFilter);
	ClassDB::bind_method(D_METHOD("addRequestLobbyListNearValueFilter", "key_to_match", "value_to_be_close_to"), &SteamServer::addRequestLobbyListNearValueFilter);
	ClassDB::bind_method(D_METHOD("addRequestLobbyListFilterSlotsAvailable", "slots_available"), &SteamServer::addRequestLobbyListFilterSlotsAvailable);
	ClassDB::bind_method(D_METHOD("addRequestLobbyListDistanceFilter", "distance_filter"), &SteamServer::addRequestLobbyListDistanceFilter);
	ClassDB::bind_method(D_METHOD("addRequestLobbyListResultCountFilter", "max_results"), &SteamServer::addRequestLobbyListResultCountFilter);
	ClassDB::bind_method(D_METHOD("createLobby", "lobby_type", "max_members"), &SteamServer::createLobby, DEFVAL(2));
	ClassDB::bind_method(D_METHOD("joinLobby", "steam_lobby_id"), &SteamServer::joinLobby);
	ClassDB::bind_method(D_METHOD("leaveLobby", "steam_lobby_id"), &SteamServer::leaveLobby);
	ClassDB::bind_method(D_METHOD("inviteUserToLobby", "steam_lobby_id", "steam_id_invitee"), &SteamServer::inviteUserToLobby);
	ClassDB::bind_method(D_METHOD("getNumLobbyMembers", "steam_lobby_id"), &SteamServer::getNumLobbyMembers);
	ClassDB::bind_method(D_METHOD("getLobbyMemberByIndex", "steam_lobby_id", "member"), &SteamServer::getLobbyMemberByIndex);
	ClassDB::bind_method(D_METHOD("getLobbyData", "steam_lobby_id", "key"), &SteamServer::getLobbyData);
	ClassDB::bind_method(D_METHOD("setLobbyData", "steam_lobby_id", "key", "value"), &SteamServer::setLobbyData);
	ClassDB::bind_method(D_METHOD("getAllLobbyData", "steam_lobby_id"), &SteamServer::getAllLobbyData);
	ClassDB::bind_method(D_METHOD("deleteLobbyData", "steam_lobby_id", "key"), &SteamServer::deleteLobbyData);
	ClassDB::bind_method(D_METHOD("getLobbyMemberData", "steam_lobby_id", "steam_id_user", "key"), &SteamServer::getLobbyMemberData);
	ClassDB::bind_method(D_METHOD("setLobbyMemberData", "steam_lobby_id", "key", "value"), &SteamServer::setLobbyMemberData);
	ClassDB::bind_method(D_METHOD("sendLobbyChatMsg", "steam_lobby_id", "message_body"), &SteamServer::sendLobbyChatMsg);
	ClassDB::bind_method(D_METHOD("requestLobbyData", "steam_lobby_id"), &SteamServer::requestLobbyData);
	ClassDB::bind_method(D_METHOD("setLobbyGameServer", "steam_lobby_id", "server_ip", "server_port", "steam_id_game_server"), &SteamServer::setLobbyGameServer);
	ClassDB::bind_method(D_METHOD("getLobbyGameServer", "steam_lobby_id"), &SteamServer::getLobbyGameServer);
	ClassDB::bind_method(D_METHOD("setLobbyMemberLimit", "steam_lobby_id", "max_members"), &SteamServer::setLobbyMemberLimit);
	ClassDB::bind_method(D_METHOD("getLobbyMemberLimit", "steam_lobby_id"), &SteamServer::getLobbyMemberLimit);
	ClassDB::bind_method(D_METHOD("setLobbyType", "steam_lobby_id", "lobby_type"), &SteamServer::setLobbyType);
	ClassDB::bind_method(D_METHOD("setLobbyJoinable", "steam_lobby_id", "joinable"), &SteamServer::setLobbyJoinable);
	ClassDB::bind_method(D_METHOD("getLobbyOwner", "steam_lobby_id"), &SteamServer::getLobbyOwner);
	ClassDB::bind_method(D_METHOD("setLobbyOwner", "steam_lobby_id", "steam_id_new_owner"), &SteamServer::setLobbyOwner);

	// GAME SERVERS BIND METHODS /////////
	ClassDB::bind_method(D_METHOD("associateWithClan", "clan_id"), &SteamServer::associateWithClan);
	ClassDB::bind_method(D_METHOD("beginServerAuthSession", "auth_ticket", "ticket_size", "steam_id"), &SteamServer::beginServerAuthSession);
	ClassDB::bind_method(D_METHOD("cancelServerAuthTicket", "auth_ticket"), &SteamServer::cancelServerAuthTicket);
	ClassDB::bind_method(D_METHOD("clearAllKeyValues"), &SteamServer::clearAllKeyValues);
	ClassDB::bind_method(D_METHOD("computeNewPlayerCompatibility", "steam_id"), &SteamServer::computeNewPlayerCompatibility);
	ClassDB::bind_method(D_METHOD("endServerAuthSession", "steam_id"), &SteamServer::endServerAuthSession);
	ClassDB::bind_method(D_METHOD("getPublicIP"), &SteamServer::getPublicIP);
	ClassDB::bind_method(D_METHOD("getSteamServerID"), &SteamServer::getSteamServerID);
	ClassDB::bind_method(D_METHOD("getServerAuthSessionTicket", "auth_ticket", "max_ticket_size", "ticket_size", "ip_addr", "port"), &SteamServer::getServerAuthSessionTicket);
	ClassDB::bind_method(D_METHOD("logOff"), &SteamServer::logOff);
	ClassDB::bind_method(D_METHOD("logOn", "token"), &SteamServer::logOn);
	ClassDB::bind_method(D_METHOD("logOnAnonymous"), &SteamServer::logOnAnonymous);
	ClassDB::bind_method(D_METHOD("secure"), &SteamServer::secure);
	ClassDB::bind_method(D_METHOD("setAdvertiseServerActive", "active"), &SteamServer::setAdvertiseServerActive);
	ClassDB::bind_method(D_METHOD("setBotPlayerCount", "bots"), &SteamServer::setBotPlayerCount);
	ClassDB::bind_method(D_METHOD("setDedicatedServer", "dedicated"), &SteamServer::setDedicatedServer);
	ClassDB::bind_method(D_METHOD("setGameData", "data"), &SteamServer::setGameData);
	ClassDB::bind_method(D_METHOD("setGameDescription", "description"), &SteamServer::setGameDescription);
	ClassDB::bind_method(D_METHOD("setGameTags", "tag_list"), &SteamServer::setGameTags);
	ClassDB::bind_method(D_METHOD("setKeyValue", "password_protected"), &SteamServer::setKeyValue);
	ClassDB::bind_method(D_METHOD("setMapName", "map"), &SteamServer::setMapName);
	ClassDB::bind_method(D_METHOD("setMaxPlayerCount", "players_max"), &SteamServer::setMaxPlayerCount);
	ClassDB::bind_method(D_METHOD("setModDir", "mod_dir"), &SteamServer::setModDir);
	ClassDB::bind_method(D_METHOD("setPasswordProtected", "password_protected"), &SteamServer::setPasswordProtected);
	ClassDB::bind_method(D_METHOD("setProduct", "product_name"), &SteamServer::setProduct);
	ClassDB::bind_method(D_METHOD("setRegion", "region"), &SteamServer::setRegion);
	ClassDB::bind_method(D_METHOD("setServerName", "name"), &SteamServer::setServerName);
	ClassDB::bind_method(D_METHOD("setSpectatorPort", "port"), &SteamServer::setSpectatorPort);
	ClassDB::bind_method(D_METHOD("setSpectatorServerName", "name"), &SteamServer::setSpectatorServerName);
	ClassDB::bind_method(D_METHOD("serverUserHasLicenseForApp", "steam_id", "app_id"), &SteamServer::serverUserHasLicenseForApp);

	ClassDB::bind_method(D_METHOD("get_init_error_message"), &SteamServer::get_init_error_message);
	ADD_PROPERTY(PropertyInfo(Variant::STRING, "init_error_message"), "set_init_error_message", "get_init_error_message");

	// MATCHMAKING SERVERS BIND METHODS /////////
	ClassDB::bind_method(D_METHOD("cancelQuery", "server_list_request"), &SteamServer::cancelQuery);
	ClassDB::bind_method(D_METHOD("cancelServerQuery", "server_query"), &SteamServer::cancelServerQuery);
	ClassDB::bind_method(D_METHOD("getServerCount", "server_list_request"), &SteamServer::getServerCount);
	ClassDB::bind_method(D_METHOD("getServerDetails", "server", "server_list_request"), &SteamServer::getServerDetails);
	ClassDB::bind_method(D_METHOD("isRefreshing", "server_list_request"), &SteamServer::isRefreshing);
	ClassDB::bind_method(D_METHOD("pingServer", "ip", "port"), &SteamServer::pingServer);
	ClassDB::bind_method(D_METHOD("playerDetails", "ip", "port"), &SteamServer::playerDetails);
	ClassDB::bind_method(D_METHOD("refreshQuery", "server_list_request"), &SteamServer::refreshQuery);
	ClassDB::bind_method(D_METHOD("refreshServer", "server", "server_list_request"), &SteamServer::refreshServer);
	ClassDB::bind_method(D_METHOD("releaseRequest", "server_list_request"), &SteamServer::releaseRequest);
	ClassDB::bind_method(D_METHOD("requestFavoritesServerList", "app_id", "filters"), &SteamServer::requestFavoritesServerList);
	ClassDB::bind_method(D_METHOD("requestFriendsServerList", "app_id", "filters"), &SteamServer::requestFriendsServerList);
	ClassDB::bind_method(D_METHOD("requestHistoryServerList", "app_id", "filters"), &SteamServer::requestHistoryServerList);
	ClassDB::bind_method(D_METHOD("requestInternetServerList", "app_id", "filters"), &SteamServer::requestInternetServerList);
	ClassDB::bind_method(D_METHOD("requestLANServerList", "app_id"), &SteamServer::requestLANServerList);
	ClassDB::bind_method(D_METHOD("requestSpectatorServerList", "app_id", "filters"), &SteamServer::requestSpectatorServerList);
	ClassDB::bind_method(D_METHOD("serverRules", "ip", "port"), &SteamServer::serverRules);

	// NETWORKING BIND METHODS //////////////////
	ClassDB::bind_method(D_METHOD("acceptP2PSessionWithUser", "steam_id_remote"), &SteamServer::acceptP2PSessionWithUser);
	ClassDB::bind_method(D_METHOD("allowP2PPacketRelay", "allow"), &SteamServer::allowP2PPacketRelay);
	ClassDB::bind_method(D_METHOD("closeP2PChannelWithUser", "steam_id_remote", "channel"), &SteamServer::closeP2PChannelWithUser);
	ClassDB::bind_method(D_METHOD("closeP2PSessionWithUser", "steam_id_remote"), &SteamServer::closeP2PSessionWithUser);
	ClassDB::bind_method(D_METHOD("getP2PSessionState", "steam_id_remote"), &SteamServer::getP2PSessionState);
	ClassDB::bind_method(D_METHOD("getAvailableP2PPacketSize", "channel"), &SteamServer::getAvailableP2PPacketSize, DEFVAL(0));
	ClassDB::bind_method(D_METHOD("readP2PPacket", "packet", "channel"), &SteamServer::readP2PPacket, DEFVAL(0));
	ClassDB::bind_method(D_METHOD("sendP2PPacket", "steam_id_remote", "data", "send_type", "channel"), &SteamServer::sendP2PPacket, DEFVAL(0));

	// NETWORKING MESSAGES BIND METHODS /////////
	ClassDB::bind_method(D_METHOD("acceptSessionWithUser", "identity_reference"), &SteamServer::acceptSessionWithUser);
	ClassDB::bind_method(D_METHOD("closeChannelWithUser", "identity_reference", "channel"), &SteamServer::closeChannelWithUser);
	ClassDB::bind_method(D_METHOD("closeSessionWithUser", "identity_reference"), &SteamServer::closeSessionWithUser);
	ClassDB::bind_method(D_METHOD("getSessionConnectionInfo", "identity_reference", "get_connection", "get_status"), &SteamServer::getSessionConnectionInfo);
	ClassDB::bind_method(D_METHOD("receiveMessagesOnChannel", "channel", "max_messages"), &SteamServer::receiveMessagesOnChannel);
	ClassDB::bind_method(D_METHOD("sendMessageToUser", "identity_reference", "data", "flags", "channel"), &SteamServer::sendMessageToUser);

	// NETWORKING SOCKETS BIND METHODS //////////
	ClassDB::bind_method(D_METHOD("acceptConnection", "connection"), &SteamServer::acceptConnection);
	ClassDB::bind_method(D_METHOD("beginAsyncRequestFakeIP", "num_ports"), &SteamServer::beginAsyncRequestFakeIP);
	ClassDB::bind_method(D_METHOD("closeConnection", "peer", "reason", "debug_message", "linger"), &SteamServer::closeConnection);
	ClassDB::bind_method(D_METHOD("closeListenSocket", "socket"), &SteamServer::closeListenSocket);
	ClassDB::bind_method(D_METHOD("configureConnectionLanes", "connection", "lanes", "priorities", "weights"), &SteamServer::configureConnectionLanes);
	ClassDB::bind_method(D_METHOD("connectP2P", "identity_reference", "virtual_port", "options"), &SteamServer::connectP2P);
	ClassDB::bind_method(D_METHOD("connectByIPAddress", "ip_address_with_port", "options"), &SteamServer::connectByIPAddress);
	ClassDB::bind_method(D_METHOD("connectToHostedDedicatedServer", "identity_reference", "virtual_port", "options"), &SteamServer::connectToHostedDedicatedServer);
	ClassDB::bind_method(D_METHOD("createFakeUDPPort", "fake_server_port"), &SteamServer::createFakeUDPPort);
	ClassDB::bind_method(D_METHOD("createHostedDedicatedServerListenSocket", "virtual_port", "options"), &SteamServer::createHostedDedicatedServerListenSocket);
	ClassDB::bind_method(D_METHOD("createListenSocketIP", "ip_reference", "options"), &SteamServer::createListenSocketIP);
	ClassDB::bind_method(D_METHOD("createListenSocketP2P", "virtual_port", "options"), &SteamServer::createListenSocketP2P);
	ClassDB::bind_method(D_METHOD("createListenSocketP2PFakeIP", "fake_port", "options"), &SteamServer::createListenSocketP2PFakeIP);
	ClassDB::bind_method("createPollGroup", &SteamServer::createPollGroup);
	ClassDB::bind_method(D_METHOD("createSocketPair", "loopback", "identity_reference1", "identity_reference2"), &SteamServer::createSocketPair);
	ClassDB::bind_method(D_METHOD("destroyPollGroup", "poll_group"), &SteamServer::destroyPollGroup);
	//	ClassDB::bind_method(D_METHOD("findRelayAuthTicketForServer", "port"), &Steam::findRelayAuthTicketForServer);	<------ Uses datagram relay structs which were removed from base SDK
	ClassDB::bind_method(D_METHOD("flushMessagesOnConnection", "connection_handle"), &SteamServer::flushMessagesOnConnection);
	ClassDB::bind_method("getAuthenticationStatus", &SteamServer::getAuthenticationStatus);
	ClassDB::bind_method("getCertificateRequest", &SteamServer::getCertificateRequest);
	ClassDB::bind_method(D_METHOD("getConnectionInfo", "connection_handle"), &SteamServer::getConnectionInfo);
	ClassDB::bind_method(D_METHOD("getConnectionName", "peer"), &SteamServer::getConnectionName);
	ClassDB::bind_method(D_METHOD("getConnectionRealTimeStatus", "connection_handle", "lanes", "get_status"), &SteamServer::getConnectionRealTimeStatus, DEFVAL(true));
	ClassDB::bind_method(D_METHOD("getConnectionUserData", "peer"), &SteamServer::getConnectionUserData);
	ClassDB::bind_method(D_METHOD("getDetailedConnectionStatus", "connection_handle"), &SteamServer::getDetailedConnectionStatus);
	ClassDB::bind_method(D_METHOD("getFakeIP", "first_port"), &SteamServer::getFakeIP, DEFVAL(0));
	//	ClassDB::bind_method(D_METHOD("getGameCoordinatorServerLogin", "app_data"), &Steam::getGameCoordinatorServerLogin);	<------ Uses datagram relay structs which were removed from base SDK
	//	ClassDB::bind_method("getHostedDedicatedServerAddress", &Steam::getHostedDedicatedServerAddress);	<------ Uses datagram relay structs which were removed from base SDK
	ClassDB::bind_method("getHostedDedicatedServerPOPId", &SteamServer::getHostedDedicatedServerPOPId);
	ClassDB::bind_method("getHostedDedicatedServerPort", &SteamServer::getHostedDedicatedServerPort);
	ClassDB::bind_method(D_METHOD("getListenSocketAddress", "socket"), &SteamServer::getListenSocketAddress);
	ClassDB::bind_method("getIdentity", &SteamServer::getIdentity);
	ClassDB::bind_method(D_METHOD("getRemoteFakeIPForConnection", "connection"), &SteamServer::getRemoteFakeIPForConnection);
	ClassDB::bind_method("initAuthentication", &SteamServer::initAuthentication);
	ClassDB::bind_method(D_METHOD("receiveMessagesOnConnection", "connection", "max_messages"), &SteamServer::receiveMessagesOnConnection);
	ClassDB::bind_method(D_METHOD("receiveMessagesOnPollGroup", "poll_group", "max_messages"), &SteamServer::receiveMessagesOnPollGroup);
	//	ClassDB::bind_method("receivedRelayAuthTicket", &Steam::receivedRelayAuthTicket);	<------ Uses datagram relay structs which were removed from base SDK
	ClassDB::bind_method(D_METHOD("resetIdentity", "this_identity"), &SteamServer::resetIdentity);
	ClassDB::bind_method("runNetworkingCallbacks", &SteamServer::runNetworkingCallbacks);
	ClassDB::bind_method(D_METHOD("sendMessages", "messages", "data", "connection_handle", "flags"), &SteamServer::sendMessages);
	ClassDB::bind_method(D_METHOD("sendMessageToConnection", "connection_handle", "data", "flags"), &SteamServer::sendMessageToConnection);
	ClassDB::bind_method(D_METHOD("setCertificate", "certificate"), &SteamServer::setCertificate);
	ClassDB::bind_method(D_METHOD("setConnectionPollGroup", "connection_handle", "poll_group"), &SteamServer::setConnectionPollGroup);
	ClassDB::bind_method(D_METHOD("setConnectionName", "peer", "name"), &SteamServer::setConnectionName);

	// NETWORKING TYPES BIND METHODS ////////////
	ClassDB::bind_method(D_METHOD("addIdentity", "reference_name"), &SteamServer::addIdentity);
	ClassDB::bind_method(D_METHOD("addIPAddress", "reference_name"), &SteamServer::addIPAddress);
	ClassDB::bind_method(D_METHOD("clearIdentity", "reference_name"), &SteamServer::clearIdentity);
	ClassDB::bind_method(D_METHOD("clearIPAddress", "reference_name"), &SteamServer::clearIPAddress);
	ClassDB::bind_method(D_METHOD("getGenericBytes", "reference_name"), &SteamServer::getGenericBytes);
	ClassDB::bind_method(D_METHOD("getGenericString", "reference_name"), &SteamServer::getGenericString);
	ClassDB::bind_method("getIdentities", &SteamServer::getIdentities);
	ClassDB::bind_method(D_METHOD("getIdentityIPAddr", "reference_name"), &SteamServer::getIdentityIPAddr);
	ClassDB::bind_method(D_METHOD("getIdentitySteamID", "reference_name"), &SteamServer::getIdentitySteamID);
	ClassDB::bind_method(D_METHOD("getIdentitySteamID64", "reference_name"), &SteamServer::getIdentitySteamID64);
	ClassDB::bind_method("getIPAddresses", &SteamServer::getIPAddresses);
	ClassDB::bind_method(D_METHOD("getIPv4", "reference_name"), &SteamServer::getIPv4);
	ClassDB::bind_method(D_METHOD("getPSNID", "reference_name"), &SteamServer::getPSNID);
	ClassDB::bind_method(D_METHOD("getStadiaID", "reference_name"), &SteamServer::getStadiaID);
	ClassDB::bind_method(D_METHOD("getXboxPairwiseID", "reference_name"), &SteamServer::getXboxPairwiseID);
	ClassDB::bind_method(D_METHOD("isAddressLocalHost", "reference_name"), &SteamServer::isAddressLocalHost);
	ClassDB::bind_method(D_METHOD("isIdentityInvalid", "reference_name"), &SteamServer::isIdentityInvalid);
	ClassDB::bind_method(D_METHOD("isIdentityLocalHost", "reference_name"), &SteamServer::isIdentityLocalHost);
	ClassDB::bind_method(D_METHOD("isIPv4", "reference_name"), &SteamServer::isIPv4);
	ClassDB::bind_method(D_METHOD("isIPv6AllZeros", "reference_name"), &SteamServer::isIPv6AllZeros);
	ClassDB::bind_method(D_METHOD("parseIdentityString", "reference_name", "string_to_parse"), &SteamServer::parseIdentityString);
	ClassDB::bind_method(D_METHOD("parseIPAddressString", "reference_name", "string_to_parse"), &SteamServer::parseIPAddressString);
	ClassDB::bind_method(D_METHOD("setGenericBytes", "reference_name", "data"), &SteamServer::setGenericBytes);
	ClassDB::bind_method(D_METHOD("setGenericString", "reference_name", "this_string"), &SteamServer::setGenericString);
	ClassDB::bind_method(D_METHOD("setIdentityIPAddr", "reference_name", "ip_address_name"), &SteamServer::setIdentityIPAddr);
	ClassDB::bind_method(D_METHOD("setIdentityLocalHost", "reference_name"), &SteamServer::setIdentityLocalHost);
	ClassDB::bind_method(D_METHOD("setIdentitySteamID", "reference_name", "steam_id"), &SteamServer::setIdentitySteamID);
	ClassDB::bind_method(D_METHOD("setIdentitySteamID64", "reference_name", "steam_id"), &SteamServer::setIdentitySteamID64);
	ClassDB::bind_method(D_METHOD("setIPv4", "reference_name", "ip", "port"), &SteamServer::setIPv4);
	ClassDB::bind_method(D_METHOD("setIPv6", "reference_name", "ipv6", "port"), &SteamServer::setIPv6);
	ClassDB::bind_method(D_METHOD("setIPv6LocalHost", "reference_name", "port"), &SteamServer::setIPv6LocalHost, DEFVAL(0));
	ClassDB::bind_method(D_METHOD("setPSNID", "reference_name", "psn_id"), &SteamServer::setPSNID);
	ClassDB::bind_method(D_METHOD("setStadiaID", "reference_name", "stadia_id"), &SteamServer::setStadiaID);
	ClassDB::bind_method(D_METHOD("setXboxPairwiseID", "reference_name", "xbox_id"), &SteamServer::setXboxPairwiseID);
	ClassDB::bind_method(D_METHOD("toIdentityString", "reference_name"), &SteamServer::toIdentityString);
	ClassDB::bind_method(D_METHOD("toIPAddressString", "reference_name", "with_port"), &SteamServer::toIPAddressString);

	// NETWORKING UTILS BIND METHODS ////////////
	ClassDB::bind_method(D_METHOD("checkPingDataUpToDate", "max_age_in_seconds"), &SteamServer::checkPingDataUpToDate);
	ClassDB::bind_method(D_METHOD("convertPingLocationToString", "location"), &SteamServer::convertPingLocationToString);
	ClassDB::bind_method(D_METHOD("estimatePingTimeBetweenTwoLocations", "location1", "location2"), &SteamServer::estimatePingTimeBetweenTwoLocations);
	ClassDB::bind_method(D_METHOD("estimatePingTimeFromLocalHost", "location"), &SteamServer::estimatePingTimeFromLocalHost);
	ClassDB::bind_method(D_METHOD("getConfigValue", "config_value", "scope_type", "connection_handle"), &SteamServer::getConfigValue);
	ClassDB::bind_method(D_METHOD("getConfigValueInfo", "config_value"), &SteamServer::getConfigValueInfo);
	ClassDB::bind_method(D_METHOD("getDirectPingToPOP", "pop_id"), &SteamServer::getDirectPingToPOP);
	ClassDB::bind_method("getLocalPingLocation", &SteamServer::getLocalPingLocation);
	ClassDB::bind_method("getLocalTimestamp", &SteamServer::getLocalTimestamp);
	ClassDB::bind_method(D_METHOD("getPingToDataCenter", "pop_id"), &SteamServer::getPingToDataCenter);
	ClassDB::bind_method("getPOPCount", &SteamServer::getPOPCount);
	ClassDB::bind_method("getPOPList", &SteamServer::getPOPList);
	ClassDB::bind_method("getRelayNetworkStatus", &SteamServer::getRelayNetworkStatus);
	ClassDB::bind_method("initRelayNetworkAccess", &SteamServer::initRelayNetworkAccess);
	ClassDB::bind_method(D_METHOD("parsePingLocationString", "string"), &SteamServer::parsePingLocationString);
	ClassDB::bind_method(D_METHOD("setConnectionConfigValueFloat", "connection", "config", "value"), &SteamServer::setConnectionConfigValueFloat);
	ClassDB::bind_method(D_METHOD("setConnectionConfigValueInt32", "connection", "config", "value"), &SteamServer::setConnectionConfigValueInt32);
	ClassDB::bind_method(D_METHOD("setConnectionConfigValueString", "connection", "config", "value"), &SteamServer::setConnectionConfigValueString);
	//	ClassDB::bind_method(D_METHOD("setConfigValue", "setting", "scope_type", "connection_handle", "data_type", "value"), &Steam::setConfigValue);
	ClassDB::bind_method(D_METHOD("setGlobalConfigValueFloat", "config", "value"), &SteamServer::setGlobalConfigValueFloat);
	ClassDB::bind_method(D_METHOD("setGlobalConfigValueInt32", "config", "value"), &SteamServer::setGlobalConfigValueInt32);
	ClassDB::bind_method(D_METHOD("setGlobalConfigValueString", "config", "value"), &SteamServer::setGlobalConfigValueString);

	// PARTIES BIND METHODS /////////////////////
	ClassDB::bind_method(D_METHOD("cancelReservation", "beacon_id", "steam_id"), &SteamServer::cancelReservation);
	ClassDB::bind_method(D_METHOD("changeNumOpenSlots", "beacon_id", "open_slots"), &SteamServer::changeNumOpenSlots);
	ClassDB::bind_method(D_METHOD("createBeacon", "open_slots", "location_id", "type", "connect_string", "metadata"), &SteamServer::createBeacon);
	ClassDB::bind_method(D_METHOD("destroyBeacon", "beacon_id"), &SteamServer::destroyBeacon);
	ClassDB::bind_method(D_METHOD("getAvailableBeaconLocations", "max"), &SteamServer::getAvailableBeaconLocations);
	ClassDB::bind_method(D_METHOD("getBeaconByIndex", "index"), &SteamServer::getBeaconByIndex);
	ClassDB::bind_method(D_METHOD("getBeaconDetails", "beacon_id"), &SteamServer::getBeaconDetails);
	ClassDB::bind_method(D_METHOD("getBeaconLocationData", "location_id", "location_type", "location_data"), &SteamServer::getBeaconLocationData);
	ClassDB::bind_method("getNumActiveBeacons", &SteamServer::getNumActiveBeacons);
	ClassDB::bind_method(D_METHOD("joinParty", "beacon_id"), &SteamServer::joinParty);
	ClassDB::bind_method(D_METHOD("onReservationCompleted", "beacon_id", "steam_id"), &SteamServer::onReservationCompleted);

	// REMOTE STORAGE BIND METHODS //////////////
	ClassDB::bind_method("beginFileWriteBatch", &SteamServer::beginFileWriteBatch);
	ClassDB::bind_method("endFileWriteBatch", &SteamServer::endFileWriteBatch);
	ClassDB::bind_method(D_METHOD("fileDelete", "file"), &SteamServer::fileDelete);
	ClassDB::bind_method(D_METHOD("fileExists", "file"), &SteamServer::fileExists);
	ClassDB::bind_method(D_METHOD("fileForget", "file"), &SteamServer::fileForget);
	ClassDB::bind_method(D_METHOD("filePersisted", "file"), &SteamServer::filePersisted);
	ClassDB::bind_method(D_METHOD("fileRead", "file", "data_to_read"), &SteamServer::fileRead);
	ClassDB::bind_method(D_METHOD("fileReadAsync", "file", "offset", "data_to_read"), &SteamServer::fileReadAsync);
	ClassDB::bind_method(D_METHOD("fileShare", "file"), &SteamServer::fileShare);
	ClassDB::bind_method(D_METHOD("fileWrite", "file", "data", "size"), &SteamServer::fileWrite, DEFVAL(0));
	ClassDB::bind_method(D_METHOD("fileWriteAsync", "file", "data", "size"), &SteamServer::fileWriteAsync, DEFVAL(0));
	ClassDB::bind_method(D_METHOD("fileWriteStreamCancel", "write_handle"), &SteamServer::fileWriteStreamCancel);
	ClassDB::bind_method(D_METHOD("fileWriteStreamClose", "write_handle"), &SteamServer::fileWriteStreamClose);
	ClassDB::bind_method(D_METHOD("fileWriteStreamOpen", "file"), &SteamServer::fileWriteStreamOpen);
	ClassDB::bind_method(D_METHOD("fileWriteStreamWriteChunk", "write_handle", "data"), &SteamServer::fileWriteStreamWriteChunk);
	ClassDB::bind_method("getCachedUGCCount", &SteamServer::getCachedUGCCount);
	ClassDB::bind_method(D_METHOD("getCachedUGCHandle", "content"), &SteamServer::getCachedUGCHandle);
	ClassDB::bind_method("getFileCount", &SteamServer::getFileCount);
	ClassDB::bind_method(D_METHOD("getFileNameAndSize", "file"), &SteamServer::getFileNameAndSize);
	ClassDB::bind_method(D_METHOD("getFileSize", "file"), &SteamServer::getFileSize);
	ClassDB::bind_method(D_METHOD("getFileTimestamp", "file"), &SteamServer::getFileTimestamp);
	ClassDB::bind_method(D_METHOD("getLocalFileChange", "file"), &SteamServer::getLocalFileChange);
	ClassDB::bind_method("getLocalFileChangeCount", &SteamServer::getLocalFileChangeCount);
	ClassDB::bind_method("getQuota", &SteamServer::getQuota);
	ClassDB::bind_method(D_METHOD("getSyncPlatforms", "file"), &SteamServer::getSyncPlatforms);
	ClassDB::bind_method(D_METHOD("getUGCDetails", "content"), &SteamServer::getUGCDetails);
	ClassDB::bind_method(D_METHOD("getUGCDownloadProgress", "content"), &SteamServer::getUGCDownloadProgress);
	ClassDB::bind_method("isCloudEnabledForAccount", &SteamServer::isCloudEnabledForAccount);
	ClassDB::bind_method("isCloudEnabledForApp", &SteamServer::isCloudEnabledForApp);
	ClassDB::bind_method(D_METHOD("setCloudEnabledForApp", "enabled"), &SteamServer::setCloudEnabledForApp);
	ClassDB::bind_method(D_METHOD("setSyncPlatforms", "file", "platform"), &SteamServer::setSyncPlatforms);
	ClassDB::bind_method(D_METHOD("ugcDownload", "content", "priority"), &SteamServer::ugcDownload);
	ClassDB::bind_method(D_METHOD("ugcDownloadToLocation", "content", "location", "priority"), &SteamServer::ugcDownloadToLocation);
	ClassDB::bind_method(D_METHOD("ugcRead", "content", "data_size", "offset", "action"), &SteamServer::ugcRead);

	// UGC BIND METHODS ////////////////////
	ClassDB::bind_method(D_METHOD("addAppDependency", "published_file_id", "app_id"), &SteamServer::addAppDependency);
	ClassDB::bind_method(D_METHOD("addContentDescriptor", "update_handle", "descriptor_id"), &SteamServer::addContentDescriptor);
	ClassDB::bind_method(D_METHOD("addDependency", "published_file_id", "child_published_file_id"), &SteamServer::addDependency);
	ClassDB::bind_method(D_METHOD("addExcludedTag", "query_handle", "tag_name"), &SteamServer::addExcludedTag);
	ClassDB::bind_method(D_METHOD("addItemKeyValueTag", "query_handle", "key", "value"), &SteamServer::addItemKeyValueTag);
	ClassDB::bind_method(D_METHOD("addItemPreviewFile", "query_handle", "preview_file", "type"), &SteamServer::addItemPreviewFile);
	ClassDB::bind_method(D_METHOD("addItemToFavorites", "app_id", "published_file_id"), &SteamServer::addItemToFavorites);
	ClassDB::bind_method(D_METHOD("addRequiredKeyValueTag", "query_handle", "key", "value"), &SteamServer::addRequiredKeyValueTag);
	ClassDB::bind_method(D_METHOD("addRequiredTag", "query_handle", "tag_name"), &SteamServer::addRequiredTag);
	ClassDB::bind_method(D_METHOD("addRequiredTagGroup", "query_handle", "tag_array"), &SteamServer::addRequiredTagGroup);
	ClassDB::bind_method(D_METHOD("initWorkshopForGameServer", "workshop_depot_id"), &SteamServer::initWorkshopForGameServer);
	ClassDB::bind_method(D_METHOD("createItem", "app_id", "file_type"), &SteamServer::createItem);
	ClassDB::bind_method(D_METHOD("createQueryAllUGCRequest", "query_type", "matching_type", "creator_id", "consumer_id", "page"), &SteamServer::createQueryAllUGCRequest);
	ClassDB::bind_method(D_METHOD("createQueryUGCDetailsRequest", "published_file_id"), &SteamServer::createQueryUGCDetailsRequest);
	ClassDB::bind_method(D_METHOD("createQueryUserUGCRequest", "account_id", "list_type", "matching_ugc_type", "sort_order", "creator_id", "consumer_id", "page"), &SteamServer::createQueryUserUGCRequest);
	ClassDB::bind_method(D_METHOD("deleteItem", "published_file_id"), &SteamServer::deleteItem);
	ClassDB::bind_method(D_METHOD("downloadItem", "published_file_id", "high_priority"), &SteamServer::downloadItem);
	ClassDB::bind_method(D_METHOD("getItemDownloadInfo", "published_file_id"), &SteamServer::getItemDownloadInfo);
	ClassDB::bind_method(D_METHOD("getItemInstallInfo", "published_file_id"), &SteamServer::getItemInstallInfo);
	ClassDB::bind_method(D_METHOD("getItemState", "published_file_id"), &SteamServer::getItemState);
	ClassDB::bind_method(D_METHOD("getItemUpdateProgress", "update_handle"), &SteamServer::getItemUpdateProgress);
	ClassDB::bind_method("getNumSubscribedItems", &SteamServer::getNumSubscribedItems);
	ClassDB::bind_method(D_METHOD("getQueryUGCAdditionalPreview", "query_handle", "index", "preview_index"), &SteamServer::getQueryUGCAdditionalPreview);
	ClassDB::bind_method(D_METHOD("getQueryUGCChildren", "query_handle", "index", "child_count"), &SteamServer::getQueryUGCChildren);
	ClassDB::bind_method(D_METHOD("getQueryUGCContentDescriptors", "query_handle", "index", "max_entries"), &SteamServer::getQueryUGCContentDescriptors);
	ClassDB::bind_method(D_METHOD("getQueryUGCKeyValueTag", "query_handle", "index", "key_value_tag_index"), &SteamServer::getQueryUGCKeyValueTag);
	ClassDB::bind_method(D_METHOD("getQueryUGCMetadata", "query_handle", "index"), &SteamServer::getQueryUGCMetadata);
	ClassDB::bind_method(D_METHOD("getQueryUGCNumAdditionalPreviews", "query_handle", "index"), &SteamServer::getQueryUGCNumAdditionalPreviews);
	ClassDB::bind_method(D_METHOD("getQueryUGCNumKeyValueTags", "query_handle", "index"), &SteamServer::getQueryUGCNumKeyValueTags);
	ClassDB::bind_method(D_METHOD("getQueryUGCNumTags", "query_handle", "index"), &SteamServer::getQueryUGCNumTags);
	ClassDB::bind_method(D_METHOD("getQueryUGCPreviewURL", "query_handle", "index"), &SteamServer::getQueryUGCPreviewURL);
	ClassDB::bind_method(D_METHOD("getQueryUGCResult", "query_handle", "index"), &SteamServer::getQueryUGCResult);
	ClassDB::bind_method(D_METHOD("getQueryUGCStatistic", "query_handle", "index", "stat_type"), &SteamServer::getQueryUGCStatistic);
	ClassDB::bind_method(D_METHOD("getQueryUGCTag", "query_handle", "index", "tag_index"), &SteamServer::getQueryUGCTag);
	ClassDB::bind_method(D_METHOD("getQueryUGCTagDisplayName", "query_handle", "index", "tag_index"), &SteamServer::getQueryUGCTagDisplayName);
	ClassDB::bind_method("getSubscribedItems", &SteamServer::getSubscribedItems);
	ClassDB::bind_method(D_METHOD("getUserContentDescriptorPreferences", "max_entries"), &SteamServer::getUserContentDescriptorPreferences);
	ClassDB::bind_method(D_METHOD("getUserItemVote", "published_file_id"), &SteamServer::getUserItemVote);
	ClassDB::bind_method(D_METHOD("releaseQueryUGCRequest", "query_handle"), &SteamServer::releaseQueryUGCRequest);
	ClassDB::bind_method(D_METHOD("removeAppDependency", "published_file_id", "app_id"), &SteamServer::removeAppDependency);
	ClassDB::bind_method(D_METHOD("removeContentDescriptor", "update_handle", "descriptor_id"), &SteamServer::removeContentDescriptor);
	ClassDB::bind_method(D_METHOD("removeDependency", "published_file_id", "child_published_file_id"), &SteamServer::removeDependency);
	ClassDB::bind_method(D_METHOD("removeItemFromFavorites", "app_id", "published_file_id"), &SteamServer::removeItemFromFavorites);
	ClassDB::bind_method(D_METHOD("removeItemKeyValueTags", "update_handle", "key"), &SteamServer::removeItemKeyValueTags);
	ClassDB::bind_method(D_METHOD("removeItemPreview", "update_handle", "index"), &SteamServer::removeItemPreview);
	ClassDB::bind_method(D_METHOD("sendQueryUGCRequest", "update_handle"), &SteamServer::sendQueryUGCRequest);
	ClassDB::bind_method(D_METHOD("setAllowCachedResponse", "update_handle", "max_age_seconds"), &SteamServer::setAllowCachedResponse);
	ClassDB::bind_method(D_METHOD("setCloudFileNameFilter", "update_handle", "match_cloud_filename"), &SteamServer::setCloudFileNameFilter);
	ClassDB::bind_method(D_METHOD("setItemContent", "update_handle", "content_folder"), &SteamServer::setItemContent);
	ClassDB::bind_method(D_METHOD("setItemDescription", "update_handle", "description"), &SteamServer::setItemDescription);
	ClassDB::bind_method(D_METHOD("setItemMetadata", "update_handle", "metadata"), &SteamServer::setItemMetadata);
	ClassDB::bind_method(D_METHOD("setItemPreview", "update_handle", "preview_file"), &SteamServer::setItemPreview);
	ClassDB::bind_method(D_METHOD("setItemTags", "update_handle", "tag_array", "allow_admin_tags"), &SteamServer::setItemTags, DEFVAL(false));
	ClassDB::bind_method(D_METHOD("setItemTitle", "update_handle", "title"), &SteamServer::setItemTitle);
	ClassDB::bind_method(D_METHOD("setItemUpdateLanguage", "update_handle", "language"), &SteamServer::setItemUpdateLanguage);
	ClassDB::bind_method(D_METHOD("setItemVisibility", "update_handle", "visibility"), &SteamServer::setItemVisibility);
	ClassDB::bind_method(D_METHOD("setLanguage", "query_handle", "language"), &SteamServer::setLanguage);
	ClassDB::bind_method(D_METHOD("setMatchAnyTag", "query_handle", "match_any_tag"), &SteamServer::setMatchAnyTag);
	ClassDB::bind_method(D_METHOD("setRankedByTrendDays", "query_handle", "days"), &SteamServer::setRankedByTrendDays);
	ClassDB::bind_method(D_METHOD("setReturnAdditionalPreviews", "query_handle", "return_additional_previews"), &SteamServer::setReturnAdditionalPreviews);
	ClassDB::bind_method(D_METHOD("setReturnChildren", "query_handle", "return_children"), &SteamServer::setReturnChildren);
	ClassDB::bind_method(D_METHOD("setReturnKeyValueTags", "query_handle", "return_key_value_tags"), &SteamServer::setReturnKeyValueTags);
	ClassDB::bind_method(D_METHOD("setReturnLongDescription", "query_handle", "return_long_description"), &SteamServer::setReturnLongDescription);
	ClassDB::bind_method(D_METHOD("setReturnMetadata", "query_handle", "return_metadata"), &SteamServer::setReturnMetadata);
	ClassDB::bind_method(D_METHOD("setReturnOnlyIDs", "query_handle", "return_only_ids"), &SteamServer::setReturnOnlyIDs);
	ClassDB::bind_method(D_METHOD("setReturnPlaytimeStats", "query_handle", "days"), &SteamServer::setReturnPlaytimeStats);
	ClassDB::bind_method(D_METHOD("setReturnTotalOnly", "query_handle", "return_total_only"), &SteamServer::setReturnTotalOnly);
	ClassDB::bind_method(D_METHOD("setSearchText", "query_handle", "search_text"), &SteamServer::setSearchText);
	ClassDB::bind_method(D_METHOD("setUserItemVote", "published_file_id", "vote_up"), &SteamServer::setUserItemVote);
	ClassDB::bind_method(D_METHOD("startItemUpdate", "app_id", "file_id"), &SteamServer::startItemUpdate);
	ClassDB::bind_method(D_METHOD("startPlaytimeTracking", "published_file_ids"), &SteamServer::startPlaytimeTracking);
	ClassDB::bind_method(D_METHOD("stopPlaytimeTracking", "published_file_ids"), &SteamServer::stopPlaytimeTracking);
	ClassDB::bind_method("stopPlaytimeTrackingForAllItems", &SteamServer::stopPlaytimeTrackingForAllItems);
	ClassDB::bind_method(D_METHOD("getAppDependencies", "published_file_id"), &SteamServer::getAppDependencies);
	ClassDB::bind_method(D_METHOD("submitItemUpdate", "update_handle", "change_note"), &SteamServer::submitItemUpdate);
	ClassDB::bind_method(D_METHOD("subscribeItem", "published_file_id"), &SteamServer::subscribeItem);
	ClassDB::bind_method(D_METHOD("suspendDownloads", "suspend"), &SteamServer::suspendDownloads);
	ClassDB::bind_method(D_METHOD("unsubscribeItem", "published_file_id"), &SteamServer::unsubscribeItem);
	ClassDB::bind_method(D_METHOD("updateItemPreviewFile", "update_handle", "index", "preview_file"), &SteamServer::updateItemPreviewFile);
	ClassDB::bind_method(D_METHOD("updateItemPreviewVideo", "update_handle", "index", "video_id"), &SteamServer::updateItemPreviewVideo);
	ClassDB::bind_method("showWorkshopEULA", &SteamServer::showWorkshopEULA);
	ClassDB::bind_method("getWorkshopEULAStatus", &SteamServer::getWorkshopEULAStatus);
	ClassDB::bind_method(D_METHOD("setTimeCreatedDateRange", "update_handle", "start", "end"), &SteamServer::setTimeCreatedDateRange);
	ClassDB::bind_method(D_METHOD("setTimeUpdatedDateRange", "update_handle", "start", "end"), &SteamServer::setTimeUpdatedDateRange);

	// USER BIND METHODS ////////////////////////
	ClassDB::bind_method(D_METHOD("advertiseGame", "server_ip", "port"), &SteamServer::advertiseGame);
	ClassDB::bind_method(D_METHOD("beginAuthSession", "ticket", "ticket_size", "steam_id"), &SteamServer::beginAuthSession);
	ClassDB::bind_method(D_METHOD("cancelAuthTicket", "auth_ticket"), &SteamServer::cancelAuthTicket);
	ClassDB::bind_method(D_METHOD("decompressVoice", "voice", "voice_size", "sample_rate"), &SteamServer::decompressVoice);
	ClassDB::bind_method(D_METHOD("endAuthSession", "steam_id"), &SteamServer::endAuthSession);
	ClassDB::bind_method(D_METHOD("getAuthSessionTicket", "identity_reference"), &SteamServer::getUserAuthSessionTicket, DEFVAL(""));
	ClassDB::bind_method(D_METHOD("getAuthTicketForWebApi", "identity_reference"), &SteamServer::getAuthTicketForWebApi, DEFVAL(""));
	ClassDB::bind_method("getAvailableVoice", &SteamServer::getAvailableVoice);
	ClassDB::bind_method("getDurationControl", &SteamServer::getDurationControl);
	ClassDB::bind_method("getEncryptedAppTicket", &SteamServer::getEncryptedAppTicket);
	ClassDB::bind_method(D_METHOD("getGameBadgeLevel", "series", "foil"), &SteamServer::getGameBadgeLevel);
	ClassDB::bind_method("getPlayerSteamLevel", &SteamServer::getPlayerSteamLevel);
	ClassDB::bind_method("getSteamID", &SteamServer::getSteamID);
	ClassDB::bind_method("getVoice", &SteamServer::getVoice);
	ClassDB::bind_method("getVoiceOptimalSampleRate", &SteamServer::getVoiceOptimalSampleRate);
	ClassDB::bind_method(D_METHOD("initiateGameConnection", "server_id", "server_ip", "server_port", "secure"), &SteamServer::initiateGameConnection);
	ClassDB::bind_method("isBehindNAT", &SteamServer::isBehindNAT);
	ClassDB::bind_method("isPhoneIdentifying", &SteamServer::isPhoneIdentifying);
	ClassDB::bind_method("isPhoneRequiringVerification", &SteamServer::isPhoneRequiringVerification);
	ClassDB::bind_method("isPhoneVerified", &SteamServer::isPhoneVerified);
	ClassDB::bind_method("isTwoFactorEnabled", &SteamServer::isTwoFactorEnabled);
	ClassDB::bind_method("userLoggedOn", &SteamServer::userLoggedOn);
	ClassDB::bind_method(D_METHOD("requestEncryptedAppTicket", "secret"), &SteamServer::requestEncryptedAppTicket);
	ClassDB::bind_method(D_METHOD("requestStoreAuthURL", "redirect"), &SteamServer::requestStoreAuthURL);
	ClassDB::bind_method("startVoiceRecording", &SteamServer::startVoiceRecording);
	ClassDB::bind_method(D_METHOD("setDurationControlOnlineState", "new_state"), &SteamServer::setDurationControlOnlineState);
	ClassDB::bind_method("stopVoiceRecording", &SteamServer::stopVoiceRecording);
	ClassDB::bind_method(D_METHOD("terminateGameConnection", "server_ip", "server_port"), &SteamServer::terminateGameConnection);
	ClassDB::bind_method(D_METHOD("userHasLicenseForApp", "steam_id", "app_id"), &SteamServer::userHasLicenseForApp);

	// USER STATS BIND METHODS //////////////////
	ClassDB::bind_method(D_METHOD("attachLeaderboardUGC", "ugc_handle", "this_leaderboard"), &SteamServer::attachLeaderboardUGC, DEFVAL(0));
	ClassDB::bind_method(D_METHOD("clearAchievement", "achievement_name"), &SteamServer::clearAchievement);
	ClassDB::bind_method(D_METHOD("downloadLeaderboardEntries", "start", "end", "type", "this_leaderboard"), &SteamServer::downloadLeaderboardEntries, DEFVAL(0), DEFVAL(0));
	ClassDB::bind_method(D_METHOD("downloadLeaderboardEntriesForUsers", "users_id", "this_leaderboard"), &SteamServer::downloadLeaderboardEntriesForUsers, DEFVAL(0));
	ClassDB::bind_method(D_METHOD("findLeaderboard", "leaderboard_name"), &SteamServer::findLeaderboard);
	ClassDB::bind_method(D_METHOD("findOrCreateLeaderboard", "leaderboard_name", "sort_method", "display_type"), &SteamServer::findOrCreateLeaderboard);
	ClassDB::bind_method(D_METHOD("getAchievement", "achievement_name"), &SteamServer::getAchievement);
	ClassDB::bind_method(D_METHOD("getAchievementAchievedPercent", "achievement_name"), &SteamServer::getAchievementAchievedPercent);
	ClassDB::bind_method(D_METHOD("getAchievementAndUnlockTime", "achievement_name"), &SteamServer::getAchievementAndUnlockTime);
	ClassDB::bind_method(D_METHOD("getAchievementDisplayAttribute", "achievement_name", "key"), &SteamServer::getAchievementDisplayAttribute);
	ClassDB::bind_method(D_METHOD("getAchievementIcon", "achievement_name"), &SteamServer::getAchievementIcon);
	ClassDB::bind_method(D_METHOD("getAchievementName", "achievement"), &SteamServer::getAchievementName);
	ClassDB::bind_method(D_METHOD("getAchievementProgressLimitsInt", "achievement_name"), &SteamServer::getAchievementProgressLimitsInt);
	ClassDB::bind_method(D_METHOD("getAchievementProgressLimitsFloat", "achievement_name"), &SteamServer::getAchievementProgressLimitsFloat);
	ClassDB::bind_method(D_METHOD("getGlobalStatInt", "stat_name"), &SteamServer::getGlobalStatInt);
	ClassDB::bind_method(D_METHOD("getGlobalStatFloat", "stat_name"), &SteamServer::getGlobalStatFloat);
	ClassDB::bind_method(D_METHOD("getGlobalStatIntHistory", "stat_name"), &SteamServer::getGlobalStatIntHistory);
	ClassDB::bind_method(D_METHOD("getGlobalStatFloatHistory", "stat_name"), &SteamServer::getGlobalStatFloatHistory);
	ClassDB::bind_method(D_METHOD("getLeaderboardDisplayType", "this_leaderboard"), &SteamServer::getLeaderboardDisplayType, DEFVAL(0));
	ClassDB::bind_method(D_METHOD("getLeaderboardEntryCount", "this_leaderboard"), &SteamServer::getLeaderboardEntryCount, DEFVAL(0));
	ClassDB::bind_method(D_METHOD("getLeaderboardName", "this_leaderboard"), &SteamServer::getLeaderboardName, DEFVAL(0));
	ClassDB::bind_method(D_METHOD("getLeaderboardSortMethod", "this_leaderboard"), &SteamServer::getLeaderboardSortMethod, DEFVAL(0));
	ClassDB::bind_method("getMostAchievedAchievementInfo", &SteamServer::getMostAchievedAchievementInfo);
	ClassDB::bind_method(D_METHOD("getNextMostAchievedAchievementInfo", "iterator"), &SteamServer::getNextMostAchievedAchievementInfo);
	ClassDB::bind_method("getNumAchievements", &SteamServer::getNumAchievements);
	ClassDB::bind_method("getNumberOfCurrentPlayers", &SteamServer::getNumberOfCurrentPlayers);
	ClassDB::bind_method(D_METHOD("getStatFloat", "stat_name"), &SteamServer::getStatFloat);
	ClassDB::bind_method(D_METHOD("getStatInt", "stat_name"), &SteamServer::getStatInt);
	ClassDB::bind_method(D_METHOD("getUserAchievement", "steam_id", "name"), &SteamServer::getUserAchievement);
	ClassDB::bind_method(D_METHOD("getUserAchievementAndUnlockTime", "steam_id", "name"), &SteamServer::getUserAchievementAndUnlockTime);
	ClassDB::bind_method(D_METHOD("getUserStatFloat", "steam_id", "name"), &SteamServer::getUserStatFloat);
	ClassDB::bind_method(D_METHOD("getUserStatInt", "steam_id", "name"), &SteamServer::getUserStatInt);
	ClassDB::bind_method(D_METHOD("indicateAchievementProgress", "name", "current_progress", "max_progress"), &SteamServer::indicateAchievementProgress);
	ClassDB::bind_method("requestCurrentStats", &SteamServer::requestCurrentStats);
	ClassDB::bind_method("requestGlobalAchievementPercentages", &SteamServer::requestGlobalAchievementPercentages);
	ClassDB::bind_method(D_METHOD("requestGlobalStats", "history_days"), &SteamServer::requestGlobalStats);
	ClassDB::bind_method(D_METHOD("requestUserStats", "steam_id"), &SteamServer::requestUserStats);
	ClassDB::bind_method(D_METHOD("resetAllStats", "achievements_too"), &SteamServer::resetAllStats);
	ClassDB::bind_method(D_METHOD("setAchievement", "name"), &SteamServer::setAchievement);
	ClassDB::bind_method(D_METHOD("setLeaderboardDetailsMax", "max"), &SteamServer::setLeaderboardDetailsMax);
	ClassDB::bind_method(D_METHOD("setStatFloat", "name", "value"), &SteamServer::setStatFloat);
	ClassDB::bind_method(D_METHOD("setStatInt", "name", "value"), &SteamServer::setStatInt);
	ClassDB::bind_method("storeStats", &SteamServer::storeStats);
	ClassDB::bind_method(D_METHOD("updateAvgRateStat", "name", "this_session", "session_length"), &SteamServer::updateAvgRateStat);
	ClassDB::bind_method(D_METHOD("uploadLeaderboardScore", "score", "keep_best", "details", "this_leaderboard"), &SteamServer::uploadLeaderboardScore, DEFVAL(true), DEFVAL(PackedInt32Array()), DEFVAL(0));
	ClassDB::bind_method("getLeaderboardEntries", &SteamServer::getLeaderboardEntries);

	// UTILS BIND METHODS ///////////////////////
	ClassDB::bind_method(D_METHOD("filterText", "context", "steam_id", "message"), &SteamServer::filterText);
	ClassDB::bind_method("getAPICallFailureReason", &SteamServer::getAPICallFailureReason);
	ClassDB::bind_method("getAppID", &SteamServer::getAppID);
	ClassDB::bind_method("getCurrentBatteryPower", &SteamServer::getCurrentBatteryPower);
	ClassDB::bind_method(D_METHOD("getImageRGBA", "image"), &SteamServer::getImageRGBA);
	ClassDB::bind_method(D_METHOD("getImageSize", "image"), &SteamServer::getImageSize);
	ClassDB::bind_method("getIPCCallCount", &SteamServer::getIPCCallCount);
	ClassDB::bind_method("getIPCountry", &SteamServer::getIPCountry);
	ClassDB::bind_method("getSecondsSinceAppActive", &SteamServer::getSecondsSinceAppActive);
	ClassDB::bind_method("getSecondsSinceComputerActive", &SteamServer::getSecondsSinceComputerActive);
	ClassDB::bind_method("getServerRealTime", &SteamServer::getServerRealTime);
	ClassDB::bind_method("getSteamUILanguage", &SteamServer::getSteamUILanguage);
	ClassDB::bind_method("initFilterText", &SteamServer::initFilterText);
	ClassDB::bind_method("isAPICallCompleted", &SteamServer::isAPICallCompleted);
	ClassDB::bind_method("isOverlayEnabled", &SteamServer::isOverlayEnabled);
	ClassDB::bind_method("isSteamChinaLauncher", &SteamServer::isSteamChinaLauncher);
	ClassDB::bind_method("isSteamInBigPictureMode", &SteamServer::isSteamInBigPictureMode);
	ClassDB::bind_method("isSteamRunningInVR", &SteamServer::isSteamRunningInVR);
	ClassDB::bind_method("isVRHeadsetStreamingEnabled", &SteamServer::isVRHeadsetStreamingEnabled);
	ClassDB::bind_method("overlayNeedsPresent", &SteamServer::overlayNeedsPresent);
	ClassDB::bind_method(D_METHOD("setOverlayNotificationInset", "horizontal", "vertical"), &SteamServer::setOverlayNotificationInset);
	ClassDB::bind_method(D_METHOD("setOverlayNotificationPosition", "pos"), &SteamServer::setOverlayNotificationPosition);
	ClassDB::bind_method(D_METHOD("setVRHeadsetStreamingEnabled", "enabled"), &SteamServer::setVRHeadsetStreamingEnabled);
	ClassDB::bind_method(D_METHOD("setGameLauncherMode", "mode"), &SteamServer::setGameLauncherMode);
	ClassDB::bind_method("startVRDashboard", &SteamServer::startVRDashboard);
	ClassDB::bind_method("isSteamRunningOnSteamDeck", &SteamServer::isSteamRunningOnSteamDeck);

	// VIDEO BIND METHODS ///////////////////////
	ClassDB::bind_method(D_METHOD("getOPFSettings", "app_id"), &SteamServer::getOPFSettings);
	ClassDB::bind_method(D_METHOD("getOPFStringForApp", "app_id"), &SteamServer::getOPFStringForApp);
	ClassDB::bind_method(D_METHOD("getVideoURL", "app_id"), &SteamServer::getVideoURL);
	ClassDB::bind_method("isBroadcasting", &SteamServer::isBroadcasting);

	/////////////////////////////////////////////
	// CALLBACK SIGNAL BINDS
	/////////////////////////////////////////////
	//
	// STEAMWORKS SIGNALS ///////////////////////
	ADD_SIGNAL(MethodInfo("steamworks_error", PropertyInfo(Variant::STRING, "failed_signal"), PropertyInfo(Variant::STRING, "io_failure")));

	// APPS SIGNALS /////////////////////////////
	ADD_SIGNAL(MethodInfo("file_details_result", PropertyInfo(Variant::INT, "result"), PropertyInfo(Variant::INT, "file_size"), PropertyInfo(Variant::INT, "file_hash"), PropertyInfo(Variant::INT, "flags")));
	ADD_SIGNAL(MethodInfo("dlc_installed", PropertyInfo(Variant::INT, "app")));
	ADD_SIGNAL(MethodInfo("new_launch_url_parameters"));
	ADD_SIGNAL(MethodInfo("timed_trial_status", PropertyInfo(Variant::INT, "app_id"), PropertyInfo(Variant::BOOL, "is_offline"), PropertyInfo(Variant::INT, "seconds_allowed"), PropertyInfo(Variant::INT, "seconds_played")));

	// APP LIST SIGNALS /////////////////////////
	ADD_SIGNAL(MethodInfo("app_installed", PropertyInfo(Variant::INT, "app_id"), PropertyInfo(Variant::INT, "install_folder_index")));
	ADD_SIGNAL(MethodInfo("app_uninstalled", PropertyInfo(Variant::INT, "app_id"), PropertyInfo(Variant::INT, "install_folder_index")));

	// FRIENDS SIGNALS //////////////////////////
	ADD_SIGNAL(MethodInfo("avatar_loaded", PropertyInfo(Variant::INT, "avatar_id"), PropertyInfo(Variant::INT, "size"), PropertyInfo(Variant::ARRAY, "data")));
	ADD_SIGNAL(MethodInfo("avatar_image_loaded", PropertyInfo(Variant::INT, "avatar_id"), PropertyInfo(Variant::INT, "avatar_index"), PropertyInfo(Variant::INT, "width"), PropertyInfo(Variant::INT, "height")));
	ADD_SIGNAL(MethodInfo("request_clan_officer_list", PropertyInfo(Variant::STRING, "message"), PropertyInfo(Variant::ARRAY, "officer_list")));
	ADD_SIGNAL(MethodInfo("clan_activity_downloaded", PropertyInfo(Variant::DICTIONARY, "activity")));
	ADD_SIGNAL(MethodInfo("friend_rich_presence_update", PropertyInfo(Variant::INT, "steam_id"), PropertyInfo(Variant::INT, "app_id")));
	ADD_SIGNAL(MethodInfo("enumerate_following_list", PropertyInfo(Variant::STRING, "message"), PropertyInfo(Variant::ARRAY, "following")));
	ADD_SIGNAL(MethodInfo("get_follower_count", PropertyInfo(Variant::INT, "result"), PropertyInfo(Variant::INT, "steam_id"), PropertyInfo(Variant::INT, "count")));
	ADD_SIGNAL(MethodInfo("is_following", PropertyInfo(Variant::INT, "result"), PropertyInfo(Variant::INT, "steam_id"), PropertyInfo(Variant::BOOL, "following")));
	ADD_SIGNAL(MethodInfo("connected_chat_join", PropertyInfo(Variant::INT, "chat_id"), PropertyInfo(Variant::INT, "steam_id")));
	ADD_SIGNAL(MethodInfo("connected_chat_leave", PropertyInfo(Variant::INT, "chat_id"), PropertyInfo(Variant::INT, "steam_id"), PropertyInfo(Variant::BOOL, "kicked"), PropertyInfo(Variant::BOOL, "dropped")));
	ADD_SIGNAL(MethodInfo("connected_clan_chat_message", PropertyInfo(Variant::DICTIONARY, "chat")));
	ADD_SIGNAL(MethodInfo("connected_friend_chat_message", PropertyInfo(Variant::DICTIONARY, "chat")));
	ADD_SIGNAL(MethodInfo("join_requested", PropertyInfo(Variant::INT, "lobby_id"), PropertyInfo(Variant::INT, "steam_id")));
	ADD_SIGNAL(MethodInfo("overlay_toggled", PropertyInfo(Variant::BOOL, "active"), PropertyInfo(Variant::BOOL, "user_initiated"), PropertyInfo(Variant::INT, "app_id")));
	ADD_SIGNAL(MethodInfo("join_game_requested", PropertyInfo(Variant::INT, "user"), PropertyInfo(Variant::STRING, "connect")));
	ADD_SIGNAL(MethodInfo("change_server_requested", PropertyInfo(Variant::STRING, "server"), PropertyInfo(Variant::STRING, "password")));
	ADD_SIGNAL(MethodInfo("join_clan_chat_complete", PropertyInfo(Variant::INT, "chat_id"), PropertyInfo(Variant::INT, "response")));
	ADD_SIGNAL(MethodInfo("persona_state_change", PropertyInfo(Variant::INT, "steam_id"), PropertyInfo(Variant::INT, "flags")));
	ADD_SIGNAL(MethodInfo("name_changed", PropertyInfo(Variant::BOOL, "success"), PropertyInfo(Variant::BOOL, "local_success"), PropertyInfo(Variant::INT, "result")));
	ADD_SIGNAL(MethodInfo("overlay_browser_protocol", PropertyInfo(Variant::STRING, "uri")));
	ADD_SIGNAL(MethodInfo("unread_chat_messages_changed"));
	ADD_SIGNAL(MethodInfo("equipped_profile_items_changed", PropertyInfo(Variant::INT, "steam_id")));
	ADD_SIGNAL(MethodInfo("equipped_profile_items", PropertyInfo(Variant::INT, "result"), PropertyInfo(Variant::INT, "steam_id"), PropertyInfo(Variant::DICTIONARY, "profile_data")));

	// GAME SEARCH SIGNALS //////////////////////
	ADD_SIGNAL(MethodInfo("search_for_game_progress", PropertyInfo(Variant::INT, "result"), PropertyInfo(Variant::INT, "search_id"), PropertyInfo(Variant::DICTIONARY, "search_progress")));
	ADD_SIGNAL(MethodInfo("search_for_game_result", PropertyInfo(Variant::INT, "result"), PropertyInfo(Variant::INT, "search_id"), PropertyInfo(Variant::DICTIONARY, "search_result")));
	ADD_SIGNAL(MethodInfo("request_players_for_game_progress", PropertyInfo(Variant::INT, "result"), PropertyInfo(Variant::INT, "search_id")));
	ADD_SIGNAL(MethodInfo("request_players_for_game_result", PropertyInfo(Variant::INT, "result"), PropertyInfo(Variant::INT, "search_id"), PropertyInfo(Variant::DICTIONARY, "player_data")));
	ADD_SIGNAL(MethodInfo("request_players_for_game_final_result", PropertyInfo(Variant::INT, "result"), PropertyInfo(Variant::INT, "search_id"), PropertyInfo(Variant::INT, "game_id")));
	ADD_SIGNAL(MethodInfo("submit_player_result", PropertyInfo(Variant::INT, "result"), PropertyInfo(Variant::INT, "game_id"), PropertyInfo(Variant::INT, "player_id")));
	ADD_SIGNAL(MethodInfo("end_game_result", PropertyInfo(Variant::INT, "result"), PropertyInfo(Variant::INT, "game_id")));

	// HTTP SIGNALS /////////////////////////////
	ADD_SIGNAL(MethodInfo("http_request_completed", PropertyInfo(Variant::INT, "cookie_handle"), PropertyInfo(Variant::INT, "context_value"), PropertyInfo(Variant::BOOL, "request_success"), PropertyInfo(Variant::INT, "status_code"), PropertyInfo(Variant::INT, "body_size")));
	ADD_SIGNAL(MethodInfo("http_request_data_received", PropertyInfo(Variant::INT, "cookie_handle"), PropertyInfo(Variant::INT, "context_value"), PropertyInfo(Variant::INT, "offset"), PropertyInfo(Variant::INT, "bytes_received")));
	ADD_SIGNAL(MethodInfo("http_request_headers_received", PropertyInfo(Variant::INT, "cookie_handle"), PropertyInfo(Variant::INT, "context_value")));

	// INVENTORY SIGNALS ////////////////////////
	ADD_SIGNAL(MethodInfo("inventory_definition_update", PropertyInfo(Variant::ARRAY, "definitions")));
	ADD_SIGNAL(MethodInfo("inventory_eligible_promo_item", PropertyInfo(Variant::INT, "result"), PropertyInfo(Variant::BOOL, "cached"), PropertyInfo(Variant::ARRAY, "definitions")));
	ADD_SIGNAL(MethodInfo("inventory_full_update", PropertyInfo(Variant::INT, "inventory_handle")));
	ADD_SIGNAL(MethodInfo("inventory_result_ready", PropertyInfo(Variant::INT, "result"), PropertyInfo(Variant::INT, "inventory_handle")));
	ADD_SIGNAL(MethodInfo("inventory_start_purchase_result", PropertyInfo(Variant::STRING, "result"), PropertyInfo(Variant::INT, "order_id"), PropertyInfo(Variant::INT, "transaction_id")));
	ADD_SIGNAL(MethodInfo("inventory_request_prices_result", PropertyInfo(Variant::INT, "result"), PropertyInfo(Variant::STRING, "currency")));

	// GAME SERVER SIGNALS //////////////////////
	ADD_SIGNAL(MethodInfo("client_approved", PropertyInfo(Variant::INT, "steam_id"), PropertyInfo(Variant::INT, "owner_id")));
	ADD_SIGNAL(MethodInfo("client_denied", PropertyInfo(Variant::INT, "steam_id"), PropertyInfo(Variant::INT, "reason")));
	ADD_SIGNAL(MethodInfo("client_kick", PropertyInfo(Variant::INT, "steam_id"), PropertyInfo(Variant::INT, "reason")));
	ADD_SIGNAL(MethodInfo("player_compat", PropertyInfo(Variant::INT, "result"), PropertyInfo(Variant::INT, "players_that_dont_like_candidate"), PropertyInfo(Variant::INT, "players_that_candidate_doesnt_like"), PropertyInfo(Variant::INT, "clan_players_that_dont_like_candidate"), PropertyInfo(Variant::INT, "steam_id")));
	ADD_SIGNAL(MethodInfo("policy_response", PropertyInfo(Variant::INT, "secure")));
	ADD_SIGNAL(MethodInfo("server_connected"));
	ADD_SIGNAL(MethodInfo("server_disconnected", PropertyInfo(Variant::INT, "result")));
	ADD_SIGNAL(MethodInfo("server_connect_failure", PropertyInfo(Variant::INT, "result"), PropertyInfo(Variant::BOOL, "retrying")));

	// MATCHMAKING SIGNALS //////////////////////
	ADD_SIGNAL(MethodInfo("favorites_list_accounts_updated", PropertyInfo(Variant::INT, "result")));
	ADD_SIGNAL(MethodInfo("favorites_list_changed", PropertyInfo(Variant::DICTIONARY, "favorite")));
	ADD_SIGNAL(MethodInfo("lobby_message", PropertyInfo(Variant::INT, "lobby_id"), PropertyInfo(Variant::INT, "user"), PropertyInfo(Variant::STRING, "message"), PropertyInfo(Variant::INT, "chat_type")));
	ADD_SIGNAL(MethodInfo("lobby_chat_update", PropertyInfo(Variant::INT, "lobby_id"), PropertyInfo(Variant::INT, "changed_id"), PropertyInfo(Variant::INT, "making_change_id"), PropertyInfo(Variant::INT, "chat_state")));
	ADD_SIGNAL(MethodInfo("lobby_created", PropertyInfo(Variant::INT, "connect"), PropertyInfo(Variant::INT, "lobby_id")));
	ADD_SIGNAL(MethodInfo("lobby_data_update"));
	ADD_SIGNAL(MethodInfo("lobby_joined", PropertyInfo(Variant::INT, "lobby"), PropertyInfo(Variant::INT, "permissions"), PropertyInfo(Variant::BOOL, "locked"), PropertyInfo(Variant::INT, "response")));
	ADD_SIGNAL(MethodInfo("lobby_game_created", PropertyInfo(Variant::INT, "lobby_id"), PropertyInfo(Variant::INT, "server_id"), PropertyInfo(Variant::STRING, "server_ip"), PropertyInfo(Variant::INT, "port")));
	ADD_SIGNAL(MethodInfo("lobby_invite", PropertyInfo(Variant::INT, "inviter"), PropertyInfo(Variant::INT, "lobby"), PropertyInfo(Variant::INT, "game")));
	ADD_SIGNAL(MethodInfo("lobby_match_list", PropertyInfo(Variant::ARRAY, "lobbies")));
	ADD_SIGNAL(MethodInfo("lobby_kicked", PropertyInfo(Variant::INT, "lobby_id"), PropertyInfo(Variant::INT, "admin_id"), PropertyInfo(Variant::INT, "due_to_disconnect")));

	// MATCHMAKING SERVER SIGNALS ///////////////
	ADD_SIGNAL(MethodInfo("server_responded"));
	ADD_SIGNAL(MethodInfo("server_failed_to_respond"));

	// NETWORKING SIGNALS ///////////////////////
	ADD_SIGNAL(MethodInfo("p2p_session_request", PropertyInfo(Variant::INT, "steam_id_remote")));
	ADD_SIGNAL(MethodInfo("p2p_session_connect_fail", PropertyInfo(Variant::INT, "steam_id_remote"), PropertyInfo(Variant::INT, "session_error")));

	// NETWORKING MESSAGES //////////////////////
	ADD_SIGNAL(MethodInfo("network_messages_session_request", PropertyInfo(Variant::STRING, "identity")));
	ADD_SIGNAL(MethodInfo("network_messages_session_failed", PropertyInfo(Variant::INT, "reason")));

	// NETWORKING SOCKETS SIGNALS ///////////////
	ADD_SIGNAL(MethodInfo("network_connection_status_changed", PropertyInfo(Variant::INT, "connect_handle"), PropertyInfo(Variant::DICTIONARY, "connection"), PropertyInfo(Variant::INT, "old_state")));
	ADD_SIGNAL(MethodInfo("network_authentication_status", PropertyInfo(Variant::INT, "available"), PropertyInfo(Variant::STRING, "debug_message")));
	ADD_SIGNAL(MethodInfo("fake_ip_result", PropertyInfo(Variant::INT, "result"), PropertyInfo(Variant::STRING, "identity"), PropertyInfo(Variant::STRING, "fake_ip"), PropertyInfo(Variant::ARRAY, "port_list")));

	// NETWORKING UTILS SIGNALS /////////////////
	ADD_SIGNAL(MethodInfo("relay_network_status", PropertyInfo(Variant::INT, "available"), PropertyInfo(Variant::INT, "ping_measurement"), PropertyInfo(Variant::INT, "available_config"), PropertyInfo(Variant::INT, "available_relay"), PropertyInfo(Variant::STRING, "debug_message")));

	// PARTIES SIGANLS //////////////////////////
	ADD_SIGNAL(MethodInfo("join_party", PropertyInfo(Variant::INT, "result"), PropertyInfo(Variant::INT, "beacon_id"), PropertyInfo(Variant::INT, "steam_id"), PropertyInfo(Variant::STRING, "connect_string")));
	ADD_SIGNAL(MethodInfo("create_beacon", PropertyInfo(Variant::INT, "result"), PropertyInfo(Variant::INT, "beacon_id")));
	ADD_SIGNAL(MethodInfo("reservation_notification", PropertyInfo(Variant::INT, "beacon_id"), PropertyInfo(Variant::INT, "steam_id")));
	ADD_SIGNAL(MethodInfo("change_num_open_slots", PropertyInfo(Variant::INT, "result")));
	ADD_SIGNAL(MethodInfo("available_beacon_locations_updated"));
	ADD_SIGNAL(MethodInfo("active_beacons_updated"));

	// REMOTE STORAGE SIGNALS ///////////////////
	ADD_SIGNAL(MethodInfo("file_read_async_complete", PropertyInfo(Variant::DICTIONARY, "file_read")));
	ADD_SIGNAL(MethodInfo("file_share_result", PropertyInfo(Variant::INT, "result"), PropertyInfo(Variant::INT, "handle"), PropertyInfo(Variant::STRING, "name")));
	ADD_SIGNAL(MethodInfo("file_write_async_complete", PropertyInfo(Variant::INT, "result")));
	ADD_SIGNAL(MethodInfo("download_ugc_result", PropertyInfo(Variant::INT, "result"), PropertyInfo(Variant::DICTIONARY, "download_data")));
	ADD_SIGNAL(MethodInfo("unsubscribe_item", PropertyInfo(Variant::INT, "result"), PropertyInfo(Variant::INT, "file_id")));
	ADD_SIGNAL(MethodInfo("subscribe_item", PropertyInfo(Variant::INT, "result"), PropertyInfo(Variant::INT, "file_id")));
	ADD_SIGNAL(MethodInfo("local_file_changed"));

	// UGC SIGNALS //////////////////////////////
	ADD_SIGNAL(MethodInfo("add_app_dependency_result", PropertyInfo(Variant::INT, "result"), PropertyInfo(Variant::INT, "file_id"), PropertyInfo(Variant::INT, "app_id")));
	ADD_SIGNAL(MethodInfo("add_ugc_dependency_result", PropertyInfo(Variant::INT, "result"), PropertyInfo(Variant::INT, "file_id"), PropertyInfo(Variant::INT, "child_id")));
	ADD_SIGNAL(MethodInfo("item_created", PropertyInfo(Variant::INT, "result"), PropertyInfo(Variant::INT, "file_id"), PropertyInfo(Variant::BOOL, "accept_tos")));
	ADD_SIGNAL(MethodInfo("item_downloaded", PropertyInfo(Variant::INT, "result"), PropertyInfo(Variant::INT, "file_id"), PropertyInfo(Variant::INT, "app_id")));
	ADD_SIGNAL(MethodInfo("get_app_dependencies_result", PropertyInfo(Variant::INT, "result"), PropertyInfo(Variant::INT, "file_id"), PropertyInfo(Variant::INT, "app_dependencies"), PropertyInfo(Variant::INT, "total_app_dependencies")));
	ADD_SIGNAL(MethodInfo("item_deleted", PropertyInfo(Variant::INT, "result"), PropertyInfo(Variant::INT, "file_id")));
	ADD_SIGNAL(MethodInfo("get_item_vote_result", PropertyInfo(Variant::INT, "result"), PropertyInfo(Variant::INT, "file_id"), PropertyInfo(Variant::BOOL, "vote_up"), PropertyInfo(Variant::BOOL, "vote_down"), PropertyInfo(Variant::BOOL, "vote_skipped")));
	ADD_SIGNAL(MethodInfo("item_installed", PropertyInfo(Variant::INT, "app_id"), PropertyInfo(Variant::INT, "file_id")));
	ADD_SIGNAL(MethodInfo("remove_app_dependency_result", PropertyInfo(Variant::INT, "result"), PropertyInfo(Variant::INT, "file_id"), PropertyInfo(Variant::INT, "app_id")));
	ADD_SIGNAL(MethodInfo("remove_ugc_dependency_result", PropertyInfo(Variant::INT, "result"), PropertyInfo(Variant::INT, "file_id"), PropertyInfo(Variant::INT, "child_id")));
	ADD_SIGNAL(MethodInfo("set_user_item_vote", PropertyInfo(Variant::INT, "result"), PropertyInfo(Variant::INT, "file_id"), PropertyInfo(Variant::BOOL, "vote_up")));
	ADD_SIGNAL(MethodInfo("start_playtime_tracking", PropertyInfo(Variant::INT, "result")));
	ADD_SIGNAL(MethodInfo("ugc_query_completed", PropertyInfo(Variant::INT, "handle"), PropertyInfo(Variant::INT, "result"), PropertyInfo(Variant::INT, "results_returned"), PropertyInfo(Variant::INT, "total_matching"), PropertyInfo(Variant::BOOL, "cached")));
	ADD_SIGNAL(MethodInfo("stop_playtime_tracking", PropertyInfo(Variant::INT, "result")));
	ADD_SIGNAL(MethodInfo("item_updated", PropertyInfo(Variant::INT, "result"), PropertyInfo(Variant::BOOL, "accept_tos")));
	ADD_SIGNAL(MethodInfo("user_favorite_items_list_changed", PropertyInfo(Variant::INT, "result"), PropertyInfo(Variant::INT, "file_id"), PropertyInfo(Variant::BOOL, "was_add_request")));
	ADD_SIGNAL(MethodInfo("workshop_eula_status", PropertyInfo(Variant::INT, "result"), PropertyInfo(Variant::INT, "app_id"), PropertyInfo(Variant::DICTIONARY, "eula_data")));
	ADD_SIGNAL(MethodInfo("user_subscribed_items_list_changed", PropertyInfo(Variant::INT, "app_id")));

	// USER SIGNALS /////////////////////////////
	ADD_SIGNAL(MethodInfo("client_game_server_deny", PropertyInfo(Variant::INT, "app_id"), PropertyInfo(Variant::STRING, "ip"), PropertyInfo(Variant::INT, "server_port"), PropertyInfo(Variant::INT, "secure"), PropertyInfo(Variant::INT, "reason")));
	ADD_SIGNAL(MethodInfo("duration_control", PropertyInfo(Variant::INT, "result"), PropertyInfo(Variant::DICTIONARY, "duration")));
	ADD_SIGNAL(MethodInfo("encrypted_app_ticket_response", PropertyInfo(Variant::STRING, "result")));
	ADD_SIGNAL(MethodInfo("game_web_callback", PropertyInfo(Variant::STRING, "url")));
	ADD_SIGNAL(MethodInfo("get_auth_session_ticket_response", PropertyInfo(Variant::INT, "auth_ticket"), PropertyInfo(Variant::INT, "result")));
	ADD_SIGNAL(MethodInfo("get_ticket_for_web_api", PropertyInfo(Variant::INT, "auth_ticket"), PropertyInfo(Variant::INT, "result"), PropertyInfo(Variant::INT, "ticket_size"), PropertyInfo(Variant::ARRAY, "ticket_buffer")));
	ADD_SIGNAL(MethodInfo("ipc_failure", PropertyInfo(Variant::INT, "type")));
	ADD_SIGNAL(MethodInfo("licenses_updated"));
	ADD_SIGNAL(MethodInfo("microtransaction_auth_response", PropertyInfo(Variant::INT, "app_id"), PropertyInfo(Variant::INT, "order_id"), PropertyInfo(Variant::BOOL, "authorized")));
	ADD_SIGNAL(MethodInfo("steam_server_connect_failed", PropertyInfo(Variant::INT, "result"), PropertyInfo(Variant::BOOL, "retrying")));
	ADD_SIGNAL(MethodInfo("steam_server_connected"));
	ADD_SIGNAL(MethodInfo("steam_server_disconnected"));
	ADD_SIGNAL(MethodInfo("store_auth_url_response", PropertyInfo(Variant::STRING, "url")));
	ADD_SIGNAL(MethodInfo("validate_auth_ticket_response", PropertyInfo(Variant::INT, "auth_id"), PropertyInfo(Variant::INT, "reponse"), PropertyInfo(Variant::INT, "owner_id")));

	// USER STATS SIGNALS ///////////////////////
	ADD_SIGNAL(MethodInfo("global_achievement_percentages_ready", PropertyInfo(Variant::INT, "game_id"), PropertyInfo(Variant::INT, "result")));
	ADD_SIGNAL(MethodInfo("global_stats_received", PropertyInfo(Variant::INT, "game_id"), PropertyInfo(Variant::STRING, "result")));
	ADD_SIGNAL(MethodInfo("leaderboard_find_result", PropertyInfo(Variant::INT, "leaderboard_handle"), PropertyInfo(Variant::INT, "found")));
	ADD_SIGNAL(MethodInfo("leaderboard_scores_downloaded", PropertyInfo(Variant::STRING, "message"), PropertyInfo(Variant::ARRAY, "leaderboard_entries")));
	ADD_SIGNAL(MethodInfo("leaderboard_score_uploaded", PropertyInfo(Variant::BOOL, "success"), PropertyInfo(Variant::INT, "this_handle"), PropertyInfo(Variant::DICTIONARY, "this_score")));
	ADD_SIGNAL(MethodInfo("leaderboard_ugc_set", PropertyInfo(Variant::INT, "leaderboard_handle"), PropertyInfo(Variant::STRING, "result")));
	ADD_SIGNAL(MethodInfo("number_of_current_players", PropertyInfo(Variant::INT, "success"), PropertyInfo(Variant::INT, "players")));
	ADD_SIGNAL(MethodInfo("user_achievement_stored", PropertyInfo(Variant::INT, "game_id"), PropertyInfo(Variant::BOOL, "group_achieve"), PropertyInfo(Variant::STRING, "achievement_name"), PropertyInfo(Variant::INT, "current_progress"), PropertyInfo(Variant::INT, "max_progress")));
	ADD_SIGNAL(MethodInfo("current_stats_received", PropertyInfo(Variant::INT, "game_id"), PropertyInfo(Variant::INT, "result"), PropertyInfo(Variant::INT, "user_id")));
	ADD_SIGNAL(MethodInfo("user_stats_received", PropertyInfo(Variant::INT, "game_id"), PropertyInfo(Variant::INT, "result"), PropertyInfo(Variant::INT, "user_id")));
	ADD_SIGNAL(MethodInfo("user_stats_stored", PropertyInfo(Variant::INT, "game_id"), PropertyInfo(Variant::INT, "result")));
	ADD_SIGNAL(MethodInfo("user_stats_unloaded", PropertyInfo(Variant::INT, "user_id")));

	// UTILITY SIGNALS //////////////////////////
	ADD_SIGNAL(MethodInfo("check_file_signature", PropertyInfo(Variant::STRING, "signature")));
	ADD_SIGNAL(MethodInfo("gamepad_text_input_dismissed", PropertyInfo(Variant::BOOL, "submitted"), PropertyInfo(Variant::STRING, "entered_text"), PropertyInfo(Variant::INT, "app_id")));
	ADD_SIGNAL(MethodInfo("ip_country"));
	ADD_SIGNAL(MethodInfo("low_power", PropertyInfo(Variant::INT, "power")));
	ADD_SIGNAL(MethodInfo("steam_api_call_completed", PropertyInfo(Variant::INT, "async_call"), PropertyInfo(Variant::INT, "callback"), PropertyInfo(Variant::INT, "parameter")));
	ADD_SIGNAL(MethodInfo("steam_shutdown"));
	ADD_SIGNAL(MethodInfo("app_resuming_from_suspend"));
	ADD_SIGNAL(MethodInfo("floating_gamepad_text_input_dismissed"));
	ADD_SIGNAL(MethodInfo("filter_text_dictionary_changed", PropertyInfo(Variant::INT, "language")));

	// VIDEO SIGNALS ////////////////////////////
	ADD_SIGNAL(MethodInfo("get_opf_settings_result", PropertyInfo(Variant::INT, "result"), PropertyInfo(Variant::INT, "app_id")));
	ADD_SIGNAL(MethodInfo("get_video_result", PropertyInfo(Variant::INT, "result"), PropertyInfo(Variant::INT, "app_id"), PropertyInfo(Variant::STRING, "url")));

	/////////////////////////////////////////////
	// CONSTANT BINDS
	/////////////////////////////////////////////
	//
	// STEAM API CONSTANTS //////////////////////
	BIND_CONSTANT(INVALID_BREAKPAD_HANDLE); // (BREAKPAD_HANDLE)0
	BIND_CONSTANT(GAME_EXTRA_INFO_MAX); // 64
	BIND_CONSTANT(AUTH_TICKET_INVALID); // 0
	BIND_CONSTANT(API_CALL_INVALID); // 0x0
	BIND_CONSTANT(APP_ID_INVALID); // 0x0
	BIND_CONSTANT(DEPOT_ID_INVALID); // 0x0
	BIND_CONSTANT(STEAM_ACCOUNT_ID_MASK); // 0xFFFFFFFF
	BIND_CONSTANT(STEAM_ACCOUNT_INSTANCE_MASK); // 0x000FFFFF
	BIND_CONSTANT(STEAM_USER_CONSOLE_INSTANCE); // 2
	BIND_CONSTANT(STEAM_USER_DESKTOP_INSTANCE); // 1
	BIND_CONSTANT(STEAM_USER_WEB_INSTANCE); // 4
	BIND_CONSTANT(QUERY_PORT_ERROR); // 0xFFFE
	BIND_CONSTANT(QUERY_PORT_NOT_INITIALIZED); // 0xFFFF
	BIND_CONSTANT(STEAM_MAX_ERROR_MESSAGE);

	// FRIENDS CONSTANTS ////////////////////////
	BIND_CONSTANT(CHAT_METADATA_MAX); // 8192
	BIND_CONSTANT(ENUMERATED_FOLLOWERS_MAX); // 50
	BIND_CONSTANT(FRIENDS_GROUP_LIMIT); // 100
	BIND_CONSTANT(INVALID_FRIEND_GROUP_ID); // -1
	BIND_CONSTANT(MAX_FRIENDS_GROUP_NAME); // 64
	BIND_CONSTANT(MAX_RICH_PRESENCE_KEY_LENGTH); // 64
	BIND_CONSTANT(MAX_RICH_PRESENCE_KEYS); // 20
	BIND_CONSTANT(MAX_RICH_PRESENCE_VALUE_LENTH); // 256
	BIND_CONSTANT(PERSONA_NAME_MAX_UTF8); // 128
	BIND_CONSTANT(PERSONA_NAME_MAX_UTF16); // 32

	// HTTP CONSTANTS ///////////////////////////
	BIND_CONSTANT(INVALID_HTTPCOOKIE_HANDLE); // 0
	BIND_CONSTANT(INVALID_HTTPREQUEST_HANDLE); // 0

	// INVENTORY CONSTANTS //////////////////////
	BIND_CONSTANT(INVENTORY_RESULT_INVALID); // -1
	BIND_CONSTANT(ITEM_INSTANCE_ID_INVALID); // 0

	// MATCHMAKING CONSTANTS ////////////////////
	BIND_CONSTANT(SERVER_QUERY_INVALID); // 0xffffffff
	BIND_CONSTANT(MAX_LOBBY_KEY_LENGTH); // 255
	BIND_CONSTANT(FAVORITE_FLAG_FAVORITE); // 0x01
	BIND_CONSTANT(FAVORITE_FLAG_HISTORY); // 0x02
	BIND_CONSTANT(FAVORITE_FLAG_NONE); // 0x00

	// MATCHMAKING SERVERS CONSTANTS ////////////
	BIND_CONSTANT(MAX_GAME_SERVER_GAME_DATA); // 2048
	BIND_CONSTANT(MAX_GAME_SERVER_GAME_DESCRIPTION); // 64
	BIND_CONSTANT(MAX_GAME_SERVER_GAME_DIR); // 32
	BIND_CONSTANT(MAX_GAME_SERVER_MAP_NAME); // 32
	BIND_CONSTANT(MAX_GAME_SERVER_NAME); // 64
	BIND_CONSTANT(MAX_GAME_SERVER_TAGS); // 128

	// NETWORKING MESSAGE CONSTANTS /////////////
	BIND_CONSTANT(NETWORKING_SEND_UNRELIABLE); // 0
	BIND_CONSTANT(NETWORKING_SEND_NO_NAGLE); // 1
	BIND_CONSTANT(NETWORKING_SEND_NO_DELAY); // 4
	BIND_CONSTANT(NETWORKING_SEND_RELIABLE); // 8

	// REMOTE STORAGE CONSTANTS /////////////////
	BIND_CONSTANT(FILENAME_MAX); // 260
	BIND_CONSTANT(PUBLISHED_DOCUMENT_CHANGE_DESCRIPTION_MAX); // 8000
	BIND_CONSTANT(PUBLISHED_DOCUMENT_DESCRIPTION_MAX); // 8000
	BIND_CONSTANT(PUBLISHED_DOCUMENT_TITLE_MAX); // 128 + 1
	BIND_CONSTANT(PUBLISHED_FILE_URL_MAX); // 256
	BIND_CONSTANT(TAG_LIST_MAX); // 1024 + 1
	BIND_CONSTANT(PUBLISHED_FILE_ID_INVALID); // 0
	BIND_CONSTANT(PUBLISHED_FILE_UPDATE_HANDLE_INVALID); // 0xffffffffffffffffull
	BIND_CONSTANT(UGC_FILE_STREAM_HANDLE_INVALID); // 0xffffffffffffffffull
	BIND_CONSTANT(UGC_HANDLE_INVALID); // 0xffffffffffffffffull
	BIND_CONSTANT(ENUMERATE_PUBLISHED_FILES_MAX_RESULTS); // 50
	BIND_CONSTANT(MAX_CLOUD_FILE_CHUNK_SIZE); // 100 * 1024 * 1024

	// UGC CONSTANTS ////////////////////////////
	BIND_CONSTANT(NUM_UGC_RESULTS_PER_PAGE); // 50
	BIND_CONSTANT(DEVELOPER_METADATA_MAX); // 5000
	BIND_CONSTANT(UGC_QUERY_HANDLE_INVALID); // 0xffffffffffffffffull
	BIND_CONSTANT(UGC_UPDATE_HANDLE_INVALID); // 0xffffffffffffffffull

	// USER STATS CONSTANTS /////////////////////
	BIND_CONSTANT(LEADERBOARD_DETAIL_MAX); // 64
	BIND_CONSTANT(LEADERBOARD_NAME_MAX); // 128
	BIND_CONSTANT(STAT_NAME_MAX); // 128

	/////////////////////////////////////////////
	// ENUM CONSTANT BINDS //////////////////////
	/////////////////////////////////////////////
	//
	// AccountType Enums
	BIND_ENUM_CONSTANT(ACCOUNT_TYPE_INVALID);
	BIND_ENUM_CONSTANT(ACCOUNT_TYPE_INDIVIDUAL);
	BIND_ENUM_CONSTANT(ACCOUNT_TYPE_MULTISEAT);
	BIND_ENUM_CONSTANT(ACCOUNT_TYPE_GAME_SERVER);
	BIND_ENUM_CONSTANT(ACCOUNT_TYPE_ANON_GAME_SERVER);
	BIND_ENUM_CONSTANT(ACCOUNT_TYPE_PENDING);
	BIND_ENUM_CONSTANT(ACCOUNT_TYPE_CONTENT_SERVER);
	BIND_ENUM_CONSTANT(ACCOUNT_TYPE_CLAN);
	BIND_ENUM_CONSTANT(ACCOUNT_TYPE_CHAT);
	BIND_ENUM_CONSTANT(ACCOUNT_TYPE_CONSOLE_USER);
	BIND_ENUM_CONSTANT(ACCOUNT_TYPE_ANON_USER);
	BIND_ENUM_CONSTANT(ACCOUNT_TYPE_MAX);

	// APICallFailure Enums
	BIND_ENUM_CONSTANT(STEAM_API_CALL_FAILURE_NONE);
	BIND_ENUM_CONSTANT(STEAM_API_CALL_FAILURE_STEAM_GONE);
	BIND_ENUM_CONSTANT(STEAM_API_CALL_FAILURE_NETWORK_FAILURE);
	BIND_ENUM_CONSTANT(STEAM_API_CALL_FAILURE_INVALID_HANDLE);
	BIND_ENUM_CONSTANT(STEAM_API_CALL_FAILURE_MISMATCHED_CALLBACK);

	// AuthSessionResponse Enums
	BIND_ENUM_CONSTANT(AUTH_SESSION_RESPONSE_OK);
	BIND_ENUM_CONSTANT(AUTH_SESSION_RESPONSE_USER_NOT_CONNECTED_TO_STEAM);
	BIND_ENUM_CONSTANT(AUTH_SESSION_RESPONSE_NO_LICENSE_OR_EXPIRED);
	BIND_ENUM_CONSTANT(AUTH_SESSION_RESPONSE_VAC_BANNED);
	BIND_ENUM_CONSTANT(AUTH_SESSION_RESPONSE_LOGGED_IN_ELSEWHERE);
	BIND_ENUM_CONSTANT(AUTH_SESSION_RESPONSE_VAC_CHECK_TIMED_OUT);
	BIND_ENUM_CONSTANT(AUTH_SESSION_RESPONSE_AUTH_TICKET_CANCELED);
	BIND_ENUM_CONSTANT(AUTH_SESSION_RESPONSE_AUTH_TICKET_INVALID_ALREADY_USED);
	BIND_ENUM_CONSTANT(AUTH_SESSION_RESPONSE_AUTH_TICKET_INVALID);
	BIND_ENUM_CONSTANT(AUTH_SESSION_RESPONSE_PUBLISHER_ISSUED_BAN);
	BIND_ENUM_CONSTANT(AUTH_SESSION_RESPONSE_AUTH_TICKET_NETWORK_IDENTITY_FAILURE);

	// AvatarSizes Enums
	BIND_ENUM_CONSTANT(AVATAR_SMALL);
	BIND_ENUM_CONSTANT(AVATAR_MEDIUM);
	BIND_ENUM_CONSTANT(AVATAR_LARGE);

	// BeginAuthSessionResult Enums
	BIND_ENUM_CONSTANT(BEGIN_AUTH_SESSION_RESULT_OK);
	BIND_ENUM_CONSTANT(BEGIN_AUTH_SESSION_RESULT_INVALID_TICKET);
	BIND_ENUM_CONSTANT(BEGIN_AUTH_SESSION_RESULT_DUPLICATE_REQUEST);
	BIND_ENUM_CONSTANT(BEGIN_AUTH_SESSION_RESULT_INVALID_VERSION);
	BIND_ENUM_CONSTANT(BEGIN_AUTH_SESSION_RESULT_GAME_MISMATCH);
	BIND_ENUM_CONSTANT(BEGIN_AUTH_SESSION_RESULT_EXPIRED_TICKET);

	// BroadcastUploadResult Enums
	BIND_ENUM_CONSTANT(BROADCAST_UPLOAD_RESULT_NONE);
	BIND_ENUM_CONSTANT(BROADCAST_UPLOAD_RESULT_OK);
	BIND_ENUM_CONSTANT(BROADCAST_UPLOAD_RESULT_INIT_FAILED);
	BIND_ENUM_CONSTANT(BROADCAST_UPLOAD_RESULT_FRAME_FAILED);
	BIND_ENUM_CONSTANT(BROADCAST_UPLOAD_RESULT_TIME_OUT);
	BIND_ENUM_CONSTANT(BROADCAST_UPLOAD_RESULT_BANDWIDTH_EXCEEDED);
	BIND_ENUM_CONSTANT(BROADCAST_UPLOAD_RESULT_LOW_FPS);
	BIND_ENUM_CONSTANT(BROADCAST_UPLOAD_RESULT_MISSING_KEYFRAMES);
	BIND_ENUM_CONSTANT(BROADCAST_UPLOAD_RESULT_NO_CONNECTION);
	BIND_ENUM_CONSTANT(BROADCAST_UPLOAD_RESULT_RELAY_FAILED);
	BIND_ENUM_CONSTANT(BROADCAST_UPLOAD_RESULT_SETTINGS_CHANGED);
	BIND_ENUM_CONSTANT(BROADCAST_UPLOAD_RESULT_MISSING_AUDIO);
	BIND_ENUM_CONSTANT(BROADCAST_UPLOAD_RESULT_TOO_FAR_BEHIND);
	BIND_ENUM_CONSTANT(BROADCAST_UPLOAD_RESULT_TRANSCODE_BEHIND);
	BIND_ENUM_CONSTANT(BROADCAST_UPLOAD_RESULT_NOT_ALLOWED_TO_PLAY);
	BIND_ENUM_CONSTANT(BROADCAST_UPLOAD_RESULT_BUSY);
	BIND_ENUM_CONSTANT(BROADCAST_UPLOAD_RESULT_BANNED);
	BIND_ENUM_CONSTANT(BROADCAST_UPLOAD_RESULT_ALREADY_ACTIVE);
	BIND_ENUM_CONSTANT(BROADCAST_UPLOAD_RESULT_FORCED_OFF);
	BIND_ENUM_CONSTANT(BROADCAST_UPLOAD_RESULT_AUDIO_BEHIND);
	BIND_ENUM_CONSTANT(BROADCAST_UPLOAD_RESULT_SHUTDOWN);
	BIND_ENUM_CONSTANT(BROADCAST_UPLOAD_RESULT_DISCONNECT);
	BIND_ENUM_CONSTANT(BROADCAST_UPLOAD_RESULT_VIDEO_INIT_FAILED);
	BIND_ENUM_CONSTANT(BROADCAST_UPLOAD_RESULT_AUDIO_INIT_FAILED);

	// ChatEntryType Enums
	BIND_ENUM_CONSTANT(CHAT_ENTRY_TYPE_INVALID);
	BIND_ENUM_CONSTANT(CHAT_ENTRY_TYPE_CHAT_MSG);
	BIND_ENUM_CONSTANT(CHAT_ENTRY_TYPE_TYPING);
	BIND_ENUM_CONSTANT(CHAT_ENTRY_TYPE_INVITE_GAME);
	BIND_ENUM_CONSTANT(CHAT_ENTRY_TYPE_EMOTE);
	//	BIND_ENUM_CONSTANT(CHAT_ENTRY_TYPE_LOBBY_GAME_START);
	BIND_ENUM_CONSTANT(CHAT_ENTRY_TYPE_LEFT_CONVERSATION);
	BIND_ENUM_CONSTANT(CHAT_ENTRY_TYPE_ENTERED);
	BIND_ENUM_CONSTANT(CHAT_ENTRY_TYPE_WAS_KICKED);
	BIND_ENUM_CONSTANT(CHAT_ENTRY_TYPE_WAS_BANNED);
	BIND_ENUM_CONSTANT(CHAT_ENTRY_TYPE_DISCONNECTED);
	BIND_ENUM_CONSTANT(CHAT_ENTRY_TYPE_HISTORICAL_CHAT);
	BIND_ENUM_CONSTANT(CHAT_ENTRY_TYPE_LINK_BLOCKED);

	// ChatMemberStateChange Enums
	BIND_BITFIELD_FLAG(CHAT_MEMBER_STATE_CHANGE_ENTERED);
	BIND_BITFIELD_FLAG(CHAT_MEMBER_STATE_CHANGE_LEFT);
	BIND_BITFIELD_FLAG(CHAT_MEMBER_STATE_CHANGE_DISCONNECTED);
	BIND_BITFIELD_FLAG(CHAT_MEMBER_STATE_CHANGE_KICKED);
	BIND_BITFIELD_FLAG(CHAT_MEMBER_STATE_CHANGE_BANNED);

	// ChatRoomEnterResponse Enums
	BIND_ENUM_CONSTANT(CHAT_ROOM_ENTER_RESPONSE_SUCCESS);
	BIND_ENUM_CONSTANT(CHAT_ROOM_ENTER_RESPONSE_DOESNT_EXIST);
	BIND_ENUM_CONSTANT(CHAT_ROOM_ENTER_RESPONSE_NOT_ALLOWED);
	BIND_ENUM_CONSTANT(CHAT_ROOM_ENTER_RESPONSE_FULL);
	BIND_ENUM_CONSTANT(CHAT_ROOM_ENTER_RESPONSE_ERROR);
	BIND_ENUM_CONSTANT(CHAT_ROOM_ENTER_RESPONSE_BANNED);
	BIND_ENUM_CONSTANT(CHAT_ROOM_ENTER_RESPONSE_LIMITED);
	BIND_ENUM_CONSTANT(CHAT_ROOM_ENTER_RESPONSE_CLAN_DISABLED);
	BIND_ENUM_CONSTANT(CHAT_ROOM_ENTER_RESPONSE_COMMUNITY_BAN);
	BIND_ENUM_CONSTANT(CHAT_ROOM_ENTER_RESPONSE_MEMBER_BLOCKED_YOU);
	BIND_ENUM_CONSTANT(CHAT_ROOM_ENTER_RESPONSE_YOU_BLOCKED_MEMBER);
	BIND_ENUM_CONSTANT(CHAT_ROOM_ENTER_RESPONSE_RATE_LIMIT_EXCEEDED);

	// ChatSteamIDInstanceFlags Enums
	BIND_BITFIELD_FLAG(CHAT_ACCOUNT_INSTANCE_MASK);
	BIND_BITFIELD_FLAG(CHAT_INSTANCE_FLAG_CLAN);
	BIND_BITFIELD_FLAG(CHAT_INSTANCE_FLAG_LOBBY);
	BIND_BITFIELD_FLAG(CHAT_INSTANCE_FLAG_MMS_LOBBY);

	// CheckFileSignature Enums
	BIND_ENUM_CONSTANT(CHECK_FILE_SIGNATURE_INVALID_SIGNATURE);
	BIND_ENUM_CONSTANT(CHECK_FILE_SIGNATURE_VALID_SIGNATURE);
	BIND_ENUM_CONSTANT(CHECK_FILE_SIGNATURE_FILE_NOT_FOUND);
	BIND_ENUM_CONSTANT(CHECK_FILE_SIGNATURE_NO_SIGNATURES_FOUND_FOR_THIS_APP);
	BIND_ENUM_CONSTANT(CHECK_FILE_SIGNATURE_NO_SIGNATURES_FOUND_FOR_THIS_FILE);

	// CommunityProfileItemType Enums
	BIND_ENUM_CONSTANT(PROFILE_ITEM_TYPE_ANIMATED_AVATAR);
	BIND_ENUM_CONSTANT(PROFILE_ITEM_TYPE_AVATAR_FRAME);
	BIND_ENUM_CONSTANT(PROFILE_ITEM_TYPE_PROFILE_MODIFIER);
	BIND_ENUM_CONSTANT(PROFILE_ITEM_TYPE_PROFILE_BACKGROUND);
	BIND_ENUM_CONSTANT(PROFILE_ITEM_TYPE_MINI_PROFILE_BACKGROUND);

	// CommunityProfileItemProperty Enums
	BIND_ENUM_CONSTANT(PROFILE_ITEM_PROPERTY_IMAGE_SMALL);
	BIND_ENUM_CONSTANT(PROFILE_ITEM_PROPERTY_IMAGE_LARGE);
	BIND_ENUM_CONSTANT(PROFILE_ITEM_PROPERTY_INTERNAL_NAME);
	BIND_ENUM_CONSTANT(PROFILE_ITEM_PROPERTY_TITLE);
	BIND_ENUM_CONSTANT(PROFILE_ITEM_PROPERTY_DESCRIPTION);
	BIND_ENUM_CONSTANT(PROFILE_ITEM_PROPERTY_APP_ID);
	BIND_ENUM_CONSTANT(PROFILE_ITEM_PROPERTY_TYPE_ID);
	BIND_ENUM_CONSTANT(PROFILE_ITEM_PROPERTY_CLASS);
	BIND_ENUM_CONSTANT(PROFILE_ITEM_PROPERTY_MOVIE_WEBM);
	BIND_ENUM_CONSTANT(PROFILE_ITEM_PROPERTY_MOVIE_MP4);
	BIND_ENUM_CONSTANT(PROFILE_ITEM_PROPERTY_MOVIE_WEBM_SMALL);
	BIND_ENUM_CONSTANT(PROFILE_ITEM_PROPERTY_MOVIE_MP4_SMALL);

	// DenyReason Enums
	BIND_ENUM_CONSTANT(DENY_INVALID);
	BIND_ENUM_CONSTANT(DENY_INVALID_VERSION);
	BIND_ENUM_CONSTANT(DENY_GENERIC);
	BIND_ENUM_CONSTANT(DENY_NOT_LOGGED_ON);
	BIND_ENUM_CONSTANT(DENY_NO_LICENSE);
	BIND_ENUM_CONSTANT(DENY_CHEATER);
	BIND_ENUM_CONSTANT(DENY_LOGGED_IN_ELSEWHERE);
	BIND_ENUM_CONSTANT(DENY_UNKNOWN_TEXT);
	BIND_ENUM_CONSTANT(DENY_INCOMPATIBLE_ANTI_CHEAT);
	BIND_ENUM_CONSTANT(DENY_MEMORY_CORRUPTION);
	BIND_ENUM_CONSTANT(DENY_INCOMPATIBLE_SOFTWARE);
	BIND_ENUM_CONSTANT(DENY_STEAM_CONNECTION_LOST);
	BIND_ENUM_CONSTANT(DENY_STEAM_CONNECTION_ERROR);
	BIND_ENUM_CONSTANT(DENY_STEAM_RESPONSE_TIMED_OUT);
	BIND_ENUM_CONSTANT(DENY_STEAM_VALIDATION_STALLED);
	BIND_ENUM_CONSTANT(DENY_STEAM_OWNER_LEFT_GUEST_USER);

	// DurationControlNotification Enums
	BIND_ENUM_CONSTANT(DURATION_CONTROL_NOTIFICATION_NONE);
	BIND_ENUM_CONSTANT(DURATION_CONTROL_NOTIFICATION_1_HOUR);
	BIND_ENUM_CONSTANT(DURATION_CONTROL_NOTIFICATION_3_HOURS);
	BIND_ENUM_CONSTANT(DURATION_CONTROL_NOTIFICATION_HALF_PROGRESS);
	BIND_ENUM_CONSTANT(DURATION_CONTROL_NOTIFICATION_NO_PROGRESS);
	BIND_ENUM_CONSTANT(DURATION_CONTROL_NOTIFICATION_EXIT_SOON_3H);
	BIND_ENUM_CONSTANT(DURATION_CONTROL_NOTIFICATION_EXIT_SOON_5H);
	BIND_ENUM_CONSTANT(DURATION_CONTROL_NOTIFICATION_EXIT_SOON_NIGHT);

	// DurationControlOnlineState Enums
	BIND_ENUM_CONSTANT(DURATION_CONTROL_ONLINE_STATE_INVALID);
	BIND_ENUM_CONSTANT(DURATION_CONTROL_ONLINE_STATE_OFFLINE);
	BIND_ENUM_CONSTANT(DURATION_CONTROL_ONLINE_STATE_ONLINE);
	BIND_ENUM_CONSTANT(DURATION_CONTROL_ONLINE_STATE_ONLINE_HIGH_PRIORITY);

	// DurationControlProgress Enums
	BIND_ENUM_CONSTANT(DURATION_CONTROL_PROGRESS_FULL);
	BIND_ENUM_CONSTANT(DURATION_CONTROL_PROGRESS_HALF);
	BIND_ENUM_CONSTANT(DURATION_CONTROL_PROGRESS_NONE);
	BIND_ENUM_CONSTANT(DURATION_CONTROL_EXIT_SOON_3H);
	BIND_ENUM_CONSTANT(DURATION_CONTROL_EXIT_SOON_5H);
	BIND_ENUM_CONSTANT(DURATION_CONTROL_EXIT_SOON_NIGHT);

	// FilePathType Enums
	BIND_ENUM_CONSTANT(FILE_PATH_TYPE_INVALID);
	BIND_ENUM_CONSTANT(FILE_PATH_TYPE_ABSOLUTE);
	BIND_ENUM_CONSTANT(FILE_PATH_TYPE_API_FILENAME);

	// FriendFlags Enums
	BIND_BITFIELD_FLAG(FRIEND_FLAG_NONE);
	BIND_BITFIELD_FLAG(FRIEND_FLAG_BLOCKED);
	BIND_BITFIELD_FLAG(FRIEND_FLAG_FRIENDSHIP_REQUESTED);
	BIND_BITFIELD_FLAG(FRIEND_FLAG_IMMEDIATE);
	BIND_BITFIELD_FLAG(FRIEND_FLAG_CLAN_MEMBER);
	BIND_BITFIELD_FLAG(FRIEND_FLAG_ON_GAME_SERVER);
	//	BIND_BITFIELD_FLAG(FRIEND_FLAG_HAS_PLAYED_WITH);
	//	BIND_BITFIELD_FLAG(FRIEND_FLAG_FRIEND_OF_FRIEND);
	BIND_BITFIELD_FLAG(FRIEND_FLAG_REQUESTING_FRIENDSHIP);
	BIND_BITFIELD_FLAG(FRIEND_FLAG_REQUESTING_INFO);
	BIND_BITFIELD_FLAG(FRIEND_FLAG_IGNORED);
	BIND_BITFIELD_FLAG(FRIEND_FLAG_IGNORED_FRIEND);
	//	BIND_BITFIELD_FLAG(FRIEND_FLAG_SUGGESTED);
	BIND_BITFIELD_FLAG(FRIEND_FLAG_CHAT_MEMBER);
	BIND_BITFIELD_FLAG(FRIEND_FLAG_ALL);

	// FriendRelationship Enums
	BIND_ENUM_CONSTANT(FRIEND_RELATION_NONE);
	BIND_ENUM_CONSTANT(FRIEND_RELATION_BLOCKED);
	BIND_ENUM_CONSTANT(FRIEND_RELATION_REQUEST_RECIPIENT);
	BIND_ENUM_CONSTANT(FRIEND_RELATION_FRIEND);
	BIND_ENUM_CONSTANT(FRIEND_RELATION_REQUEST_INITIATOR);
	BIND_ENUM_CONSTANT(FRIEND_RELATION_IGNORED);
	BIND_ENUM_CONSTANT(FRIEND_RELATION_IGNORED_FRIEND);
	BIND_ENUM_CONSTANT(FRIEND_RELATION_SUGGESTED);
	BIND_ENUM_CONSTANT(FRIEND_RELATION_MAX);

	// GameIDType Enums
	BIND_ENUM_CONSTANT(GAME_TYPE_APP);
	BIND_ENUM_CONSTANT(GAME_TYPE_GAME_MOD);
	BIND_ENUM_CONSTANT(GAME_TYPE_SHORTCUT);
	BIND_ENUM_CONSTANT(GAME_TYPE_P2P);

	// GameSearchErrorCode Enums
	BIND_ENUM_CONSTANT(GAME_SEARCH_ERROR_CODE_OK);
	BIND_ENUM_CONSTANT(GAME_SEARCH_ERROR_CODE_SEARCH_AREADY_IN_PROGRESS);
	BIND_ENUM_CONSTANT(GAME_SEARCH_ERROR_CODE_NO_SEARCH_IN_PROGRESS);
	BIND_ENUM_CONSTANT(GAME_SEARCH_ERROR_CODE_NOT_LOBBY_LEADER);
	BIND_ENUM_CONSTANT(GAME_SEARCH_ERROR_CODE_NO_HOST_AVAILABLE);
	BIND_ENUM_CONSTANT(GAME_SEARCH_ERROR_CODE_SEARCH_PARAMS_INVALID);
	BIND_ENUM_CONSTANT(GAME_SEARCH_ERROR_CODE_OFFLINE);
	BIND_ENUM_CONSTANT(GAME_SEARCH_ERROR_CODE_NOT_AUTHORIZED);
	BIND_ENUM_CONSTANT(GAME_SEARCH_ERROR_CODE_UNKNOWN_ERROR);

	// HTTPMethod Enums
	BIND_ENUM_CONSTANT(HTTP_METHOD_INVALID);
	BIND_ENUM_CONSTANT(HTTP_METHOD_GET);
	BIND_ENUM_CONSTANT(HTTP_METHOD_HEAD);
	BIND_ENUM_CONSTANT(HTTP_METHOD_POST);
	BIND_ENUM_CONSTANT(HTTP_METHOD_PUT);
	BIND_ENUM_CONSTANT(HTTP_METHOD_DELETE);
	BIND_ENUM_CONSTANT(HTTP_METHOD_OPTIONS);
	BIND_ENUM_CONSTANT(HTTP_METHOD_PATCH);

	// HTTPStatusCode Enums
	BIND_ENUM_CONSTANT(HTTP_STATUS_CODE_INVALID);
	BIND_ENUM_CONSTANT(HTTP_STATUS_CODE_100_CONTINUE);
	BIND_ENUM_CONSTANT(HTTP_STATUS_CODE_101_SWITCHING_PROTOCOLS);
	BIND_ENUM_CONSTANT(HTTP_STATUS_CODE_200_OK);
	BIND_ENUM_CONSTANT(HTTP_STATUS_CODE_201_CREATED);
	BIND_ENUM_CONSTANT(HTTP_STATUS_CODE_202_ACCEPTED);
	BIND_ENUM_CONSTANT(HTTP_STATUS_CODE_203_NON_AUTHORITATIVE);
	BIND_ENUM_CONSTANT(HTTP_STATUS_CODE_204_NO_CONTENT);
	BIND_ENUM_CONSTANT(HTTP_STATUS_CODE_205_RESET_CONTENT);
	BIND_ENUM_CONSTANT(HTTP_STATUS_CODE_206_PARTIAL_CONTENT);
	BIND_ENUM_CONSTANT(HTTP_STATUS_CODE_300_MULTIPLE_CHOICES);
	BIND_ENUM_CONSTANT(HTTP_STATUS_CODE_301_MOVED_PERMANENTLY);
	BIND_ENUM_CONSTANT(HTTP_STATUS_CODE_302_FOUND);
	BIND_ENUM_CONSTANT(HTTP_STATUS_CODE_303_SEE_OTHER);
	BIND_ENUM_CONSTANT(HTTP_STATUS_CODE_304_NOT_MODIFIED);
	BIND_ENUM_CONSTANT(HTTP_STATUS_CODE_305_USE_PROXY);
	BIND_ENUM_CONSTANT(HTTP_STATUS_CODE_307_TEMPORARY_REDIRECT);
	BIND_ENUM_CONSTANT(HTTP_STATUS_CODE_308_PERMANENT_REDIRECT);
	BIND_ENUM_CONSTANT(HTTP_STATUS_CODE_400_BAD_REQUEST);
	BIND_ENUM_CONSTANT(HTTP_STATUS_CODE_401_UNAUTHORIZED);
	BIND_ENUM_CONSTANT(HTTP_STATUS_CODE_402_PAYMENT_REQUIRED);
	BIND_ENUM_CONSTANT(HTTP_STATUS_CODE_403_FORBIDDEN);
	BIND_ENUM_CONSTANT(HTTP_STATUS_CODE_404_NOT_FOUND);
	BIND_ENUM_CONSTANT(HTTP_STATUS_CODE_405_METHOD_NOT_ALLOWED);
	BIND_ENUM_CONSTANT(HTTP_STATUS_CODE_406_NOT_ACCEPTABLE);
	BIND_ENUM_CONSTANT(HTTP_STATUS_CODE_407_PROXY_AUTH_REQUIRED);
	BIND_ENUM_CONSTANT(HTTP_STATUS_CODE_408_REQUEST_TIMEOUT);
	BIND_ENUM_CONSTANT(HTTP_STATUS_CODE_409_CONFLICT);
	BIND_ENUM_CONSTANT(HTTP_STATUS_CODE_410_GONE);
	BIND_ENUM_CONSTANT(HTTP_STATUS_CODE_411_LENGTH_REQUIRED);
	BIND_ENUM_CONSTANT(HTTP_STATUS_CODE_412_PRECONDITION_FAILED);
	BIND_ENUM_CONSTANT(HTTP_STATUS_CODE_413_REQUEST_ENTITY_TOO_LARGE);
	BIND_ENUM_CONSTANT(HTTP_STATUS_CODE_414_REQUEST_URI_TOO_LONG);
	BIND_ENUM_CONSTANT(HTTP_STATUS_CODE_415_UNSUPPORTED_MEDIA_TYPE);
	BIND_ENUM_CONSTANT(HTTP_STATUS_CODE_416_REQUESTED_RANGE_NOT_SATISFIABLE);
	BIND_ENUM_CONSTANT(HTTP_STATUS_CODE_417_EXPECTATION_FAILED);
	BIND_ENUM_CONSTANT(HTTP_STATUS_CODE_4XX_UNKNOWN);
	BIND_ENUM_CONSTANT(HTTP_STATUS_CODE_429_TOO_MANY_REQUESTS);
	BIND_ENUM_CONSTANT(HTTP_STATUS_CODE_444_CONNECTION_CLOSED);
	BIND_ENUM_CONSTANT(HTTP_STATUS_CODE_500_INTERNAL_SERVER_ERROR);
	BIND_ENUM_CONSTANT(HTTP_STATUS_CODE_501_NOT_IMPLEMENTED);
	BIND_ENUM_CONSTANT(HTTP_STATUS_CODE_502_BAD_GATEWAY);
	BIND_ENUM_CONSTANT(HTTP_STATUS_CODE_503_SERVICE_UNAVAILABLE);
	BIND_ENUM_CONSTANT(HTTP_STATUS_CODE_504_GATEWAY_TIMEOUT);
	BIND_ENUM_CONSTANT(HTTP_STATUS_CODE_505_HTTP_VERSION_NOT_SUPPORTED);
	BIND_ENUM_CONSTANT(HTTP_STATUS_CODE_5XX_UNKNOWN);

	// IPType Enums
	BIND_ENUM_CONSTANT(IP_TYPE_IPV4);
	BIND_ENUM_CONSTANT(IP_TYPE_IPV6);

	// IPv6ConnectivityProtocol Enums
	BIND_ENUM_CONSTANT(IPV6_CONNECTIVITY_PROTOCOL_INVALID);
	BIND_ENUM_CONSTANT(IPV6_CONNECTIVITY_PROTOCOL_HTTP);
	BIND_ENUM_CONSTANT(IPV6_CONNECTIVITY_PROTOCOL_UDP);

	// IPv6ConnectivityState Enums
	BIND_ENUM_CONSTANT(IPV6_CONNECTIVITY_STATE_UNKNOWN);
	BIND_ENUM_CONSTANT(IPV6_CONNECTIVITY_STATE_GOOD);
	BIND_ENUM_CONSTANT(IPV6_CONNECTIVITY_STATE_BAD);

	// ItemFlags Enums
	BIND_BITFIELD_FLAG(STEAM_ITEM_NO_TRADE);
	BIND_BITFIELD_FLAG(STEAM_ITEM_REMOVED);
	BIND_BITFIELD_FLAG(STEAM_ITEM_CONSUMED);

	// ItemPreviewType Enums
	BIND_ENUM_CONSTANT(ITEM_PREVIEW_TYPE_IMAGE);
	BIND_ENUM_CONSTANT(ITEM_PREVIEW_TYPE_YOUTUBE_VIDEO);
	BIND_ENUM_CONSTANT(ITEM_PREVIEW_TYPE_SKETCHFAB);
	BIND_ENUM_CONSTANT(ITEM_PREVIEW_TYPE_ENVIRONMENTMAP_HORIZONTAL_CROSS);
	BIND_ENUM_CONSTANT(ITEM_PREVIEW_TYPE_ENVIRONMENTMAP_LAT_LONG);
	BIND_ENUM_CONSTANT(ITEM_PREVIEW_TYPE_RESERVED_MAX);

	// ItemState Enums
	BIND_BITFIELD_FLAG(ITEM_STATE_NONE);
	BIND_BITFIELD_FLAG(ITEM_STATE_SUBSCRIBED);
	BIND_BITFIELD_FLAG(ITEM_STATE_LEGACY_ITEM);
	BIND_BITFIELD_FLAG(ITEM_STATE_INSTALLED);
	BIND_BITFIELD_FLAG(ITEM_STATE_NEEDS_UPDATE);
	BIND_BITFIELD_FLAG(ITEM_STATE_DOWNLOADING);
	BIND_BITFIELD_FLAG(ITEM_STATE_DOWNLOAD_PENDING);

	// ItemStatistic Enums
	BIND_ENUM_CONSTANT(ITEM_STATISTIC_NUM_SUBSCRIPTIONS);
	BIND_ENUM_CONSTANT(ITEM_STATISTIC_NUM_FAVORITES);
	BIND_ENUM_CONSTANT(ITEM_STATISTIC_NUM_FOLLOWERS);
	BIND_ENUM_CONSTANT(ITEM_STATISTIC_NUM_UNIQUE_SUBSCRIPTIONS);
	BIND_ENUM_CONSTANT(ITEM_STATISTIC_NUM_UNIQUE_FAVORITES);
	BIND_ENUM_CONSTANT(ITEM_STATISTIC_NUM_UNIQUE_FOLLOWERS);
	BIND_ENUM_CONSTANT(ITEM_STATISTIC_NUM_UNIQUE_WEBSITE_VIEWS);
	BIND_ENUM_CONSTANT(ITEM_STATISTIC_REPORT_SCORE);
	BIND_ENUM_CONSTANT(ITEM_STATISTIC_NUM_SECONDS_PLAYED);
	BIND_ENUM_CONSTANT(ITEM_STATISTIC_NUM_PLAYTIME_SESSIONS);
	BIND_ENUM_CONSTANT(ITEM_STATISTIC_NUM_COMMENTS);
	BIND_ENUM_CONSTANT(ITEM_STATISTIC_NUM_SECONDS_PLAYED_DURING_TIME_PERIOD);
	BIND_ENUM_CONSTANT(ITEM_STATISTIC_NUM_PLAYTIME_SESSIONS_DURING_TIME_PERIOD);

	// ItemUpdateStatus Enums
	BIND_ENUM_CONSTANT(ITEM_UPDATE_STATUS_INVALID);
	BIND_ENUM_CONSTANT(ITEM_UPDATE_STATUS_PREPARING_CONFIG);
	BIND_ENUM_CONSTANT(ITEM_UPDATE_STATUS_PREPARING_CONTENT);
	BIND_ENUM_CONSTANT(ITEM_UPDATE_STATUS_UPLOADING_CONTENT);
	BIND_ENUM_CONSTANT(ITEM_UPDATE_STATUS_UPLOADING_PREVIEW_FILE);
	BIND_ENUM_CONSTANT(ITEM_UPDATE_STATUS_COMMITTING_CHANGES);

	// LeaderboardDataRequest Enums
	BIND_ENUM_CONSTANT(LEADERBOARD_DATA_REQUEST_GLOBAL);
	BIND_ENUM_CONSTANT(LEADERBOARD_DATA_REQUEST_GLOBAL_AROUND_USER);
	BIND_ENUM_CONSTANT(LEADERBOARD_DATA_REQUEST_FRIENDS);
	BIND_ENUM_CONSTANT(LEADERBOARD_DATA_REQUEST_USERS);

	// LeaderboardDisplayType Enums
	BIND_ENUM_CONSTANT(LEADERBOARD_DISPLAY_TYPE_NONE);
	BIND_ENUM_CONSTANT(LEADERBOARD_DISPLAY_TYPE_NUMERIC);
	BIND_ENUM_CONSTANT(LEADERBOARD_DISPLAY_TYPE_TIME_SECONDS);
	BIND_ENUM_CONSTANT(LEADERBOARD_DISPLAY_TYPE_TIME_MILLISECONDS);

	// LeaderboardSortMethod Enums
	BIND_ENUM_CONSTANT(LEADERBOARD_SORT_METHOD_NONE);
	BIND_ENUM_CONSTANT(LEADERBOARD_SORT_METHOD_ASCENDING);
	BIND_ENUM_CONSTANT(LEADERBOARD_SORT_METHOD_DESCENDING);

	// LeaderboardUploadScoreMethod Enums
	BIND_ENUM_CONSTANT(LEADERBOARD_UPLOAD_SCORE_METHOD_NONE);
	BIND_ENUM_CONSTANT(LEADERBOARD_UPLOAD_SCORE_METHOD_KEEP_BEST);
	BIND_ENUM_CONSTANT(LEADERBOARD_UPLOAD_SCORE_METHOD_FORCE_UPDATE);

	// LobbyComparison Enums
	BIND_ENUM_CONSTANT(LOBBY_COMPARISON_EQUAL_TO_OR_LESS_THAN);
	BIND_ENUM_CONSTANT(LOBBY_COMPARISON_LESS_THAN);
	BIND_ENUM_CONSTANT(LOBBY_COMPARISON_EQUAL);
	BIND_ENUM_CONSTANT(LOBBY_COMPARISON_GREATER_THAN);
	BIND_ENUM_CONSTANT(OBBY_COMPARISON_EQUAL_TO_GREATER_THAN);
	BIND_ENUM_CONSTANT(LOBBY_COMPARISON_NOT_EQUAL)

	// LobbyDistanceFilter Enums
	BIND_ENUM_CONSTANT(LOBBY_DISTANCE_FILTER_CLOSE);
	BIND_ENUM_CONSTANT(LOBBY_DISTANCE_FILTER_DEFAULT);
	BIND_ENUM_CONSTANT(LOBBY_DISTANCE_FILTER_FAR);
	BIND_ENUM_CONSTANT(LOBBY_DISTANCE_FILTER_WORLDWIDE);

	// LobbyType Enums
	BIND_ENUM_CONSTANT(LOBBY_TYPE_PRIVATE);
	BIND_ENUM_CONSTANT(LOBBY_TYPE_FRIENDS_ONLY);
	BIND_ENUM_CONSTANT(LOBBY_TYPE_PUBLIC);
	BIND_ENUM_CONSTANT(LOBBY_TYPE_INVISIBLE);
	BIND_ENUM_CONSTANT(LOBBY_TYPE_PRIVATE_UNIQUE);

	// LocalFileChange Enums
	BIND_ENUM_CONSTANT(LOCAL_FILE_CHANGE_INVALID);
	BIND_ENUM_CONSTANT(LOCAL_FILE_CHANGE_FILE_UPDATED);
	BIND_ENUM_CONSTANT(LOCAL_FILE_CHANGE_FILE_DELETED);

	// MarketNotAllowedReasonFlags Enums
	BIND_ENUM_CONSTANT(MARKET_NOT_ALLOWED_REASON_NONE);
	BIND_ENUM_CONSTANT(MARKET_NOT_ALLOWED_REASON_TEMPORARY_FAILURE);
	BIND_ENUM_CONSTANT(MARKET_NOT_ALLOWED_REASON_ACCOUNT_DISABLED);
	BIND_ENUM_CONSTANT(MARKET_NOT_ALLOWED_REASON_ACCOUNT_LOCKED_DOWN);
	BIND_ENUM_CONSTANT(MARKET_NOT_ALLOWED_REASON_ACCOUNT_LIMITED);
	BIND_ENUM_CONSTANT(MARKET_NOT_ALLOWED_REASON_TRADE_BANNED);
	BIND_ENUM_CONSTANT(MARKET_NOT_ALLOWED_REASON_ACCOUNT_NOT_TRUSTED);
	BIND_ENUM_CONSTANT(MARKET_NOT_ALLOWED_REASON_STEAM_GUARD_NOT_ENABLED);
	BIND_ENUM_CONSTANT(MARKET_NOT_ALLOWED_REASON_STEAM_GAURD_ONLY_RECENTLY_ENABLED);
	BIND_ENUM_CONSTANT(MARKET_NOT_ALLOWED_REASON_RECENT_PASSWORD_RESET);
	BIND_ENUM_CONSTANT(MARKET_NOT_ALLOWED_REASON_NEW_PAYMENT_METHOD);
	BIND_ENUM_CONSTANT(MARKET_NOT_ALLOWED_REASON_INVALID_COOKIE);
	BIND_ENUM_CONSTANT(MARKET_NOT_ALLOWED_REASON_USING_NEW_DEVICE);
	BIND_ENUM_CONSTANT(MARKET_NOT_ALLOWED_REASON_RECENT_SELF_REFUND);
	BIND_ENUM_CONSTANT(MARKET_NOT_ALLOWED_REASON_NEW_PAYMENT_METHOD_CANNOT_BE_VERIFIED);
	BIND_ENUM_CONSTANT(MARKET_NOT_ALLOWED_REASON_NO_RECENT_PURCHASES);
	BIND_ENUM_CONSTANT(MARKET_NOT_ALLOWED_REASON_ACCEPTED_WALLET_GIFT);

	// GameServer Enums
	BIND_ENUM_CONSTANT(SERVER_MODE_INVALID);
	BIND_ENUM_CONSTANT(SERVER_MODE_NO_AUTHENTICATION);
	BIND_ENUM_CONSTANT(SERVER_MODE_AUTHENTICATION);
	BIND_ENUM_CONSTANT(SERVER_MODE_AUTHENTICATION_AND_SECURE);

	// MatchMakingServerResponse Enums
	BIND_ENUM_CONSTANT(SERVER_RESPONDED);
	BIND_ENUM_CONSTANT(SERVER_FAILED_TO_RESPOND);
	BIND_ENUM_CONSTANT(NO_SERVERS_LISTED_ON_MASTER_SERVER);

	// NetworkingAvailability Enums
	BIND_ENUM_CONSTANT(NETWORKING_AVAILABILITY_CANNOT_TRY);
	BIND_ENUM_CONSTANT(NETWORKING_AVAILABILITY_FAILED);
	BIND_ENUM_CONSTANT(NETWORKING_AVAILABILITY_PREVIOUSLY);
	BIND_ENUM_CONSTANT(NETWORKING_AVAILABILITY_RETRYING);
	BIND_ENUM_CONSTANT(NETWORKING_AVAILABILITY_NEVER_TRIED);
	BIND_ENUM_CONSTANT(NETWORKING_AVAILABILITY_WAITING);
	BIND_ENUM_CONSTANT(NETWORKING_AVAILABILITY_ATTEMPTING);
	BIND_ENUM_CONSTANT(NETWORKING_AVAILABILITY_CURRENT);
	BIND_ENUM_CONSTANT(NETWORKING_AVAILABILITY_UNKNOWN);
	BIND_ENUM_CONSTANT(NETWORKING_AVAILABILITY_FORCE_32BIT);

	// NetworkingConfigDataType Enums
	BIND_ENUM_CONSTANT(NETWORKING_CONFIG_TYPE_INT32);
	BIND_ENUM_CONSTANT(NETWORKING_CONFIG_TYPE_INT64);
	BIND_ENUM_CONSTANT(NETWORKING_CONFIG_TYPE_FLOAT);
	BIND_ENUM_CONSTANT(ETWORKING_CONFIG_TYPE_STRING);
	BIND_ENUM_CONSTANT(NETWORKING_CONFIG_TYPE_FUNCTION_PTR);
	BIND_ENUM_CONSTANT(NETWORKING_CONFIG_TYPE_FORCE_32BIT);

	// NetworkingConfigScope Enums
	BIND_ENUM_CONSTANT(NETWORKING_CONFIG_SCOPE_GLOBAL);
	BIND_ENUM_CONSTANT(NETWORKING_CONFIG_SCOPE_SOCKETS_INTERFACE);
	BIND_ENUM_CONSTANT(NETWORKING_CONFIG_SCOPE_LISTEN_SOCKET);
	BIND_ENUM_CONSTANT(NETWORKING_CONFIG_SCOPE_CONNECTION);
	BIND_ENUM_CONSTANT(NETWORKING_CONFIG_SCOPE_FORCE_32BIT);

	// NetworkingConfigValue Enums
	BIND_ENUM_CONSTANT(NETWORKING_CONFIG_INVALID);
	BIND_ENUM_CONSTANT(NETWORKING_CONFIG_FAKE_PACKET_LOSS_SEND);
	BIND_ENUM_CONSTANT(NETWORKING_CONFIG_FAKE_PACKET_LOSS_RECV);
	BIND_ENUM_CONSTANT(NETWORKING_CONFIG_FAKE_PACKET_LAG_SEND);
	BIND_ENUM_CONSTANT(NETWORKING_CONFIG_FAKE_PACKET_LAG_RECV);
	BIND_ENUM_CONSTANT(NETWORKING_CONFIG_FAKE_PACKET_REORDER_SEND);
	BIND_ENUM_CONSTANT(NETWORKING_CONFIG_FAKE_PACKET_REORDER_RECV);
	BIND_ENUM_CONSTANT(NETWORKING_CONFIG_FAKE_PACKET_REORDER_TIME);
	BIND_ENUM_CONSTANT(NETWORKING_CONFIG_FAKE_PACKET_DUP_SEND);
	BIND_ENUM_CONSTANT(NETWORKING_CONFIG_FAKE_PACKET_DUP_REVC);
	BIND_ENUM_CONSTANT(NETWORKING_CONFIG_FAKE_PACKET_DUP_TIME_MAX);
	BIND_ENUM_CONSTANT(NETWORKING_CONFIG_PACKET_TRACE_MAX_BYTES);
	BIND_ENUM_CONSTANT(NETWORKING_CONFIG_FAKE_RATE_LIMIT_SEND_RATE);
	BIND_ENUM_CONSTANT(NETWORKING_CONFIG_FAKE_RATE_LIMIT_SEND_BURST);
	BIND_ENUM_CONSTANT(NETWORKING_CONFIG_FAKE_RATE_LIMIT_RECV_RATE);
	BIND_ENUM_CONSTANT(NETWORKING_CONFIG_FAKE_RATE_LIMIT_RECV_BURST);
	BIND_ENUM_CONSTANT(NETWORKING_CONFIG_CONNECTION_USER_DATA);
	BIND_ENUM_CONSTANT(NETWORKING_CONFIG_TIMEOUT_INITIAL);
	BIND_ENUM_CONSTANT(NETWORKING_CONFIG_TIMEOUT_CONNECTED);
	BIND_ENUM_CONSTANT(NETWORKING_CONFIG_SEND_BUFFER_SIZE);
	BIND_ENUM_CONSTANT(NETWORKING_CONFIG_RECV_BUFFER_SIZE);
	BIND_ENUM_CONSTANT(NETWORKING_CONFIG_RECV_BUFFER_MESSAGES);
	BIND_ENUM_CONSTANT(NETWORKING_CONFIG_RECV_MAX_MESSAGE_SIZE);
	BIND_ENUM_CONSTANT(NETWORKING_CONFIG_RECV_MAX_SEGMENTS_PER_PACKET);
	BIND_ENUM_CONSTANT(NETWORKING_CONFIG_SEND_RATE_MIN);
	BIND_ENUM_CONSTANT(NETWORKING_CONFIG_SEND_RATE_MAX);
	BIND_ENUM_CONSTANT(NETWORKING_CONFIG_NAGLE_TIME);
	BIND_ENUM_CONSTANT(NETWORKING_CONFIG_IP_ALLOW_WITHOUT_AUTH);
	BIND_ENUM_CONSTANT(NETWORKING_CONFIG_MTU_PACKET_SIZE);
	BIND_ENUM_CONSTANT(NETWORKING_CONFIG_MTU_DATA_SIZE);
	BIND_ENUM_CONSTANT(NETWORKING_CONFIG_UNENCRYPTED);
	BIND_ENUM_CONSTANT(NETWORKING_CONFIG_SYMMETRIC_CONNECT);
	BIND_ENUM_CONSTANT(NETWORKING_CONFIG_LOCAL_VIRTUAL_PORT);
	BIND_ENUM_CONSTANT(NETWORKING_CONFIG_DUAL_WIFI_ENABLE);
	BIND_ENUM_CONSTANT(NETWORKING_CONFIG_ENABLE_DIAGNOSTICS_UI);
	BIND_ENUM_CONSTANT(NETWORKING_CONFIG_SDR_CLIENT_CONSEC_PING_TIMEOUT_FAIL_INITIAL);
	BIND_ENUM_CONSTANT(NETWORKING_CONFIG_SDR_CLIENT_CONSEC_PING_TIMEOUT_FAIL);
	BIND_ENUM_CONSTANT(NETWORKING_CONFIG_SDR_CLIENT_MIN_PINGS_BEFORE_PING_ACCURATE);
	BIND_ENUM_CONSTANT(NETWORKING_CONFIG_SDR_CLIENT_SINGLE_SOCKET);
	BIND_ENUM_CONSTANT(NETWORKING_CONFIG_SDR_CLIENT_FORCE_RELAY_CLUSTER);
	BIND_ENUM_CONSTANT(NETWORKING_CONFIG_SDR_CLIENT_DEBUG_TICKET_ADDRESS);
	BIND_ENUM_CONSTANT(NETWORKING_CONFIG_SDR_CLIENT_FORCE_PROXY_ADDR);
	BIND_ENUM_CONSTANT(NETWORKING_CONFIG_SDR_CLIENT_FAKE_CLUSTER_PING);
	BIND_ENUM_CONSTANT(NETWORKING_CONFIG_LOG_LEVEL_ACK_RTT);
	BIND_ENUM_CONSTANT(NETWORKING_CONFIG_LOG_LEVEL_PACKET_DECODE);
	BIND_ENUM_CONSTANT(NETWORKING_CONFIG_LOG_LEVEL_MESSAGE);
	BIND_ENUM_CONSTANT(NETWORKING_CONFIG_LOG_LEVEL_PACKET_GAPS);
	BIND_ENUM_CONSTANT(NETWORKING_CONFIG_LOG_LEVEL_P2P_RENDEZVOUS);
	BIND_ENUM_CONSTANT(NETWORKING_CONFIG_LOG_LEVEL_SRD_RELAY_PINGS);
	BIND_ENUM_CONSTANT(NETWORKING_CONFIG_CALLBACK_CONNECTION_STATUS_CHANGED);
	BIND_ENUM_CONSTANT(NETWORKING_CONFIG_CALLBACK_AUTH_STATUS_CHANGED);
	BIND_ENUM_CONSTANT(NETWORKING_CONFIG_CALLBACK_RELAY_NETWORK_STATUS_CHANGED);
	BIND_ENUM_CONSTANT(NETWORKING_CONFIG_CALLBACK_MESSAGE_SESSION_REQUEST);
	BIND_ENUM_CONSTANT(NETWORKING_CONFIG_CALLBACK_MESSAGES_SESSION_FAILED);
	BIND_ENUM_CONSTANT(NETWORKING_CONFIG_CALLBACK_CREATE_CONNECTION_SIGNALING);
	BIND_ENUM_CONSTANT(NETWORKING_CONFIG_CALLBACK_FAKE_IP_RESULT);
	BIND_ENUM_CONSTANT(NETWORKING_CONFIG_P2P_STUN_SERVER_LIST);
	BIND_ENUM_CONSTANT(NETWORKING_CONFIG_P2P_TRANSPORT_ICE_ENABLE);
	BIND_ENUM_CONSTANT(NETWORKING_CONFIG_P2P_TRANSPORT_ICE_PENALTY);
	BIND_ENUM_CONSTANT(NETWORKING_CONFIG_P2P_TRANSPORT_SDR_PENALTY);
	BIND_ENUM_CONSTANT(NETWORKING_CONFIG_P2P_TURN_SERVER_LIST);
	BIND_ENUM_CONSTANT(NETWORKING_CONFIG_P2P_TURN_uSER_LIST);
	BIND_ENUM_CONSTANT(NETWORKING_CONFIG_P2P_TURN_PASS_LIST);
	//	BIND_ENUM_CONSTANT(NETWORKING_CONFIG_P2P_TRANSPORT_LAN_BEACON_PENALTY);		// Commented out in the SDK
	BIND_ENUM_CONSTANT(NETWORKING_CONFIG_P2P_TRANSPORT_ICE_IMPLEMENTATION);
	BIND_ENUM_CONSTANT(NETWORKING_CONFIG_VALUE_FORCE32BIT);

	// NetworkingConnectionEnd Enums
	BIND_ENUM_CONSTANT(CONNECTION_END_INVALID);
	BIND_ENUM_CONSTANT(CONNECTION_END_APP_MIN);
	BIND_ENUM_CONSTANT(CONNECTION_END_APP_GENERIC);
	BIND_ENUM_CONSTANT(CONNECTION_END_APP_MAX);
	BIND_ENUM_CONSTANT(CONNECTION_END_APP_EXCEPTION_MIN);
	BIND_ENUM_CONSTANT(CONNECTION_END_APP_EXCEPTION_GENERIC);
	BIND_ENUM_CONSTANT(CONNECTION_END_APP_EXCEPTION_MAX);
	BIND_ENUM_CONSTANT(CONNECTION_END_LOCAL_MIN);
	BIND_ENUM_CONSTANT(CONNECTION_END_LOCAL_OFFLINE_MODE);
	BIND_ENUM_CONSTANT(CONNECTION_END_LOCAL_MANY_RELAY_CONNECTIVITY);
	BIND_ENUM_CONSTANT(CONNECTION_END_LOCAL_HOSTED_SERVER_PRIMARY_RELAY);
	BIND_ENUM_CONSTANT(CONNECTION_END_LOCAL_NETWORK_CONFIG);
	BIND_ENUM_CONSTANT(CONNECTION_END_LOCAL_RIGHTS);
	BIND_ENUM_CONSTANT(CONNECTION_END_NO_PUBLIC_ADDRESS);
	BIND_ENUM_CONSTANT(CONNECTION_END_LOCAL_MAX);
	BIND_ENUM_CONSTANT(CONNECTION_END_REMOVE_MIN);
	BIND_ENUM_CONSTANT(CONNECTION_END_REMOTE_TIMEOUT);
	BIND_ENUM_CONSTANT(CONNECTION_END_REMOTE_BAD_CRYPT);
	BIND_ENUM_CONSTANT(CONNECTION_END_REMOTE_BAD_CERT);
	BIND_ENUM_CONSTANT(CONNECTION_END_BAD_PROTOCOL_VERSION);
	BIND_ENUM_CONSTANT(CONNECTION_END_REMOTE_P2P_ICE_NO_PUBLIC_ADDRESSES);
	BIND_ENUM_CONSTANT(CONNECTION_END_REMOTE_MAX);
	BIND_ENUM_CONSTANT(CONNECTION_END_MISC_MIN);
	BIND_ENUM_CONSTANT(CONNECTION_END_MISC_GENERIC);
	BIND_ENUM_CONSTANT(CONNECTION_END_MISC_INTERNAL_ERROR);
	BIND_ENUM_CONSTANT(CONNECTION_END_MISC_TIMEOUT);
	BIND_ENUM_CONSTANT(CONNECTION_END_MISC_STEAM_CONNECTIVITY);
	BIND_ENUM_CONSTANT(CONNECTION_END_MISC_NO_RELAY_SESSIONS_TO_CLIENT);
	BIND_ENUM_CONSTANT(CONNECTION_END_MISC_P2P_RENDEZVOUS);
	BIND_ENUM_CONSTANT(CONNECTION_END_MISC_P2P_NAT_FIREWALL);
	BIND_ENUM_CONSTANT(CONNECTION_END_MISC_PEER_SENT_NO_CONNECTION);
	BIND_ENUM_CONSTANT(CONNECTION_END_MISC_MAX);
	BIND_ENUM_CONSTANT(CONNECTION_END_FORCE32BIT);

	// NetworkingConnectionState Enums
	BIND_ENUM_CONSTANT(CONNECTION_STATE_NONE);
	BIND_ENUM_CONSTANT(CONNECTION_STATE_CONNECTING);
	BIND_ENUM_CONSTANT(CONNECTION_STATE_FINDING_ROUTE);
	BIND_ENUM_CONSTANT(CONNECTION_STATE_CONNECTED);
	BIND_ENUM_CONSTANT(CONNECTION_STATE_CLOSED_BY_PEER);
	BIND_ENUM_CONSTANT(CONNECTION_STATE_PROBLEM_DETECTED_LOCALLY);
	BIND_ENUM_CONSTANT(CONNECTION_STATE_FIN_WAIT);
	BIND_ENUM_CONSTANT(CONNECTION_STATE_LINGER);
	BIND_ENUM_CONSTANT(CONNECTION_STATE_DEAD);
	BIND_ENUM_CONSTANT(CONNECTION_STATE_FORCE_32BIT);

	// NetworkingFakeIPType Enums
	BIND_ENUM_CONSTANT(FAKE_IP_TYPE_INVALID);
	BIND_ENUM_CONSTANT(FAKE_IP_TYPE_NOT_FAKE);
	BIND_ENUM_CONSTANT(FAKE_IP_TYPE_GLOBAL_IPV4);
	BIND_ENUM_CONSTANT(FAKE_IP_TYPE_LOCAL_IPV4);
	BIND_ENUM_CONSTANT(FAKE_IP_TYPE_FORCE32BIT);

	// NetworkingGetConfigValueResult Enums
	BIND_ENUM_CONSTANT(NETWORKING_GET_CONFIG_VALUE_BAD_VALUE);
	BIND_ENUM_CONSTANT(NETWORKING_GET_CONFIG_VALUE_BAD_SCOPE_OBJ);
	BIND_ENUM_CONSTANT(NETWORKING_GET_CONFIG_VALUE_BUFFER_TOO_SMALL);
	BIND_ENUM_CONSTANT(NETWORKING_GET_CONFIG_VALUE_OK);
	BIND_ENUM_CONSTANT(NETWORKING_GET_CONFIG_VALUE_OK_INHERITED);
	BIND_ENUM_CONSTANT(NETWORKING_GET_CONFIG_VALUE_FORCE_32BIT);

	// NetworkingIdentityType Enums
	BIND_ENUM_CONSTANT(IDENTITY_TYPE_INVALID);
	BIND_ENUM_CONSTANT(IDENTITY_TYPE_STEAMID);
	BIND_ENUM_CONSTANT(IDENTITY_TYPE_IP_ADDRESS);
	BIND_ENUM_CONSTANT(IDENTITY_TYPE_GENERIC_STRING);
	BIND_ENUM_CONSTANT(IDENTITY_TYPE_GENERIC_BYTES);
	BIND_ENUM_CONSTANT(IDENTITY_TYPE_UNKNOWN_TYPE);
	BIND_ENUM_CONSTANT(IDENTITY_TYPE_XBOX_PAIRWISE);
	BIND_ENUM_CONSTANT(IDENTITY_TYPE_SONY_PSN);
	BIND_ENUM_CONSTANT(IDENTITY_TYPE_GOOGLE_STADIA);
	//	BIND_ENUM_CONSTANT(IDENTITY_TYPE_NINTENDO);
	//	BIND_ENUM_CONSTANT(IDENTITY_TYPE_EPIC_GS);
	//	BIND_ENUM_CONSTANT(IDENTITY_TYPE_WEGAME);
	BIND_ENUM_CONSTANT(IDENTITY_TYPE_FORCE_32BIT);

	// NetworkingSocketsDebugOutputType Enums
	BIND_ENUM_CONSTANT(NETWORKING_SOCKET_DEBUG_OUTPUT_TYPE_NONE);
	BIND_ENUM_CONSTANT(NETWORKING_SOCKET_DEBUG_OUTPUT_TYPE_BUG);
	BIND_ENUM_CONSTANT(NETWORKING_SOCKET_DEBUG_OUTPUT_TYPE_ERROR);
	BIND_ENUM_CONSTANT(NETWORKING_SOCKET_DEBUG_OUTPUT_TYPE_IMPORTANT);
	BIND_ENUM_CONSTANT(NETWORKING_SOCKET_DEBUG_OUTPUT_TYPE_WARNING);
	BIND_ENUM_CONSTANT(NETWORKING_SOCKET_DEBUG_OUTPUT_TYPE_MSG);
	BIND_ENUM_CONSTANT(NETWORKING_SOCKET_DEBUG_OUTPUT_TYPE_VERBOSE);
	BIND_ENUM_CONSTANT(NETWORKING_SOCKET_DEBUG_OUTPUT_TYPE_DEBUG);
	BIND_ENUM_CONSTANT(NETWORKING_SOCKET_DEBUG_OUTPUT_TYPE_EVERYTHING);
	BIND_ENUM_CONSTANT(NETWORKING_SOCKET_DEBUG_OUTPUT_TYPE_FORCE_32BIT);

	// NotificationPosition Enums
	BIND_ENUM_CONSTANT(POSITION_INVALID);
	BIND_ENUM_CONSTANT(POSITION_TOP_LEFT);
	BIND_ENUM_CONSTANT(POSITION_TOP_RIGHT);
	BIND_ENUM_CONSTANT(POSITION_BOTTOM_LEFT);
	BIND_ENUM_CONSTANT(POSITION_BOTTOM_RIGHT);

	// P2PSend Enums
	BIND_ENUM_CONSTANT(P2P_SEND_UNRELIABLE);
	BIND_ENUM_CONSTANT(P2P_SEND_UNRELIABLE_NO_DELAY);
	BIND_ENUM_CONSTANT(P2P_SEND_RELIABLE);
	BIND_ENUM_CONSTANT(P2P_SEND_RELIABLE_WITH_BUFFERING);

	// P2PSessionError Enums
	BIND_ENUM_CONSTANT(P2P_SESSION_ERROR_NONE);
	BIND_ENUM_CONSTANT(P2P_SESSION_ERROR_NOT_RUNNING_APP);
	BIND_ENUM_CONSTANT(P2P_SESSION_ERROR_NO_RIGHTS_TO_APP);
	BIND_ENUM_CONSTANT(P2P_SESSION_ERROR_DESTINATION_NOT_LOGGED_ON);
	BIND_ENUM_CONSTANT(P2P_SESSION_ERROR_TIMEOUT);
	BIND_ENUM_CONSTANT(P2P_SESSION_ERROR_MAX);

	// PartyBeaconLocationData Enums
	BIND_ENUM_CONSTANT(STEAM_PARTY_BEACON_LOCATION_DATA);
	BIND_ENUM_CONSTANT(STEAM_PARTY_BEACON_LOCATION_DATA_NAME);
	BIND_ENUM_CONSTANT(STEAM_PARTY_BEACON_LOCATION_DATA_URL_SMALL);
	BIND_ENUM_CONSTANT(STEAM_PARTY_BEACON_LOCATION_DATA_URL_MEDIUM);
	BIND_ENUM_CONSTANT(STEAM_PARTY_BEACON_LOCATION_DATA_URL_LARGE);

	// PartyBeaconLocationType Enums
	BIND_ENUM_CONSTANT(STEAM_PARTY_BEACON_LOCATIONTYPE_INVALID);
	BIND_ENUM_CONSTANT(STEAM_PARTY_BEACON_LOCATIONTYPE_CHAT_GROUP);
	BIND_ENUM_CONSTANT(STEAM_PARTY_BEACON_LOCATION_TYPE_MAX);

	// PersonaChange Enums
	BIND_BITFIELD_FLAG(PERSONA_CHANGE_NAME);
	BIND_BITFIELD_FLAG(PERSONA_CHANGE_STATUS);
	BIND_BITFIELD_FLAG(PERSONA_CHANGE_COME_ONLINE);
	BIND_BITFIELD_FLAG(PERSONA_CHANGE_GONE_OFFLINE);
	BIND_BITFIELD_FLAG(PERSONA_CHANGE_GAME_PLAYED);
	BIND_BITFIELD_FLAG(PERSONA_CHANGE_GAME_SERVER);
	BIND_BITFIELD_FLAG(PERSONA_CHANGE_AVATAR);
	BIND_BITFIELD_FLAG(PERSONA_CHANGE_JOINED_SOURCE);
	BIND_BITFIELD_FLAG(PERSONA_CHANGE_LEFT_SOURCE);
	BIND_BITFIELD_FLAG(PERSONA_CHANGE_RELATIONSHIP_CHANGED);
	BIND_BITFIELD_FLAG(PERSONA_CHANGE_NAME_FIRST_SET);
	BIND_BITFIELD_FLAG(PERSONA_CHANGE_FACEBOOK_INFO);
	BIND_BITFIELD_FLAG(PERSONA_CHANGE_NICKNAME);
	BIND_BITFIELD_FLAG(PERSONA_CHANGE_STEAM_LEVEL);
	BIND_BITFIELD_FLAG(PERSONA_CHANGE_RICH_PRESENCE);

	// PersonaState Enums
	BIND_ENUM_CONSTANT(PERSONA_STATE_OFFLINE);
	BIND_ENUM_CONSTANT(PERSONA_STATE_ONLINE);
	BIND_ENUM_CONSTANT(PERSONA_STATE_BUSY);
	BIND_ENUM_CONSTANT(PERSONA_STATE_AWAY);
	BIND_ENUM_CONSTANT(PERSONA_STATE_SNOOZE);
	BIND_ENUM_CONSTANT(PERSONA_STATE_LOOKING_TO_TRADE);
	BIND_ENUM_CONSTANT(PERSONA_STATE_LOOKING_TO_PLAY);
	BIND_ENUM_CONSTANT(PERSONA_STATE_INVISIBLE);
	BIND_ENUM_CONSTANT(PERSONA_STATE_MAX);

	// PlayerResult Enums
	BIND_ENUM_CONSTANT(PLAYER_RESULT_FAILED_TO_CONNECT);
	BIND_ENUM_CONSTANT(PLAYER_RESULT_ABANDONED);
	BIND_ENUM_CONSTANT(PLAYER_RESULT_KICKED);
	BIND_ENUM_CONSTANT(PLAYER_RESULT_INCOMPLETE);
	BIND_ENUM_CONSTANT(PLAYER_RESULT_COMPLETED);

	// RemoteStoragePlatform Enums
	BIND_BITFIELD_FLAG(REMOTE_STORAGE_PLATFORM_NONE);
	BIND_BITFIELD_FLAG(REMOTE_STORAGE_PLATFORM_WINDOWS);
	BIND_BITFIELD_FLAG(REMOTE_STORAGE_PLATFORM_OSX);
	BIND_BITFIELD_FLAG(REMOTE_STORAGE_PLATFORM_PS3);
	BIND_BITFIELD_FLAG(REMOTE_STORAGE_PLATFORM_LINUX);
	BIND_BITFIELD_FLAG(REMOTE_STORAGE_PLATFORM_SWITCH);
	BIND_BITFIELD_FLAG(REMOTE_STORAGE_PLATFORM_ANDROID);
	BIND_BITFIELD_FLAG(REMOTE_STORAGE_PLATFORM_IOS);
	BIND_BITFIELD_FLAG(REMOTE_STORAGE_PLATFORM_ALL);

	// RemoteStoragePublishedFileVisibility Enums
	BIND_ENUM_CONSTANT(REMOTE_STORAGE_PUBLISHED_VISIBILITY_PUBLIC);
	BIND_ENUM_CONSTANT(REMOTE_STORAGE_PUBLISHED_VISIBILITY_FRIENDS_ONLY);
	BIND_ENUM_CONSTANT(REMOTE_STORAGE_PUBLISHED_VISIBILITY_PRIVATE);
	BIND_ENUM_CONSTANT(REMOTE_STORAGE_PUBLISHED_VISIBILITY_UNLISTED);

	// Result Enums
	BIND_ENUM_CONSTANT(RESULT_NONE);
	BIND_ENUM_CONSTANT(RESULT_OK);
	BIND_ENUM_CONSTANT(RESULT_FAIL);
	BIND_ENUM_CONSTANT(RESULT_NO_CONNECTION);
	BIND_ENUM_CONSTANT(RESULT_INVALID_PASSWORD);
	BIND_ENUM_CONSTANT(RESULT_LOGGED_IN_ELSEWHERE);
	BIND_ENUM_CONSTANT(RESULT_INVALID_PROTOCOL_VER);
	BIND_ENUM_CONSTANT(RESULT_INVALID_PARAM);
	BIND_ENUM_CONSTANT(RESULT_FILE_NOT_FOUND);
	BIND_ENUM_CONSTANT(RESULT_BUSY);
	BIND_ENUM_CONSTANT(RESULT_INVALID_STATE);
	BIND_ENUM_CONSTANT(RESULT_INVALID_NAME);
	BIND_ENUM_CONSTANT(RESULT_INVALID_EMAIL);
	BIND_ENUM_CONSTANT(RESULT_DUPLICATE_NAME);
	BIND_ENUM_CONSTANT(RESULT_ACCESS_DENIED);
	BIND_ENUM_CONSTANT(RESULT_TIMEOUT);
	BIND_ENUM_CONSTANT(RESULT_BANNED);
	BIND_ENUM_CONSTANT(RESULT_ACCOUNT_NOT_FOUND);
	BIND_ENUM_CONSTANT(RESULT_INVALID_STEAMID);
	BIND_ENUM_CONSTANT(RESULT_SERVICE_UNAVAILABLE);
	BIND_ENUM_CONSTANT(RESULT_NOT_LOGGED_ON);
	BIND_ENUM_CONSTANT(RESULT_PENDING);
	BIND_ENUM_CONSTANT(RESULT_ENCRYPTION_FAILURE);
	BIND_ENUM_CONSTANT(RESULT_INSUFFICIENT_PRIVILEGE);
	BIND_ENUM_CONSTANT(RESULT_LIMIT_EXCEEDED);
	BIND_ENUM_CONSTANT(RESULT_REVOKED);
	BIND_ENUM_CONSTANT(RESULT_EXPIRED);
	BIND_ENUM_CONSTANT(RESULT_ALREADY_REDEEMED);
	BIND_ENUM_CONSTANT(RESULT_DUPLICATE_REQUEST);
	BIND_ENUM_CONSTANT(RESULT_ALREADY_OWNED);
	BIND_ENUM_CONSTANT(RESULT_IP_NOT_FOUND);
	BIND_ENUM_CONSTANT(RESULT_PERSIST_FAILED);
	BIND_ENUM_CONSTANT(RESULT_LOCKING_FAILED);
	BIND_ENUM_CONSTANT(RESULT_LOG_ON_SESSION_REPLACED);
	BIND_ENUM_CONSTANT(RESULT_CONNECT_FAILED);
	BIND_ENUM_CONSTANT(RESULT_HANDSHAKE_FAILED);
	BIND_ENUM_CONSTANT(RESULT_IO_FAILURE);
	BIND_ENUM_CONSTANT(RESULT_REMOTE_DISCONNECT);
	BIND_ENUM_CONSTANT(RESULT_SHOPPING_CART_NOT_FOUND);
	BIND_ENUM_CONSTANT(RESULT_BLOCKED);
	BIND_ENUM_CONSTANT(RESULT_IGNORED);
	BIND_ENUM_CONSTANT(RESULT_NO_MATCH);
	BIND_ENUM_CONSTANT(RESULT_ACCOUNT_DISABLED);
	BIND_ENUM_CONSTANT(RESULT_SERVICE_READ_ONLY);
	BIND_ENUM_CONSTANT(RESULT_ACCOUNT_NOT_FEATURED);
	BIND_ENUM_CONSTANT(RESULT_ADMINISTRATO_ROK);
	BIND_ENUM_CONSTANT(RESULT_CONTENT_VERSION);
	BIND_ENUM_CONSTANT(RESULT_TRY_ANOTHER_CM);
	BIND_ENUM_CONSTANT(RESULT_PASSWORD_REQUIRED_TO_KICK_SESSION);
	BIND_ENUM_CONSTANT(RESULT_ALREADY_LOGGED_IN_ELSEWHERE);
	BIND_ENUM_CONSTANT(RESULT_SUSPENDED);
	BIND_ENUM_CONSTANT(RESULT_CANCELLED);
	BIND_ENUM_CONSTANT(RESULT_DATA_CORRUPTION);
	BIND_ENUM_CONSTANT(RESULT_DISK_FULL);
	BIND_ENUM_CONSTANT(RESULT_REMOTE_CALL_FAILED);
	BIND_ENUM_CONSTANT(RESULT_PASSWORD_UNSET);
	BIND_ENUM_CONSTANT(RESULT_EXTERNAL_ACCOUNT_UNLINKED);
	BIND_ENUM_CONSTANT(RESULT_PSN_TICKET_INVALID);
	BIND_ENUM_CONSTANT(RESULT_EXTERNAL_ACCOUNT_ALREADY_LINKED);
	BIND_ENUM_CONSTANT(RESULT_REMOTE_FILE_CONFLICT);
	BIND_ENUM_CONSTANT(RESULT_ILLEGAL_PASSWORD);
	BIND_ENUM_CONSTANT(RESULT_SAME_AS_PREVIOUS_VALUE);
	BIND_ENUM_CONSTANT(RESULT_ACCOUNT_LOG_ON_DENIED);
	BIND_ENUM_CONSTANT(RESULT_CANNOT_USE_OLD_PASSWORD);
	BIND_ENUM_CONSTANT(RESULT_INVALID_LOG_IN_AUTH_CODE);
	BIND_ENUM_CONSTANT(RESULT_ACCOUNT_LOG_ON_DENIED_NO_MAIL);
	BIND_ENUM_CONSTANT(RESULT_HARDWARE_NOT_CAPABLE_OF_IPT);
	BIND_ENUM_CONSTANT(RESULT_IPT_INIT_ERROR);
	BIND_ENUM_CONSTANT(RESULT_PARENTAL_CONTROL_RESTRICTED);
	BIND_ENUM_CONSTANT(RESULT_FACEBOOK_QUERY_ERROR);
	BIND_ENUM_CONSTANT(RESULT_EXPIRED_LOGIN_AUTH_CODE);
	BIND_ENUM_CONSTANT(RESULT_IP_LOGIN_RESTRICTION_FAILED);
	BIND_ENUM_CONSTANT(RESULT_ACCOUNT_LOCKED_DOWN);
	BIND_ENUM_CONSTANT(RESULT_ACCOUNT_LOG_ON_DENIED_VERIFIED_EMAIL_REQUIRED);
	BIND_ENUM_CONSTANT(RESULT_NO_MATCHING_URL);
	BIND_ENUM_CONSTANT(RESULT_BAD_RESPONSE);
	BIND_ENUM_CONSTANT(RESULT_REQUIRE_PASSWORD_REENTRY);
	BIND_ENUM_CONSTANT(RESULT_VALUE_OUT_OF_RANGE);
	BIND_ENUM_CONSTANT(RESULT_UNEXPECTED_ERROR);
	BIND_ENUM_CONSTANT(RESULT_DISABLED);
	BIND_ENUM_CONSTANT(RESULT_INVALID_CEG_SUBMISSION);
	BIND_ENUM_CONSTANT(RESULT_RESTRICTED_DEVICE);
	BIND_ENUM_CONSTANT(RESULT_REGION_LOCKED);
	BIND_ENUM_CONSTANT(RESULT_RATE_LIMIT_EXCEEDED);
	BIND_ENUM_CONSTANT(RESULT_ACCOUNT_LOGIN_DENIED_NEED_TWO_FACTOR);
	BIND_ENUM_CONSTANT(RESULT_ITEM_DELETED);
	BIND_ENUM_CONSTANT(RESULT_ACCOUNT_LOGIN_DENIED_THROTTLE);
	BIND_ENUM_CONSTANT(RESULT_TWO_FACTOR_CODE_MISMATCH);
	BIND_ENUM_CONSTANT(RESULT_TWO_FACTOR_ACTIVATION_CODE_MISMATCH);
	BIND_ENUM_CONSTANT(RESULT_ACCOUNT_ASSOCIATED_TO_MULTIPLE_PARTNERS);
	BIND_ENUM_CONSTANT(RESULT_NOT_MODIFIED);
	BIND_ENUM_CONSTANT(RESULT_NO_MOBILE_DEVICE);
	BIND_ENUM_CONSTANT(RESULT_TIME_NOT_SYNCED);
	BIND_ENUM_CONSTANT(RESULT_SMS_CODE_FAILED);
	BIND_ENUM_CONSTANT(RESULT_ACCOUNT_LIMIT_EXCEEDED);
	BIND_ENUM_CONSTANT(RESULT_ACCOUNT_ACTIVITY_LIMIT_EXCEEDED);
	BIND_ENUM_CONSTANT(RESULT_PHONE_ACTIVITY_LIMIT_EXCEEDED);
	BIND_ENUM_CONSTANT(RESULT_REFUND_TO_WALLET);
	BIND_ENUM_CONSTANT(RESULT_EMAIL_SEND_FAILURE);
	BIND_ENUM_CONSTANT(RESULT_NOT_SETTLED);
	BIND_ENUM_CONSTANT(RESULT_NEED_CAPTCHA);
	BIND_ENUM_CONSTANT(RESULT_GSLT_DENIED);
	BIND_ENUM_CONSTANT(RESULT_GS_OWNER_DENIED);
	BIND_ENUM_CONSTANT(RESULT_INVALID_ITEM_TYPE);
	BIND_ENUM_CONSTANT(RESULT_IP_BANNED);
	BIND_ENUM_CONSTANT(RESULT_GSLT_EXPIRED);
	BIND_ENUM_CONSTANT(RESULT_INSUFFICIENT_FUNDS);
	BIND_ENUM_CONSTANT(RESULT_TOO_MANY_PENDING);
	BIND_ENUM_CONSTANT(RESULT_NO_SITE_LICENSES_FOUND);
	BIND_ENUM_CONSTANT(RESULT_WG_NETWORK_SEND_EXCEEDED);
	BIND_ENUM_CONSTANT(RESULT_ACCOUNT_NOT_FRIENDS);
	BIND_ENUM_CONSTANT(RESULT_LIMITED_USER_ACCOUNT);
	BIND_ENUM_CONSTANT(RESULT_CANT_REMOVE_ITEM);
	BIND_ENUM_CONSTANT(RESULT_ACCOUNT_DELETED);
	BIND_ENUM_CONSTANT(RESULT_EXISTING_USER_CANCELLED_LICENSE);
	BIND_ENUM_CONSTANT(RESULT_COMMUNITY_COOLDOWN);
	BIND_ENUM_CONSTANT(RESULT_NO_LAUNCHER_SPECIFIED);
	BIND_ENUM_CONSTANT(RESULT_MUST_AGREE_TO_SSA);
	BIND_ENUM_CONSTANT(RESULT_LAUNCHER_MIGRATED);
	BIND_ENUM_CONSTANT(RESULT_STEAM_REALM_MISMATCH);
	BIND_ENUM_CONSTANT(RESULT_INVALID_SIGNATURE);
	BIND_ENUM_CONSTANT(RESULT_PARSE_FAILURE);
	BIND_ENUM_CONSTANT(RESULT_NO_VERIFIED_PHONE);
	BIND_ENUM_CONSTANT(RESULT_INSUFFICIENT_BATTERY);
	BIND_ENUM_CONSTANT(RESULT_CHARGER_REQUIRED);
	BIND_ENUM_CONSTANT(RESULT_CACHED_CREDENTIAL_INVALID);
	BIND_ENUM_CONSTANT(RESULT_PHONE_NUMBER_IS_VOIP);

	// SocketConnectionType Enums
	BIND_ENUM_CONSTANT(NET_SOCKET_CONNECTION_TYPE_NOT_CONNECTED);
	BIND_ENUM_CONSTANT(NET_SOCKET_CONNECTION_TYPE_UDP);
	BIND_ENUM_CONSTANT(NET_SOCKET_CONNECTION_TYPE_UDP_RELAY);

	// SocketState Enums
	BIND_ENUM_CONSTANT(NET_SOCKET_STATE_INVALID);
	BIND_ENUM_CONSTANT(NET_SOCKET_STATE_CONNECTED);
	BIND_ENUM_CONSTANT(NET_SOCKET_STATE_INITIATED);
	BIND_ENUM_CONSTANT(NET_SOCKET_STATE_LOCAL_CANDIDATE_FOUND);
	BIND_ENUM_CONSTANT(NET_SOCKET_STATE_RECEIVED_REMOTE_CANDIDATES);
	BIND_ENUM_CONSTANT(NET_SOCKET_STATE_CHALLENGE_HANDSHAKE);
	BIND_ENUM_CONSTANT(NET_SOCKET_STATE_DISCONNECTING);
	BIND_ENUM_CONSTANT(NET_SOCKET_STATE_LOCAL_DISCONNECT);
	BIND_ENUM_CONSTANT(NET_SOCKET_STATE_TIMEOUT_DURING_CONNECT);
	BIND_ENUM_CONSTANT(NET_SOCKET_STATE_REMOTE_END_DISCONNECTED);
	BIND_ENUM_CONSTANT(NET_SOCKET_STATE_BROKEN);

	// SteamAPIInitResult Enums
	BIND_ENUM_CONSTANT(STEAM_API_INIT_RESULT_OK);
	BIND_ENUM_CONSTANT(STEAM_API_INIT_RESULT_FAILED_GENERIC);
	BIND_ENUM_CONSTANT(STEAM_API_INIT_RESULT_NO_STEAM_CLIENT);
	BIND_ENUM_CONSTANT(STEAM_API_INIT_RESULT_VERSION_MISMATCH);

	// TextFilteringContext Enums
	BIND_ENUM_CONSTANT(TEXT_FILTERING_CONTEXT_UNKNOWN);
	BIND_ENUM_CONSTANT(TEXT_FILTERING_CONTEXT_GAME_CONTENT);
	BIND_ENUM_CONSTANT(TEXT_FILTERING_CONTEXT_CHAT);
	BIND_ENUM_CONSTANT(TEXT_FILTERING_CONTEXT_NAME);

	// Universe Enums
	BIND_ENUM_CONSTANT(UNIVERSE_INVALID);
	BIND_ENUM_CONSTANT(UNIVERSE_PUBLIC);
	BIND_ENUM_CONSTANT(UNIVERSE_BETA);
	BIND_ENUM_CONSTANT(UNIVERSE_INTERNAL);
	BIND_ENUM_CONSTANT(UNIVERSE_DEV);
	BIND_ENUM_CONSTANT(UNIVERSE_MAX);

	// UGCContentDescriptorID Enums
	BIND_ENUM_CONSTANT(UGCCONTENTDESCRIPTOR_NUDITY_OR_SEXUAL_CONTENT);
	BIND_ENUM_CONSTANT(UGCCONTENTDESCRIPTOR_FREQUENT_VIOLENCE_OR_GORE);
	BIND_ENUM_CONSTANT(UGCCONTENTDESCRIPTOR_ADULT_ONLY_SEXUAL_CONTENT);
	BIND_ENUM_CONSTANT(UGCCONTENTDESCRIPTOR_GRATUITOUS_SEXUAL_CONTENT);
	BIND_ENUM_CONSTANT(UGCCONTENTDESCRIPTOR_ANY_MATURE_CONTENT);

	// UGCMatchingUGCType Enums
	BIND_ENUM_CONSTANT(UGC_MATCHINGUGCTYPE_ITEMS);
	BIND_ENUM_CONSTANT(UGC_MATCHING_UGC_TYPE_ITEMS_MTX);
	BIND_ENUM_CONSTANT(UGC_MATCHING_UGC_TYPE_ITEMS_READY_TO_USE);
	BIND_ENUM_CONSTANT(UGC_MATCHING_UGC_TYPE_COLLECTIONS);
	BIND_ENUM_CONSTANT(UGC_MATCHING_UGC_TYPE_ARTWORK);
	BIND_ENUM_CONSTANT(UGC_MATCHING_UGC_TYPE_VIDEOS);
	BIND_ENUM_CONSTANT(UGC_MATCHING_UGC_TYPE_SCREENSHOTS);
	BIND_ENUM_CONSTANT(UGC_MATCHING_UGC_TYPE_ALL_GUIDES);
	BIND_ENUM_CONSTANT(UGC_MATCHING_UGC_TYPE_WEB_GUIDES);
	BIND_ENUM_CONSTANT(UGC_MATCHING_UGC_TYPE_INTEGRATED_GUIDES);
	BIND_ENUM_CONSTANT(UGC_MATCHING_UGC_TYPE_USABLE_IN_GAME);
	BIND_ENUM_CONSTANT(UGC_MATCHING_UGC_TYPE_CONTROLLER_BINDINGS);
	BIND_ENUM_CONSTANT(UGC_MATCHING_UGC_TYPE_GAME_MANAGED_ITEMS);
	BIND_ENUM_CONSTANT(UGC_MATCHING_UGC_TYPE_ALL);

	// UGCQuery Enums
	BIND_ENUM_CONSTANT(UGC_QUERY_RANKED_BY_VOTE);
	BIND_ENUM_CONSTANT(UGC_QUERY_RANKED_BY_PUBLICATION_DATE);
	BIND_ENUM_CONSTANT(UGC_QUERY_ACCEPTED_FOR_GAME_RANKED_BY_ACCEPTANCE_DATE);
	BIND_ENUM_CONSTANT(UGC_QUERY_RANKED_BY_TREND);
	BIND_ENUM_CONSTANT(UGC_QUERY_FAVORITED_BY_FRIENDS_RANKED_BY_PUBLICATION_DATE);
	BIND_ENUM_CONSTANT(UGC_QUERY_CREATED_BY_FRIENDS_RANKED_BY_PUBLICATION_DATE);
	BIND_ENUM_CONSTANT(UGC_QUERY_RANKED_BY_NUM_TIMES_REPORTED);
	BIND_ENUM_CONSTANT(UGC_QUERY_CREATED_BY_FOLLOWED_USERS_RANKED_BY_PUBLICATION_DATE);
	BIND_ENUM_CONSTANT(UGC_QUERY_NOT_YET_RATED);
	BIND_ENUM_CONSTANT(UGC_QUERY_RANKED_BY_TOTAL_VOTES_ASC);
	BIND_ENUM_CONSTANT(UGC_QUERY_RANKED_BY_VOTES_UP);
	BIND_ENUM_CONSTANT(UGC_QUERY_RANKED_BY_TEXT_SEARCH);
	BIND_ENUM_CONSTANT(UGC_QUERY_RANKED_BY_TOTAL_UNIQUE_SUBSCRIPTIONS);
	BIND_ENUM_CONSTANT(UGC_QUERY_RANKED_BY_PLAYTIME_TREND);
	BIND_ENUM_CONSTANT(UGC_QUERY_RANKED_BY_TOTAL_PLAYTIME);
	BIND_ENUM_CONSTANT(UGC_QUERY_RANKED_BY_AVERAGE_PLAYTIME_TREND);
	BIND_ENUM_CONSTANT(UGC_QUERY_RANKED_BY_LIFETIME_AVERAGE_PLAYTIME);
	BIND_ENUM_CONSTANT(UGC_QUERY_RANKED_BY_PLAYTIME_SESSIONS_TREND);
	BIND_ENUM_CONSTANT(UGC_QUERY_RANKED_BY_LIFETIME_PLAYTIME_SESSIONS);
	BIND_ENUM_CONSTANT(UGC_QUERY_RANKED_BY_LAST_UPDATED_DATE);

	// UGCReadAction Enums
	BIND_ENUM_CONSTANT(UGC_READ_CONTINUE_READING_UNTIL_FINISHED);
	BIND_ENUM_CONSTANT(UGC_READ_CONTINUE_READING);
	BIND_ENUM_CONSTANT(UGC_READ_CLOSE);

	// UserHasLicenseForAppResult Enums
	BIND_ENUM_CONSTANT(USER_HAS_LICENSE_RESULT_HAS_LICENSE);
	BIND_ENUM_CONSTANT(USER_HAS_LICENSE_RESULT_DOES_NOT_HAVE_LICENSE);
	BIND_ENUM_CONSTANT(USER_HAS_LICENSE_RESULT_NO_AUTH);

	// UserRestriction Enums
	BIND_BITFIELD_FLAG(USER_RESTRICTION_NONE);
	BIND_BITFIELD_FLAG(USER_RESTRICTION_UNKNOWN);
	BIND_BITFIELD_FLAG(USER_RESTRICTION_ANY_CHAT);
	BIND_BITFIELD_FLAG(USER_RESTRICTION_VOICE_CHAT);
	BIND_BITFIELD_FLAG(USER_RESTRICTION_GROUP_CHAT);
	BIND_BITFIELD_FLAG(USER_RESTRICTION_RATING);
	BIND_BITFIELD_FLAG(USER_RESTRICTION_GAME_INVITES);
	BIND_BITFIELD_FLAG(USER_RESTRICTION_TRADING);

	// UserUGCList Enums
	BIND_ENUM_CONSTANT(USER_UGC_LIST_PUBLISHED);
	BIND_ENUM_CONSTANT(USER_UGC_LIST_VOTED_ON);
	BIND_ENUM_CONSTANT(USER_UGC_LIST_VOTED_UP);
	BIND_ENUM_CONSTANT(USER_UGC_LIST_VOTED_DOWN);
	BIND_ENUM_CONSTANT(USER_UGC_LIST_WILL_VOTE_LATER);
	BIND_ENUM_CONSTANT(USER_UGC_LIST_FAVORITED);
	BIND_ENUM_CONSTANT(USER_UGC_LIST_SUBSCRIBED);
	BIND_ENUM_CONSTANT(USER_UGC_LIST_USED_OR_PLAYED);
	BIND_ENUM_CONSTANT(USER_UGC_LIST_FOLLOWED);

	// UserUGCListSortOrder Enums
	BIND_ENUM_CONSTANT(USERUGCLISTSORTORDER_CREATIONORDERDESC);
	BIND_ENUM_CONSTANT(USERUGCLISTSORTORDER_CREATIONORDERASC);
	BIND_ENUM_CONSTANT(USERUGCLISTSORTORDER_TITLEASC);
	BIND_ENUM_CONSTANT(USERUGCLISTSORTORDER_LASTUPDATEDDESC);
	BIND_ENUM_CONSTANT(USERUGCLISTSORTORDER_SUBSCRIPTIONDATEDESC);
	BIND_ENUM_CONSTANT(USERUGCLISTSORTORDER_VOTESCOREDESC);
	BIND_ENUM_CONSTANT(SERUGCLISTSORTORDER_FORMODERATION);

	// VoiceResult Enums
	BIND_ENUM_CONSTANT(VOICE_RESULT_OK);
	BIND_ENUM_CONSTANT(VOICE_RESULT_NOT_INITIALIZED);
	BIND_ENUM_CONSTANT(VOICE_RESULT_NOT_RECORDING);
	BIND_ENUM_CONSTANT(VOICE_RESULT_NO_DATE);
	BIND_ENUM_CONSTANT(VOICE_RESULT_BUFFER_TOO_SMALL);
	BIND_ENUM_CONSTANT(VOICE_RESULT_DATA_CORRUPTED);
	BIND_ENUM_CONSTANT(VOICE_RESULT_RESTRICTED);
	BIND_ENUM_CONSTANT(VOICE_RESULT_UNSUPPORTED_CODEC);
	BIND_ENUM_CONSTANT(VOICE_RESULT_RECEIVER_OUT_OF_DATE);
	BIND_ENUM_CONSTANT(VOICE_RESULT_RECEIVER_DID_NOT_ANSWER);

	// WorkshopEnumerationType Enums
	BIND_ENUM_CONSTANT(WORKSHOP_ENUMERATION_TYPE_RANKED_BY_VOTE);
	BIND_ENUM_CONSTANT(WORKSHOP_ENUMERATION_TYPE_RECENT);
	BIND_ENUM_CONSTANT(WORKSHOP_ENUMERATION_TYPE_TRENDING);
	BIND_ENUM_CONSTANT(WORKSHOP_ENUMERATION_TYPE_FAVORITES_OF_FRIENDS);
	BIND_ENUM_CONSTANT(WORKSHOP_ENUMERATION_TYPE_VOTED_BY_FRIENDS);
	BIND_ENUM_CONSTANT(WORKSHOP_ENUMERATION_TYPE_CONTENT_BY_FRIENDS);
	BIND_ENUM_CONSTANT(WORKSHOP_ENUMERATION_TYPE_RECENT_FROM_FOLLOWED_USERS);

	// WorkshopFileAction Enums
	BIND_ENUM_CONSTANT(WORKSHOP_FILE_ACTION_PLAYED);
	BIND_ENUM_CONSTANT(WORKSHOP_FILE_ACTION_COMPLETED);

	// WorkshopFileType Enums
	BIND_ENUM_CONSTANT(WORKSHOP_FILE_TYPE_FIRST);
	BIND_ENUM_CONSTANT(WORKSHOP_FILE_TYPE_COMMUNITY);
	BIND_ENUM_CONSTANT(WORKSHOP_FILE_TYPE_MICROTRANSACTION);
	BIND_ENUM_CONSTANT(WORKSHOP_FILE_TYPE_COLLECTION);
	BIND_ENUM_CONSTANT(WORKSHOP_FILE_TYPE_ART);
	BIND_ENUM_CONSTANT(wORKSHOP_FILE_TYPE_VIDEO);
	BIND_ENUM_CONSTANT(WORKSHOP_FILE_TYPE_SCREENSHOT);
	BIND_ENUM_CONSTANT(WORKSHOP_FILE_TYPE_GAME);
	BIND_ENUM_CONSTANT(WORKSHOP_FILE_TYPE_SOFTWARE);
	BIND_ENUM_CONSTANT(WORKSHOP_FILE_TYPE_CONCEPT);
	BIND_ENUM_CONSTANT(WORKSHOP_FILE_TYPE_WEB_GUIDE);
	BIND_ENUM_CONSTANT(WORKSHOP_FILE_TYPE_INTEGRATED_GUIDE);
	BIND_ENUM_CONSTANT(WORKSHOP_FILE_TYPE_MERCH);
	BIND_ENUM_CONSTANT(WORKSHOP_FILE_TYPE_CONTROLLER_BINDING);
	BIND_ENUM_CONSTANT(wORKSHOP_FILE_TYPE_STEAMWORKS_ACCESS_INVITE);
	BIND_ENUM_CONSTANT(WORKSHOP_FILE_TYPE_STEAM_VIDEO);
	BIND_ENUM_CONSTANT(WORKSHOP_FILE_TYPE_GAME_MANAGED_ITEM);
	BIND_ENUM_CONSTANT(WORKSHOP_FILE_TYPE_MAX);

	// WorkshopVideoProvider Enums
	BIND_ENUM_CONSTANT(WORKSHOP_VIDEO_PROVIDER_NONE);
	BIND_ENUM_CONSTANT(WORKSHOP_VIDEO_PROVIDER_YOUTUBE);

	// WorkshopVote Enums
	BIND_ENUM_CONSTANT(WORKSHOP_VOTE_UNVOTED);
	BIND_ENUM_CONSTANT(WORKSHOP_VOTE_FOR);
	BIND_ENUM_CONSTANT(WORKSHOP_VOTE_AGAINST);
	BIND_ENUM_CONSTANT(WORKSHOP_VOTE_LATER);
}

SteamServer::~SteamServer() {
	// Store stats then shut down ///////////////
	if (is_init_success) {
		SteamUserStats()->StoreStats();
		SteamGameServer_Shutdown();
	}

	// Clear app ID and singleton variables /////
	singleton = NULL;
	current_app_id = 0;
}
