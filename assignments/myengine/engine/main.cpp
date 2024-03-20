#include "sokol/sokol_app.h"
#include "sokol/sokol_gfx.h"
#include "sokol/sokol_glue.h"
#include "sokol/sokol_glue.h"

namespace engine {


	void init()
	{

	}

	void frame()
	{

	}

	void event(const sapp_event* event)
	{

	}

	void cleanup()
	{

	}
}

//sapp_desc sokol_main(int agrc, char* argv[])
//{
//	sapp_desc desc;
//	desc.init_cb = engine::init;
//	desc.frame_cb = engine::frame;
//	desc.event_cb = engine::event;
//	desc.cleanup_cb = ;
//	desc.width = 800;
//	desc.height = 600;
//
//	sapp_run(&desc);
//	return desc;
//}

sapp_desc sokol_main(int argc, char* argv[]) {
	sapp_desc desc;
	desc.init_cb = engine::init;
	desc.frame_cb = engine::frame;
	desc.cleanup_cb = engine::cleanup;
	desc.event_cb = engine::event;
	desc.width = 640;
	desc.height = 480;
	return desc;
}