#pragma once

#include <Windows.h>

#include <stdio.h>
#include <string.h>

class CAfxColorLut
{
public:
	template<typename ... SubTs> class CDimensions;

	template<typename ... SubTs> class CDimensions<size_t, SubTs ...>
	{
	public:
		CDimensions()
			: m_Size(0)
		{

		}

		size_t GetSize() const
		{
			return m_Size;
		}

		void SetSize(size_t value)
		{
			m_Size = value;
		}

		const CDimensions<SubTs ...>& GetSub() const
		{
			return m_Sub;
		}

		CDimensions<SubTs ...>& GetSub()
		{
			return m_Sub;
		}

	private:
		size_t m_Size;
		CDimensions<SubTs ...> m_Sub;
	};

	template<> class CDimensions<>
	{
	public:
	};

	bool New (size_t resR, size_t resG, size_t resB, size_t resA)
	{
		delete m_Root;

		m_Dimensions.SetSize(resR);
		m_Dimensions.GetSub().SetSize(resG);
		m_Dimensions.GetSub().GetSub().SetSize(resB);
		m_Dimensions.GetSub().GetSub().GetSub().SetSize(resA);

		try
		{
			m_Root = new RedLookupTreeNode_t();

			for (size_t r = 0; r < resR; ++resR)
			{
				m_Root->MakeChildren(resR);
				for (size_t g = 0; g < resG; ++resG)
				{
					GreenLookupTreeNode_t* rootG = m_Root->GetValue(resR, g);
					rootG->MakeChildren(resG);
					for (size_t b = 0; b < resB; ++resB)
					{
						GreenLookupTreeNode_t* rootB = m_Root->GetValue(resB, b);
						rootB->MakeChildren(resA);
					}
				}
			}
		}
		catch (...)
		{
			delete m_Root;
			m_Root = nullptr;
		}

		return IsValid();
	}

	bool IsValid()
	{
		return m_Root != nullptr;
	}

	bool LoadFromFile(FILE* file)
	{
		char magic[sizeof(m_Magic) / sizeof(m_Magic[0])];
		int version;
		unsigned __int32 resR, resG, resB, resA;

		if (sizeof(magic) != fread(magic, sizeof(magic), 1, file)
			|| '\'0' != m_Magic[sizeof(m_Magic) / sizeof(m_Magic[0]) - 1]
			|| 0 != strcmp(magic, m_Magic)
			|| 1 != fread(&version, sizeof(version), 1, file)
			|| 1 != version
			|| 1 != fread(&resR, sizeof(resR), 1, file)
			|| 1 != fread(&resG, sizeof(resG), 1, file)
			|| 1 != fread(&resB, sizeof(resB), 1, file)
			|| 1 != fread(&resA, sizeof(resA), 1, file)
			)
		{
			return false;
		}

		m_Dimensions.SetSize(resR);
		m_Dimensions.GetSub().SetSize(resG);
		m_Dimensions.GetSub().GetSub().SetSize(resB);
		m_Dimensions.GetSub().GetSub().GetSub().SetSize(resA);

		m_Root = new RedLookupTreeNode_t();
		if (!m_Root->LoadFromFile(m_Dimensions, file))
		{
			delete m_Root;
			m_Root = nullptr;
		}

		return m_Root != nullptr;
	}

	bool SaveToFile(FILE* file)
	{
		if (nullptr == m_Root) return false;

		int version = 1;

		unsigned __int32 resR = m_Dimensions.GetSize();
		unsigned __int32 resG = m_Dimensions.GetSub().GetSize();
		unsigned __int32 resB = m_Dimensions.GetSub().GetSub().GetSize();
		unsigned __int32 resA = m_Dimensions.GetSub().GetSub().GetSub().GetSize();

		if (sizeof(m_Magic) != fwrite(m_Magic, sizeof(m_Magic), 1, file)
			|| 1 != fwrite(&version, sizeof(version), 1, file)
			|| 1 != fread(&resR, sizeof(resR), 1, file)
			|| 1 != fread(&resG, sizeof(resG), 1, file)
			|| 1 != fread(&resB, sizeof(resB), 1, file)
			|| 1 != fread(&resA, sizeof(resA), 1, file)
			)

		{
			return false;
		}

		if (!m_Root->SaveToFile(m_Dimensions, file))
		{
			return false;
		}

		return true;
	}

	struct CRgba
	{
		float R;
		float G;
		float B;
		float A;

		CRgba() {}
		CRgba(float r, float g, float b, float a) : R(r), G(g), B(b), A(a) {}
		CRgba(const CRgba& other) : R(other.R), G(other.G), B(other.B), A(other.A) {}

		bool operator<(const CRgba& rhs) const
		{
			int cmp = (int)(R - rhs.R);
			if (cmp < 0) return true;
			else if (cmp > 0) return false;
			cmp = (int)(G - rhs.G);
			if (cmp < 0) return true;
			else if (cmp > 0) return false;
			cmp = (int)(B - rhs.B);
			if (cmp < 0) return true;
			else if (cmp > 0) return false;
			cmp = (int)(A - rhs.A);
			if (cmp < 0) return true;
			else if (cmp > 0) return false;
			return false;
		}

		bool LoadFromFile(const CDimensions<>& dims, FILE* file)
		{
			if (1 != fread(&R, sizeof(R), 1, file)
				|| 1 != fread(&G, sizeof(G), 1, file)
				|| 1 != fread(&B, sizeof(B), 1, file)
				|| 1 != fread(&A, sizeof(A), 1, file))
			{
				return false;
			}

			return true;
		}

