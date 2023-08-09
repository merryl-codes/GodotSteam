# Functions - Video

---

## getOPFSettings

!!! function "getOPFSettings( ```uint32_t``` app_id )"
    Get the OPF details for 360 video playback. 

    To retrieve the 360 OPF (open projection format) data to playback a 360 video, start by making a call to this, then the callback will indicate whether the request was successful. If it was successful, the actual OPF JSON data can be retrieved with a call to [getOPFStringForApp](/functions/video/#getopfstringforapp).

    **Returns:** void

    **Notes:** Triggers a [get_opf_settings_result callback.](/functions/video/#get_opf_settings_result)

    ---
    [:fontawesome-brands-steam: Read more in the official Steamworks SDK documentation](https://partner.steamgames.com/doc/api/ISteamVideo#GetOPFSettings){ .md-button .md-button--store target="_blank" }

## getOPFStringForApp

!!! function "getOPFStringForApp( ```uint32_t``` app_id )"
    Gets the OPF string for the specified video app ID.

    Once the callback for [get_opf_settings_result](/signals/video/#get_opf_settings_result) has been raised and the EResult indicates success, then calling this will return back the actual OPF data in a JSON format. The size of the OPF string varies, but at this time 48,000 bytes should be sufficient to contain the full string.

    **Returns:** string

    **Note:** The data returned in a successful call to [getOPFStringForApp](/functions/video/#getopfstringforapp) can only be retrieved once. If you need to retrieve it multiple times, a call to [getOPFSettings](/functions/video/#getopfsettings) will need to be made each time.

    ---
    [:fontawesome-brands-steam: Read more in the official Steamworks SDK documentation](https://partner.steamgames.com/doc/api/ISteamVideo#GetOPFStringForApp){ .md-button .md-button--store target="_blank" }

## getVideoURL

!!! function "getVideoURL( ```uint32_t``` app_id )"
    Asynchronously gets the URL suitable for streaming the video associated with the specified video app ID. 

    **Returns:** void

    **Notes:** Triggers a [get_video_result](/signals/video/#get_video_result) callback.

    ---
    [:fontawesome-brands-steam: Read more in the official Steamworks SDK documentation](https://partner.steamgames.com/doc/api/ISteamVideo#GetVideoURL){ .md-button .md-button--store target="_blank" }

## isBroadcasting

!!! function "isBroadcasting()"
    Checks if the user is currently live broadcasting and gets the number of users.

    **Returns:** dictionary

    Contains the following keys:

    * broadcasting (bool)
    * viewers (int)

    ---
    [:fontawesome-brands-steam: Read more in the official Steamworks SDK documentation](https://partner.steamgames.com/doc/api/ISteamVideo#IsBroadcasting){ .md-button .md-button--store target="_blank" }