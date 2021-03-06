
#include "Program.h"

#include "ShortKeyManager.h"
#include "Menus.h"
#include "StatusBars.h"
#include "Toolbar.h"

#include <D3D11.h>

#include "PlatformUtils.h"

#include "Core/StringBuilder.h"
#include "Core/StringUtils.h"

#include "Graphics/TextureLoader.h"
#include "Graphics/TextureWriter.h"

#include "Graphics/TextureLoaders/TextureLoaderSTBI.h"
#include "Graphics/TextureLoaders/TextureLoaderDDS.h"
#include "Graphics/TextureLoaders/TextureLoaderEXR.h"
#include "Graphics/TextureLoaders/TextureLoaderKTX.h"

#include "Graphics/TextureWriters/TextureWriterDDS.h"
#include "Graphics/TextureWriters/TextureWriterPNG.h"
#include "Graphics/TextureWriters/TextureWriterEXR.h"

#include "Resources/Fonts/Consolas_ttf.h"
#include "Resources/Fonts/Consolas_Bold_ttf.h"

#include "Windows/WorkAreaWindow.h"

#include "GraphicResources/Texture2D.h"

const char* ProgramModeEnumStrings[ProgramModeEnum::_COUNT] =
{
	"Viewer",
	"Edit color",
	"Edit normal",
};

Program* Program::s_pInstance = NULL;

