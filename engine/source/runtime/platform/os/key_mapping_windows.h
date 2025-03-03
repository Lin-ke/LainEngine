/**************************************************************************/
/*  key_mapping_windows.h                                                 */
/**************************************************************************/

#ifndef KEY_MAPPING_WINDOWS_H
#define KEY_MAPPING_WINDOWS_H

#include "core/input/keyboard.h"

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <winuser.h>
namespace lain{

class KeyMappingWindows {
	KeyMappingWindows() {}

public:
	static void initialize();

	static Key get_keysym(unsigned int p_code);
	static unsigned int get_scancode(Key p_keycode);
	static Key get_scansym(unsigned int p_code, bool p_extended);
	static bool is_extended_key(unsigned int p_code);
	static KeyLocation get_location(unsigned int p_code, bool p_extended);
};
}

#endif // KEY_MAPPING_WINDOWS_H
