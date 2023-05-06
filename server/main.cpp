#include "ScreenCapture.h"
#include <atomic>
#include <iostream>

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

int main(void) {
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
	while(1) {};
	return 0;
}