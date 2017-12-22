#pragma once

#include "xmltweaker.h"

namespace pd2hook {
	namespace tweaker {
		typedef unsigned long long idstring;

		extern idstring *last_loaded_name, *last_loaded_ext;

		void* __cdecl tweak_pd2_xml(char* text);
		void __cdecl free_tweaked_pd2_xml(char* text);

		void note_loaded_file(idstring name, idstring ext);

		/**
		* Transforms the contents of the file
		* the return result MUST be valid immediately after the call returns,
		* but not necessaraly after that.
		*/
		const char* transform_file(const char* contents);
	};
};