Program::Program(int iArgCount, char** pArgs)
	: m_bRun(true)
	, m_eMode(ProgramModeEnum::VIEWER)
	, m_oImWindowMgr(true)
	, m_pTexture2D(NULL)
	, m_pTexturePath(NULL)
{
	s_pInstance = this;

	// Checking PixelFormats
	for (int i = 0; i < Graphics::PixelFormatEnum::_COUNT; ++i)
	{
		CORE_ASSERT(Graphics::PixelFormatEnumInfos[i].eFormat == i);
	}

	Graphics::TextureLoader::RegisterLoaderSTBI();
	Graphics::TextureLoader::RegisterLoaderDDS();
	Graphics::TextureLoader::RegisterLoaderEXR();
	Graphics::TextureLoader::RegisterLoaderKTX();

	Graphics::TextureWriter::RegisterWriterDDS();
	Graphics::TextureWriter::RegisterWriterPNG();
	Graphics::TextureWriter::RegisterWriterEXR();

	ImFontConfig oConfig;
	oConfig.MergeMode = false;
	oConfig.FontDataOwnedByAtlas = false;
	//ImGui::GetIO().Fonts->AddFontDefault();
	m_oFonts.pFontConsolasBold = ImGui::GetIO().Fonts->AddFontFromMemoryTTF((void*)Resources::Fonts::Consolas_Bold_ttf::Data, Resources::Fonts::Consolas_Bold_ttf::Size, 16.f, &oConfig, NULL);
	m_oFonts.pFontConsolas = ImGui::GetIO().Fonts->AddFontFromMemoryTTF((void*)Resources::Fonts::Consolas_ttf::Data, Resources::Fonts::Consolas_ttf::Size, 16.f, &oConfig, NULL);
	oConfig.MergeMode = true;

	m_oDisplayOptions.bShowPixelGrid = false;
	m_oDisplayOptions.bTiling = false;
	m_oDisplayOptions.eShowChannels = ChannelFlag::RED | ChannelFlag::GREEN | ChannelFlag::BLUE | ChannelFlag::ALPHA;
	m_oDisplayOptions.iMip = 0;
	m_oDisplayOptions.iFace = 0;
	m_oDisplayOptions.fRange[0] = 0.f;
	m_oDisplayOptions.fRange[1] = 1.f;
	m_oDisplayOptions.fGamma = 1.f;

	ImGui::GetIO().Fonts->Build();

	m_oImWindowMgr.Init();
	m_oImWindowMgr.SetMainTitle("Texeled");

	ImGuiStyle& style = ImGui::GetStyle();
	style.Colors[ImGuiCol_Text]                  = ImVec4(1.00f, 1.00f, 1.00f, 1.00f);
	style.Colors[ImGuiCol_TextDisabled]          = ImVec4(0.70f, 0.70f, 0.70f, 1.00f);
	style.Colors[ImGuiCol_WindowBg]              = ImVec4(0.12f, 0.12f, 0.12f, 1.00f);
	style.Colors[ImGuiCol_ChildWindowBg]         = ImVec4(0.20f, 0.20f, 0.20f, 1.00f);
	style.Colors[ImGuiCol_PopupBg]               = ImVec4(0.31f, 0.31f, 0.31f, 1.00f);
	style.Colors[ImGuiCol_MenuBarBg]             = ImVec4(0.12f, 0.12f, 0.12f, 1.00f);
	style.Colors[ImGuiCol_ScrollbarBg]           = ImVec4(0.02f, 0.02f, 0.02f, 0.53f);
	style.Colors[ImGuiCol_ScrollbarGrab]         = ImVec4(0.31f, 0.31f, 0.31f, 1.00f);
	style.Colors[ImGuiCol_ScrollbarGrabHovered]  = ImVec4(0.41f, 0.41f, 0.41f, 1.00f);
	style.Colors[ImGuiCol_ScrollbarGrabActive]   = ImVec4(0.48f, 0.48f, 0.48f, 1.00f);
	style.Colors[ImGuiCol_CheckMark]             = ImVec4(0.79f, 0.79f, 0.79f, 1.00f);
	style.Colors[ImGuiCol_SliderGrab]            = ImVec4(0.48f, 0.47f, 0.47f, 0.91f);
	style.Colors[ImGuiCol_SliderGrabActive]      = ImVec4(0.56f, 0.55f, 0.55f, 0.62f);
	style.Colors[ImGuiCol_Button]                = ImVec4(0.50f, 0.50f, 0.50f, 0.63f);
	style.Colors[ImGuiCol_ButtonHovered]         = ImVec4(0.67f, 0.67f, 0.68f, 0.63f);
	style.Colors[ImGuiCol_ButtonActive]          = ImVec4(0.26f, 0.26f, 0.26f, 0.63f);
	style.Colors[ImGuiCol_Header]                = ImVec4(0.54f, 0.54f, 0.54f, 0.58f);
	style.Colors[ImGuiCol_HeaderHovered]         = ImVec4(0.64f, 0.65f, 0.65f, 0.50f);
	style.Colors[ImGuiCol_HeaderActive]          = ImVec4(0.25f, 0.25f, 0.25f, 0.50f);

	m_pShortKeyManager = new ShortKeyManager();
	m_oShortkeys.pClose = m_pShortKeyManager->RegisterShortKey("Close", EasyWindow::KEY_ALT, EasyWindow::KEY_NONE, EasyWindow::KEY_NONE, EasyWindow::KEY_F4, new EasyWindow::InstanceCaller<Program, void>(this, &Program::CloseCurrentWindow), false);
	m_oShortkeys.pClose = m_pShortKeyManager->RegisterShortKey("Escape", EasyWindow::KEY_NONE, EasyWindow::KEY_NONE, EasyWindow::KEY_NONE, EasyWindow::KEY_ESC, new EasyWindow::InstanceCaller<Program, void>( this, &Program::CloseCurrentWindow), false);
	m_oShortkeys.pOpen = m_pShortKeyManager->RegisterShortKey("Open", EasyWindow::KEY_CTRL, EasyWindow::KEY_NONE, EasyWindow::KEY_NONE, EasyWindow::KEY_O, new EasyWindow::InstanceCaller<Program, void>(this, &Program::Open), false);
	m_oShortkeys.pSave = m_pShortKeyManager->RegisterShortKey("Save", EasyWindow::KEY_CTRL, EasyWindow::KEY_NONE, EasyWindow::KEY_NONE, EasyWindow::KEY_S, new EasyWindow::InstanceCaller<Program, void>(this, &Program::Save), false);
	m_oShortkeys.pSaveAs = m_pShortKeyManager->RegisterShortKey("Save As", EasyWindow::KEY_CTRL, EasyWindow::KEY_SHIFT, EasyWindow::KEY_NONE, EasyWindow::KEY_S, new EasyWindow::InstanceCaller<Program, void>(this, &Program::SaveAs), false);

	m_pMenus = new Menus();
	new StatusBars();
	new Toolbar();

	m_pWorkAreaWindow = new Windows::WorkAreaWindow();

	for (int iArg = 1; iArg < iArgCount; ++iArg)
	{
		LoadFile(pArgs[iArg]);
	}
}

Program::~Program()
{
	ImwSafeDelete(m_pTexture2D);
	ImwSafeDelete(m_pShortKeyManager);
	ImwSafeFree(m_pTexturePath);
}

Program* Program::CreateInstance(int iArgCount, char** pArgs)
{
	if (NULL == s_pInstance)
		new Program(iArgCount, pArgs);
	return s_pInstance;
}

void Program::DestroyInstance()
{
	if (NULL != s_pInstance)
	{
		delete s_pInstance;
		s_pInstance = NULL;

		ImGui::Shutdown();
	}
}

IDXGIFactory* Program::GetDXGIFactory() const
{
	return m_oImWindowMgr.GetDXGIFactory();
}

ID3D11Device* Program::GetDX11Device() const
{
	return m_oImWindowMgr.GetDX11Device();
}

ID3D11DeviceContext* Program::GetDX11DeviceContext() const
{
	return m_oImWindowMgr.GetDX11DeviceContext();
}

