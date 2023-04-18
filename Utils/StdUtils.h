#pragma once

#include <glm/glm.hpp>
#include <assimp/mesh.h>
#include "Utils/image_data.h"

namespace std
{
	// Image Data support
	template <>
	struct hash<dmbrn::image_data>
	{
		size_t operator()(const dmbrn::image_data& image_data) const noexcept
		{
			std::hash<unsigned char> hasher;
			size_t res = 0;
			for (int i = 0; i < image_data.getLength(); ++i)
			{
				res ^= hasher(image_data.data[i]);
			}
			return res;
		}
	};

	template <>
	struct hash<glm::vec4>
	{
		size_t operator()(const glm::vec4& vec) const noexcept
		{
			std::hash<float> hasher;
			size_t res = hasher(vec.x) ^ hasher(vec.y) ^ hasher(vec.z) ^ hasher(vec.w);
			return res;
		}
	};

	// Mesh Support

	template <>
	struct hash<aiVector3D>
	{
		size_t operator()(const aiVector3D& vec) const noexcept
		{
			std::hash<float> hasher;
			size_t res = 0;
			res = hasher(vec.x) ^ hasher(vec.y) ^ hasher(vec.z);

			return res;
		}
	};

	template <>
	struct hash<std::vector<aiVector3D>>
	{
		size_t operator()(const std::vector<aiVector3D>& vertices) const noexcept
		{
			std::hash<aiVector3D> hasher;
			size_t res = 0;
			for (const auto& vec : vertices)
			{
				res ^= hasher(vec);
			}
			return res;
		}
	};

	template <>
	struct equal_to<std::vector<aiVector3D>>
	{
		bool operator()(const std::vector<aiVector3D>& lhs, const std::vector<aiVector3D>& rhs) const
		{
			if (lhs.size() != rhs.size())
				return false;

			bool same = true;
			for (size_t i = 0; i < lhs.size() && same; ++i)
			{
				if (lhs[i] != rhs[i])
					same = false;
			}

			return same;
		}
	};

	inline string to_string(const aiVector3D& vec)
	{
		string res;

		for (int i = 0; i < 3; ++i)
		{
			res+=to_string(vec[i])+" ";
		}

		return res;
	}

	inline string to_string(const aiMatrix4x4& mat)
	{
		string res;
		for (int i = 0; i < 4; ++i)
		{
			auto vec = mat[i];
			for (int j = 0; j < 4; ++j)
			{
				res += to_string(vec[j]) + " ";
			}
			res += "\n";
		}
		return res;
	}

	inline string to_string(const glm::mat4& mat)
	{
		string res{};
		for (int i = 0; i < 4; ++i)
		{
			for (int j = 0; j < 4; ++j)
			{
				res+=to_string(mat[j][i])+" ";
			}
			res+="\n";
		}
		return res;
	}
}
