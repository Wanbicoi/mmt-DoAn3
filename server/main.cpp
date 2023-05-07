#include <iostream>
#include <string>
#include <asio.hpp>
#include <atomic>
#include "list_running_processes.hpp"
#include "ScreenCapture.h"
using asio::ip::tcp;

void ExtractAndConvertToRGBA(const SL::Screen_Capture::Image &img, unsigned char *dst, size_t dst_size) {
	memcpy(dst, img.Data, dst_size);
	return;
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

	auto framgrabber =
		SL::Screen_Capture::CreateCaptureConfiguration([&]() { return mons; })
			->onNewFrame([&](const SL::Screen_Capture::Image &img, const SL::Screen_Capture::Monitor &monitor) {
				imgbufferchanged = true;
				ExtractAndConvertToRGBA(img, imgbuffer.get(), imgbuffersize);
				if (std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now() - onNewFramestart).count() >=
					1000) {
					std::cout << "onNewFrame fps" << onNewFramecounter << std::endl;
					onNewFramecounter = 0;
					onNewFramestart = std::chrono::high_resolution_clock::now();
				}
				onNewFramecounter += 1;
			})
			->start_capturing();
	framgrabber->setFrameChangeInterval(std::chrono::milliseconds(100));
	try {
		asio::io_context io_context;

		tcp::acceptor acceptor(io_context, tcp::endpoint(tcp::v4(), 13));

		tcp::resolver resolver(io_context);
		tcp::resolver::query query(asio::ip::host_name(),"");
		tcp::resolver::iterator it=resolver.resolve(query);

		//List server's ip addresses
		while (it != tcp::resolver::iterator()) {
			asio::ip::address addr=(it++)->endpoint().address();
			std::cout<<addr.to_string()<<std::endl;
		}

		tcp::socket socket(io_context);
		acceptor.accept(socket);

		asio::error_code ignored_error;
		asio::write(socket, asio::buffer(&monitor.Width, sizeof(int)), ignored_error);
		asio::write(socket, asio::buffer(&monitor.Height, sizeof(int)), ignored_error);
		//std::string message = list_running_processes();
		while (1) {
			asio::write(socket, asio::buffer(imgbuffer.get(), imgbuffersize), ignored_error);
		}
	}
	catch (std::exception &e) {
		std::cerr << e.what() << std::endl;
	}

	return 0;
}