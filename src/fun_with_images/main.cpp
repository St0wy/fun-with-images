#include <MiniFB_cpp.h>
#include <random>
#include <ranges>
#include <vector>
#include <spdlog/spdlog.h>

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

constexpr std::size_t DefaultBufferWidth = 800;
constexpr std::size_t DefaultBufferHeight = 600;
constexpr std::size_t DefaultBufferSize = DefaultBufferWidth * DefaultBufferHeight;

int main()
{
	mfb_window *window = mfb_open_ex("my display", DefaultBufferWidth, DefaultBufferHeight, WF_RESIZABLE);
	if (!static_cast<bool>(window))
	{
		spdlog::error("Could not instantiate window");
		return EXIT_FAILURE;
	}


	try
	{
		std::vector<std::uint32_t> buffer(DefaultBufferSize);

		mfb_set_resize_callback([&buffer](mfb_window *, const int width, const int height) {
				const std::size_t newSize = static_cast<std::size_t>(width) * static_cast<std::size_t>(height);
				buffer.resize(newSize);
			},
			window);

		std::random_device dev;
		std::mt19937 rng(dev());
		std::uniform_int_distribution<std::mt19937::result_type> distribution(0,
			std::numeric_limits<std::uint8_t>::max());


		while (mfb_wait_sync(window))
		{
			std::ranges::generate(buffer,
				[&] { return MFB_ARGB(0xff, distribution(rng), distribution(rng), distribution(rng)); });
			const int state = mfb_update_ex(window, buffer.data(), DefaultBufferWidth, DefaultBufferHeight);

			if (state < 0)
			{
				spdlog::info("Window closing...");
				window = nullptr;
				break;
			}
		}
	}
	catch (std::exception &exception)
	{
		spdlog::error(exception.what());
		return EXIT_FAILURE;
	}
}
