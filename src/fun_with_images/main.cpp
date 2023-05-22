#include <cstdint>
#include <MiniFB_cpp.h>
#include <random>
#include <ranges>
#include <vector>
#include <spdlog/spdlog.h>

#define STB_IMAGE_IMPLEMENTATION
#include <numeric>
#include <stb_image.h>

constexpr std::size_t DefaultBufferWidth = 800;
constexpr std::size_t DefaultBufferHeight = 600;
constexpr std::size_t DefaultBufferSize = DefaultBufferWidth * DefaultBufferHeight;
constexpr std::size_t PaletteBufferSize = 256;

struct Rgb
{
	std::uint8_t r;
	std::uint8_t g;
	std::uint8_t b;
};

struct Hsl
{
	std::uint8_t hue;
	std::uint8_t saturation;
	std::uint8_t lightness;
};

constexpr Rgb HslToRgb(const Hsl hsl)
{
	// I'm sorry, idk how to remove the warning otherwise :(
	constexpr int zero = 0;
	constexpr int one = 1;
	constexpr int two = 2;
	constexpr int three = 3;
	constexpr int four = 4;
	constexpr int five = 5;

	const double doubleHue = hsl.hue / 255.0;
	const double doubleSaturation = hsl.saturation / 255.0;
	const double doubleLightness = hsl.lightness / 255.0;

	const double chroma = (1 - std::abs(2 * doubleLightness - 1)) * doubleSaturation;
	const double huePrime = doubleHue * 6.0;
	const double x = chroma * (1 - std::abs(std::fmod(huePrime, 2.0) - 1));

	double red1 = 0.0;
	double green1 = 0.0;
	double blue1 = 0.0;

	switch (static_cast<int>(huePrime))
	{
	case zero:
		red1 = chroma;
		green1 = x;
		break;
	case one:
		red1 = x;
		green1 = chroma;
		break;
	case two:
		green1 = chroma;
		blue1 = x;
		break;
	case three:
		green1 = x;
		blue1 = chroma;
		break;
	case four:
		red1 = x;
		blue1 = chroma;
		break;
	case five:
		red1 = chroma;
		blue1 = x;
		break;
	default:
		break;
	}

	const double intensity = doubleLightness - chroma / 2.0;

	return { static_cast<std::uint8_t>((red1 + intensity) * std::numeric_limits<std::uint8_t>::max()),
	         static_cast<std::uint8_t>((green1 + intensity) * std::numeric_limits<std::uint8_t>::max()),
	         static_cast<std::uint8_t>((blue1 + intensity) * std::numeric_limits<std::uint8_t>::max()) };
}

constexpr std::uint32_t RgbToIntArgb(const Rgb rgb) { return MFB_ARGB(0xff, rgb.r, rgb.g, rgb.b); }

bool DrawBitmap(std::string_view imagePath,
	std::uint32_t xPosition,
	std::uint32_t yPosition,
	std::uint32_t bufferWidth,
	std::uint32_t bufferHeight,
	std::vector<std::uint32_t>& buffer);

