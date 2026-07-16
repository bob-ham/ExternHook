# ExternHook injection libaray
* basically like MinHook, but external*
* going to include the mmaper, handle hijacking, and the pointer to pointer hooking*


# MAIN ENUM
```
enum class STATUS
{
	EH_OK,
	all of the other error stuff
};
```


# FUNCTION DEFS

# ExternHook_Init()
- returns a enum
- EH_OK(yes i stole these names from MinHook)
- will set up the handle hijacking, all that other stuff

# ExternHook_Map()
- returns an enum
- EH_OK
- manually maps your DLL into the process


# ExternHook_CreateHook()
- return an enum
- EH_OK
- performs the hook, on whatever function(ill use QPC as an example)

# ExternHook_CleanHook()
- returns an enum
- EH_OK
- cleans up the hook the user made, should be called a few seconds after CreateHook, or not
- there will be other functions


# other helper functions like reading/writing memory and allat