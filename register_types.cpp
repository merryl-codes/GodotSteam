#include "register_types.h"
#include "core/object/class_db.h"
#include "core/config/engine.h"
#include "godotsteam.h"

static SteamServer* SteamPtr = NULL;

void initialize_godotsteam_module(ModuleInitializationLevel level){
	if(level == MODULE_INITIALIZATION_LEVEL_SERVERS){
		ClassDB::register_class<SteamServer>();
		SteamPtr = memnew(SteamServer);
		Engine::get_singleton()->add_singleton(Engine::Singleton("SteamServer", SteamServer::get_singleton()));
	}
}

void uninitialize_godotsteam_module(ModuleInitializationLevel level){
	if(level == MODULE_INITIALIZATION_LEVEL_SERVERS){
		memdelete(SteamPtr);
	}
}