int main()
{
	std::size_t bufferWidth = DefaultBufferWidth;
	std::size_t bufferHeight = DefaultBufferHeight;

	mfb_window* window = mfb_open_ex("my display", DefaultBufferWidth, DefaultBufferHeight, WF_RESIZABLE);
	if (!static_cast<bool>(window))
	{
		spdlog::error("Could not instantiate window");
		return EXIT_FAILURE;
	}

	try
	{
		std::vector<std::uint32_t> buffer(DefaultBufferSize);
		std::vector<std::uint8_t> fireBuffer(DefaultBufferSize);
		std::vector<std::uint32_t> paletteBuffer(PaletteBufferSize);

		std::ranges::fill(fireBuffer, 0);

		for (std::uint16_t i = 0; i < static_cast<std::uint16_t>(PaletteBufferSize); i++)
		{
			const auto smallerI = static_cast<std::uint8_t>(i);
			const std::uint8_t left = smallerI * 2;
			constexpr std::uint8_t right = 255;
			const std::uint8_t lightness = std::min(left, right);
			const std::uint8_t hue = smallerI / 3;
			const auto color = HslToRgb({ hue, 255, lightness });
			paletteBuffer[smallerI] = RgbToIntArgb(color);
		}

		mfb_set_resize_callback([&](mfb_window*, int width, const int height) {
				bufferWidth = static_cast<std::size_t>(width);
				bufferHeight = static_cast<std::size_t>(height);

				buffer.resize(bufferWidth * bufferHeight);
				fireBuffer.resize(bufferWidth * bufferHeight);
			},
			window);
		mfb_timer* timer = mfb_timer_create();

		std::random_device dev;
		std::mt19937 rng(dev());
		std::uniform_int_distribution<std::mt19937::result_type> distribution(0,
			std::numeric_limits<std::uint8_t>::max());

		while (mfb_wait_sync(window))
		{
			//const double deltaTime = mfb_timer_delta(timer);
			std::ranges::fill(buffer, MFB_ARGB(0xff, 0, 0, 0));

			for (std::size_t i = 0; i < bufferWidth; i++)
			{
				fireBuffer[i + (bufferHeight - 1) * bufferWidth] = static_cast<std::uint8_t>(distribution(rng));
			}

			for (int bufferY = 0; bufferY < static_cast<int>(bufferHeight - 1); bufferY++)
			{
				for (int bufferX = 0; bufferX < static_cast<int>(bufferWidth); bufferX++)
				{
					const int width = static_cast<int>(bufferWidth);
					const int height = static_cast<int>(bufferHeight);
					const std::size_t index = static_cast<std::size_t>(bufferX) + static_cast<std::size_t>(bufferY) *
					                          bufferWidth;
					auto fireIndex = (bufferX - 1 + width) % width + (bufferY + 1) % height * width;
					const auto tmp1 = fireBuffer[static_cast<std::size_t>(fireIndex)];

					fireIndex = bufferX % width + (bufferY + 1) % height * width;
					const auto tmp2 = fireBuffer[static_cast<std::size_t>(fireIndex)];

					fireIndex = (bufferX + 1) % width + (bufferY + 1) % height * height;
					const auto tpm3 = fireBuffer[static_cast<std::size_t>(fireIndex)];

					fireIndex = bufferX % width + (bufferY + 2) % height * height;
					const auto tpm4 = fireBuffer[static_cast<std::size_t>(fireIndex)];

					const auto tpm5 = (tmp1 + tmp2 + tpm3 + tpm4) * 32 / 129;
					fireBuffer[index] = static_cast<std::uint8_t>(tpm5);
				}
			}

			for (std::size_t bufferY = 0; bufferY < bufferHeight; bufferY++)
			{
				for (std::size_t bufferX = 0; bufferX < bufferWidth; bufferX++)
				{
					const std::size_t index = bufferX + bufferY * bufferWidth;
					buffer[index] = paletteBuffer[fireBuffer[index]];
				}
			}


			//DrawBitmap("../../data/face.png",
			//	0,
			//	0,
			//	static_cast<std::uint32_t>(bufferWidth),
			//	static_cast<std::uint32_t>(bufferHeight),
			//	buffer);

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

		mfb_timer_destroy(timer);
	}
	catch (std::exception& exception)
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
	std::vector<std::uint32_t>& buffer)
{
	int width = 0;
	int height = 0;
	int channelsInFile = 0;
	unsigned char* data = stbi_load(imagePath.data(), &width, &height, &channelsInFile, 0);

	if (data == nullptr)
	{
		stbi_image_free(data);
		return false;
	}

	const std::uint32_t imageRightLimit = xPosition + static_cast<std::uint32_t>(width);
	const std::uint32_t imageDownLimit = yPosition + static_cast<std::uint32_t>(height);

	if (imageRightLimit > bufferWidth || imageDownLimit > bufferHeight)
	{
		stbi_image_free(data);
		return false;
	}

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

	stbi_image_free(data);

	return true;
}
