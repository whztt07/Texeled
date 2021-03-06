
#ifndef __PROGRAM_H__
#define __PROGRAM_H__

#include "ImwWindowManagerCustom.h"

#include "Graphics/Texture.h"
#include "Graphics/TextureLoader.h"

#include "ShortKeyManager.h"

class Menus;

struct IDXGIFactory;
struct ID3D11Device;
struct ID3D11DeviceContext;

namespace GraphicResources
{
	class Texture2D;
}

namespace Windows
{
	class WorkAreaWindow;
}


struct Shortkeys
{
	const ShortKeyManager::ShortKey* pClose;
	const ShortKeyManager::ShortKey* pOpen;
	const ShortKeyManager::ShortKey* pSave;
	const ShortKeyManager::ShortKey* pSaveAs;
};

struct Fonts
{
	ImFont* pFontConsolasBold;
	ImFont* pFontConsolas;
};

struct _ProgramModeEnum
{
	enum Enum
	{
		VIEWER,
		EDIT_COLOR,
		EDIT_NORMAL,

		_COUNT,
		_LAST = _COUNT - 1
	};
};
typedef _ProgramModeEnum::Enum ProgramModeEnum;
extern const char* ProgramModeEnumStrings[ProgramModeEnum::_COUNT];

struct _ChannelFlag
{
	enum Enum
	{
		RED		= (1 << 0),
		GREEN	= (1 << 1),
		BLUE	= (1 << 2),
		ALPHA	= (1 << 3),

		_COUNT,
		_NONE = 0
	};
};
typedef _ChannelFlag::Enum ChannelFlag;
typedef int ChannelFlags;
extern const char* ChannelFlagStrings[ChannelFlag::_COUNT];

struct DisplayOptions
{
	bool bShowPixelGrid;
	bool bTiling;

	ChannelFlags eShowChannels;

	float fRange[2];
	float fGamma;

	int iMip;
	int iFace;
};

class Program
{
	Program(int iArgCount, char** pArgs);
	~Program();
public:

	static Program*							CreateInstance(int iArgCount, char** pArgs);
	static void								DestroyInstance();
	static Program*							GetInstance() { return s_pInstance; }

	bool									Run();
	void									AskExit();
	void									CloseCurrentWindow();

	ImWindow::ImwWindowManager*				GetWindowManager() { return &m_oImWindowMgr; }

	IDXGIFactory*							GetDXGIFactory() const;
	ID3D11Device*							GetDX11Device() const;
	ID3D11DeviceContext*					GetDX11DeviceContext() const;

	bool									LoadFile(const char* pFile, const Graphics::TextureLoaderInfo* pUseLoader = NULL);
	ErrorCode								LoadFileInternal(const char* pFile, const Graphics::TextureLoaderInfo* pUseLoader = NULL);
	Graphics::Texture&						GetTexture() { return m_oTexture; }
	GraphicResources::Texture2D*			GetTexture2DRes() { return m_pTexture2D; }
	const char*								GetTextureFilePath() const { return m_pTexturePath; }
	void									UpdateTexture2DRes();

	void									Open();
	void									Save();
	void									SaveAs();

	const Shortkeys&						GetShortkeys() const { return m_oShortkeys; }
	const Fonts&							GetFonts() const { return m_oFonts; }

	ProgramModeEnum							GetMode() const { return m_eMode; }
	void									SetMode(ProgramModeEnum eMode) { m_eMode = eMode; }

	DisplayOptions&							GetDisplayOptions() { return m_oDisplayOptions; }
protected:
	static Program*							s_pInstance;

	bool									m_bRun;

	ImWindow::ImwWindowManagerCustom		m_oImWindowMgr;

	Fonts									m_oFonts;

	ShortKeyManager*						m_pShortKeyManager;
	Shortkeys								m_oShortkeys;
	Menus*									m_pMenus;

	ProgramModeEnum							m_eMode;
	DisplayOptions							m_oDisplayOptions;

	Windows::WorkAreaWindow*				m_pWorkAreaWindow;

	Graphics::Texture						m_oTexture;
	GraphicResources::Texture2D*			m_pTexture2D;
	char*									m_pTexturePath;
};

#endif // __PROGRAM_H__