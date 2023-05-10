#include <iostream>
#include <string>
#include <asio.hpp>
#include <atomic>
#include "ScreenCapture.h"
#include "screen_server.h"
#include "control_server.h"
using asio::ip::tcp;

void ExtractAndConvertToRGBA(const SL::Screen_Capture::Image &img, unsigned char *dst, size_t dst_size) {
	assert(dst_size >= static_cast<size_t>(SL::Screen_Capture::Width(img) * SL::Screen_Capture::Height(img) * sizeof(SL::Screen_Capture::ImageBGRA)));
	auto imgsrc = StartSrc(img);
	auto imgdist = dst;
	if (img.isContiguous) {
		memcpy(imgdist, imgsrc, dst_size);
	}
	else {
		for (auto h = 0; h < Height(img); h++) {
			auto startimgsrc = imgsrc;
			for (auto w = 0; w < Width(img); w++) {
				*imgdist++ = imgsrc->R;
				*imgdist++ = imgsrc->G;
				*imgdist++ = imgsrc->B;
				*imgdist++ = 0; // alpha should be zero
				imgsrc++;
			}
			imgsrc = SL::Screen_Capture::GotoNextRow(img, startimgsrc);
		}
	}
}

int main() {
	auto onNewFramestart = std::chrono::high_resolution_clock::now();
	std::atomic<int> onNewFramecounter = 0;
	auto mons = SL::Screen_Capture::GetMonitors();
	mons.resize(1);
	auto monitor = mons[0];
	std::atomic<int> imgbuffersize = monitor.Width * monitor.Height * sizeof(SL::Screen_Capture::ImageBGRA);
	std::unique_ptr<unsigned char[]> imgbuffer(std::make_unique<unsigned char[]>(imgbuffersize));
	memset(imgbuffer.get(), 0, imgbuffersize); // create a black image to start with
	std::atomic<bool> imgbufferchanged = false;
	std::atomic<bool> mouseimgchanged = false;
	SL::Screen_Capture::Point mouse;

	auto framgrabber = SL::Screen_Capture::CreateCaptureConfiguration([&]() { return mons; })
		->onNewFrame([&](const SL::Screen_Capture::Image &img, const SL::Screen_Capture::Monitor &monitor) {
			imgbufferchanged = true;
			ExtractAndConvertToRGBA(img, imgbuffer.get(), imgbuffersize);
			// if (std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now() - onNewFramestart).count() >=
			// 	1000) {
			// 	std::cout << "onNewFrame fps" << onNewFramecounter << std::endl;
			// 	onNewFramecounter = 0;
			// 	onNewFramestart = std::chrono::high_resolution_clock::now();
			// }
			// onNewFramecounter += 1;
		})
		->onMouseChanged([&](const SL::Screen_Capture::Image* img, const SL::Screen_Capture::MousePoint &mousepoint) {
			mouse = mousepoint.Position;
			if (img) mouseimgchanged = true;
		})
		->start_capturing();
	framgrabber->setFrameChangeInterval(std::chrono::milliseconds(16));
	framgrabber->setMouseChangeInterval(std::chrono::milliseconds(16));
	try {
		asio::io_context io_context;

		tcp::resolver resolver(io_context);
		tcp::resolver::query query(asio::ip::host_name(),"");
		tcp::resolver::iterator it=resolver.resolve(query);

		//List server's ip addresses
		while (it != tcp::resolver::iterator()) {
			asio::ip::address addr=(it++)->endpoint().address();
			std::cout<<addr.to_string()<<std::endl;
		}


		ScreenServer screen_server(io_context, {monitor.Width, monitor.Height}, [&]() {
			ScreenBuffer buf;
			buf.screen = imgbuffer.get();
			buf.screen_size = imgbuffersize;
			buf.mouse_x = mouse.x;
			buf.mouse_y = mouse.y;
			buf.mouse_changed = mouseimgchanged;
			mouseimgchanged = false;
			if (buf.mouse_changed) {
				buf.mouse_img = NULL;
				buf.mouse_width = 0;
				buf.mouse_height = 0;
				buf.mouse_size = 0;
			}
			return buf;
		});
		ControlServer control_server(io_context, [&]() {

		});
		io_context.run();

	}
	catch (std::exception &e) {
		std::cerr << e.what() << std::endl;
	}

	return 0;
}