#include "pch.h"
#include "RNUpload.h"
#include "base64.h"
#include <winrt/Windows.Storage.Pickers.h>

#include <winrt/Windows.Graphics.Imaging.h>
#include <winrt/Windows.Storage.Streams.h>
#include <winrt/Windows.Storage.FileProperties.h>
#include <winrt/Windows.UI.Xaml.Media.Imaging.h>

namespace winrt::livestocked::implementation
{

    IAsyncAction RNUpload::UploadAsyncHelper(
        RNUploadOptions options,
        ReactPromise<JSValueObject> promise) noexcept
    {
        auto capturedPromise = promise;
        auto capturedOptions = options;

        Pickers::FileOpenPicker filePicker = Windows::Storage::Pickers::FileOpenPicker();
        filePicker.ViewMode(Pickers::PickerViewMode::Thumbnail);
        filePicker.SuggestedStartLocation(winrt::Windows::Storage::Pickers::PickerLocationId::Desktop);
        filePicker.FileTypeFilter().Append(winrt::to_hstring(L".jpg"));
        filePicker.FileTypeFilter().Append(winrt::to_hstring(L".png"));
        filePicker.FileTypeFilter().Append(winrt::to_hstring(L".pdf"));
        StorageFile selectedFile = co_await filePicker.PickSingleFileAsync();
        if (selectedFile != NULL) {
            if (selectedFile.FileType() == L".jpg" || selectedFile.FileType() == L".png") {

                //Scale image
                Windows::Storage::Streams::IRandomAccessStream fileStream{ co_await selectedFile.OpenAsync(Windows::Storage::FileAccessMode::ReadWrite) };
                auto decoder{ co_await Windows::Graphics::Imaging::BitmapDecoder::CreateAsync(fileStream) };

                //Get image dimensions
                Windows::Storage::FileProperties::ImageProperties properties = co_await selectedFile.Properties().GetImagePropertiesAsync();
                uint32_t width = decoder.OrientedPixelWidth();
                uint32_t height = decoder.OrientedPixelHeight();

                InMemoryRandomAccessStream ras = InMemoryRandomAccessStream();
                auto encoder = co_await Windows::Graphics::Imaging::BitmapEncoder::CreateForTranscodingAsync(ras, decoder);
                encoder.BitmapTransform().InterpolationMode(Windows::Graphics::Imaging::BitmapInterpolationMode::Linear);

                //Scale the image
                double scale = 1;
                if (width > height) {
                    if (width > 1200) {
                        scale = (double)1200 / width;
                    }
                }
                else {
                    if (height > 1200) {
                        scale = (double)1200 / height;
                    }
                }
                encoder.BitmapTransform().ScaledWidth((uint32_t)floor((double)width * scale));
                encoder.BitmapTransform().ScaledHeight((uint32_t)floor((double)height * scale));

                co_await encoder.FlushAsync();

                DataReader reader = DataReader(ras.GetInputStreamAt(0));
                co_await reader.LoadAsync(ras.Size());
                std::vector<BYTE> returnBuffer;
                returnBuffer.resize(ras.Size());
                reader.ReadBytes(returnBuffer);
                std::string base64 = base64_encode(&returnBuffer[0], returnBuffer.size());

                JSValueObject map = JSValueObject{};
                map["base64"] = base64;
                map["fileName"] = winrt::to_string(selectedFile.Name());
                map["fileSize"] = static_cast<uint64_t>(base64.length());
                promise.Resolve(map);
            }
            else {

                //winrt::Windows::Storage::FileProperties::BasicProperties fileProps = nullptr;
                //fileProps = co_await selectedFile.GetBasicPropertiesAsync();
                //uint64_t fileSize = co_await fileProps.Size();

                //Create base64
                IBuffer fileBuffer = co_await FileIO::ReadBufferAsync(selectedFile);
                std::vector<BYTE> returnBuffer;
                returnBuffer.resize(fileBuffer.Length());
                DataReader::FromBuffer(fileBuffer).ReadBytes(returnBuffer);
                std::string base64 = base64_encode(&returnBuffer[0], returnBuffer.size());

                //Create return object
                JSValueObject map = JSValueObject{};
                map["base64"] = base64;
                map["fileName"] = winrt::to_string(selectedFile.Name());
                map["fileSize"] = static_cast<uint64_t>(base64.length());
                promise.Resolve(map);
            }
        }
        else {
            promise.Reject(L"No file selected.");
        }
    }

    void RNUpload::Upload(JSValueObject&& options, ReactPromise<JSValueObject> promise) noexcept
    {
        RNUploadOptions uploadOptions;

        //std::optional<std::string> FileName = std::nullopt;
        //if (options.find("FileName") != options.end())
        //{
        //    uploadOptions.FileName = options["FileName"].AsString();
        //}
        //else {
        //    promise.Reject("File name was not provided.");
        //    return;
        //}

        //std::optional<std::string> Base64 = std::nullopt;
        //if (options.find("Base64") != options.end())
        //{
        //    uploadOptions.Base64 = options["Base64"].AsString();
        //}
        //else {
        //    promise.Reject("File contents were not provided.");
        //    return;
        //}

        reactContext.UIDispatcher().Post([=]()
            {
                auto asyncOp = UploadAsyncHelper(uploadOptions, promise);
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