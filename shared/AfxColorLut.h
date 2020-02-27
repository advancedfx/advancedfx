#pragma once

#include <stdio.h>
#include <string.h>

class CAfxColorLut
{
public:
	bool New (size_t resR, size_t resG, size_t resB, size_t resA)
	{
		try
		{
			m_Root = new RedLookupTreeNode_t();

			for (size_t r = 0; r < resR; ++resR)
			{
				m_Root->MakeChildren(resG);
				for (size_t g = 0; g < resG; ++resG)
				{
					GreenLookupTreeNode_t* rootG = m_Root->GetValue(resG);
					rootG->MakeChildren(resB);
					for (size_t b = 0; b < resB; ++resB)
					{
						GreenLookupTreeNode_t* rootB = m_Root->GetValue(resG);
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

		if (sizeof(magic) != fread(magic, sizeof(magic), 1, file)
			|| '\'0' != m_Magic[sizeof(m_Magic) / sizeof(m_Magic[0]) - 1]
			|| 0 != strcmp(magic, m_Magic)
			|| 1 != fread(&version, sizeof(version), 1, file)
			|| 1 != version)
		{
			return false;
		}

		m_Root = new RedLookupTreeNode_t();
		if (!m_Root->LoadFromFile(file))
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

		if (sizeof(m_Magic) != fwrite(m_Magic, sizeof(m_Magic), 1, file)
			|| 1 != fwrite(&version, sizeof(version), 1, file))
		{
			return false;
		}

		if (!m_Root->SaveToFile(file))
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

		bool LoadFromFile(FILE* file)
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

		bool SaveToFile(FILE* file)
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

	typedef const CRgba & (*IteratePutCallback_t)(const CRgba & at);

	void IteratePut(IteratePutCallback_t callBack)
	{
		if (!IsValid()) return;

		size_t resR = m_Root->GetChildrenCount();
		for (size_t r = 0; r < resR; ++resR)
		{
			float fR = (float)r * (resR - 1);
			GreenLookupTreeNode_t* rootG = m_Root->GetValue(r);
			size_t resG = rootG->GetChildrenCount();
			for (size_t g = 0; g < resG; ++resG)
			{
				float fG = (float)g * (resG - 1);
				BlueLookupTreeNode_t* rootB = rootG->GetValue(g);
				size_t resB = rootB->GetChildrenCount();
				for (size_t b = 0; b < resB; ++resB)
				{
					float fB = (float)b * (resB - 1);
					AlphaLookupTreeNode_t* rootA = rootB->GetValue(b);
					size_t resA = rootA->GetChildrenCount();
					for (size_t a = 0; a < resA; ++resA)
					{
						float fA = (float)a * (resA - 1);
						*(rootA->GetValue(a)) = callBack(CRgba(fR,fG,fB,fA));
					}
				}
			}
		}
	}

private:
	template<typename ValueT> class CLookupTreeNode
	{
	public:
		~CLookupTreeNode() {
			delete[] m_Children;
		}

		size_t GetChildrenCount() const
		{
			return m_ChildrenCount;
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
				m_ChildrenCount = 0;
				throw;
			}
		}

		ValueT* GetValue(size_t index) const
		{
			if (index >= m_ChildrenCount) return nullptr;
		}

		ValueT* GetValue(float key, float* pOutKey = nullptr, bool other = false) const
		{
			if (0 == m_ChildrenCount)
			{
				if (pOutKey) *pOutKey = key;
				return nullptr;
			}
			if (1 == m_ChildrenCount)
			{
				if (pOutKey) *pOutKey = 0.5f;
				return &(m_Children[0]);
			}

			float fIndex = key * (m_ChildrenCount - 1);
			if (key < 0) key = 0;

			size_t index = (size_t)fIndex;

			if (m_ChildrenCount < index) index = m_ChildrenCount - 1;

			if (other)
			{
				++index;
				if (m_ChildrenCount <= index) index = m_ChildrenCount - 1;
			}

			if (pOutKey)
			{
				*pOutKey = (float)index / (m_ChildrenCount - 1);
			}

			return &(m_Children[index]);
		}

		bool LoadFromFile(FILE* file)
		{
			size_t count;

			if (1 != fread(&count, sizeof(count), 1, file)) return false;

			try {
				MakeChildren(count);
			}
			catch (...) {
				throw;
			}

			for (size_t i = 0; i < count; ++i)
			{
				if (!m_Children[i].LoadFromFile(file)) return false;
			}

			return true;
		}

		bool SaveToFile(FILE* file)
		{
			if (1 != fwrite(&m_ChildrenCount, sizeof(m_ChildrenCount), 1, file)) return false;

			for (size_t i = 0; i < m_ChildrenCount; ++i)
			{
				if (!m_Children[i].SaveToFile(file)) return false;
			}

			return true;
		}

	private:
		size_t m_ChildrenCount = 0;
		ValueT* m_Children = nullptr;
	};

	template<class ValueT> void GetInterval(const CLookupTreeNode<ValueT>* node, float key, float& x1, float &x2, ValueT*& y1, ValueT*& y2)
	{
		y1 = node->GetValue(key, &x1, false);
		y2 = node->GetValue(key, &x2, false);
	}

	typedef CLookupTreeNode<CRgba> AlphaLookupTreeNode_t;
	typedef CLookupTreeNode<AlphaLookupTreeNode_t> BlueLookupTreeNode_t;
	typedef CLookupTreeNode<BlueLookupTreeNode_t> GreenLookupTreeNode_t;
	typedef CLookupTreeNode<GreenLookupTreeNode_t> RedLookupTreeNode_t;

	static const char m_Magic[17];

	RedLookupTreeNode_t* m_Root = nullptr;
	size_t m_ResR = 0;
	size_t m_ResG = 0;
	size_t m_ResB = 0;
	size_t m_ResA = 0;

	CRgba ValueOrDefault(const CRgba* value, const CRgba& defaultValue)
	{
		if (value) return *value;
		else return defaultValue;
	}
};