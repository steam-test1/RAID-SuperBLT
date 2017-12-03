
#include "XAudio.h"

#ifdef ENABLE_XAUDIO

#include "XAudioInternal.h"

namespace pd2hook {
	namespace xaudio {
		double world_scale = 1;

		map<string, xabuffer::XABuffer*> openBuffers;
		vector<xasource::XASource*> openSources;
	};

	using namespace xaudio;

	// XAResource
	void XAResource::Discard(bool force) {
		usecount--;

		if (force && usecount <= 0) {
			Close();
		}
	}
	void XAResource::Close() {
		if (!valid) return;
		valid = false;
		alDeleteBuffers(1, &alhandle);
	}

	// XALuaHandle
	XALuaHandle::XALuaHandle(XAResource *resource) : resource(resource) {
		resource->Employ();
	}
	ALuint XALuaHandle::Handle(lua_State *L) {
		if (!open) luaL_error(L, "Cannot use closed resource!");
		return resource->Handle();
	}
	void XALuaHandle::Close(bool force) {
		if (!open) return;

		resource->Discard(force);
		open = false;
	}

	static int lX_setup(lua_State *L) {
		try {
			XAudio::GetXAudioInstance();
		}
		catch (string msg) {
			PD2HOOK_LOG_ERROR("Exception while loading XAudio API: " + msg);
			lua_pushboolean(L, false);
			return 1;
		}
		lua_pushboolean(L, true);
		return 1;
	}

	static int lX_getworldscale(lua_State *L) {
		lua_pushnumber(L, world_scale);
		return 1;
	}

	static int lX_setworldscale(lua_State *L) {
		world_scale = lua_tonumber(L, 1);
		return 0;
	}

	XAudio::XAudio() {
		struct stat statbuf;

		dev = alcOpenDevice(NULL);
		if (!dev) {
			throw string("Cannot open OpenAL Device");
		}
		ctx = alcCreateContext(dev, NULL);
		alcMakeContextCurrent(ctx);
		if (!ctx) {
			throw string("Could not create OpenAL Context");
		}

		PD2HOOK_LOG_LOG("Loaded OpenAL XAudio API");
	}

	XAudio::~XAudio() {
		PD2HOOK_LOG_LOG("Closing OpenAL XAudio API");

		// Delete all sources
		// Do this first, otherwise buffers might not be deleted properly
		for (auto source : openSources) {
			source->Close();

			delete source;
		}

		// To remove dangling pointers
		// Should not be used again, but just to be safe
		openSources.clear();

		// Delete all buffers
		for (auto const& pair : openBuffers) {
			xabuffer::XABuffer *buff = pair.second;

			buff->Close();

			delete buff;
		}

		// Same as above
		openBuffers.clear();

		// TODO: Make sure above works properly.

		// Close the OpenAL context/device
		alcMakeContextCurrent(NULL);
		alcDestroyContext(ctx);
		alcCloseDevice(dev);
	}

	XAudio* XAudio::GetXAudioInstance() {
		static XAudio audio;
		return &audio;
	}

	void XAudio::Register(void * state) {
		lua_State *L = (lua_State*)state;

		// Buffer metatable
		luaL_Reg XABufferLib[] = {
			{ "close", xabuffer::XABuffer_close },
			{ NULL, NULL }
		};

		luaL_newmetatable(L, "XAudio.buffer");

		lua_pushstring(L, "__index");
		lua_pushvalue(L, -2);  /* pushes the metatable */
		lua_settable(L, -3);  /* metatable.__index = metatable */

		luaI_openlib(L, NULL, XABufferLib, 0);
		lua_pop(L, 1);

		// Source metatable
		luaL_Reg XASourceLib[] = {
			{ "close", xasource::XASource_close },
			{ "setbuffer", xasource::XASource_set_buffer },
			{ "play", xasource::XASource_play },
			{ "pause", xasource::XASource_pause },
			{ "stop", xasource::XASource_stop },
			{ "getstate", xasource::XASource_get_state },
			{ "setposition", xasource::XASource_set_position },
			{ "setvelocity", xasource::XASource_set_velocity },
			{ "setdirection", xasource::XASource_set_direction },
			{ NULL, NULL }
		};

		luaL_newmetatable(L, "XAudio.source");

		lua_pushstring(L, "__index");
		lua_pushvalue(L, -2);  /* pushes the metatable */
		lua_settable(L, -3);  /* metatable.__index = metatable */

		luaI_openlib(L, NULL, XASourceLib, 0);
		lua_pop(L, 1);

		// blt.xaudio table
		luaL_Reg lib[] = {
			{ "setup", lX_setup },
			{ "loadbuffer", xabuffer::lX_loadbuffer },
			{ "newsource", xasource::lX_new_source },
			{ "getworldscale", lX_getworldscale },
			{ "setworldscale", lX_setworldscale },
			{ NULL, NULL }
		};

		// Grab the BLT table
		lua_getglobal(L, "blt");

		// Make a new table and populate it with XAudio stuff
		lua_newtable(L);
		luaI_openlib(L, NULL, lib, 0);

		// Add the blt.xaudio.listener table
		xalistener::lua_register(L);
		// Put listener into xaudio
		lua_setfield(L, -2, "listener");

		// Put that table into the BLT table, calling it XAudio. This removes said table from the stack.
		lua_setfield(L, -2, "xaudio");

		// Remove the BLT table from the stack.
		lua_pop(L, 1);
	}

}

#endif // ENABLE_XAUDIO
