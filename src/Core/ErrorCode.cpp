#include "Core/ErrorCode.h"

#include "Core/Assert.h"

#include "Core/StringUtils.h"

#include <stdlib.h> // malloc / free
#include <string.h> // memcpy
#include <stdarg.h> // va_list / va_start / va_end

bool ErrorCode::s_bStaticTime = true;

ErrorCode ErrorCode::Ok(0, "Ok");
ErrorCode ErrorCode::Fail(-1, "Fail");
ErrorCode ErrorCode::NotImplemented(-2, "Not Implemented");
ErrorCode ErrorCode::InvalidArgument(-3, "Invalid argument(s)");

ErrorCode::StaticResolver ErrorCode::s_oStaticResolver;

ErrorCode::StaticResolver::StaticResolver()
{
	ErrorCode::s_bStaticTime = false;
}

ErrorCode::ErrorCode(int iCode, const char* pMessage, ...)
{
	m_bChecked = false;
	m_iCode = iCode;
	m_bStatic = s_bStaticTime;
	if (pMessage == NULL)
		pMessage = "";

	char pBuffer[1024];
	va_list oArgs;
	va_start(oArgs, pMessage);
	Core::StringUtils::VSNPrintf(pBuffer, sizeof(pBuffer), pMessage, oArgs);
	va_end(oArgs);

	size_t iLen = Core::StringUtils::StrLen(pBuffer, sizeof(pBuffer)) + 1;
	m_pMessage = (char*)malloc(iLen);
	CORE_ASSERT(m_pMessage != NULL);
	memcpy(m_pMessage, pBuffer, iLen);
}

ErrorCode::ErrorCode(const ErrorCode& oRight)
{
	m_bChecked = true;
	m_iCode = 0;
	m_pMessage = NULL;
	m_bStatic = s_bStaticTime;
	*this = oRight;
}

ErrorCode::~ErrorCode()
{
	Release();
}

bool ErrorCode::operator==(ErrorCode& oRight)
{
	oRight.m_bChecked = true;
	m_bChecked = true;
	return oRight.m_iCode == m_iCode;
}

bool ErrorCode::operator!=(ErrorCode& oRight)
{
	oRight.m_bChecked = true;
	m_bChecked = true;
	return oRight.m_iCode != m_iCode;
}

void ErrorCode::operator=(const ErrorCode& oRight)
{
	Release();
	m_bChecked = false;
	m_iCode = oRight.m_iCode;

	size_t iLen = strlen(oRight.m_pMessage) + 1;
	m_pMessage = (char*)malloc(iLen);
	CORE_ASSERT(m_pMessage != NULL);
	memcpy(m_pMessage, oRight.m_pMessage, iLen);

	oRight.m_bChecked = true;
}

void ErrorCode::Release()
{
	CORE_ASSERT(m_bChecked || m_bStatic, "ErrorCode never tested");
	free(m_pMessage);

	m_bChecked = true;
	m_pMessage = NULL;
}