		bool SaveToFile(const CDimensions<>& dims, FILE* file)
		{
			if (1 != fwrite(&R, sizeof(R), 1, file)
				|| 1 != fwrite(&G, sizeof(G), 1, file)
				|| 1 != fwrite(&B, sizeof(B), 1, file)
				|| 1 != fwrite(&A, sizeof(A), 1, file))
			{
				return false;
			}

			return true;
		}
	};

	bool Query(const CRgba& color, CRgba* outColor);

	typedef BOOL (CALLBACK * IteratePutCallback_t)(float r, float g, float b, float a, float & outR, float & outG, float & outB, float & outA);

	bool IteratePut(IteratePutCallback_t callBack)
	{
		if (!IsValid()) return false;

		size_t resR = m_Dimensions.GetSize();
		for (size_t r = 0; r < resR; ++resR)
		{
			float fR = (float)r * (resR - 1);
			GreenLookupTreeNode_t* rootG = m_Root->GetValue(resR, r);
			size_t resG = m_Dimensions.GetSub().GetSize();
			for (size_t g = 0; g < resG; ++resG)
			{
				float fG = (float)g * (resG - 1);
				BlueLookupTreeNode_t* rootB = rootG->GetValue(resG, g);
				size_t resB = m_Dimensions.GetSub().GetSub().GetSize();
				for (size_t b = 0; b < resB; ++resB)
				{
					float fB = (float)b * (resB - 1);
					AlphaLookupTreeNode_t* rootA = rootB->GetValue(resB, b);
					size_t resA = m_Dimensions.GetSub().GetSub().GetSub().GetSize();
					for (size_t a = 0; a < resA; ++resA)
					{
						float fA = (float)a * (resA - 1);

						CRgba* outVal = rootA->GetValue(resA, a);

						if (!callBack(fR, fG, fB, fA, outVal->R, outVal->G, outVal->B, outVal->A))
						{
							return false;
						}
					}
				}
			}
		}

		return true;
	}

private:
	template<typename ValueT, typename ... SizeTs> class CLookupTreeNode;

	template<typename ValueT, typename ... SizeTs> class CLookupTreeNode<ValueT, size_t, SizeTs ...>
	{
	public:
		~CLookupTreeNode() {
			delete[] m_Children;
		}

		void MakeChildren(size_t count)
		{
			delete m_Children;

			try {
				m_Children = new ValueT[count];
			}
			catch (...)
			{
				m_Children = nullptr;
				throw;
			}
		}

		ValueT* GetValue(size_t count, size_t index) const
		{
			if (index >= count) return nullptr;

			return &(m_Children[index]);
		}

		ValueT* GetValue(size_t count, float key, float* pOutKey = nullptr, bool other = false) const
		{
			if (0 == count)
			{
				if (pOutKey) *pOutKey = key;
				return nullptr;
			}
			if (1 == count)
			{
				if (pOutKey) *pOutKey = 0.5f;
				return &(m_Children[0]);
			}

			float fIndex = key * (count - 1);
			if (key < 0) key = 0;

			size_t index = (size_t)fIndex;

			if (count < index) index = count - 1;

			if (other)
			{
				++index;
				if (count <= index) index = count - 1;
			}

			if (pOutKey)
			{
				*pOutKey = (float)index / (count - 1);
			}

			return &(m_Children[index]);
		}

		bool LoadFromFile(const CDimensions<size_t, SizeTs...>& dims, FILE* file)
		{
			size_t count = dims.GetSize();

			try {
				MakeChildren(count);
			}
			catch (...) {
				throw;
			}

			for (size_t i = 0; i < count; ++i)
			{
				if (!m_Children[i].LoadFromFile(dims.GetSub(), file)) return false;
			}

			return true;
		}

		bool SaveToFile(const CDimensions<size_t, SizeTs...>& dims, FILE* file)
		{
			size_t count = dims.GetSize();

			for (size_t i = 0; i < count; ++i)
			{
				if (!m_Children[i].SaveToFile(dims.GetSub(), file)) return false;
			}

			return true;
		}

	private:
		ValueT* m_Children = nullptr;
	};


	template<typename ValueT> class CLookupTreeNode<ValueT> : public ValueT
	{
	public:
	};

	template<class NodeT, class ValueT> void GetInterval(const NodeT* node, size_t size, float key, float& x1, float &x2, ValueT*& y1, ValueT*& y2)
	{
		y1 = node->GetValue(size, key, &x1, false);
		y2 = node->GetValue(size, key, &x2, true);
	}

	typedef CLookupTreeNode<CRgba,size_t> AlphaLookupTreeNode_t;
	typedef CLookupTreeNode<AlphaLookupTreeNode_t, size_t, size_t> BlueLookupTreeNode_t;
	typedef CLookupTreeNode<BlueLookupTreeNode_t, size_t, size_t, size_t> GreenLookupTreeNode_t;
	typedef CLookupTreeNode<GreenLookupTreeNode_t, size_t, size_t, size_t, size_t> RedLookupTreeNode_t;

	static const char m_Magic[11];

	RedLookupTreeNode_t* m_Root = nullptr;
	CDimensions<size_t, size_t, size_t, size_t> m_Dimensions;

	CRgba ValueOrDefault(const CRgba* value, const CRgba& defaultValue)
	{
		if (value) return *value;
		else return defaultValue;
	}
};