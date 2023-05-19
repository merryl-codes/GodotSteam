# Godot 4.x GDExtension Change-Log

### A history of all changes to the **gdextension** branch.

---

**Version 4.2.2 Changes**

- Added: new Input callback _input_gamepad_slot_change_
- Added: new User callback _get_ticket_for_web_api_
- Added: new User function _getAuthTicketForWebApi_
- Changed: getAuthSessionTicket argument is now optional, defaults to NULL

**Version 4.2.1 Changes**

- Added: new return values for _overlay_toggled_; this will break compatibility with this
- Added: new Input and Parental Settings enums
- Added: new UGC Content Descriptor ID enums
- Added: new UGC functions _removeContentDescriptor_, _addContentDescriptor_, and _getQueryUGCContentDescriptors_
- Added: new signal _filter_text_dictionary_changed_
- Changed: getAuthSessionTicket now uses networking identities
- Changed: gamepad_text_input_dismissed now passes back the app ID
- Changed: Steam Input max analog and digital actions values
- Removed: ERegisterActivationCodeResult due to removal in SDK

**Version 4.2 Changes**

- Changed: code so it works like a singleton / module
- Changed: brought this version up-to-speed with the 4.x module version
- Fixed: weird issues with uint8/uint16 bindings