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

bool DrawBitmap(std::string_view imagePath,
	std::uint32_t xPosition,
	std::uint32_t yPosition,
	std::uint32_t bufferWidth,
	std::uint32_t bufferHeight,
	std::vector<std::uint32_t> &buffer);

int main()
{
	std::size_t bufferWidth = DefaultBufferWidth;
	std::size_t bufferHeight = DefaultBufferHeight;

	mfb_window *window = mfb_open_ex("my display", DefaultBufferWidth, DefaultBufferHeight, WF_RESIZABLE);
	if (!static_cast<bool>(window))
	{
		spdlog::error("Could not instantiate window");
		return EXIT_FAILURE;
	}

	try
	{
		std::vector<std::uint32_t> buffer(DefaultBufferSize);

		mfb_set_resize_callback([&](mfb_window *, int width, const int height) {
				bufferWidth = static_cast<std::size_t>(width);
				bufferHeight = static_cast<std::size_t>(height);

				buffer.resize(bufferWidth * bufferHeight);
			},
			window);

		while (mfb_wait_sync(window))
		{
			std::ranges::fill(buffer, MFB_ARGB(0xff, 0, 0, 0));

			const bool result = DrawBitmap("../../data/face.png",
				0,
				0,
				static_cast<std::uint32_t>(bufferWidth),
				static_cast<std::uint32_t>(bufferHeight),
				buffer);

			if (!result)
			{
				spdlog::error("Could not draw image");
				return EXIT_FAILURE;
			}

			const int state = mfb_update_ex(window,
				buffer.data(),
				static_cast<unsigned>(bufferWidth),
				static_cast<unsigned>(bufferHeight));


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

bool DrawBitmap(const std::string_view imagePath,
	const std::uint32_t xPosition,
	const std::uint32_t yPosition,
	const std::uint32_t bufferWidth,
	const std::uint32_t bufferHeight,
	std::vector<std::uint32_t> &buffer)
{
	int width = 0;
	int height = 0;
	int channelsInFile = 0;
	const unsigned char *data = stbi_load(imagePath.data(), &width, &height, &channelsInFile, 0);

	if (data == nullptr) { return false; }

	const std::uint32_t imageRightLimit = xPosition + static_cast<std::uint32_t>(width);
	const std::uint32_t imageDownLimit = yPosition + static_cast<std::uint32_t>(height);

	if (imageRightLimit > bufferWidth || imageDownLimit > bufferHeight) { return false; }

	const auto unsignedWidth = static_cast<std::size_t>(width);
	const auto unsignedHeight = static_cast<std::size_t>(height);
	const auto unsignedNumChannels = static_cast<std::size_t>(channelsInFile);

	const auto image = std::span(data, unsignedWidth * unsignedHeight * unsignedNumChannels);

	for (std::size_t yImage = 0; yImage < unsignedHeight; yImage++)
	{
		for (std::size_t xImage = 0; xImage < unsignedWidth; xImage++)
		{
			const std::size_t imageIndex = xImage * unsignedNumChannels + yImage * unsignedWidth * unsignedNumChannels;
			const auto pixelColorR = image[imageIndex + 0];
			const auto pixelColorG = image[imageIndex + 1];
			const auto pixelColorB = image[imageIndex + 2];
			const std::size_t bufferIndex = xImage + xPosition + (yImage + yPosition) * bufferWidth;
			buffer[bufferIndex] = MFB_ARGB(0xff, pixelColorR, pixelColorG, pixelColorB);
		}
	}

	return true;
}
