#include "pch.h"
#include "RNExport.h"
#include "base64.h"
#include <winrt/Windows.Storage.Pickers.h>

namespace winrt::livestocked::implementation
{

    IAsyncAction RNExport::ExportAsyncHelper(
        RNExportOptions options,
        ReactPromise<JSValue> promise) noexcept
    {
        try {
            auto capturedPromise = promise;
            auto capturedOptions = options;

            auto plainTextExtensions{ winrt::single_threaded_vector<winrt::hstring>() };
            plainTextExtensions.Append(winrt::to_hstring(*capturedOptions.FileExtension));

            Pickers::FileSavePicker savePicker = Windows::Storage::Pickers::FileSavePicker();
            savePicker.FileTypeChoices().Insert(L"Plain Text", plainTextExtensions);
            savePicker.SuggestedFileName(winrt::to_hstring(*capturedOptions.FileName));
            savePicker.SuggestedStartLocation(winrt::Windows::Storage::Pickers::PickerLocationId::Desktop);
            StorageFile selectedFile = co_await savePicker.PickSaveFileAsync();
            if (selectedFile != NULL) {
                std::vector<BYTE> decodedData;
                decodedData = base64_decode(*capturedOptions.Base64);
                co_await FileIO::WriteBytesAsync(selectedFile, decodedData);
                promise.Resolve(true);            }
            else {
                promise.Reject(L"No folder selected.");
            }
        }
        catch (winrt::hresult_error const& ex) {
            winrt::hresult hr = ex.code();
            winrt::hstring message = ex.message();
            std::wstring widestr = std::wstring(message.begin(), message.end());
            const wchar_t* widecstr = widestr.c_str();
            promise.Reject(widecstr);
        }
    }

    //IAsyncAction RNExport::ExportAsyncHelper(
    //    RNExportOptions options,
    //    ReactPromise<JSValue> promise) noexcept
    //{
    //    try {
    //        auto capturedPromise = promise;
    //        auto capturedOptions = options;

    //        Pickers::FolderPicker folderPicker = Windows::Storage::Pickers::FolderPicker();
    //        folderPicker.ViewMode(Pickers::PickerViewMode::Thumbnail);
    //        folderPicker.SuggestedStartLocation(winrt::Windows::Storage::Pickers::PickerLocationId::Desktop);
    //        folderPicker.FileTypeFilter().ReplaceAll({ winrt::to_hstring(L".csv") });
    //        StorageFolder selectedFolder = co_await folderPicker.PickSingleFolderAsync();
    //        if (selectedFolder != NULL) {

    //            std::vector<BYTE> decodedData;
    //            decodedData = base64_decode(*capturedOptions.Base64);
    //            StorageFile tempFile = co_await selectedFolder.CreateFileAsync(winrt::to_hstring(*capturedOptions.FileName), CreationCollisionOption::ReplaceExisting);
    //            co_await FileIO::WriteBytesAsync(tempFile, decodedData);
    //            promise.Resolve(true);
    //        }
    //        else {
    //            promise.Reject(L"No folder selected.");
    //        }
    //    }
    //    catch (winrt::hresult_error const& ex) {
    //        winrt::hresult hr = ex.code();
    //        winrt::hstring message = ex.message(); // The system cannot find the file specified.
    //        std::wstring widestr = std::wstring(message.begin(), message.end());
    //        const wchar_t* widecstr = widestr.c_str();
    //        promise.Reject(widecstr);
    //    }
    //}

    //// Asynchronously exports text to file
    //IAsyncAction RNExport::ExportAsyncHelper(
    //    RNExportOptions options,
    //    ReactPromise<JSValue> promise) noexcept
    //{
    //    auto capturedPromise = promise;
    //    auto capturedOptions = options;

    //    std::vector<BYTE> decodedData;
    //    decodedData = base64_decode(*capturedOptions.Base64);
    //    StorageFile tempFile = co_await Windows::Storage::DownloadsFolder::CreateFileAsync(winrt::to_hstring(*capturedOptions.FileName), CreationCollisionOption::FailIfExists);
    //    co_await FileIO::WriteBytesAsync(tempFile, decodedData);
    //    
    //    promise.Resolve(true);
    //}

    void RNExport::Export(JSValueObject&& options, ReactPromise<JSValue> promise) noexcept
    {
        RNExportOptions exportOptions;

        std::optional<std::string> FileName = std::nullopt;
        if (options.find("FileName") != options.end())
        {
            exportOptions.FileName = options["FileName"].AsString();
        }
        else {
            promise.Reject("File name was not provided.");
            return;
        }

        std::optional<std::string> FileExtension = std::nullopt;
        if (options.find("FileExtension") != options.end())
        {
            exportOptions.FileExtension = options["FileExtension"].AsString();
        }
        else {
            promise.Reject("File name was not provided.");
            return;
        }

        std::optional<std::string> Base64 = std::nullopt;
        if (options.find("Base64") != options.end())
        {
            exportOptions.Base64 = options["Base64"].AsString();
        } else {
            promise.Reject("File contents were not provided.");
            return;
        }

        reactContext.UIDispatcher().Post([=]()
            {
                auto asyncOp = ExportAsyncHelper(exportOptions, promise);
                asyncOp.Completed([=](auto action, auto status)
                    {
                        // Here we handle any unhandled exceptions thrown during the
                        // asynchronous call by rejecting the promise with the error code
                        if (status == winrt::Windows::Foundation::AsyncStatus::Error) {
                            std::stringstream errorCode;
                            errorCode << "0x" << std::hex << action.ErrorCode() << std::dec << std::endl;

                            auto error = winrt::Microsoft::ReactNative::ReactError();
                            error.Message = "HRESULT " + errorCode.str() + ": " + std::system_category().message(action.ErrorCode());
                            promise.Reject(error);
                        }
                    });
            });



    }
}