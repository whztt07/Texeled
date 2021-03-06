#include "Core/Memory.h"

#include <stdlib.h> // malloc / free
#include <string.h> // memcpy

namespace Core
{
	////////////////////////////////////////////////////////////////
	// PointerVoid
	////////////////////////////////////////////////////////////////

	PointerVoid::PointerVoid()
	{
		m_iMemory = -1;
		m_iSize = 0;
		m_iPos = 0;
	}

	PointerVoid::PointerVoid(void* pMemory, size_t iSize, size_t iPos)
	{
		m_iMemory = (intptr_t)pMemory;
		m_iSize = iSize;
		m_iPos = iPos;
	}

	bool PointerVoid::IsRootAllocation() const
	{
		return m_iPos == 0;
	}

	bool PointerVoid::operator ==(intptr_t pRight) const
	{
		CORE_ASSERT(m_iMemory != -1, "Using of an uninitialized Pointer");
		return (m_iMemory + (intptr_t)m_iPos) == pRight;
	}

	bool PointerVoid::operator !=(intptr_t pRight) const
	{
		CORE_ASSERT(m_iMemory != -1, "Using of an uninitialized Pointer");
		return (m_iMemory + (intptr_t)m_iPos) != pRight;
	}

	PointerVoid::operator PointerVoid() const
	{
		return PointerVoid((void*)m_iMemory, m_iSize, m_iPos);
	}

	////////////////////////////////////////////////////////////////
	// Functions
	////////////////////////////////////////////////////////////////

	uint16_t c_iAllocMagicNumber = 0xf6b4;

	CORE_PTR_VOID Malloc(size_t iSize)
	{
#ifdef CORE_MEMORY_DEBUG
		void* pAlloc = malloc(iSize + sizeof(uint16_t));
		if (pAlloc != NULL)
		{
			memcpy(pAlloc, &c_iAllocMagicNumber, sizeof(uint16_t));
			return PointerVoid((char*)pAlloc + sizeof(uint16_t), iSize);
		}
		return PointerVoid(NULL, 0);
#else
		return malloc(iSize);
#endif
	}

	void Free(CORE_PTR_VOID pMemory)
	{
#ifdef CORE_MEMORY_DEBUG
		CORE_ASSERT((intptr_t)(void*)pMemory != -1, "Using of an uninitialized Pointer");
		CORE_ASSERT(pMemory.IsRootAllocation());
		uint16_t* pAlloc = (uint16_t*)pMemory - 1;
		CORE_ASSERT(*pAlloc == c_iAllocMagicNumber, "Trying to free an already free allocation");
		*pAlloc = 0;
		free((void*)pAlloc);
#else
		free(pMemory);
#endif
	}

	void MemCpy(CORE_PTR_VOID pDest, CORE_PTR_VOID pSource, size_t iSize)
	{
#ifdef CORE_MEMORY_DEBUG
		((CORE_PTR(char))pDest) + iSize;
		((CORE_PTR(char))pSource) + iSize;
#endif
		memcpy((void*)pDest, (void*)pSource, iSize);
	}
}