bool Program::Run()
{
	m_pShortKeyManager->Manage(false);

	if (m_oTexture.IsValid() && m_oDisplayOptions.iMip >= m_oTexture.GetMipCount())
	{
		m_oDisplayOptions.iMip = m_oTexture.GetMipCount() - 1;
	}

	if (m_oTexture.IsValid() && m_oDisplayOptions.iFace >= m_oTexture.GetFaceCount())
	{
		m_oDisplayOptions.iFace = m_oTexture.GetFaceCount() - 1;
	}

	return m_bRun && m_oImWindowMgr.Run(false) && m_oImWindowMgr.Run(true);
}

void Program::AskExit()
{
	m_bRun = false;
}

void Program::CloseCurrentWindow()
{
	if (m_oImWindowMgr.GetFocusedPlatformWindow() != NULL)
	{
		m_oImWindowMgr.ClosePlatformWindow(m_oImWindowMgr.GetFocusedPlatformWindow());
	}
}

void Program::UpdateTexture2DRes()
{
	ImwSafeDelete(m_pTexture2D);
	if (m_oTexture.IsValid())
	{
		CORE_VERIFY_OK(GraphicResources::Texture2D::CreateFromTexture(&m_oTexture, &m_pTexture2D));
	}
}

bool Program::LoadFile(const char* pFile, const Graphics::TextureLoaderInfo* pUseLoader)
{
	ErrorCode oErr = LoadFileInternal(pFile, pUseLoader);
	if (oErr == ErrorCode::Ok)
	{
		return true;
	}
	const char* pMsg = (oErr == ErrorCode::Fail) ? "Not supported or corrupted file" : oErr.ToString();
	PlatformUtils::MsgBox(PlatformUtils::MessageBoxStyleEnum::CRITICAL, PlatformUtils::MessageBoxTypeEnum::OK, "Can't load file", pMsg);
	return false;
}

ErrorCode Program::LoadFileInternal(const char* pFile, const Graphics::TextureLoaderInfo* pUseLoader)
{
	ErrorCode oErr = Graphics::LoadFromFile(&m_oTexture, pFile, pUseLoader);
	if (oErr == ErrorCode::Ok)
	{
		if (m_pTexturePath != NULL)
			free(m_pTexturePath);
		m_pTexturePath = Core::StringUtils::StrDup(pFile);
		UpdateTexture2DRes();
	}
	return oErr;
}

void Program::Open()
{
	char pBuffer[1024] = { 0 };
	int iSelectedIndex = 0;

	Core::StringBuilder oExts;
	static char* s_pExts = NULL;

	const Graphics::TextureLoaderInfo* pLoaders;
	int iLoaderCount;
	GetTextureLoaders(&pLoaders, &iLoaderCount);

	oExts += "All supported formats";
	oExts += (char)0;
	for (int iLoaderIndex = 0; iLoaderIndex < iLoaderCount; ++iLoaderIndex)
	{
		if (iLoaderIndex > 0)
			oExts += ';';
		oExts += pLoaders[iLoaderIndex].pExts;
	}
	oExts += (char)0;

	for (int iLoaderIndex = 0; iLoaderIndex < iLoaderCount; ++iLoaderIndex)
	{
		oExts += pLoaders[iLoaderIndex].pName;
		oExts += " (";
		oExts += pLoaders[iLoaderIndex].pExts;
		oExts += ')';
		oExts += (char)0;
		oExts += pLoaders[iLoaderIndex].pExts;
		oExts += (char)0;
	}

	char* pExts = oExts.Export();
	if (PlatformUtils::OpenFileDialog("Open file", pExts, pBuffer, sizeof(pBuffer), &iSelectedIndex))
	{
		const Graphics::TextureLoaderInfo* pLoaderInfo = NULL;
		if (iSelectedIndex > 0)
		{
			pLoaderInfo = &pLoaders[iSelectedIndex - 1];
		}
		LoadFile(pBuffer, pLoaderInfo);
	}
	free(pExts);
}

void Program::Save()
{
}

void Program::SaveAs()
{
	if (m_oTexture.IsValid() == false)
		return;

	char pBuffer[1024] = { 0 };
	int iSelectedIndex = 0;

	Core::StringBuilder oExts;
	static char* s_pExts = NULL;

	const Graphics::TextureWriterInfo* pWriters;
	int iWriterCount;
	Graphics::GetTextureWriters(&pWriters, &iWriterCount);

	for (int iWriterIndex = 0; iWriterIndex < iWriterCount; ++iWriterIndex)
	{
		oExts += pWriters[iWriterIndex].pName;
		oExts += " (";
		oExts += pWriters[iWriterIndex].pExt;
		oExts += ')';
		oExts += (char)0;
		oExts += pWriters[iWriterIndex].pExt;
		oExts += (char)0;
	}

	char* pExts = oExts.Export();
	if (PlatformUtils::SaveFileDialog("Save as", pExts, pBuffer, sizeof(pBuffer), &iSelectedIndex))
	{
		CORE_VERIFY_OK(Graphics::SaveToFile(&m_oTexture, NULL, pBuffer, &pWriters[iSelectedIndex]));
		LoadFile(pBuffer);
	}
	free(pExts);
}