#include <iostream>
#include <string>
#include <vector>
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
	auto mons = SL::Screen_Capture::GetMonitors();
	mons.resize(1);
	auto monitor = mons[0];
	std::atomic<int> imgbuffersize = monitor.Width * monitor.Height * sizeof(SL::Screen_Capture::ImageBGRA);
	std::unique_ptr<unsigned char[]> imgbuffer(std::make_unique<unsigned char[]>(imgbuffersize));
	memset(imgbuffer.get(), 0, imgbuffersize); // create a black image to start with
	std::atomic<bool> imgbufferchanged = false;
	std::atomic<bool> mouseimgchanged = false;
	std::atomic<int> mouse_x = 0;
	std::atomic<int> mouse_y = 0;
	std::atomic<int> mouse_width = 0;
	std::atomic<int> mouse_height = 0;
	unsigned char *mouseimgbuffer = nullptr;

	auto framgrabber = SL::Screen_Capture::CreateCaptureConfiguration([&]() { return mons; })
		->onNewFrame([&](const SL::Screen_Capture::Image &img, const SL::Screen_Capture::Monitor &monitor) {
			imgbufferchanged = true;
			ExtractAndConvertToRGBA(img, imgbuffer.get(), imgbuffersize);
			
		})
		->onMouseChanged([&](const SL::Screen_Capture::Image* img, const SL::Screen_Capture::MousePoint &mousepoint) {
			mouse_x = mousepoint.Position.x;
			mouse_y = mousepoint.Position.y;
			if (img) {
				mouseimgchanged = true;
				mouse_width = Width(*img);
				mouse_height = Height(*img);
				int size = mouse_width * mouse_height * 4;
				mouseimgbuffer = (unsigned char*)realloc(mouseimgbuffer, size);
				memcpy(mouseimgbuffer, StartSrc(*img), size);
			}
		})
		->start_capturing();
	framgrabber->setFrameChangeInterval(std::chrono::milliseconds(16));
	framgrabber->setMouseChangeInterval(std::chrono::milliseconds(16));
	try {
		asio::io_context io_context;

		tcp::resolver resolver(io_context);
		tcp::resolver::query query(asio::ip::host_name(), "");
		tcp::resolver::iterator it=resolver.resolve(query);

		std::vector<std::string> local_ips;
		//List server's ip addresses
		while (it != tcp::resolver::iterator()) {
			asio::ip::address addr=(it++)->endpoint().address();
			if (addr.is_v4())
				local_ips.push_back(addr.to_string());
		}
		if (local_ips.size()) {
			std::cout << "The server is hosted on the following ";
			if (local_ips.size() > 1) std::cout << "IPs: " << std::endl;
			else std::cout << "IP: " << std::endl;
			for (auto &ip: local_ips)
				std::cout << ip << std::endl;
		}
		else {
			std::cout << "Error! Can't find your machine IP" std::endl;
			return -2;
		}

		ScreenServer screen_server(io_context, {monitor.Width, monitor.Height}, [&]() {
			ScreenBuffer buf;
			buf.screen = imgbuffer.get();
			buf.screen_size = imgbuffersize;
			buf.mouse_x = mouse_x;
			buf.mouse_y = mouse_y;
			buf.mouse_changed = mouseimgchanged;
			mouseimgchanged = false;
			if (buf.mouse_changed) {
				buf.mouse_image.data = mouseimgbuffer;
				buf.mouse_image.width = mouse_width;
				buf.mouse_image.height = mouse_height;
				buf.mouse_image.size = mouse_width * mouse_height * 4;
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
	free(mouseimgbuffer);

	return 0;
}