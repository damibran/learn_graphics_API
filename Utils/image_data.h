#pragma once

#include <vector>
#include <stb_image.h>

namespace dmbrn
{

	struct image_data
	{
		bool operator==(const image_data& other)const
		{
			if (width != other.width || height != other.height || comp_per_pix != other.comp_per_pix)
				return false;
			else
			{
				bool same = true;
				for (size_t i = 0; i < getLength() && same; ++i)
				{
					if (data[i] != other.data[i])
						same = false;
				}
				return same;
			}
		}

		size_t getLength() const
		{
			return width * height * comp_per_pix;
		}

		void copyData(const stbi_uc* src, size_t len)
		{
			data.clear();
			data.insert(data.begin(),src,src+len);
		}

	public:
		std::vector<unsigned char> data;
		int width, height, comp_per_pix;
	};

}