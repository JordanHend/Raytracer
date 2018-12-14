#include "OpenFile.h"
#define INDEX_WORDDOC 1
#include <locale>
#include <codecvt>
#include <string>

std::string ws2s(const std::wstring& wstr)
{
	using convert_typeX = std::codecvt_utf8<wchar_t>;
	std::wstring_convert<convert_typeX, wchar_t> converterX;

	return converterX.to_bytes(wstr);
}


std::string openFile()
{
	std::string returnfilename = "";
	HRESULT hr = CoInitializeEx(NULL, COINIT_APARTMENTTHREADED |
		COINIT_DISABLE_OLE1DDE);
	if (SUCCEEDED(hr))
	{
		IFileOpenDialog *pFileOpen;

		// Create the FileOpenDialog object.
		hr = CoCreateInstance(CLSID_FileOpenDialog, NULL, CLSCTX_ALL,
			IID_IFileOpenDialog, reinterpret_cast<void**>(&pFileOpen));

		if (SUCCEEDED(hr))
		{	

			LPCWSTR szUWU = L"Equipment Files";
		
			COMDLG_FILTERSPEC filter[] =
			{
				{ szUWU, L"*.owo" },
			};
	
			
			pFileOpen->SetFileTypes(1, filter);
			// Show the Open dialog box.
			hr = pFileOpen->Show(NULL);

			// Get the file name from the dialog box.
			if (SUCCEEDED(hr))
			{
				IShellItem *pItem;
				hr = pFileOpen->GetResult(&pItem);
				if (SUCCEEDED(hr))
				{
					PWSTR pszFilePath;
					hr = pItem->GetDisplayName(SIGDN_FILESYSPATH, &pszFilePath);

					// Display the file name to the user.
					if (SUCCEEDED(hr))
					{
						// Fetch Local App Data folder path.
						wchar_t* filename = new wchar_t[128];
						filename = pszFilePath;
						std::wstring text(filename);
						returnfilename = ws2s(text);
						
					}
					pItem->Release();
				}
			}
			pFileOpen->Release();
		}
		CoUninitialize();
	}
	return returnfilename;

}

std::string openTextureFile(std::string dialog)
{
	std::string returnfilename = "";
	HRESULT hr = CoInitializeEx(NULL, COINIT_APARTMENTTHREADED |
		COINIT_DISABLE_OLE1DDE);
	if (SUCCEEDED(hr))
	{
		IFileOpenDialog *pFileOpen;

		// Create the FileOpenDialog object.
		hr = CoCreateInstance(CLSID_FileOpenDialog, NULL, CLSCTX_ALL,
			IID_IFileOpenDialog, reinterpret_cast<void**>(&pFileOpen));

		if (SUCCEEDED(hr))
		{

			LPCWSTR szImages = L"Images";
			std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> converter;
			std::wstring wide = converter.from_bytes(dialog);
			LPCWSTR name = wide.c_str();
			pFileOpen->SetTitle(name);
			COMDLG_FILTERSPEC filter[] =
			{
				{ szImages, L"*.jpg;*.jpeg;*.bmp;*.png;*.tga" },
			
			};


			pFileOpen->SetFileTypes(1, filter);
			// Show the Open dialog box.
			hr = pFileOpen->Show(NULL);

			// Get the file name from the dialog box.
			if (SUCCEEDED(hr))
			{
				IShellItem *pItem;
				hr = pFileOpen->GetResult(&pItem);
				if (SUCCEEDED(hr))
				{
					PWSTR pszFilePath;
					hr = pItem->GetDisplayName(SIGDN_FILESYSPATH, &pszFilePath);

					// Display the file name to the user.
					if (SUCCEEDED(hr))
					{
						// Fetch Local App Data folder path.
						wchar_t* filename = new wchar_t[128];
						filename = pszFilePath;
						std::wstring text(filename);
						returnfilename = ws2s(text);

					}
					pItem->Release();
				}
			}
			pFileOpen->Release();
		}
		CoUninitialize();
	}
	return returnfilename;

}


std::string getSaveFile()
{
	std::string returnfilename = "";
	HRESULT hr = CoInitializeEx(NULL, COINIT_APARTMENTTHREADED |
		COINIT_DISABLE_OLE1DDE);
	if (SUCCEEDED(hr))
	{
		IFileSaveDialog *pFileSave;

		// Create the FileOpenDialog object.
		hr = CoCreateInstance(CLSID_FileSaveDialog, NULL, CLSCTX_ALL,
			IID_IFileSaveDialog, reinterpret_cast<void**>(&pFileSave));

		if (SUCCEEDED(hr))
		{
			// Show the Open dialog box.
			hr = pFileSave->Show(NULL);

			// Get the file name from the dialog box.
			if (SUCCEEDED(hr))
			{
				IShellItem *pItem;
				hr = pFileSave->GetResult(&pItem);
				if (SUCCEEDED(hr))
				{
					PWSTR pszFilePath;
					hr = pItem->GetDisplayName(SIGDN_FILESYSPATH, &pszFilePath);

					// Display the file name to the user.
					if (SUCCEEDED(hr))
					{
						// Fetch Local App Data folder path.
						wchar_t* filename = new wchar_t[128];
						filename = pszFilePath;
						std::wstring text(filename);
						returnfilename = ws2s(text);

					}
					pItem->Release();
				}
			}
			pFileSave->Release();
		}
		CoUninitialize();
	}
	return returnfilename;
}
